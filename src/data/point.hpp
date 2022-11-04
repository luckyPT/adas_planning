#pragma once
#include "../../libs/easy_x/graphics.h"

class Point{
    public:
        float x, y;
    Point(){
        
    }
    Point(float x, float y){
        this->x = x;
        this->y = y;
    }

    public:
        void draw(){
            setlinestyle(PS_DASH, 1);
            fillcircle(x, y, 25);
        }
};