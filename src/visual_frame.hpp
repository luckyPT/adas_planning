#pragma once
#include <iostream>
#include "data/scene.hpp"
#define graph_width  640
#define graph_height 720
#define hz 100
int origin_x = 0;
int origin_y = 0;
bool closed_graph = false;
Scene scene;

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
                setaspectratio(x_aspectratio + 0.02, y_aspectratio - 0.02);
            }else{
                // 防止坐标轴反向
                if(x_aspectratio > 0.05 && y_aspectratio < -0.05){
                    setaspectratio(x_aspectratio - 0.02, y_aspectratio + 0.02);
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