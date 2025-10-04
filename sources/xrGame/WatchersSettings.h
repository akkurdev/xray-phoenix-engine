#pragma once
#include <string>
#include "RelationDefinitions.h"

using namespace Stalker::Mechanics::Relations;

namespace Stalker::Mechanics::Watchers
{
    /// <summary>
    /// Настройки системы свидетелей
    /// </summary>
    class WatcherSettings final
    {
    public:
        WatcherSettings();
        ~WatcherSettings() = default;

        /// <summary>
        /// Включена ли система свидетелей
        /// </summary>
        bool IsEnabled() const;

        /// <summary>
        /// Обработчик ситуации, 
        /// когда NPC увидел причинение ранения/смерти жертве или заметил труп
        /// </summary>
        std::string_view WatchedHandler() const;

        /// <summary>
        /// Обработчик ситуации, когда актор был пойман с поличным
        /// </summary>
        std::string_view BustedHandler() const;

        /// <summary>
        /// Обработчик ситуации, когда NPC пришло время рассказать об увиденном
        /// </summary>
        std::string_view RevealedHandler() const;
        
        /// <summary>
        /// Минимальное время, через которое NPC расскажет всем об увиденном 
        /// (в игровых часах)
        /// </summary>
        float RevealPeriodMin() const;

        /// <summary>
        /// Максимальное время, через которое NPC расскажет всем об увиденном 
        /// (в игровых часах, но не более 100 часов)
        /// </summary>
        float RevealPeriodMax() const;

        /// <summary>
        /// Радиус пойска свидетелей (в метрах, не должен быть больше, чем радиус a-life)
        /// </summary>
        float SearchRadius() const;

        /// <summary>
        /// Время поиска свидетелей (в игровых минутах, не более 10 минут)
        /// </summary>
        float SearchPeriod() const;

        /// <summary>
        /// Учитывать ли репутацию свидетеля при расчете reveal time
        /// </summary>
        bool IsReputationFactorEnabled() const;

        /// <summary>
        /// Учитывать ли ранг свидетеля при расчете reveal time
        /// </summary>
        bool IsRankFactorEnabled() const;

    private:
        bool m_isEnabled;
        std::string m_watchedHandler;
        std::string m_bustedHandler;
        std::string m_revealedHandler;
        float m_revealPeriodMin;
        float m_revealPeriodMax;
        float m_searchRadius;
        float m_searchPeriod;
        bool m_isReputationFactorEnabled;
        bool m_isRankFactorEnabled;
    };
}