#pragma once
#include <algorithm>
#include <xrCore/xrCore.h>
#include <alife_space.h>
#include <character_rank.h>
#include <character_reputation.h>
#include "character_info_defs.h"

namespace Stalker::Mechanics::Relations
{
    /// <summary>
    /// Отношения между персонажами, от крайне враждебного до крайне дружелюбного
    /// </summary>
    using CharacterGoodwill = CHARACTER_GOODWILL;

    /// <summary>
    /// Репутация персонажа, от крайне отвратительной (беспредел) до крайне уважительной (благородство)
    /// </summary>
    using CharacterReputation = CHARACTER_REPUTATION_VALUE;

    /// <summary>
    /// Ранг персонажа, от крайне неопытного новичка до профессионала
    /// </summary>
    using CharacterRank = CHARACTER_RANK_VALUE;

    /// <summary>
    /// Индекс группировки
    /// </summary>
    using CommunityIndex = CHARACTER_COMMUNITY_INDEX;

    /// <summary>
    /// Название группировки
    /// </summary>
    using CharacterCommunityId = CHARACTER_COMMUNITY_ID;

    /// <summary>
    /// Карта персональных отношений персонажа
    /// </summary>
    using PersonalRelationMap = xr_map<u16, CharacterGoodwill>;

    /// <summary>
    /// Карта отношений персонажа с группировками
    /// </summary>
    using CommunityRelationMap = xr_map<CommunityIndex, CharacterGoodwill>;

    /// <summary>
    /// Тип отношений между персонажами
    /// </summary>
    using RelationType = ALife::ERelationType;

    /// <summary>
    /// Название секции с настройками отношений
    /// </summary>
    constexpr auto GAME_RELATIONS_SECTION = "game_relations";

    /// <summary>
    /// Название секции с настройками очков за действия
    /// </summary>
    constexpr auto ACTIONS_POINTS_SECTION = "action_points";

    /// <summary>
    /// Название секции с настройками улучшенной системы отношений
    /// </summary>
    constexpr auto RELATION_SYSTEM_SECTION = "relation_system";

    /// <summary>
    /// Величина нейтральной доброжелательности
    /// </summary>
    constexpr s32 NEUTRAL_GOODWILL_AMOUNT = 0;

    /// <summary>
    /// Величина нейтральной репутации
    /// </summary>
    constexpr s32 NEUTRAL_REPUTATION_AMOUNT = 0;

    /// <summary>
    /// Неизвестное значение идентификатора
    /// </summary>
    constexpr u16 UNKNOWN_ID = std::numeric_limits<u16>::max();

    /// <summary>
    /// Неизвестное значение доброжелательности
    /// </summary>
    constexpr s32 UNKNOWN_GOODWILL = std::numeric_limits<CharacterGoodwill>::max();

    /// <summary>
    /// Неизвестное значение репутации
    /// </summary>
    constexpr s32 UNKNOWN_REPUTATION = std::numeric_limits<CharacterReputation>::max();

    /// <summary>
    /// Неизвестное значение ранга
    /// </summary>
    constexpr s32 UNKNOWN_RANK = std::numeric_limits<CharacterRank>::max();
}