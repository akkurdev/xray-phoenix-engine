#pragma once

namespace Stalker::Mechanics::Relations
{
    /// <summary>
    /// Тип действия в отношении персонажа
    /// </summary>
    enum class ActionType
    {
        /// <summary>
        /// Ранение
        /// </summary>
        Wound = 0x0,

        /// <summary>
        /// Убийство
        /// </summary>
        Kill = 0x1
    };
}