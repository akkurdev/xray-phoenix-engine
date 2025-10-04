#include "stdafx.h"
#include <algorithm>
#include "Level.h"
#include "alife_space.h"
#include "alife_registry_wrappers.h"
#include "DefaultRelationTypeChanger.h"
#include "DefaultActionController.h"

namespace Stalker::Mechanics::Relations
{    
    DefaultActionController::DefaultActionController(CRelationRegistryWrapper* wrapper, RelationSettings* settings) :
        m_settings(settings),
        m_observers(std::vector<IActionObserver*>()),
        m_actionList(std::vector<Action>()),
        m_lock(std::mutex())
    {
        VERIFY(wrapper);

        m_relationChanger = new DefaultRelationTypeChanger(wrapper, settings);
        m_metricTable = new CharacterMetricTable(m_settings);
    }

    DefaultActionController::~DefaultActionController()
    {
        // Уничтожение в обратном порядке!
        delete m_metricTable;
        delete m_relationChanger;
    }

    void DefaultActionController::RegisterAction(CEntityAlive* attacker, CEntityAlive* defender, const ActionType& actionType, const u16& bone)
    {        
        if (!attacker || !defender || attacker->ID() != 0)
        {
            return;
        }

        auto action = buildAction(attacker, defender, actionType, bone);
        addAction(action);
        NotifyObservers(action);        
    }

    void DefaultActionController::AddObserver(IActionObserver* observer)
    {
        m_observers.emplace_back(observer);
    }

    void DefaultActionController::RemoveObserver(IActionObserver* observer)
    {
        auto it = std::find(m_observers.begin(), m_observers.end(), observer);

        if (it != m_observers.end())
        {
            size_t index = it - m_observers.begin();
            std::swap(m_observers[index], m_observers.back());
            m_observers.pop_back();
        }
    }

    void DefaultActionController::NotifyObservers(const Action& action)
    {
        for (auto* observer : m_observers)
        {
            observer->OnAction(action);
        }
    }

    std::vector<Action> DefaultActionController::Actions() const
    {
        return m_actionList;
    }

    void DefaultActionController::Load(IReader& reader)
    {
        R_ASSERT2(reader.find_chunk(OBJECT_CHUNK_DATA), "Can't find chunk OBJECT_CHUNK_DATA!");
        auto pos = reader.tell();
        reader.seek(pos);

        m_actionList.clear();
        auto actionsCount = reader.r_u16();

        for (auto i = 0; i < actionsCount; ++i)
        {
            Action action;
            action.VictimId         = reader.r_u16();
            action.Type             = (ActionType)reader.r_u32();
            action.Time             = reader.r_u64();
            action.GoodwillDelta    = reader.r_s16();
            action.ReputationDelta  = reader.r_s16();
            action.RankDelta        = reader.r_s16();
            action.DamagedBone      = reader.r_u16();
            action.HasOneShot       = (bool)reader.r_u8();

            reader.r_fvector3(action.ActorPosition);
            reader.r_fvector3(action.VictimPosition);            
            
            addAction(action);
        }
    }

    void DefaultActionController::Save(IWriter& writer)
    {
        writer.open_chunk(OBJECT_CHUNK_DATA);
        auto pos = writer.tell();
        writer.seek(pos);
        
        writer.w_u16(m_actionList.size());
        for (const auto& action : m_actionList)
        {            
            writer.w_u16(action.VictimId);
            writer.w_u32((u32)action.Type);
            writer.w_u64(action.Time);
            writer.w_s16(action.GoodwillDelta);
            writer.w_s16(action.ReputationDelta);
            writer.w_s16(action.RankDelta);
            writer.w_u16(action.DamagedBone);
            writer.w_u8((u8)action.HasOneShot);
            writer.w_fvector3(action.ActorPosition);
            writer.w_fvector3(action.VictimPosition);            
        }
        writer.close_chunk();
    }

    Action* DefaultActionController::findAction(const u16& victimId)
    {
        auto index = findActionIndex(victimId);
        return index >= 0 && index < m_actionList.size()
            ? &m_actionList[index]
            : nullptr;
    }

    int32_t DefaultActionController::findActionIndex(const u16& victimId) const
    {
        auto it = std::find_if(m_actionList.begin(), m_actionList.end(), [&](const Action& action)
        {
            return action.VictimId == victimId;
        });        
        return it != m_actionList.end() ? it - m_actionList.begin() : -1;
    }

    bool DefaultActionController::isTeammate(CInventoryOwner* attacker, CInventoryOwner* defender, bool isFriendlyFireEnabled) const
    {
        return !isFriendlyFireEnabled && attacker->object_id() && defender->object_id() && attacker->Community() == defender->Community();
    }

    CharacterGoodwill DefaultActionController::calculateGoodwill(ActionType actionType, RelationType relationType, bool isTeammate) const
    {
        return isTeammate ? NEUTRAL_GOODWILL_AMOUNT : m_metricTable->FindGoodwill(actionType, relationType);
    }

    CharacterReputation DefaultActionController::calculateReputation(ActionType actionType, RelationType relationType, bool isTeammate) const
    {
        return isTeammate ? NEUTRAL_REPUTATION_AMOUNT : m_metricTable->FindReputation(actionType, relationType);
    }

    CharacterRank DefaultActionController::calculateRank(CInventoryOwner* npc, ActionType actionType, bool isTeammate) const
    {
        if (actionType == ActionType::Kill)
        {
            auto rankTableIndex = CHARACTER_RANK::ValueToIndex(npc->Rank());
            return CHARACTER_RANK::rank_kill_points(rankTableIndex);
        }
        return CharacterRank(0);
    }

    Action DefaultActionController::buildAction(CEntityAlive* attacker, CEntityAlive* defender, const ActionType& actionType, const u16& bone)
    {
        CInventoryOwner* attacker_ = smart_cast<CInventoryOwner*>(attacker);
        CInventoryOwner* defender_ = smart_cast<CInventoryOwner*>(defender);

        auto teammate = isTeammate(attacker_, defender_, m_settings->IsFriendlyFireEnabled());
        auto relation = m_relationChanger->GetTotalRelationType(attacker_, defender_);

        auto goodwill = calculateGoodwill(actionType, relation, teammate);
        auto reputation = calculateReputation(actionType, relation, teammate);
        auto rank = calculateRank(attacker_, actionType, teammate);

        Action action;
        action.VictimId = defender->ID();
        action.Type = actionType;
        action.Time = Level().GetGameTime();
        action.GoodwillDelta = s16(goodwill);
        action.ReputationDelta = s16(reputation);
        action.RankDelta = s16(rank);
        action.ActorPosition = attacker->Position();
        action.VictimPosition = defender->Position();
        action.DamagedBone = bone;
        action.HasOneShot = actionType == ActionType::Kill && !findAction(action.VictimId);

        return action;
    }

    void DefaultActionController::addAction(const Action& action)
    {
        m_lock.lock();
        auto index = findActionIndex(action.VictimId);

        if (index >= 0)
        {
            m_actionList[index] = action;
        }
        else
        {
            m_actionList.emplace_back(action);
        }
        m_lock.unlock();
    }

    void DefaultActionController::removeAction(const Action& action)
    {
        m_lock.lock();

        auto index = findActionIndex(action.VictimId);
        if (index >= 0 && index < m_actionList.size())
        {
            std::swap(m_actionList[index], m_actionList.back());
            m_actionList.pop_back();
        }
        m_lock.unlock();
    }
}