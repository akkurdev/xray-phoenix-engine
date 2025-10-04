#pragma once
#include "UnaryMetricChangerBase.h"

namespace Stalker::Mechanics::Relations
{
    /// <summary>
    /// Класс по умолчанию для изменения ранга
    /// </summary>
    class DefaultRankChanger final : public RankChangerBase
    {
    public:
        DefaultRankChanger() = default;
        ~DefaultRankChanger() = default;

        /// <summary>
        /// Получение текущего значения ранга
        /// </summary>
        /// <param name="targetId">Идентификатор персонажа, ранг которого запрашивается</param>
        virtual CharacterRank Get(const u16& targetId) const override;

        /// <summary>
        /// Установка нового значения ранга
        /// </summary>
        /// <param name="targetId">Идентификатор персонажа, ранг которого будет изменена</param>
        /// <param name="newValue">Новое значение ранга</param>
        virtual void Set(const u16& targetId, const CharacterRank& newValue) override;

        /// <summary>
        /// Изменение значения ранга на указанную дельту
        /// </summary>
        /// <param name="targetId">Идентификатор персонажа, репутация которого будет изменена</param>
        /// <param name="valueDelta">Дельта изменения ранга</param>
        virtual void Change(const u16& targetId, const CharacterRank& valueDelta) override;
    };
}
