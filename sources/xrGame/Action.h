#pragma once
#include "RelationDefinitions.h"
#include "ActionType.h"
#include "script_export_space.h"

using namespace luabind;

namespace Stalker::Mechanics::Relations
{
    /// <summary>
    /// Действие актора в адрес NPC
    /// </summary>
    struct Action final
    {
    public:
        /// <summary>
        /// Идентификатор жертвы
        /// </summary>
        u16 VictimId;

        /// <summary>
        /// Тип действия
        /// </summary>
        ActionType Type;

        /// <summary>
        /// Время совершения действия
        /// </summary>
        u64 Time;

        /// <summary>
        /// Позиция актора
        /// </summary>
        Fvector ActorPosition;

        /// <summary>
        /// Позиция жертвы
        /// </summary>
        Fvector VictimPosition;

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
        /// Кость скелета персонажа, в которую был нанесен хит
        /// </summary>
        u16 DamagedBone;

        /// <summary>
        /// Убит ли персонаж с первого выстрела. В случае ранения всегда false.
        /// </summary>
        bool HasOneShot;

        DECLARE_SCRIPT_REGISTER_FUNCTION

    private:
        u8 typeToInt();
    };

    add_to_type_list(Action)
    #undef script_type_list
    #define script_type_list save_type_list(Action)
}