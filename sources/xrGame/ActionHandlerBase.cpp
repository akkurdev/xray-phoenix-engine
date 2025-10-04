#include "stdafx.h"
#include <algorithm>
#include "Level.h"
#include "ActionHandlerBase.h"

namespace Stalker::Mechanics::Relations
{    
    PersonalGoodwillChangerBase* ActionHandlerBase::PersonalGoodwill()
    {
        return m_personalGoodwillChanger;
    }

    CommunityGoodwillChangerBase* ActionHandlerBase::CommunityGoodwill()
    {
        return m_communityGoodwillChanger;
    }

    ReputationChangerBase* ActionHandlerBase::Reputation()
    {
        return m_reputationChanger;
    }

    RankChangerBase* ActionHandlerBase::Rank()
    {
        return m_rankChanger;
    }

    IRelationTypeChanger* ActionHandlerBase::RelationType()
    {
        return m_relationTypeChanger;
    }
}