#pragma once
#include "RelationSettings.h"
#include "ActionHandlerBase.h"

class CRelationRegistryWrapper;

namespace Stalker::Mechanics::Relations
{
    /// <summary>
    /// Скриптовый обработчик запросов
    /// </summary>
    class ScriptActionHandler final : public ActionHandlerBase
    {
    public:
        ScriptActionHandler(CRelationRegistryWrapper* wrapper, RelationSettings* settings);
        ~ScriptActionHandler();

        /// <summary>
        /// Обработка входящих запросов
        /// </summary>
        virtual void OnAction(const Action& action) override;

    private:
        RelationSettings* m_settings;
    };
}