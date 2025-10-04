#include "stdafx.h"
#include "WatcherEvent.h"

namespace Stalker::Mechanics::Watchers
{
#pragma optimize("s", on)

    void WatcherEvent::script_register(lua_State* L)
    {
        module(L)[class_<WatcherEvent>("WatcherEvent")
            .def_readonly("status", &WatcherEvent::Status)
            .def_readonly("watcher_id", &WatcherEvent::WatcherId)
            .def_readonly("action_type", &WatcherEvent::ActionType)
            .def_readonly("victim_id", &WatcherEvent::VictimId)
            .def_readonly("goodwill_delta", &WatcherEvent::GoodwillDelta)
            .def_readonly("reputation_delta", &WatcherEvent::ReputationDelta)
            .def_readonly("rank_delta", &WatcherEvent::RankDelta)
        ];
    }
}