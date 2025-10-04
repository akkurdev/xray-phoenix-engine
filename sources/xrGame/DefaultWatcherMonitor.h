#pragma once
#include <vector>
#include "script_entity.h"
#include "WatchersSettings.h"
#include "IWatcherMonitor.h"

class CInventoryOwner;

namespace Stalker::Mechanics::Watchers
{
    /// <summary>
    /// Монитор свидетелей по умолчанию
    /// </summary>
    class DefaultWatcherMonitor final : public IWatcherMonitor
    {
    public:
        DefaultWatcherMonitor() = default;
        DefaultWatcherMonitor(const WatcherSettings& settings);
        virtual ~DefaultWatcherMonitor() = default;

        /// <summary>
        /// Активен ли монитор в текущий момент
        /// </summary>
        virtual bool IsActive() const;

        /// <summary>
        /// Идентификатор последней жертвы, инициировавшей мониторинг
        /// </summary>
        u16 LastVictimId() const;

        /// <summary>
        /// Обновление монитора при новых действиях в адрес NPC
        /// </summary>
        /// <param name="characterId">Идентификатор NPC</param>
        virtual void Update(const u16& characterId);

        /// <summary>
        /// Проверка свидетелей для указанного действия
        /// </summary>
        /// <param name="action">Наблюдаемое действие</param>
        /// <returns>Набор пар, описывающих измененные состояния свидетелей</returns>
        virtual std::vector<std::pair<u16, u64>> CheckWatchers(WatchableAction& action);

    private:
        CInventoryOwner* getCharacterById(u16 characterId) const;
        RelationType getRelationByCommunity(CInventoryOwner* npc1, CInventoryOwner* npc2) const;
        std::vector<CScriptEntity*> findNearestEntities(CInventoryOwner* character, float radius) const;

        float getRevealTime(CInventoryOwner* attacker, CInventoryOwner* watcher) const;
        float calculateRevealTimeByRelation(RelationType relationType) const;
        float calculateRevealTimeByReputation(u16 watcherReputation) const;
        float calculateRevealTimeByRank(u16 watcherRank) const;

    private:
        WatcherSettings m_settings;
        u16 m_lastVictimId;
        u64 m_time;
    };
}