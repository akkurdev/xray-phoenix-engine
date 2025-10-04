#pragma once
#include "WatcherStatus.h"
#include "ActionType.h"
#include "script_export_space.h"

using namespace luabind;

namespace Stalker::Mechanics::Watchers
{
    /// <summary>
    /// Событие, оповещающее об изменении статуса свидетеля
    /// </summary>
    struct WatcherEvent final
    {
    public:
        /// <summary>
        /// Статус свидетеля
        /// </summary>
        u32 Status;
        
        /// <summary>
        /// Идентификатор свидетеля
        /// </summary>
        u16 WatcherId;

        /// <summary>
        /// Тип действия в адрес жертвы
        /// </summary>
        u32 ActionType;
        
        /// <summary>
        /// Идентификатор жертвы
        /// </summary>
        u16 VictimId;

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

        DECLARE_SCRIPT_REGISTER_FUNCTION_STRUCT
    };

    add_to_type_list(WatcherEvent)
    #undef script_type_list
    #define script_type_list save_type_list(WatcherEvent)
}