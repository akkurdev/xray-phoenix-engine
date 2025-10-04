#pragma once
#include "Concepts.h"
#include "RelationDefinitions.h"
#include "InventoryOwner.h"

namespace Stalker::Mechanics::Relations
{
    /// <summary>
    /// Интерфейс для изменения типа отношений между персонажами
    /// </summary>
    __interface IRelationTypeChanger
    {
        /// <summary>
        /// Определение окончательного типа отношений между персонажами
        /// </summary>
        /// <param name="npc1">Первый персонаж</param>
        /// <param name="npc2">Второй персонаж</param>
        /// <returns>Тип отношения первого персонажа ко второму</returns>
        RelationType GetTotalRelationType(CInventoryOwner* npc1, CInventoryOwner* npc2) const;

        /// <summary>
        /// Суммарная доброжелательность одного персонажа к другому, с учетом всех факторов
        /// </summary>
        /// <param name="npc1">Первый персонаж</param>
        /// <param name="npc2">Второй персонаж</param>
        /// <returns>Значение доброжелательности первого персонажа ко второму</returns>
        CharacterGoodwill GetTotalRelationAmount(CInventoryOwner* npc1, CInventoryOwner* npc2) const;

        /// <summary>
        /// Установка типа отношений одного персонажа к другому
        /// </summary>
        /// <param name="npc1">Первый персонаж</param>
        /// <param name="npc2">Второй персонаж</param>
        /// <param name="relationType">Новый тип отношений</param>
        void SetTotalRelationType(CInventoryOwner* npc1, CInventoryOwner* npc2, RelationType relationType);

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
        shared_str GetSpotName(RelationType relationType) const;        
        
        /// <summary>
        /// Инициализация списков недоброжелателей для всех существующих персонажей
        /// </summary>
        void InitializeCharactersFoes();

        /// <summary>
        /// Удаление персонажей из списка недоброжелателей друг друга
        /// </summary>
        /// <param name="npc1">Идентификатор первого персонажа</param>
        /// <param name="npc2">Идентификатор второго персонажа</param>
        void ClearCharactersFoes(u16 npc1, u16 npc2);

        /// <summary>
        /// Сброс отношений (примирение)
        /// </summary>
        /// <param name="npc1">Идентификатор первого персонажа</param>
        /// <param name="npc2">Идентификатор второго персонажа</param>
        void ResetRelations(u16 npc1, u16 npc2);

        /// <summary>
        /// Сброс всех отношений персонажа
        /// </summary>
        /// <param name="npcId">Идентификатор персонажа</param>
        void ResetById(u16 npcId);
    };
}