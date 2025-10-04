#pragma once
#include <map>
#include "RelationDefinitions.h"
#include "RelationSettings.h"
#include "ActionType.h"

namespace Stalker::Mechanics::Relations
{
    /// <summary>
    /// Таблица метрик отношений
    /// </summary>
    class CharacterMetricTable final
    {
    public:
        /// <summary>
        /// Предоставляет новый экземпляр CharacterMetricTable
        /// </summary>
        /// <param name="settings">Настройки системы отношений</param>
        CharacterMetricTable(RelationSettings* settings);
        ~CharacterMetricTable() = default;

        /// <summary>
        /// Поиск значения доброжелательности
        /// </summary>
        /// <param name="action">Тип действия в адрес персонажа</param>
        /// <param name="relation">Отношение обороняющегося к нападающему</param>
        /// <returns>Значения, на которое изменится доброжелательность к нападающему</returns>
        CharacterGoodwill FindGoodwill(ActionType action, RelationType relation);

        /// <summary>
        /// Поиск значения репутации
        /// </summary>
        /// <param name="action">Тип действия в адрес персонажа</param>
        /// <param name="relation">Отношение обороняющегося к нападающему</param>
        /// <returns>Значение, на которое изменится репутация нападающего</returns>
        CharacterReputation FindReputation(ActionType action, RelationType relation);

    private:
        using GoodwillTable = std::map<RelationType, CharacterGoodwill>;
        using ReputationTable = std::map<RelationType, CharacterReputation>;

        template<typename MetricT>
            requires std::is_same_v<MetricT, CharacterGoodwill> || std::is_same_v<MetricT, CharacterReputation>
        MetricT TryGetValue(const std::map<RelationType, MetricT>& table, RelationType relation);

    private:
        GoodwillTable m_attackGoodwillTable;
        GoodwillTable m_killGoodwillTable;
        GoodwillTable m_helpGoodwillTable;

        ReputationTable m_attackReputationTable;
        ReputationTable m_killReputationTable;
        ReputationTable m_helpReputationTable;
    };

    template<typename MetricT>
        requires std::is_same_v<MetricT, CharacterGoodwill> || std::is_same_v<MetricT, CharacterReputation>
    inline MetricT CharacterMetricTable::TryGetValue(const std::map<RelationType, MetricT>& table, RelationType relation)
    {
        return table.contains(relation)
            ? table.at(relation)
            : MetricT();
    }
}