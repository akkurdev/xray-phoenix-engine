#pragma once
#include <type_traits>
#include <_types.h>

namespace Stalker::Mechanics::Relations
{
    /// <summary>
    /// Концепт типа метрики отношений
    /// </summary>
    template<typename T>
    concept MetricType = std::is_arithmetic_v<T>;

    /// <summary>
    /// Концепт типа персонажа для системы отношений.
    /// Тип должен иметь реализации указанных методов для успешной компиляции проекта.
    /// </summary>
    template<typename T>
    concept CharacterType = requires(T character)
    {
        character.object_id();
        character.Community();
        character.Reputation();
        character.Rank();
    };

    /// <summary>
    /// Концепт типа идентификатора
    /// </summary>
    template<typename T>
    concept IdType = std::is_same_v<T, u16> || std::is_same_v<T, u32> || std::is_same_v<T, s32>;
}