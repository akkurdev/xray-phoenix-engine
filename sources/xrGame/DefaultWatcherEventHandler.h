#pragma once
#include "WatchersSettings.h"
#include "IWatcherEventHandler.h"

namespace Stalker::Mechanics::Watchers
{
    /// <summary>
    /// Обработчик свидетельских событий по умолчанию
    /// </summary>
    class DefaultWatcherEventHandler final : public IWatcherEventHandler
    {
    public:
        DefaultWatcherEventHandler(const WatcherSettings& settings);
        virtual ~DefaultWatcherEventHandler() = default;

        /// <summary>
        /// Обработка свидетельского события
        /// </summary>
        /// <param name="event">Свидетельское событие</param>
        virtual void HandleEvent(const WatcherEvent& event);

    private:
        std::string_view getHandlerByStatus(const WatcherStatus& status);

    private:
        WatcherSettings m_settings;
    };
}