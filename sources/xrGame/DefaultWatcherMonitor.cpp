#include "stdafx.h"
#include <numeric>
#include "Actor.h"
#include "Level.h"
#include "character_community.h"
#include "InventoryOwner.h"
#include "RelationSystem.h"
#include "DefaultWatcherMonitor.h"

namespace Stalker::Mechanics::Watchers
{    
    DefaultWatcherMonitor::DefaultWatcherMonitor(const WatcherSettings& settings) :
        m_settings(settings),
        m_lastVictimId(0),
        m_time(0)
    {
    }

    bool DefaultWatcherMonitor::IsActive() const
    {
        return Level().GetGameTime() <= m_time;
    }

    u16 DefaultWatcherMonitor::LastVictimId() const
    {
        return m_lastVictimId;
    }

    void DefaultWatcherMonitor::Update(const u16& characterId)
    {
        auto victim = getCharacterById(characterId);        
        if (victim)
        {
            m_lastVictimId = characterId;
            m_time = Level().GetGameTime() + u64((m_settings.SearchPeriod() * 60 * 1000));
        }
    }

    std::vector<std::pair<u16, u64>> DefaultWatcherMonitor::CheckWatchers(WatchableAction& action)
    {
        std::vector<std::pair<u16, u64>> changedWatchers {};
        CInventoryOwner* victim;
        
        if (IsActive() && g_actor && (victim = getCharacterById(action.VictimId)))
        {
            for (auto& character : findNearestEntities(victim, m_settings.SearchRadius()))
            {
                const auto id = character->object().ID();
                if (action.Watchers.contains(id) && action.Watchers[id] != 0)
                {
                    continue;
                }

                auto isVictimWatched = character->CheckObjectVisibility(victim->cast_game_object());
                auto isActorWatched = character->CheckObjectVisibility(g_actor->cast_game_object());

                if (isVictimWatched && !action.Watchers.contains(id))
                {
                    action.Watchers[id] = 0;
                    changedWatchers.emplace_back(std::pair<u16, u64>{id, 0 });
                }

                if (isActorWatched && action.Watchers.contains(id) && action.Watchers[id] == 0)
                {
                    auto revealTimeDelta = getRevealTime(g_actor->cast_inventory_owner(), getCharacterById(id));
                    auto revealTime = Level().GetGameTime() + u64(revealTimeDelta * 3600 * 1000);
                    action.Watchers[id] = revealTime;
                    changedWatchers.emplace_back(std::pair<u16, u64>{id, action.Watchers[id] });
                }
            }
        }        
        return changedWatchers;
    }

    CInventoryOwner* DefaultWatcherMonitor::getCharacterById(u16 characterId) const
    {
        if (CObject* obj = Level().Objects.net_Find(characterId))
        {
            if (CInventoryOwner* character = smart_cast<CInventoryOwner*>(obj))
            {
                return character;
            }
        }
        return nullptr;
    }

    RelationType DefaultWatcherMonitor::getRelationByCommunity(CInventoryOwner* npc1, CInventoryOwner* npc2) const
    {        
        if (!npc1 || !npc2)
        {
            return RelationType::eRelationTypeDummy;
        }

        auto goodwill = CHARACTER_COMMUNITY::relation(npc1->Community(), npc2->Community());
        auto relationSettings = RelationSystem::Instance().Settings();

        if (goodwill >= relationSettings.FriendRelationThreshold())
        {
            return RelationType::eRelationTypeFriend;
        }
        else if (goodwill >= relationSettings.NeutralRelationThreshold())
        {
            return RelationType::eRelationTypeNeutral;
        }
        else return RelationType::eRelationTypeEnemy;
    }

    std::vector<CScriptEntity*> DefaultWatcherMonitor::findNearestEntities(CInventoryOwner* character, float radius) const
    {
        auto entities = std::vector<CScriptEntity*>();
        auto object = character->cast_game_object();

        xr_vector<CObject*> nearest;
        Level().ObjectSpace.GetNearest(nearest, object->Position(), radius, object->dcast_CObject());

        for (auto& obj : nearest)
        {
            CEntityAlive* entity = smart_cast<CEntityAlive*>(obj);
            if (entity && entity->g_Alive() && entity->human_being())
            {
                if (CScriptEntity* scriptEntity = smart_cast<CScriptEntity*>(obj))
                {
                    entities.emplace_back(scriptEntity);
                }
            }
        }
        return entities;
    }

    float DefaultWatcherMonitor::getRevealTime(CInventoryOwner* attacker, CInventoryOwner* watcher) const
    {
        auto revealTime = 0.0f;
        if (!attacker || !watcher)
        {
            return revealTime;
        }        

        auto times = std::vector<float>();
        auto relation = getRelationByCommunity(watcher, attacker);
        auto timeByRelation = calculateRevealTimeByRelation(relation);
        times.emplace_back(timeByRelation);

        if (m_settings.IsReputationFactorEnabled())
        {
            auto timeByReputation = calculateRevealTimeByReputation(watcher->Reputation());
            times.emplace_back(timeByReputation);
        }

        if (m_settings.IsRankFactorEnabled())
        {
            auto timeByRank = calculateRevealTimeByRank(watcher->Rank());
            times.emplace_back(timeByRank);
        }

        return std::accumulate(times.cbegin(), times.cend(), 0.0f) / times.size();
    }

    float DefaultWatcherMonitor::calculateRevealTimeByRelation(RelationType relationType) const
    {
        auto timeMin = m_settings.RevealPeriodMin();
        auto timeMax = m_settings.RevealPeriodMax();

        switch (relationType)
        {
            case RelationType::eRelationTypeFriend:
                return timeMax;
            case RelationType::eRelationTypeNeutral:
                return timeMin + ((timeMax - timeMin) / 2);
            case RelationType::eRelationTypeEnemy:
                return timeMin;
            default: return 0.0f;
        }
    }

    float DefaultWatcherMonitor::calculateRevealTimeByReputation(u16 watcherReputation) const
    {
        auto timeMin = m_settings.RevealPeriodMin();
        auto timeMax = m_settings.RevealPeriodMax();
        auto timeScale = timeMax - timeMin;
        auto relationSettings = RelationSystem::Instance().Settings();

        auto reputationScale = relationSettings.ReputationLimits().y - relationSettings.ReputationLimits().x;
        auto reputationScalePercent = reputationScale / 100;

        auto watcherReputationPercent = watcherReputation / reputationScalePercent;
        return timeMax - (timeScale * (watcherReputationPercent / 100.f));
    }

    float DefaultWatcherMonitor::calculateRevealTimeByRank(u16 watcherRank) const
    {
        auto timeMin = m_settings.RevealPeriodMin();
        auto timeMax = m_settings.RevealPeriodMax();
        auto timeScale = timeMax - timeMin;
        auto relationSettings = RelationSystem::Instance().Settings();

        auto rankScale = relationSettings.RankLimits().y - relationSettings.RankLimits().x;
        auto rankScalePercent = rankScale / 100;

        auto watcherRankPercent = watcherRank / rankScalePercent;
        return timeMax - (timeScale * (watcherRankPercent / 100.f));
    }
}