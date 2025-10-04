#include "stdafx.h"
#include "character_reputation.h"
#include "Level.h"
#include "InventoryOwner.h"
#include "DefaultReputationChanger.h"

namespace Stalker::Mechanics::Relations
{
    CharacterReputation DefaultReputationChanger::Get(const u16& targetId) const
    {
        if (CObject* obj = Level().Objects.net_Find(targetId))
        {
            if (CInventoryOwner* iOwner = smart_cast<CInventoryOwner*>(obj))
            {
                return iOwner->Reputation();
            }
        }
        return NEUTRAL_REPUTATION_AMOUNT;
    }

    void DefaultReputationChanger::Set(const u16& targetId, const CharacterReputation& newValue)
    {
        if (CObject* obj = Level().Objects.net_Find(targetId))
        {
            if (CInventoryOwner* iOwner = smart_cast<CInventoryOwner*>(obj))
            {
                iOwner->SetReputation(newValue);
            }
        }
    }

    void DefaultReputationChanger::Change(const u16& targetId, const CharacterReputation& valueDelta)
    {
        if (CObject* obj = Level().Objects.net_Find(targetId))
        {
            if (CInventoryOwner* iOwner = smart_cast<CInventoryOwner*>(obj))
            {
                iOwner->ChangeReputation(valueDelta);
            }
        }
    }
}