#pragma once
#include "../data/trajectory_point.hpp"
#include "../data/lane.hpp"
class InputData{
    public:
        TrajectoryPoint start_point;
        Lane ref_lane;
};