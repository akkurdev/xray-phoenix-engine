////////////////////////////////////////////////////////////////////////////
//	Module 		: script_sound_script.cpp
//	Created 	: 06.02.2004
//  Modified 	: 06.02.2004
//	Author		: Dmitriy Iassenev
//	Description : XRay Script sound class script export
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "script_sound.h"
#include "script_game_object.h"

using namespace luabind;

#pragma optimize("s", on)
void CScriptSound::script_register(lua_State* L)
{
    module(L)[class_<SoundParams>("sound_params")
                  .def_readwrite("position", &SoundParams::Position)
                  .def_readwrite("volume", &SoundParams::Volume)
                  .def_readwrite("frequency", &SoundParams::Frequency)
                  .def_readwrite("min_distance", &SoundParams::DistanceMin)
                  .def_readwrite("max_distance", &SoundParams::DistanceMax),

              class_<CScriptSound>("sound_object")
                  .enum_("sound_play_type")[value("looped", sm_Looped), value("s2d", sm_2D), value("s3d", 0)]
                  .enum_("sound_type")[value("effect", (int)SoundType::Effect), value("music", (int)SoundType::Music)]

                  .property("frequency", &CScriptSound::GetFrequency, &CScriptSound::SetFrequency)
                  .property("min_distance", &CScriptSound::GetMinDistance, &CScriptSound::SetMinDistance)
                  .property("max_distance", &CScriptSound::GetMaxDistance, &CScriptSound::SetMaxDistance)
                  .property("volume", &CScriptSound::GetVolume, &CScriptSound::SetVolume)

                  .def(constructor<LPCSTR>())
                  .def(constructor<LPCSTR, ESoundTypes>())
                  .def(constructor<LPCSTR, ESoundTypes, SoundType>())
                  .def("get_position", &CScriptSound::GetPosition)
                  .def("set_position", &CScriptSound::SetPosition)
                  .def("play", (void(CScriptSound::*)(CScriptGameObject*))(&CScriptSound::Play))
                  .def("play", (void(CScriptSound::*)(CScriptGameObject*, float))(&CScriptSound::Play))
                  .def("play", (void(CScriptSound::*)(CScriptGameObject*, float, int))(&CScriptSound::Play))
                  .def("play_at_pos", (void(CScriptSound::*)(CScriptGameObject*, const Fvector&))(&CScriptSound::PlayAtPos))
                  .def("play_at_pos", (void(CScriptSound::*)(CScriptGameObject*, const Fvector&, float))(&CScriptSound::PlayAtPos))
                  .def("play_at_pos", (void(CScriptSound::*)(CScriptGameObject*, const Fvector&, float, int))(&CScriptSound::PlayAtPos))
                  .def("play_no_feedback", &CScriptSound::PlayNoFeedback)
                  .def("stop", &CScriptSound::Stop)
                  .def("stop_deffered", &CScriptSound::StopDeffered)
                  .def("playing", &CScriptSound::IsPlaying)
                  .def("length", &CScriptSound::Length)
                  .def("set_start_time", &CScriptSound::SetTime)
    ];
}
