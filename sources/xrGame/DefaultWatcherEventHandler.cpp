#include "stdafx.h"
#include <exception>
#include <ai_space.h>
#include "script_engine.h"
#include "DefaultWatcherEventHandler.h"

namespace Stalker::Mechanics::Watchers
{
    DefaultWatcherEventHandler::DefaultWatcherEventHandler(const WatcherSettings& settings) :
        m_settings(settings)
    {
        VERIFY(m_settings);
    }

    void DefaultWatcherEventHandler::HandleEvent(const WatcherEvent& event)
    {
        auto callbackName = getHandlerByStatus((WatcherStatus)event.Status);
        luabind::functor<void> callback;

        try
        {
            if (!callbackName.empty() && ai().script_engine().functor(callbackName.data(), callback))
            {
                callback(event.ActionType, event.VictimId, event.WatcherId, event.GoodwillDelta, event.ReputationDelta, event.RankDelta);
            }
        }
        catch (const std::exception& ex)
        {
            Msg("Watcher system handler error: %s", ex.what());
        }
    }

    std::string_view DefaultWatcherEventHandler::getHandlerByStatus(const WatcherStatus& status)
    {
        switch (status)
        {
            case WatcherStatus::Watched:
                return m_settings.WatchedHandler();
            case WatcherStatus::Busted:
                return m_settings.BustedHandler();
            case WatcherStatus::Revealed:
                return m_settings.RevealedHandler();
            default:
                return std::string_view();
        }
    }
}