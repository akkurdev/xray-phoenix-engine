#include "stdafx.h"
#include "Actor.h"
#include "game_object_space.h"
#include "Level.h"
#include "alife_registry_wrappers.h"
#include "DefaultPersonalGoodwillChanger.h"

namespace Stalker::Mechanics::Relations
{
    DefaultPersonalGoodwillChanger::DefaultPersonalGoodwillChanger(CRelationRegistryWrapper* wrapper) :
        m_wrapper(wrapper)
    {
        VERIFY(m_wrapper);
    }

    CharacterGoodwill DefaultPersonalGoodwillChanger::Get(const u16& sourceId, const u16& targetId) const
    {
        auto& relations = m_wrapper->registry().objects(sourceId);

        if (&relations)
        {
            auto personalRelations = relations.PersonalRelations;

            if (personalRelations.contains(targetId))
            {
                return personalRelations[targetId];
            }
        }
        return CharacterGoodwill(NEUTRAL_GOODWILL_AMOUNT);
    }

    void DefaultPersonalGoodwillChanger::Set(const u16& sourceId, const u16& targetId, const CharacterGoodwill& goodwill)
    {
        auto& relations = m_wrapper->registry().objects(sourceId);
        relations.PersonalRelations[targetId] = goodwill;

        if (targetId)
        {
            auto& opponentRelations = m_wrapper->registry().objects(targetId);
            auto it = std::find(opponentRelations.Foes.begin(), opponentRelations.Foes.end(), sourceId);

            if (it == opponentRelations.Foes.end())
            {
                opponentRelations.Foes.emplace_back(sourceId);
            }
        }

        if (g_actor)
        {
            g_actor->callback(GameObject::ECallbackType::eOnGoodwillChange)(sourceId, targetId);
        }
    }

    void DefaultPersonalGoodwillChanger::Change(const u16& sourceId, const u16& targetId, const CharacterGoodwill& goodwillDelta)
    {
        auto goodwillAmount = Get(sourceId, targetId) + goodwillDelta;
        Set(sourceId, targetId, CharacterGoodwill(goodwillAmount));
    }
}