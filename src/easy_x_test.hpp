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
#define graph_width  640
#define graph_height 720
#define hz 100
int origin_x = 0;
int origin_y = 0;
bool closed_graph = false;
Scene scene;

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


void msg_process(){
    // 按任意键退出
	ExMessage msg;
    while(true){
        bool has_msg = peekmessage(&msg, EX_KEY | EX_MOUSE, true);
        flushmessage(EX_KEY | EX_MOUSE);
        if(!has_msg){
            continue;
        }
        // 按任意键退出
        if(msg.message == WM_KEYDOWN){
            closegraph();
            closed_graph = true;
            break;
        }
        // 滚动缩放
        if(msg.message == WM_MOUSEWHEEL){
            std::cout << "mousewheel:" << msg.wheel << std::endl;
            float x_aspectratio = 1;
            float y_aspectratio = 1;
            getaspectratio(&x_aspectratio, &y_aspectratio);
            if(msg.wheel > 0){
                setaspectratio(x_aspectratio + 0.005, y_aspectratio - 0.005);
            }else{
                // 防止坐标轴反向
                if(x_aspectratio > 0.01 && y_aspectratio < -0.01){
                    setaspectratio(x_aspectratio - 0.005, y_aspectratio + 0.005);
                }
            }
        }
        // 按住左键 移动鼠标，平移
        if(msg.lbutton && msg.message == WM_MOUSEMOVE){
            double cur_x = msg.x;
            double cur_y = msg.y;
            while(true){
                has_msg = peekmessage(&msg, EX_MOUSE, true);
                flushmessage(EX_MOUSE);
                if(has_msg && msg.lbutton && msg.message == WM_MOUSEMOVE){
                    setorigin(origin_x + (msg.x - cur_x), origin_y + (msg.y - cur_y));
                    origin_x += (msg.x - cur_x);
                    origin_y += (msg.y - cur_y);
                    cur_x = msg.x;
                    cur_y = msg.y;
                }
                if(!msg.lbutton){
                    break;
                }
                Sleep(2);
            }
        }
        Sleep(2);
    }
}



void test_easy_x(){
    // 创建绘图窗口
    init_scene();
    initgraph(graph_width, graph_height);	// 创建绘图窗口，大小为 640x480 像素
    setaspectratio(0.5, -0.5);
    std::thread msg_thread(msg_process);
    while(!closed_graph){
        scene.update_and_draw();
        Sleep(1000 / hz);
    }
}