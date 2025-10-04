#pragma once
#include "Action.h"

namespace Stalker::Mechanics::Relations
{
    /// <summary>
    /// Интерфейс наблюдателя за новыми действиями
    /// </summary>
    __interface IActionObserver
    {
        /// <summary>
        /// Реакция на регистрацию нового действия
        /// </summary>
        /// <param name="e">Новое действие</param>
        void OnAction(const Action& action);
    };
}