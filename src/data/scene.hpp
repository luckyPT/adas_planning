#pragma once
#include <vector>
#include "car.hpp"
#include "lane.hpp"
#include "road.hpp"

class Scene{
    public:
        std::vector<Car> cars;
        std::vector<Road> roads;
        int update_freq; // 更新频率
    
    void update_and_draw() {
        for(Car& car : cars){
            car.update_status(1.0 /  update_freq);
            // car.generate_ca_trajectory();
        }
        draw();
    };
    void draw_coordinate(){
        line(0, 0, 100, 0); // x轴
        RECT x = {100, 0, 120, -20};
        drawtext(_T("x"), &x, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        line(0, 0, 0, 100); // y轴
        RECT y = {0, 100, 20, 120};
	    drawtext(_T("y"), &y, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        
    };
    void draw() {
        BeginBatchDraw();
        cleardevice();
        // draw_coordinate();
        for(Road& road : roads){
            road.draw_center_line();
        }
        for(Car& car : cars){
            car.draw();
        }
        EndBatchDraw();
    }        
};