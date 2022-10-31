#include <stdio.h>
#include <conio.h>
#include <math.h>
#include <thread>
#include "../libs/easy_x/graphics.h"
#define graph_width  640
#define graph_height 720
int origin_x = 0;
int origin_y = 0;
bool closed_graph = false;

void draw(){
    BeginBatchDraw();
    cleardevice();
    int lane_width = 36;
    int car_width = 20;
    int car_length = 48;
    int car_center_x = graph_width / 2;
    int car_center_y = graph_height / 2;
    line(graph_width / 2 - lane_width, 10, graph_width / 2 - lane_width, graph_height - 10);
    line(graph_width / 2 + lane_width, 10, graph_width / 2 + lane_width, graph_height - 10);
    setlinestyle(PS_DASH);
    line(graph_width / 2, 10, graph_width / 2, graph_height - 10);
    setlinestyle(PS_SOLID);
    fillrectangle(car_center_x - 0.5 * car_width - 0.5 * lane_width, car_center_y - 0.5 * car_length, 
                car_center_x + 0.5 * car_width - 0.5 * lane_width, car_center_y + 0.5 * car_length);
    EndBatchDraw();
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
                setaspectratio(x_aspectratio + 0.05, y_aspectratio + 0.05);
            }else{
                setaspectratio(x_aspectratio - 0.05, y_aspectratio - 0.05);
            }
            draw();
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
                    draw();
                }
                if(!msg.lbutton){
                    break;
                }
                Sleep(5);
            }
        }
        Sleep(5);
    }
}



void test_easy_x(){
    // 创建绘图窗口
    initgraph(graph_width, graph_height);	// 创建绘图窗口，大小为 640x480 像素
    std::thread msg_thread(msg_process);
    while(!closed_graph){
        draw();
        Sleep(10);
    }
}