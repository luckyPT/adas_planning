#pragma once
#include <thread>
#include "../libs/easy_x/graphics.h"
#include "../src/data/car.hpp"
#include "../src/data/lane.hpp"
#include "../src/data/scene.hpp"
#include "../src/visual_frame.hpp"
#include "../src/em_planner/input_data.hpp"
#include "../src/em_planner/path_dp_plan.hpp"
void init_scene(){
    int car_center_x = graph_width / 2;
    int car_center_y = graph_height / 2;
    Car ego_car(200, 480, 0, 0, car_center_x, -car_center_y - 600);
    std::vector<Point> lane1_pts = {{car_center_x, car_center_y + 100000}, {car_center_x, car_center_y - 2000}};
    std::vector<Point> lane2_pts = {{car_center_x + 350, car_center_y - 2000},
                                    {car_center_x + 350, car_center_y - 1000}, 
                                    {car_center_x + 350, car_center_y + 0}, 
                                    {car_center_x + 350, car_center_y + 5000},
                                    {car_center_x + 350, car_center_y + 40000}, 
                                    {car_center_x + 350, car_center_y + 80000}, 
                                    {car_center_x + 350, car_center_y + 100000}};
    std::vector<float> width = {350};
    Lane lane1(lane1_pts, width);
    Lane lane2(lane2_pts, width);
    ego_car.ref_lane = lane2;
    Road road;
    road.lanes = {lane1, lane2};
    scene.cars = {ego_car};
    scene.roads = {road};
    scene.update_freq = hz;
}

void change_lane_visual(){
    init_scene();
    initgraph(graph_width, graph_height);	// 创建绘图窗口，大小为 640x480 像素
    setaspectratio(0.3, -0.3);
    std::thread msg_thread(msg_process);
    while(!closed_graph){
        scene.update_and_draw();
        Sleep(1000 / hz);
    }
}