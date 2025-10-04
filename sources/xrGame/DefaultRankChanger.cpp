#include "stdafx.h"
#include "character_reputation.h"
#include "Level.h"
#include "InventoryOwner.h"
#include "DefaultRankChanger.h"

namespace Stalker::Mechanics::Relations
{
    CharacterRank DefaultRankChanger::Get(const u16& targetId) const
    {
        if (CObject* obj = Level().Objects.net_Find(targetId))
        {
            if (CInventoryOwner* iOwner = smart_cast<CInventoryOwner*>(obj))
            {
                return iOwner->Rank();
            }
        }
        return NEUTRAL_REPUTATION_AMOUNT;
    }

    void DefaultRankChanger::Set(const u16& targetId, const CharacterRank& newValue)
    {
        if (CObject* obj = Level().Objects.net_Find(targetId))
        {
            if (CInventoryOwner* iOwner = smart_cast<CInventoryOwner*>(obj))
            {
                iOwner->SetRank(newValue);
            }
        }
    }

    void DefaultRankChanger::Change(const u16& targetId, const CharacterRank& valueDelta)
    {
        if (CObject* obj = Level().Objects.net_Find(targetId))
        {
            if (CInventoryOwner* iOwner = smart_cast<CInventoryOwner*>(obj))
            {
                iOwner->ChangeRank(valueDelta);
            }
        }
    }
}