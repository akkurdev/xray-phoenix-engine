#include "stdafx.h"
#include "Level.h"
#include "entity_alive.h"
#include <seniority_hierarchy_holder.h>
#include <team_hierarchy_holder.h>
#include <squad_hierarchy_holder.h>
#include <group_hierarchy_holder.h>
#include <memory_manager.h>
#include "alife_registry_wrappers.h"
#include "DefaultPersonalGoodwillChanger.h"
#include "DefaultCommunityGoodwillChanger.h"
#include "DefaultReputationChanger.h"
#include "DefaultRankChanger.h"
#include "DefaultRelationTypeChanger.h"
#include "EngineActionHandler.h"

namespace Stalker::Mechanics::Relations
{
    EngineActionHandler::EngineActionHandler(CRelationRegistryWrapper* wrapper, RelationSettings* settings) : 
        m_settings(settings)
    {
        m_personalGoodwillChanger = new DefaultPersonalGoodwillChanger(wrapper);
        m_communityGoodwillChanger = new DefaultCommunityGoodwillChanger(wrapper);
        m_reputationChanger = new DefaultReputationChanger();
        m_rankChanger = new DefaultRankChanger();
        m_relationTypeChanger = new DefaultRelationTypeChanger(wrapper, settings);
    }

    EngineActionHandler::~EngineActionHandler()
    {
        delete m_relationTypeChanger;
        delete m_rankChanger;
        delete m_reputationChanger;
        delete m_communityGoodwillChanger;
        delete m_personalGoodwillChanger;
    }

    void EngineActionHandler::OnAction(const Action& action)
    {
        auto attackerObject = Level().Objects.net_Find(0);
        auto defenderObject = Level().Objects.net_Find(action.VictimId);

        if (!attackerObject || !defenderObject) return;
        
        CEntityAlive* attacker = smart_cast<CEntityAlive*>(attackerObject);
        CEntityAlive* defender = smart_cast<CEntityAlive*>(defenderObject);

        if (!attacker || !defender) return;

        if (action.GoodwillDelta != 0)
        {
            auto& stalkerGroup = Level()
                .seniority_holder()
                .team(defender->g_Team())
                .squad(defender->g_Squad())
                .group(defender->g_Group());

            // Разозлить всех, у кого такие же g_Team, g_Squad и g_Group, как и у пострадавшего
            for (const auto& member : stalkerGroup.members())
            {
                m_personalGoodwillChanger->Change(member->ID(), attacker->ID(), action.GoodwillDelta);
            }
        }

        if (action.ReputationDelta != 0)
        {
            m_reputationChanger->Change(attacker->ID(), action.ReputationDelta);
        }

        if (action.RankDelta != 0)
        {
            m_rankChanger->Change(attacker->ID(), action.RankDelta);
        }        
    }
}