#pragma once
#include "WatchersSettings.h"
#include "IWatcherController.h"

class CInventoryOwner;

namespace Stalker::Mechanics::Watchers
{
    /// <summary>
    /// Интерфейс контроллера свидетелей
    /// </summary>
    class DefaultWatcherController : public IWatcherController
    {
    public:
        DefaultWatcherController(const WatcherSettings& settings);
        virtual ~DefaultWatcherController();
        
        /// <summary>
        /// Обновление состояния контроллера
        /// </summary>
        virtual void Update();

        /// <summary>
        /// Загрузка из файла
        /// </summary>
        virtual void Load(IReader& reader);

        /// <summary>
        /// Сохранение в файл
        /// </summary>
        virtual void Save(IWriter& writer);

        /// <summary>
        /// Реакция на новое действие актора в адрес NPC
        /// </summary>
        /// <param name="action">Действие</param>
        virtual void OnAction(const Action& action);

    private:
        CInventoryOwner* getCharacterById(u16 characterId) const;
        int32_t findStoryIndex(const u16& characterId) const;
        WatcherEvent buildEvent(const WatcherStatus& status, const u16& watcherId, const WatchableAction& action) const;

        void onCharacterKilled(const u16& characterId);
        void addAction(const WatchableAction& action);
        void removeAction(const WatchableAction& action);        

    private:
        WatcherSettings m_settings;
        IWatcherMonitor* m_monitor;
        IWatcherEventHandler* m_eventHandler;

        std::vector<WatchableAction> m_actions;        
        std::mutex m_lock;
    };
}