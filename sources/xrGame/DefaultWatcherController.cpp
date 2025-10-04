#include "stdafx.h"
#include <ranges>
#include "Level.h"
#include "InventoryOwner.h"
#include "DefaultWatcherMonitor.h"
#include "DefaultWatcherEventHandler.h"
#include "DefaultWatcherController.h"

namespace Stalker::Mechanics::Watchers
{
    DefaultWatcherController::DefaultWatcherController(const WatcherSettings& settings) :
        m_settings(settings),
        m_actions(std::vector<WatchableAction>()),
        m_lock(std::mutex())
    {
        VERIFY(m_settings);

        m_monitor = new DefaultWatcherMonitor(m_settings);
        m_eventHandler = new DefaultWatcherEventHandler(m_settings);
    }

    DefaultWatcherController::~DefaultWatcherController()
    {
        delete m_eventHandler;
        delete m_monitor;
    }

    void DefaultWatcherController::Update()
    {
        if (!m_settings.IsEnabled())
        {
            return;
        }

        auto currentTime = Level().GetGameTime();

        for (auto i = 0; i < m_actions.size(); ++i)
        {
            auto& action = m_actions[i];

            if (m_monitor->IsActive())
            {
                auto watchers = m_monitor->CheckWatchers(action);
                std::for_each(watchers.begin(), watchers.end(), [&](const std::pair<u16, u64>& watcher)
                {
                    auto status = watcher.second > 0
                        ? WatcherStatus::Busted
                        : WatcherStatus::Watched;

                    auto event = buildEvent(status, watcher.first, action);
                    m_eventHandler->HandleEvent(event);
                });
            }
            
            auto keys = std::ranges::views::keys(action.Watchers);
            auto watcherIds = std::vector<u16>{ keys.begin(), keys.end() };

            std::for_each(watcherIds.begin(), watcherIds.end(), [&](const u16& id)
            {
                if (action.Watchers.at(id) > 0 && action.Watchers.at(id) < currentTime)
                {
                    auto event = buildEvent(WatcherStatus::Revealed, id, action);
                    m_eventHandler->HandleEvent(event);
                    action.Watchers.erase(id);
                }
            });

            auto actorNotBusted = std::all_of(action.Watchers.begin(), action.Watchers.end(), [](const std::pair<u16, u64>& watcher)
            {
                return watcher.second == 0;
            });

            if (!m_monitor->IsActive() && (action.Watchers.size() == 0 || actorNotBusted))
            {
                removeAction(action);
            }
        }
    }

    void DefaultWatcherController::Load(IReader& reader)
    {
        R_ASSERT2(reader.find_chunk(OBJECT_CHUNK_DATA), "Can't find chunk OBJECT_CHUNK_DATA!");
        auto pos = reader.tell();
        reader.seek(pos);

        m_actions.clear();

        auto lastVictim = reader.r_u16();
        auto actionCount = reader.r_u16();

        for (auto i = 0; i < actionCount; ++i)
        {
            auto action = WatchableAction();

            action.VictimId = reader.r_u16();
            action.Action = (ActionType)reader.r_u8();
            action.Time = reader.r_u64();
            action.GoodwillDelta = reader.r_s16();
            action.ReputationDelta = reader.r_s16();
            action.RankDelta = reader.r_s16();
            auto watcherCount = reader.r_u8();

            for (auto i = 0; i < watcherCount; ++i)
            {
                auto id = reader.r_u16();
                auto time = reader.r_u64();
                action.Watchers.insert({id, time});
            }
            addAction(action);
            Msg("* [Watcher system] Loaded action '%i' with %i watchers", action.VictimId, action.Watchers.size());
        }

        if (lastVictim > 0)
        {
            m_monitor->Update(lastVictim);
        }
    }

    void DefaultWatcherController::Save(IWriter& writer)
    {
        writer.open_chunk(OBJECT_CHUNK_DATA);
        auto pos = writer.tell();
        writer.seek(pos);

        writer.w_u16(m_monitor->LastVictimId());
        writer.w_u16(m_actions.size());

        for (const auto& action : m_actions)
        {
            writer.w_u16(action.VictimId);
            writer.w_u8((u8)action.Action);
            writer.w_u64(action.Time);
            writer.w_s16(action.GoodwillDelta);
            writer.w_s16(action.ReputationDelta);
            writer.w_s16(action.RankDelta);
            writer.w_u8((u8)action.Watchers.size());

            for (const auto& watcher : action.Watchers)
            {
                writer.w_u16(watcher.first);
                writer.w_u64(watcher.second);
            }
            Msg("* [Watcher system] Saved action '%i' with %i watchers", action.VictimId, action.Watchers.size());
        }
        writer.close_chunk();
    }

    void DefaultWatcherController::OnAction(const Action& action)
    {
        if (&action == nullptr) return;

        if (action.Type == ActionType::Kill)
        {
            onCharacterKilled(action.VictimId);
        }
        
        auto wa = WatchableAction();
        wa.VictimId = action.VictimId;
        wa.Time = action.Time;
        wa.Action = action.Type;
        wa.GoodwillDelta = action.GoodwillDelta;
        wa.ReputationDelta = action.ReputationDelta;
        wa.RankDelta = action.RankDelta;        
        wa.Watchers = std::map<u16, u64>();
        
        addAction(wa);
        m_monitor->Update(action.VictimId);
    }

    void DefaultWatcherController::onCharacterKilled(const u16& characterId)
    {
        for (auto& action : m_actions)
        {
            if (action.Watchers.contains(characterId))
            {
                action.Watchers.erase(characterId);
            }
        }
    }

    CInventoryOwner* DefaultWatcherController::getCharacterById(u16 characterId) const
    {
        if (CObject* obj = Level().Objects.net_Find(characterId))
        {
            if (CInventoryOwner* character = smart_cast<CInventoryOwner*>(obj))
            {
                return character;
            }
        }
        return nullptr;
    }

    void DefaultWatcherController::addAction(const WatchableAction& action)
    {
        m_lock.lock();

        auto index = findStoryIndex(action.VictimId);
        if (index >= 0 && index < m_actions.size())
        {
            auto watchers = m_actions[index].Watchers;
            
            m_actions[index] = action;
            m_actions[index].Watchers = watchers;
        }
        else m_actions.emplace_back(action);

        m_lock.unlock();
    }

    void DefaultWatcherController::removeAction(const WatchableAction& action)
    {
        m_lock.lock();

        auto index = findStoryIndex(action.VictimId);
        if (index >= 0 && index < m_actions.size())
        {
            std::swap(m_actions[index], m_actions.back());
            m_actions.pop_back();
        }

        m_lock.unlock();
    }

    int32_t DefaultWatcherController::findStoryIndex(const u16& characterId) const
    {
        auto it = std::find_if(m_actions.begin(), m_actions.end(), [=](const WatchableAction& action)
        {
            return action.VictimId == characterId;
        });        
        return it != m_actions.end() ? int32_t(it - m_actions.begin()) : -1;
    }

    WatcherEvent DefaultWatcherController::buildEvent(const WatcherStatus& status, const u16& watcherId, const WatchableAction& action) const
    {
        auto event = WatcherEvent();

        event.Status = (u32)status;
        event.VictimId = action.VictimId;
        event.WatcherId = watcherId;
        event.ActionType = (u32)action.Action;
        event.GoodwillDelta = action.GoodwillDelta;
        event.ReputationDelta = action.ReputationDelta;
        event.RankDelta = action.RankDelta;        

        return event;
    }
}