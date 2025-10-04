#include "stdafx.h"
#include "CharacterMetricTable.h"

namespace Stalker::Mechanics::Relations
{
    CharacterMetricTable::CharacterMetricTable(RelationSettings* settings)
    {
        m_attackGoodwillTable = {
            { RelationType::eRelationTypeEnemy, settings->AttackEnemyGoodwillAmount() },
            { RelationType::eRelationTypeNeutral, settings->AttackNeutralGoodwillAmount()},
            { RelationType::eRelationTypeFriend, settings->AttackFriendGoodwillAmount()}
        };

        m_killGoodwillTable = {
            { RelationType::eRelationTypeEnemy, settings->KillEnemyGoodwillAmount() },
            { RelationType::eRelationTypeNeutral, settings->KillNeutralGoodwillAmount()},
            { RelationType::eRelationTypeFriend, settings->KillFriendGoodwillAmount()}
        };

        m_helpGoodwillTable = {
            { RelationType::eRelationTypeEnemy, settings->HelpEnemyGoodwillAmount() },
            { RelationType::eRelationTypeNeutral, settings->HelpNeutralGoodwillAmount()},
            { RelationType::eRelationTypeFriend, settings->HelpFriendGoodwillAmount()}
        };

        m_attackReputationTable = {
            { RelationType::eRelationTypeEnemy, settings->AttackEnemyReputationAmount() },
            { RelationType::eRelationTypeNeutral, settings->AttackNeutralReputationAmount()},
            { RelationType::eRelationTypeFriend, settings->AttackFriendReputationAmount()}
        };

        m_killReputationTable = {
            { RelationType::eRelationTypeEnemy, settings->KillEnemyReputationAmount() },
            { RelationType::eRelationTypeNeutral, settings->KillNeutralReputationAmount()},
            { RelationType::eRelationTypeFriend, settings->KillFriendReputationAmount()}
        };

        m_helpReputationTable = {
            { RelationType::eRelationTypeEnemy, settings->HelpEnemyReputationAmount() },
            { RelationType::eRelationTypeNeutral, settings->HelpNeutralReputationAmount()},
            { RelationType::eRelationTypeFriend, settings->HelpFriendReputationAmount()}
        };
    }

    CharacterGoodwill CharacterMetricTable::FindGoodwill(ActionType action, RelationType relation)
    {
        switch (action)
        {
            case ActionType::Wound:
                return TryGetValue(m_attackGoodwillTable, relation);
            case ActionType::Kill:
                return TryGetValue(m_killGoodwillTable, relation);
            default:
                return CharacterGoodwill(NEUTRAL_GOODWILL);
        }
    }

    CharacterReputation CharacterMetricTable::FindReputation(ActionType action, RelationType relation)
    {
        switch (action)
        {
            case ActionType::Wound:
                return TryGetValue(m_attackReputationTable, relation);
            case ActionType::Kill:
                return TryGetValue(m_killReputationTable, relation);
            default:
                return CharacterReputation(NEUTRAL_REPUTATION);
        }
    }
}