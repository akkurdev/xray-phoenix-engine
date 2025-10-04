#pragma once
#include "WatchersSettings.h"
#include "IWatcherController.h"

class CInventoryOwner;

namespace Stalker::Mechanics::Watchers
{
    /// <summary>
    /// ��������� ����������� ����������
    /// </summary>
    class DefaultWatcherController : public IWatcherController
    {
    public:
        DefaultWatcherController(const WatcherSettings& settings);
        virtual ~DefaultWatcherController();
        
        /// <summary>
        /// ���������� ��������� �����������
        /// </summary>
        virtual void Update();

        /// <summary>
        /// �������� �� �����
        /// </summary>
        virtual void Load(IReader& reader);

        /// <summary>
        /// ���������� � ����
        /// </summary>
        virtual void Save(IWriter& writer);

        /// <summary>
        /// ������� �� ����� �������� ������ � ����� NPC
        /// </summary>
        /// <param name="action">��������</param>
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