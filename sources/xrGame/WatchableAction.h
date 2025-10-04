#pragma once
#include <map>
#include "Action.h"

namespace Stalker::Mechanics::Watchers
{
    using namespace Stalker::Mechanics::Relations;
    
    /// <summary>
    /// Структура, хранящая данные об истории наблюдения NPC за конкретным действием
    /// </summary>
    struct WatchableAction final
    {
        /// <summary>
        /// Идентификатор жертвы
        /// </summary>
        u16 VictimId;

        /// <summary>
        /// Тип действия в адрес жертвы
        /// </summary>
        ActionType Action;

        /// <summary>
        /// Время совершения действия
        /// </summary>
        u64 Time;

        /// <summary>
        /// Величина, на которую изменится отношение к актору
        /// </summary>
        s16 GoodwillDelta;

        /// <summary>
        /// Величина, на которую изменится репутация актора
        /// </summary>
        s16 ReputationDelta;

        /// <summary>
        /// Величина, на которую изменится ранг актора при убийстве жертвы
        /// </summary>
        s16 RankDelta;

        /// <summary>
        /// Свидетели действия
        /// </summary>
        std::map<u16, u64> Watchers;
    };
}