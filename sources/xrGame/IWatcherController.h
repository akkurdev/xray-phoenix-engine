#pragma once
#include "IActionObserver.h"
#include "IWatcherMonitor.h"
#include "IWatcherEventHandler.h"

namespace Stalker::Mechanics::Watchers
{
    /// <summary>
    /// Интерфейс контроллера свидетелей
    /// </summary>
    __interface IWatcherController : public IActionObserver
    {
        /// <summary>
        /// Обновление состояния контроллера
        /// </summary>
        void Update();

        /// <summary>
        /// Загрузка из файла
        /// </summary>
        void Load(IReader& reader);

        /// <summary>
        /// Сохранение в файл
        /// </summary>
        void Save(IWriter& writer);
    };
}