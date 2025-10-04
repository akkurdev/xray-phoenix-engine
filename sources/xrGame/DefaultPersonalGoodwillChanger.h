#pragma once
#include "BinaryMetricChangerBase.h"

class CRelationRegistryWrapper;

namespace Stalker::Mechanics::Relations
{
    /// <summary>
    /// Класс по умолчанию, изменяюший персональные отношения между персонажами
    /// </summary>
    class DefaultPersonalGoodwillChanger final : public PersonalGoodwillChangerBase
    {
    public:
        DefaultPersonalGoodwillChanger(CRelationRegistryWrapper* wrapper);
        ~DefaultPersonalGoodwillChanger() = default;

        /// <summary>
        /// Получение отношения одного персонажа к другому
        /// </summary>
        /// <param name="sourceId">Идентификатор персонажа, чье отношение необходимо получить</param>
        /// <param name="targetId">Идентификатор целевого персонажа</param>
        /// <returns>Значение отношения персонажа</returns>
        virtual CharacterGoodwill Get(const u16& sourceId, const u16& targetId) const override;

        /// <summary>
        /// Установка отношения одного персонажа к другому
        /// </summary>
        /// <param name="sourceId">Идентификатор персонажа, чье отношение необходимо установить</param>
        /// <param name="targetId">Идентификатор целевого персонажа</param>
        /// <param name="goodwill">Новое значение отношения</param>
        virtual void Set(const u16& sourceId, const u16& targetId, const CharacterGoodwill& goodwill) override;

        /// <summary>
        /// Изменение отношения одного персонажа к другому
        /// </summary>
        /// <param name="sourceId">Идентификатор персонажа, чье отношение необходимо изменить</param>
        /// <param name="targetId">Идентификатор целевого персонажа</param>
        /// <param name="goodwillDelta">Дельта, на которую увеличится текущее значение отношения</param>
        virtual void Change(const u16& sourceId, const u16& targetId, const CharacterGoodwill& goodwillDelta) override;

    private:
        CRelationRegistryWrapper* m_wrapper;
    };
}