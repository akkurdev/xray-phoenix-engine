#include "stdafx.h"
#include <exception>
#include "Level.h"
#include "alife_registry_wrappers.h"
#include "script_engine.h"
#include "DefaultPersonalGoodwillChanger.h"
#include "DefaultCommunityGoodwillChanger.h"
#include "DefaultReputationChanger.h"
#include "DefaultRankChanger.h"
#include "DefaultRelationTypeChanger.h"
#include "ScriptActionHandler.h"

namespace Stalker::Mechanics::Relations
{
    ScriptActionHandler::ScriptActionHandler(CRelationRegistryWrapper* wrapper, RelationSettings* settings) :
        m_settings(settings)
    {
        m_personalGoodwillChanger = new DefaultPersonalGoodwillChanger(wrapper);
        m_communityGoodwillChanger = new DefaultCommunityGoodwillChanger(wrapper);
        m_reputationChanger = new DefaultReputationChanger();
        m_rankChanger = new DefaultRankChanger();
        m_relationTypeChanger = new DefaultRelationTypeChanger(wrapper, settings);
    }

    ScriptActionHandler::~ScriptActionHandler()
    {
        delete m_relationTypeChanger;
        delete m_rankChanger;
        delete m_reputationChanger;
        delete m_communityGoodwillChanger;
        delete m_personalGoodwillChanger;
    }

    void ScriptActionHandler::OnAction(const Action& action)
    {
        auto callbackName = m_settings->ScriptActionHandlerName();
        luabind::functor<void> callback;

        try
        {
            if (ai().script_engine().functor(callbackName.data(), callback))
            {
                callback(action);
            }
        }
        catch (const std::exception& ex)
        {
            Msg("Relation system script handler error: %s", ex.what());
        }
    }
}