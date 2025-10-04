#pragma once
#include "BinaryMetricChangerBase.h"

class CRelationRegistryWrapper;

namespace Stalker::Mechanics::Relations
{
    /// <summary>
    /// Класс по умолчанию для изменения отношений между персонажами и группировками
    /// </summary>
    class DefaultCommunityGoodwillChanger final : public CommunityGoodwillChangerBase
    {
    public:
        DefaultCommunityGoodwillChanger(CRelationRegistryWrapper* wrapper);
        ~DefaultCommunityGoodwillChanger() = default;

        /// <summary>
        /// Отношение группировки к персонажу
        /// </summary>
        /// <param name="comminityIndex">Индекс группировки</param>
        /// <param name="npcId">Идентификатор персонажа</param>
        virtual CharacterGoodwill Get(const CommunityIndex& communityIndex, const u16& npcId) const override;

        /// <summary>
        /// Установка отношения группировки к персонажу
        /// </summary>
        /// <param name="communityIndex">Индекс группировки</param>
        /// <param name="npcId">Идентификатор персонажа</param>
        /// <param name="goodwill">Новое значение отношения</param>
        virtual void Set(const CommunityIndex& communityIndex, const u16& npcId, const CharacterGoodwill& goodwill) override;

        /// <summary>
        /// Изменение отношения группировки к персонажу
        /// </summary>
        /// <param name="communityIndex">Индекс группировки</param>
        /// <param name="npcId">Идентификатор персонажа</param>
        /// <param name="goodwillDelta">Величина изменения отношения</param>
        virtual void Change(const CommunityIndex& communityIndex, const u16& npcId, const CharacterGoodwill& goodwillDelta) override;

    private:
        CRelationRegistryWrapper* m_wrapper;
    };
}