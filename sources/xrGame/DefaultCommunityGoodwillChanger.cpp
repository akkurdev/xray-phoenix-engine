#include "stdafx.h"
#include "character_community.h"
#include "game_object_space.h"
#include "Level.h"
#include "alife_registry_wrappers.h"
#include "DefaultCommunityGoodwillChanger.h"

namespace Stalker::Mechanics::Relations
{
    DefaultCommunityGoodwillChanger::DefaultCommunityGoodwillChanger(CRelationRegistryWrapper* wrapper) :
        m_wrapper(wrapper)
    {
        VERIFY(m_wrapper);
    }

    CharacterGoodwill DefaultCommunityGoodwillChanger::Get(const CommunityIndex& communityIndex, const u16& npcId) const
    {
        auto& relations = m_wrapper->registry().objects(npcId);

        if (&relations)
        {
            auto communityRelations = relations.CommunityRelations;

            if (communityRelations.contains(communityIndex))
            {
                return communityRelations[communityIndex];
            }
        }
        return CharacterGoodwill(NEUTRAL_GOODWILL_AMOUNT);
    }

    void DefaultCommunityGoodwillChanger::Set(const CommunityIndex& communityIndex, const u16& npcId, const CharacterGoodwill& goodwill)
    {
        CharacterRelationInfo& relations = m_wrapper->registry().objects(npcId);
        relations.CommunityRelations.insert(std::make_pair(communityIndex, goodwill));
    }

    void DefaultCommunityGoodwillChanger::Change(const CommunityIndex& communityIndex, const u16& npcId, const CharacterGoodwill& goodwillDelta)
    {
        auto goodwillAmount = Get(communityIndex, npcId) + goodwillDelta;
        Set(communityIndex, npcId, CharacterGoodwill(goodwillAmount));
    }
}