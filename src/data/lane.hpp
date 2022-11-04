#pragma once
#include <vector>
#include "math.h"
#include "point.hpp"
#include "../../libs/easy_x/graphics.h"

class Lane{
    public:
        std::vector<Point> points;
        std::vector<float> width;
        float a, b, c, d;
        float start_x;
        float end_x;
    Lane(){}
    Lane(std::vector<Point>& pts, std::vector<float>& width){
        points = pts;
        this->width = width;
    }
    Lane(float a, float b, float c, float d, float start_x, float end_x){
        this->a = a;
        this->b = b;
        this->c = c;
        this->d = d;
        this->start_x = start_x;
        this->end_x = end_x;
    }
    float calculate_y_by_x(float x){
        return a * std::pow(x,3) + b * std::pow(x,2) + c * std::pow(x,1) + d;
    }

    void generate_center_points(float interval = 1){
        if(points.size() > 0){
            throw "has center points, do not generate";
        }
        if(interval == 1 && end_x - start_x < 10){
            interval = (end_x - start_x) / 10;
        }
        
        for(float x = start_x; x < end_x; x += interval){
            points.emplace_back(std::move(Point(x, calculate_y_by_x(x))));
        }
    }

    void draw_left_boundary(){
        if(points.size() == 0){
            generate_center_points();
        }
        POINT* pts = new POINT[points.size()];
        for(int i = 0; i < points.size(); i++){
            *(pts + i) = {static_cast<long>(points[i].x - 0.5 * width[0]), static_cast<long>(points[i].y)};
        }
        setlinestyle(PS_DASH, 1);
        setlinecolor(WHITE);
        polyline(pts, points.size());
        delete[] pts;
    }
    void draw_right_boundary(){
        if(points.size() == 0){
            generate_center_points();
        }
        POINT* pts = new POINT[points.size()];
        for(int i = 0; i < points.size(); i++){
            *(pts + i) = {static_cast<long>(points[i].x + 0.5 * width[0]), static_cast<long>(points[i].y)};
        }
        setlinestyle(PS_DASH, 1);
        setlinecolor(WHITE);
        polyline(pts, points.size());
        delete[] pts;
    }
    void draw_center_line(){
        if(points.size() == 0){
            generate_center_points();
        }
        POINT* pts = new POINT[points.size()];
        for(int i = 0; i < points.size(); i++){
            *(pts + i) = {static_cast<long>(points[i].x), static_cast<long>(points[i].y)};
        }
        setlinestyle(PS_SOLID, width[0]);
        setlinecolor(DARKGRAY);
        polyline(pts, points.size());
        setlinestyle(PS_DASH, 1);
        setlinecolor(GREEN);
        polyline(pts, points.size());
        delete[] pts;
    }
};