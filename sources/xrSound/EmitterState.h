#pragma once

enum class EmitterState
{
    Stopped = 0,
    StartingDelayed,
    StartingLoopedDelayed,
    Starting,
    StartingLooped,
    Playing,
    PlayingLooped,
    Simulating,
    SimulatingLooped,
};
