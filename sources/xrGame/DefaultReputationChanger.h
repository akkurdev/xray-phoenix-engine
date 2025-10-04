#pragma once
#include "UnaryMetricChangerBase.h"

namespace Stalker::Mechanics::Relations
{
    /// <summary>
    /// Класс по умолчанию для изменения репутации
    /// </summary>
    class DefaultReputationChanger final : public ReputationChangerBase
    {
    public:
        DefaultReputationChanger() = default;
        ~DefaultReputationChanger() = default;

        /// <summary>
        /// Получение текущего значения репутации
        /// </summary>
        /// <param name="targetId">Идентификатор персонажа, репутация которого запрашивается</param>
        virtual CharacterReputation Get(const u16& targetId) const override;

        /// <summary>
        /// Установка нового значения репутации
        /// </summary>
        /// <param name="targetId">Идентификатор персонажа, репутация которого будет изменена</param>
        /// <param name="newValue">Новое значение репутации</param>
        virtual void Set(const u16& targetId, const CharacterReputation& newValue) override;

        /// <summary>
        /// Изменение значения репутации на указанную дельту
        /// </summary>
        /// <param name="targetId">Идентификатор персонажа, репутация которого будет изменена</param>
        /// <param name="valueDelta">Дельта изменения репутации</param>
        virtual void Change(const u16& targetId, const CharacterReputation& valueDelta) override;
    };
}