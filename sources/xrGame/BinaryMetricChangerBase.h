#pragma once
#include <vector>
#include "Concepts.h"
#include "RelationDefinitions.h"

namespace Stalker::Mechanics::Relations
{
    /// <summary>
    /// Базовый класс для изменения бинарных метрик
    /// </summary>
    /// <typeparam name="SourceId">Тип идентификатора источника (владельца метрики)</typeparam>
    /// <typeparam name="TargetId">Тип идентификатора цели (в отношении кого изменяется метрика)</typeparam>
    /// <typeparam name="Metric">Тип метрики</typeparam>
    template<IdType SourceId, IdType TargetId, MetricType Metric>
    class BinaryMetricChangerBase
    {
    public:
        virtual ~BinaryMetricChangerBase() = default;

        BinaryMetricChangerBase(const BinaryMetricChangerBase&) = delete;
        BinaryMetricChangerBase& operator=(const BinaryMetricChangerBase&) = delete;

    public:
        /// <summary>
        /// Получение текущего значения метрики
        /// </summary>
        /// <param name="sourceId">Идентификатор того, чья метрика будет изменена</param>
        /// <param name="targetId">Идентификатор того, в отношении кого изменяется метрика</param>
        virtual Metric Get(const SourceId& sourceId, const TargetId& targetId) const = 0;

        /// <summary>
        /// Установка нового значения метрики
        /// </summary>
        /// <param name="sourceId">Идентификатор того, чья метрика будет изменена</param>
        /// <param name="targetId">Идентификатор того, в отношении кого изменяется метрика</param>
        /// <param name="newValue">Новое значение метрики</param>
        virtual void Set(const SourceId& sourceId, const TargetId& targetId, const Metric& newValue) = 0;

        /// <summary>
        /// Изменение значения метрики на указанную дельту
        /// </summary>
        /// <param name="sourceId">Идентификатор того, чья метрика будет изменена</param>
        /// <param name="targetId">Идентификатор того, в отношении кого изменяется метрика</param>
        /// <param name="valueDelta">Дельта изменения метрики</param>
        virtual void Change(const SourceId& sourceId, const TargetId& targetId, const Metric& valueDelta) = 0;

    protected:
        BinaryMetricChangerBase() = default;
    };

#pragma region Aliases of BinaryMetricChangerBase

    /// <summary>
    /// Базовый класс для изменения персональных отношений персонажей
    /// </summary>
    using PersonalGoodwillChangerBase = BinaryMetricChangerBase<u16, u16, CharacterGoodwill>;

    /// <summary>
    /// Базовый класс для изменения отношений персонажа с группировками
    /// </summary>
    using CommunityGoodwillChangerBase = BinaryMetricChangerBase<CommunityIndex, u16, CharacterGoodwill>;

#pragma endregion
}