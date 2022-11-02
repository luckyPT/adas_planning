#pragma once
#include <vector>
#include "lane.hpp"
class Road
{
    public:
        std::vector<Lane> lanes;
    Road(){

    }
    Road(std::vector<Lane>& lanes){
        this->lanes = lanes;
    }

    void draw_center_line(){
        for(Lane& lane : lanes){
            lane.draw_center_line();
            lane.draw_left_boundary();
            lane.draw_right_boundary();
        }
    }
};
