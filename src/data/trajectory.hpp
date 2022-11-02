#include <vector>
#include "trajectory_point.hpp"
#include "../../libs/easy_x/graphics.h"
class Trajectory{
    public:
        std::vector<TrajectoryPoint> points;
    void clear(){
        points.clear();
    }
    void draw(){
        if(points.size() == 0){
            return;
        }
        POINT* pts = new POINT[points.size()];
        for(int i = 0; i < points.size(); i++){
            *(pts + i) = {static_cast<long>(points[i].x), static_cast<long>(points[i].y)};
        }
        setlinestyle(PS_SOLID, 20);
        setlinecolor(GREEN);
        polyline(pts, points.size());
        delete[] pts;
    }
};