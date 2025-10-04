#include "stdafx.h"
#include "Actor.h"
#include "game_object_space.h"
#include "alife_registry_wrappers.h"
#include "DefaultPersonalGoodwillChanger.h"
#include "DefaultCommunityGoodwillChanger.h"
#include "DefaultRelationTypeChanger.h"

namespace Stalker::Mechanics::Relations
{
    DefaultRelationTypeChanger::DefaultRelationTypeChanger(CRelationRegistryWrapper* wrapper, RelationSettings* settings) :
        m_wrapper(wrapper),
        m_settings(settings),
        m_pgChanger(nullptr),
        m_cgChanger(nullptr)
    {
        VERIFY(m_wrapper);

        m_pgChanger = new DefaultPersonalGoodwillChanger(m_wrapper);
        m_cgChanger = new DefaultCommunityGoodwillChanger(m_wrapper);
    }

    DefaultRelationTypeChanger::~DefaultRelationTypeChanger()
    {
        delete m_cgChanger;
        delete m_pgChanger;
    }

    RelationType DefaultRelationTypeChanger::GetTotalRelationType(CInventoryOwner* npc1, CInventoryOwner* npc2) const
    {
        auto finalGoodwill = GetTotalRelationAmount(npc1, npc2);

        if (finalGoodwill == NO_GOODWILL)
        {
            return RelationType::eRelationTypeNeutral;
        }

        if (finalGoodwill < m_settings->NeutralRelationThreshold())
        {
            return RelationType::eRelationTypeEnemy;
        }
        else if (finalGoodwill < m_settings->FriendRelationThreshold())
        {
            return RelationType::eRelationTypeNeutral;
        }
        else return RelationType::eRelationTypeFriend;
    }

    CharacterGoodwill DefaultRelationTypeChanger::GetTotalRelationAmount(CInventoryOwner* npc1, CInventoryOwner* npc2) const
    {
        auto personalGoodwill = m_pgChanger->Get(npc1->object_id(), npc2->object_id());
        VERIFY(personalGoodwill != NO_GOODWILL);

        auto communityGoodwill = m_cgChanger->Get(npc1->Community(), npc2->object_id());
        VERIFY(communityGoodwill != NO_GOODWILL);

        auto goodwillCommunityToCommunity = getGoodwillBetweenCommunities(npc1->Community(), npc2->Community());
        auto goodwillByReputation = getGoodwillByReputation(npc1->Reputation(), npc2->Reputation());
        auto goodwillByRank = getGoodwillByRank(npc1->Rank(), npc2->Rank());

        return personalGoodwill + communityGoodwill + goodwillCommunityToCommunity + goodwillByReputation + goodwillByRank;
    }

    void DefaultRelationTypeChanger::SetTotalRelationType(CInventoryOwner* npc1, CInventoryOwner* npc2, RelationType relationType)
    {
        auto sourceId = npc1->object_id();
        auto targetId = npc2->object_id();        
        auto goodwill = NEUTRAL_GOODWILL;

        switch (relationType)
        {
            case RelationType::eRelationTypeEnemy:
                goodwill = m_settings->DefaultEnemyGoodwillAmount();
                break;
            case RelationType::eRelationTypeNeutral:
                goodwill = m_settings->DefaultNeutralGoodwillAmount();
                break;
            case RelationType::eRelationTypeFriend:
                goodwill = m_settings->DefaultFriendGoodwillAmount();
                break;
            default: break;
        }
        m_pgChanger->Set(sourceId, targetId, goodwill);
    }

    CharacterGoodwill DefaultRelationTypeChanger::GetGoodwillBetweenCommunities(CommunityIndex communityIndex1, CommunityIndex communityIndex2) const
    {
        return getGoodwillBetweenCommunities(communityIndex1, communityIndex2);
    }

    shared_str DefaultRelationTypeChanger::GetSpotName(RelationType relationType) const
    {
        switch (relationType)
        {
            case ALife::eRelationTypeFriend: return "friend_location";
            case ALife::eRelationTypeNeutral: return "neutral_location";
            case ALife::eRelationTypeEnemy: return "enemy_location";
            case ALife::eRelationTypeWorstEnemy: return "enemy_location";
            case ALife::eRelationTypeLast: return "neutral_location";
            default: return "neutral_location";
        }
    }

