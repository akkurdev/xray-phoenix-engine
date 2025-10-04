#pragma once
#include "WatchersSettings.h"
#include "IWatcherController.h"

namespace Stalker::Mechanics::Watchers
{
    /// <summary>
    /// Класс, реализующий функционал системы свидетелей в игре. 
    /// Является синглтоном.
    /// </summary>
    class WatcherSystem final
    {
    public:
        ~WatcherSystem();

        /// <summary>
        /// Предоставляет ссылку на единственный экземпляр WatcherSystem
        /// </summary>
        static WatcherSystem& Instance();

        /// <summary>
        /// Контроллер свидетелей
        /// </summary>
        IWatcherController* Controller();

    private:
        WatcherSystem();

    private:
        static WatcherSystem* m_instance;

        IWatcherController* m_controller;
        WatcherSettings m_settings;
    };
}