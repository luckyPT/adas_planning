#pragma once
#include <stdio.h>
#include <conio.h>
#include <math.h>
#include <thread>
#include <vector>
#include "../libs/easy_x/graphics.h"
#include "../src/data/car.hpp"
#include "../src/data/lane.hpp"
#include "../src/data/scene.hpp"
#include "../src/visual_frame.hpp"

void init_scene(){
    int car_center_x = graph_width / 2;
    int car_center_y = graph_height / 2;
    Car ego_car(20, 48, 20, 0, car_center_x, -car_center_y - 600);
    Car car1(20, 48, 80, 0, car_center_x, -car_center_y - 27);
    std::vector<Point> lane1_pts = {{car_center_x, car_center_y + 2000}, {car_center_x, car_center_y - 2000}};
    std::vector<Point> lane2_pts = {{car_center_x + 35, car_center_y + 2000}, {car_center_x + 35, car_center_y - 2000}};
    std::vector<float> width = {35};
    Lane lane1(lane1_pts, width);
    Lane lane2(lane2_pts, width);
    Road road;
    road.lanes = {lane1, lane2};
    scene.cars = {ego_car};
    scene.roads = {road};
    scene.update_freq = hz;
}



void test_easy_x(){
    // 创建绘图窗口
    init_scene();
    initgraph(graph_width, graph_height);	// 创建绘图窗口，大小为 640x480 像素
    setaspectratio(0.7, -0.7);
    std::thread msg_thread(msg_process);
    while(!closed_graph){
        scene.update_and_draw();
        Sleep(1000 / hz);
    }
}