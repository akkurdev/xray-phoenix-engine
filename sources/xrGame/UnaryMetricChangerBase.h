#pragma once
#include <vector>
#include "Concepts.h"
#include "RelationDefinitions.h"

namespace Stalker::Mechanics::Relations
{
    /// <summary>
    /// Базовый класс для изменения унарных метрик
    /// </summary>
    /// <typeparam name="TargetId">Тип идентификатора персонажа, чьи метрики будут изменены</typeparam>
    /// <typeparam name="Metric">Тип метрики</typeparam>
    template<IdType TargetId, MetricType Metric>
    class UnaryMetricChangerBase
    {
    public:
        virtual ~UnaryMetricChangerBase() = default;

        UnaryMetricChangerBase(const UnaryMetricChangerBase&) = delete;
        UnaryMetricChangerBase& operator=(const UnaryMetricChangerBase&) = delete;

    public:
        /// <summary>
        /// Получение текущего значения метрики
        /// </summary>
        /// <param name="targetId">Идентификатор персонажа, метрика которого запрашивается</param>
        virtual Metric Get(const TargetId& targetId) const = 0;

        /// <summary>
        /// Установка нового значения метрики
        /// </summary>
        /// <param name="targetId">Идентификатор персонажа, метрика которого будет изменена</param>
        /// <param name="newValue">Новое значение метрики</param>
        virtual void Set(const TargetId& targetId, const Metric& newValue) = 0;

        /// <summary>
        /// Изменение значения метрики на указанную дельту
        /// </summary>
        /// <param name="targetId">Идентификатор персонажа, метрика которого будет изменена</param>
        /// <param name="valueDelta">Дельта изменения метрики</param>
        virtual void Change(const TargetId& targetId, const Metric& valueDelta) = 0;

    protected:
        UnaryMetricChangerBase() = default;
    };

#pragma region Aliases of UnaryMetricChangerBase

    /// <summary>
    /// Базовый класс для изменения значения репутации персонажа
    /// </summary>
    using ReputationChangerBase = UnaryMetricChangerBase<u16, CharacterReputation>;

    /// <summary>
    /// Базовый класс для изменения значения ранга персонажа
    /// </summary>
    using RankChangerBase = UnaryMetricChangerBase<u16, CharacterRank>;

#pragma endregion
}