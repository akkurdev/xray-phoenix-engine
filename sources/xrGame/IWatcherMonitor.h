#pragma once
#include <vector>
#include "script_entity.h"
#include "WatchableAction.h"
#include "WatcherEvent.h"

namespace Stalker::Mechanics::Watchers
{
    /// <summary>
    /// Интерфейс монитора свидетелей
    /// </summary>
    __interface IWatcherMonitor
    {
        /// <summary>
        /// Активен ли монитор в текущий момент
        /// </summary>
        bool IsActive() const;

        /// <summary>
        /// Идентификатор последней жертвы, инициировавшей мониторинг
        /// </summary>
        u16 LastVictimId() const;
        
        /// <summary>
        /// Обновление монитора при новых действиях в адрес NPC
        /// </summary>
        /// <param name="characterId">Идентификатор NPC</param>
        void Update(const u16& characterId);

        /// <summary>
        /// Проверка свидетелей для указанного действия
        /// </summary>
        /// <param name="action">Наблюдаемое действие</param>
        /// <returns>Набор пар, описывающих измененные состояния свидетелей</returns>
        std::vector<std::pair<u16, u64>> CheckWatchers(WatchableAction& action);
    };
}