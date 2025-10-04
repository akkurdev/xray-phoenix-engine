#pragma once
#include "IRelationTypeChanger.h"
#include "RelationSettings.h"

class CRelationRegistryWrapper;
class DefaultPersonalGoodwillChanger;
class DefaultCommunityGoodwillChanger;

namespace Stalker::Mechanics::Relations
{
    /// <summary>
    /// Класс для изменения типа отношений и связанных с ними вещей
    /// </summary>
    class DefaultRelationTypeChanger final : public IRelationTypeChanger
    {
    public:
        DefaultRelationTypeChanger(CRelationRegistryWrapper* wrapper, RelationSettings* settings);
        ~DefaultRelationTypeChanger();

        /// <summary>
        /// Определение окончательного типа отношений между персонажами
        /// </summary>
        /// <param name="npc1">Первый персонаж</param>
        /// <param name="npc2">Второй персонаж</param>
        /// <returns>Тип отношения первого персонажа ко второму</returns>
        virtual RelationType GetTotalRelationType(CInventoryOwner* npc1, CInventoryOwner* npc2) const;

        /// <summary>
        /// Суммарная доброжелательность одного персонажа к другому, с учетом всех факторов
        /// </summary>
        /// <param name="npc1">Первый персонаж</param>
        /// <param name="npc2">Второй персонаж</param>
        /// <returns>Значение доброжелательности первого персонажа ко второму</returns>
        virtual CharacterGoodwill GetTotalRelationAmount(CInventoryOwner* npc1, CInventoryOwner* npc2) const;

        /// <summary>
        /// Установка типа отношений одного персонажа к другому
        /// </summary>
        /// <param name="npc1">Первый персонаж</param>
        /// <param name="npc2">Второй персонаж</param>
        /// <param name="relationType">Новый тип отношений</param>
        virtual void SetTotalRelationType(CInventoryOwner* npc1, CInventoryOwner* npc2, RelationType relationType);

        /// <summary>
        /// Определение отношений между указанными группировками
        /// </summary>
        /// <param name="communityIndex1">Индекс первой группировки</param>
        /// <param name="communityIndex2">Индекс второй группировки</param>
        /// <returns>Значение доброжелательности персонажей одной группировки к персонажам другой</returns>
        virtual CharacterGoodwill GetGoodwillBetweenCommunities(CommunityIndex communityIndex1, CommunityIndex communityIndex2) const;

        /// <summary>
        /// ? 
        /// </summary>
        /// <param name="relationType">Тип отношений</param>
        virtual shared_str GetSpotName(RelationType relationType) const;

        /// <summary>
        /// Инициализация списков недоброжелателей для всех существующих персонажей
        /// </summary>
        virtual void InitializeCharactersFoes();

        /// <summary>
        /// Удаление персонажей из списка недоброжелателей друг друга
        /// </summary>
        /// <param name="npc1">Идентификатор первого персонажа</param>
        /// <param name="npc2">Идентификатор второго персонажа</param>
        virtual void ClearCharactersFoes(u16 npc1, u16 npc2);

        /// <summary>
        /// Сброс отношений (примирение)
        /// </summary>
        /// <param name="npc1">Идентификатор первого персонажа</param>
        /// <param name="npc2">Идентификатор второго персонажа</param>
        virtual void ResetRelations(u16 npc1, u16 npc2);

        /// <summary>
        /// Сброс всех отношений персонажа
        /// </summary>
        /// <param name="npcId">Идентификатор персонажа</param>
        virtual void ResetById(u16 npcId);

    private:
        CharacterGoodwill getGoodwillByReputation(const CharacterReputation& npcRep1, const CharacterReputation& npcRep2) const;
        CharacterGoodwill getGoodwillByRank(const CharacterRank& npcRank1, const CharacterRank& npcRank2) const;
        CharacterGoodwill getGoodwillBetweenCommunities(CommunityIndex communityIndex1, CommunityIndex communityIndex2) const;

    private:
        CRelationRegistryWrapper* m_wrapper;
        RelationSettings* m_settings;
        DefaultPersonalGoodwillChanger* m_pgChanger;
        DefaultCommunityGoodwillChanger* m_cgChanger;
    };
}