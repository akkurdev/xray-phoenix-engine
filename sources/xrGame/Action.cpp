#include "stdafx.h"
#include "Action.h"

namespace Stalker::Mechanics::Relations
{
#pragma optimize("s", on)

    void Action::script_register(lua_State* L)
    {
        module(L)[class_<Action>("Action")

            .def_readonly("victim_id", &Action::VictimId)
            .def_readonly("time", &Action::Time)
            .def_readonly("actor_position", &Action::ActorPosition)
            .def_readonly("victim_position", &Action::VictimPosition)
            .def_readonly("goodwill_delta", &Action::GoodwillDelta)
            .def_readonly("reputation_delta", &Action::ReputationDelta)
            .def_readonly("rank_delta", &Action::RankDelta)
            .def_readonly("damaged_bone", &Action::DamagedBone)
            .def_readonly("has_one_shot", &Action::HasOneShot)
            .def("type", &Action::typeToInt)
        ];
    }

    u8 Action::typeToInt()
    {
        return u8(Type);
    }
}