    void DefaultRelationTypeChanger::InitializeCharactersFoes()
    {
        for (const auto& obj : m_wrapper->registry().get_registry_objects())
        {
            auto npcId = obj.first;

            if (npcId) // Игнорим актора с его id == 0
            {
                auto& npcRelations = m_wrapper->registry().objects(obj.first);
                for (const auto& opponent : npcRelations.PersonalRelations)
                {
                    auto opponentId = opponent.first;

                    if (opponentId) // Игнорим актора с его id == 0
                    {
                        auto& opponentRelations = m_wrapper->registry().objects(opponent.first);
                        auto opponentFoes = opponentRelations.Foes;

                        const auto foe = std::find(opponentFoes.begin(), opponentFoes.end(), obj.first);
                        ASSERT_FMT(foe == opponentFoes.end(), "[%s]: %u already exists in revers_personal of %u", __FUNCTION__, obj.first, opponent.first);

                        opponentFoes.emplace_back(obj.first);
                    }
                }
            }
        }
    }

    void DefaultRelationTypeChanger::ClearCharactersFoes(u16 npc1, u16 npc2)
    {
        auto& relations = m_wrapper->registry().objects(npc2);

        ASSERT_FMT(npc2, "[%s]: actor detected clearing %u", __FUNCTION__, npc1);
        ASSERT_FMT(m_wrapper->registry().objects_ptr(npc2), "[%s]: %u not found clearing %u", __FUNCTION__, npc2, npc1);

        auto it = std::find(relations.Foes.begin(), relations.Foes.end(), npc1);

        ASSERT_FMT(it != relations.Foes.end(), "[%s]: %u not found in foes of %u", __FUNCTION__, npc1, npc2);
        relations.Foes.erase(it);
    }

    void DefaultRelationTypeChanger::ResetRelations(u16 npc1, u16 npc2)
    {
        auto& relations = m_wrapper->registry().objects(npc1);
        relations.PersonalRelations.erase(npc2);

        if (npc2)
        {
            ClearCharactersFoes(npc1, npc2);
        }
    }

    void DefaultRelationTypeChanger::ResetById(u16 npcId)
    {
        auto& relations = m_wrapper->registry().objects(npcId);

        for (const auto& it : relations.PersonalRelations)
        {
            if (it.first)
            {
                ClearCharactersFoes(npcId, it.first);
            }
        }

        for (const auto foe : relations.Foes)
        {
            auto& foeRelations = m_wrapper->registry().objects(foe);
            ASSERT_FMT(m_wrapper->registry().objects_ptr(foe), "[%s]: %u not found clearing %u", __FUNCTION__, foe, npcId);

            const auto& it = foeRelations.PersonalRelations.find(npcId);
            ASSERT_FMT(it != foeRelations.PersonalRelations.end(), "[%s]: %u not found in personal of %u", __FUNCTION__, npcId, foe);

            foeRelations.PersonalRelations.erase(it);
        }

        auto& objects = m_wrapper->registry().get_registry_objects();

        auto it = objects.find(npcId);
        ASSERT_FMT(it != objects.end(), "[%s]: %u not found", __FUNCTION__, npcId);

        objects.erase(it);
    }

    CharacterGoodwill DefaultRelationTypeChanger::getGoodwillByReputation(const CharacterReputation& npcRep1, const CharacterReputation& npcRep2) const
    {
        CHARACTER_REPUTATION reputation_from, reputation_to;
        reputation_from.set(npcRep1);
        reputation_to.set(npcRep2);

        return CHARACTER_REPUTATION::relation(reputation_from.index(), reputation_to.index());
    }

    CharacterGoodwill DefaultRelationTypeChanger::getGoodwillByRank(const CharacterRank& npcRank1, const CharacterRank& npcRank2) const
    {
        CHARACTER_RANK rank_from, rank_to;
        rank_from.set(npcRank1);
        rank_to.set(npcRank2);

        return CHARACTER_RANK::relation(rank_from.index(), rank_to.index());
    }

    CharacterGoodwill DefaultRelationTypeChanger::getGoodwillBetweenCommunities(CommunityIndex communityIndex1, CommunityIndex communityIndex2) const
    {
        if (communityIndex1 < 0 || communityIndex2 < 0)
        {
            return NEUTRAL_GOODWILL_AMOUNT;
        }
        return CHARACTER_COMMUNITY::relation(communityIndex1, communityIndex2);
    }
}