#pragma once
#include <vector>
#include <math.h>
#include "./point.hpp"
#include "../math/math_util.hpp"
#include <iostream>
class RefLineSimple{
    public:
        std::vector<Point> points;
        std::vector<float> points_s;
    
    bool XY2SL(float x, float y, float& s, float& l){
        float min_dis = 1e8;
        int min_index = 0;
        for (int i = 0; i < points.size(); i++){
            float cur_dis = std::hypot(points[i].x - x, points[i].y - y);
            if(min_dis > cur_dis){
                min_dis = cur_dis;
                min_index = i;
            }
        }

        if(min_index == 0 || min_index == points.size()-1){
            return false;
        }else{
            float angle1 = math_util::calculate_angle(x-points[min_index].x, y-points[min_index].y, 
                    points[min_index-1].x - points[min_index].x, points[min_index-1].y - points[min_index].y);
            float angle2 = math_util::calculate_angle(x-points[min_index].x, y-points[min_index].y, 
                    points[min_index+1].x - points[min_index].x, points[min_index+1].y - points[min_index].y);

            if(angle1 >= M_PI_2 && angle2 >= M_PI_2){
                // 此处逻辑不严谨
                s = points_s[min_index];
                l = std::hypot(x-points[min_index].x, y-points[min_index].y);
                l *= -1 * math_util::calculate_relative_pos(x-points[min_index].x, y-points[min_index].y, 
                    points[min_index + 1].x - points[min_index].x, points[min_index + 1].y - points[min_index].y);
            }else{
                float dis = std::hypot(x-points[min_index].x, y-points[min_index].y);
                if(angle1 <= angle2){
                    l = dis * std::sin(angle1);
                    l *= -1 * math_util::calculate_relative_pos(x-points[min_index].x, y-points[min_index].y, 
                        points[min_index].x - points[min_index-1].x, points[min_index].y - points[min_index-1].y);
                    s = points_s[min_index] - dis * std::cos(angle1);
                }else{
                    l = dis * std::sin(angle2);
                    l *= -1 * math_util::calculate_relative_pos(x-points[min_index].x, y-points[min_index].y, 
                        points[min_index+1].x - points[min_index].x, points[min_index+1].y - points[min_index].y);
                    s = points_s[min_index] + dis * std::cos(angle2);
                }
            }
            return true;
        }
    }

    bool SL2XY(float s, float l, float& x, float& y){
        int i = 0;
        float ratio = 0.0;
        for(; i < points.size()-1; i++){
            if(std::abs(s - points_s[i]) < 1e-5){ //这里的1e-5不能设置的太大
                return false;
            }
            if(points_s[i] < s && points_s[i+1] > s){
                ratio = (s - points_s[i]) / (points_s[i + 1] - points_s[i]);
                break;
            }
        }
        if(i == points.size()-1){
            return false;
        }
         ;
        // 垂足
        float point_sl_x = points[i].x + ratio * (points[i + 1].x - points[i].x);
        float point_sl_y = points[i].y + ratio * (points[i + 1].y - points[i].y);
        if(std::abs(l) < 1e-3){
            x = point_sl_x;
            y = point_sl_y;
            return true;
        }

        float l_ratio = std::abs(l) / (points_s[i + 1] - points_s[i]);
        float point_before_roat_x = point_sl_x + l_ratio * (points[i + 1].x - points[i].x);
        float point_before_roat_y = point_sl_y + l_ratio * (points[i + 1].y - points[i].y);
        math_util::rotate_point(point_sl_x, point_sl_y, l > 0 ? M_PI + M_PI_2 : M_PI_2, 
                                point_before_roat_x, point_before_roat_y, x, y);
        return true;
    }

    void test(){
        RefLineSimple ref_line;
        ref_line.points.emplace_back(Point({0,0}));
        ref_line.points_s.emplace_back(0);
        for(float i = 0.1; i < 2 * M_PI; i += 0.1){
            Point tmp = Point(i, std::sin(i));
            float cur_s = ref_line.points_s.back();
            Point last_point = ref_line.points.back();

            ref_line.points_s.emplace_back(cur_s + std::hypot(tmp.x - last_point.x, tmp.y - last_point.y));
            ref_line.points.emplace_back(tmp);
        }

        for(float x = 0.5; x < 6; x += 0.1){
            for(float y = -100; y < 100; y += 1){
                float tmp_s, tmp_l;
                if(ref_line.XY2SL(x, y, tmp_s, tmp_l)){
                    float tmp_x, tmp_y;
                    if(ref_line.SL2XY(tmp_s, tmp_l, tmp_x, tmp_y)){
                        if(std::abs(x - tmp_x) > 1e-3 || std::abs(y - tmp_y) > 1e-3){
                            std::cout << "0 - ERROR:" << std::abs(x - tmp_x) << std::endl;
                        }else{
                            // std::cout << "RIGHT" << std::endl;
                        }
                    }else{
                        // std::cout << "1 - ERROR" << std::endl;
                    }
                }else{
                    // std::cout << "2 - ERROR" << std::endl;
                }
                
            }
        }
        std::cout << "END---" << std::endl;
    }
};