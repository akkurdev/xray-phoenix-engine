#pragma once
#include "RelationSettings.h"
#include "IActionController.h"
#include "ActionHandlerBase.h"

using namespace Stalker::Mechanics::Relations;
class CRelationRegistryWrapper;

/// <summary>
/// Система внутриигровых отношений. Является синглтоном.
/// </summary>
class RelationSystem final
{
public:    
    ~RelationSystem();
    RelationSystem(const RelationSystem&) = delete;
    RelationSystem& operator=(const RelationSystem&) = delete;

    /// <summary>
    /// Получение экземпляра системы отношений
    /// </summary>
    static RelationSystem& Instance();

    /// <summary>
    /// Контроллер системы отношений
    /// </summary>
    IActionController* Controller();

    /// <summary>
    /// Исполнитель запросов на изменение метрик отношений
    /// </summary>
    ActionHandlerBase* Executor();

    /// <summary>
    /// Настройки системы отношений
    /// </summary>
    RelationSettings Settings() const;

 private:
     RelationSystem();

private:
    static RelationSystem* m_instance;
    
    CRelationRegistryWrapper* m_wrapper;
    RelationSettings*  m_settings;
    IActionController* m_controller;
    ActionHandlerBase* m_executor;
};
