#include "stdafx.h"
#include "DefaultWatcherController.h"
#include "RelationSystem.h"
#include "WatcherSystem.h"

namespace Stalker::Mechanics::Watchers
{
    WatcherSystem* WatcherSystem::m_instance = nullptr;
    
    WatcherSystem::WatcherSystem() :
        m_settings(WatcherSettings())
    {
        m_controller = new DefaultWatcherController(m_settings);
        RelationSystem::Instance().Controller()->AddObserver(m_controller);
    }
    
    WatcherSystem::~WatcherSystem()
    {
        RelationSystem::Instance().Controller()->RemoveObserver(m_controller);
        delete m_controller;
    }

    WatcherSystem& WatcherSystem::Instance()
    {
        if (m_instance == nullptr)
        {
            m_instance = new WatcherSystem();
        }
        return *m_instance;
    }

    IWatcherController* WatcherSystem::Controller()
    {
        return m_controller;
    }
}