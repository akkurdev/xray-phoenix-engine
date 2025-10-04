#pragma once
#include "WatcherEvent.h"

namespace Stalker::Mechanics::Watchers
{
    /// <summary>
    /// Интерфейс обработчика свидетельских событий
    /// </summary>
    __interface IWatcherEventHandler
    {
        /// <summary>
        /// Обработка свидетельского события
        /// </summary>
        /// <param name="event">Свидетельское событие</param>
        void HandleEvent(const WatcherEvent& event);
    };
}