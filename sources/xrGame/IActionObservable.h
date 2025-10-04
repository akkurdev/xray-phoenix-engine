#pragma once
#include "IActionObserver.h"

namespace Stalker::Mechanics::Relations
{
    /// <summary>
    /// Интерфейс, оповещающий о регистрации новых действий, меняющих метрики отношений
    /// </summary>
    __interface IActionObservable
    {
        /// <summary>
        /// Подписка нового наблюдателя
        /// </summary>
        /// <param name="observer">Новый наблюдатель</param>
        void AddObserver(IActionObserver* observer);

        /// <summary>
        /// Отписка существующего наблюдателя
        /// </summary>
        /// <param name="observer">Существующий наблюдатель</param>
        void RemoveObserver(IActionObserver* observer);

        /// <summary>
        /// Оповещение наблюдателей о регистрации нового действия
        /// </summary>
        /// <param name="query">Новое действие</param>
        void NotifyObservers(const Action& action);
    };
}