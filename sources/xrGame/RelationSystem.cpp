#include "stdafx.h"
#include "alife_registry_wrappers.h"
#include "DefaultActionController.h"
#include "EngineActionHandler.h"
#include "ScriptActionHandler.h"
#include "RelationSystem.h"

RelationSystem* RelationSystem::m_instance = nullptr;

RelationSystem& RelationSystem::Instance()
{
    if (!m_instance)
    {
        m_instance = new RelationSystem();
    }
    return *m_instance;
}

IActionController* RelationSystem::Controller()
{
    return m_controller;
}

ActionHandlerBase* RelationSystem::Executor()
{
    return m_executor;
}

RelationSettings RelationSystem::Settings() const
{
    return RelationSettings();
}

RelationSystem::RelationSystem()
{
    m_wrapper    = new CRelationRegistryWrapper();
    m_settings   = new RelationSettings();
    m_controller = new DefaultActionController(m_wrapper, m_settings);

    if (m_settings->IsScriptActionHandlerEnabled())
    {
        m_executor = new ScriptActionHandler(m_wrapper, m_settings);
    }
    else m_executor = new EngineActionHandler(m_wrapper, m_settings);

    m_controller->AddObserver(m_executor);
}

RelationSystem::~RelationSystem()
{
    m_controller->RemoveObserver(m_executor);
    
    // Уничтожение в порядке, обратном порядку создания
    delete m_executor;
    delete m_controller;
    delete m_settings;
    delete m_wrapper;
}