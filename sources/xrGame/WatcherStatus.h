#pragma once
#include "RelationDefinitions.h"

namespace Stalker::Mechanics::Watchers
{
    /// <summary>
    /// Статус свидетеля
    /// </summary>
    enum class WatcherStatus
    {
        /// <summary>
        /// Статус не определен
        /// </summary>
        Unknown,

        /// <summary>
        /// Свидетель увидел жертву
        /// </summary>
        Watched,

        /// <summary>
        /// Свидетель увидел актора
        /// </summary>
        Busted,

        /// <summary>
        /// Свидетель рассказал об увиденном
        /// </summary>
        Revealed
    };
}
