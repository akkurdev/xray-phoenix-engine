#pragma once
#include "RelationSettings.h"
#include "ActionHandlerBase.h"

class CRelationRegistryWrapper;

namespace Stalker::Mechanics::Relations
{
    /// <summary>
    /// Движковый обработчик действий - реализует логику оригинальной игры
    /// </summary>
    class EngineActionHandler final : public ActionHandlerBase
    {
    public:
        EngineActionHandler(CRelationRegistryWrapper* wrapper, RelationSettings* settings);
        ~EngineActionHandler();

        /// <summary>
        /// Обработка входящих запросов
        /// </summary>
        virtual void OnAction(const Action& action) override;

    private:
        RelationSettings* m_settings;
    };
}