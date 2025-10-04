#pragma once
#include "IActionObserver.h"
#include "UnaryMetricChangerBase.h"
#include "BinaryMetricChangerBase.h"
#include "IRelationTypeChanger.h"

namespace Stalker::Mechanics::Relations
{
    /// <summary>
    /// базовый класс исполнителя запросов на изменение метрик отношений
    /// </summary>
    class ActionHandlerBase : public IActionObserver
    {
    public:
        virtual ~ActionHandlerBase() = default;
        ActionHandlerBase(const ActionHandlerBase&) = delete;
        ActionHandlerBase& operator=(const ActionHandlerBase&) = delete;

        /// <summary>
        /// Интерфейс для изменения персональных отношений
        /// </summary>
        PersonalGoodwillChangerBase* PersonalGoodwill();

        /// <summary>
        /// Интерфейс для изменения отношений с группировками
        /// </summary>
        CommunityGoodwillChangerBase* CommunityGoodwill();

        /// <summary>
        /// Интерфейс для изменения репутации
        /// </summary>
        ReputationChangerBase* Reputation();
        
        /// <summary>
        /// Интерфейс для изменения рангов
        /// </summary>
        RankChangerBase* Rank();

        /// <summary>
        /// Интерфейс для изменения типа отношений
        /// </summary>
        IRelationTypeChanger* RelationType();

        /// <summary>
        /// Обработчик регистрации нового действия
        /// </summary>
        /// <param name="action">Действие актора</param>
        virtual void OnAction(const Action& action) = 0;

    protected:
        ActionHandlerBase() = default;        

    protected:
        PersonalGoodwillChangerBase* m_personalGoodwillChanger;
        CommunityGoodwillChangerBase* m_communityGoodwillChanger;
        ReputationChangerBase* m_reputationChanger;
        RankChangerBase* m_rankChanger;
        IRelationTypeChanger* m_relationTypeChanger;
    };
}
