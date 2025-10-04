#include "stdafx.h"
#include <algorithm>
#include "WatchersSettings.h"

namespace Stalker::Mechanics::Watchers
{
    WatcherSettings::WatcherSettings()
    {
        auto section = "watchers_system";
        
        m_isEnabled                 = pSettings->r_bool(section, "is_enabled");
        m_watchedHandler            = pSettings->r_string(section, "watched_handler");
        m_bustedHandler             = pSettings->r_string(section, "busted_handler");
        m_revealedHandler           = pSettings->r_string(section, "revealed_handler");
        m_revealPeriodMin           = pSettings->r_float(section, "reveal_period_min");
        m_revealPeriodMax           = pSettings->r_float(section, "reveal_period_max");
        m_searchRadius              = pSettings->r_float(section, "search_radius");
        m_searchPeriod              = pSettings->r_float(section, "search_period");
        m_isReputationFactorEnabled = pSettings->r_bool(section, "is_reputation_factor_enabled");
        m_isRankFactorEnabled       = pSettings->r_bool(section, "is_rank_factor_enabled");
    }

    bool WatcherSettings::IsEnabled() const
    {
        return m_isEnabled;
    }

    std::string_view WatcherSettings::WatchedHandler() const
    {
        return m_watchedHandler;
    }

    std::string_view WatcherSettings::BustedHandler() const
    {
        return m_bustedHandler;
    }

    std::string_view WatcherSettings::RevealedHandler() const
    {
        return m_revealedHandler;
    }

    float WatcherSettings::RevealPeriodMin() const
    {
        return std::clamp(m_revealPeriodMin, 0.f, 100.f);
    }

    float WatcherSettings::RevealPeriodMax() const
    {
        return std::clamp(m_revealPeriodMax, 0.f, 100.f);
    }

    float WatcherSettings::SearchRadius() const
    {
        return m_searchRadius;
    }

    float WatcherSettings::SearchPeriod() const
    {
        return m_searchPeriod;
    }

    bool WatcherSettings::IsReputationFactorEnabled() const
    {
        return m_isReputationFactorEnabled;
    }

    bool WatcherSettings::IsRankFactorEnabled() const
    {
        return m_isRankFactorEnabled;
    }
}