#pragma once
#include "../../libs/easy_x/graphics.h"
#include "../em_planner/path_dp_plan.hpp"
#include "../em_planner/path_qp_plan.hpp"
#include "point.hpp"
#include "trajectory.hpp"
#include <time.h>
#define CAR_MAX_SPEED 100
class Car{
    public:
        // unit cm
        int width;
        int length;
        float a;
        float v;
        float pos_x;
        float pos_y;
        float front_wheel_angle;
        Lane ref_lane;
        Trajectory trajectory;
        DpPlan dp_planner;
        QpPlan qp_planner;
    Car(){
        this->width = 20;
        this->length = 48;
        this->a = 0;
        this->v = 0;
        this->pos_x = 0;
        this->pos_y = 0;
    }
    Car(int width, int length, double a , double v, double pos_x, double pos_y){
        this->width = width;
        this->length =length;
        this->a = a;
        this->v = v;
        this->pos_x = pos_x;
        this->pos_y = pos_y;
    }

    void draw(){
        trajectory.draw();
        setlinestyle(PS_SOLID, 1);
        fillrectangle(pos_x - 0.5 * width, pos_y + 0.5 * length, 
                        pos_x + 0.5 * width, pos_y - 0.5 * length);
        draw_plan_trajectory();
    }

    void update_status(float duration){
        float cur_v = v + a * duration;
        this->pos_y += (v + cur_v) / 2 * duration;
        this->v = cur_v > CAR_MAX_SPEED ? CAR_MAX_SPEED : cur_v;
    }

    void generate_ca_trajectory(){
        trajectory.clear();
        int time_length = 3;
        float time_step = 0.1;
        for(float time = 0; time < time_length; time += time_step){
            if(trajectory.points.size() == 0){
                TrajectoryPoint point;
                point.x = this->pos_x;
                point.y = this->pos_y + 0.5 * this->length;
                point.acceleration = this->a;
                point.speed = this->v;
                trajectory.points.emplace_back(std::move(point));
            }else{
                TrajectoryPoint point;
                point.x = trajectory.points.back().x;
                point.y = trajectory.points.back().y + trajectory.points.back().speed * time_step + 0.5 * trajectory.points.back().acceleration * std::pow(time_step, 2);
                point.acceleration = trajectory.points.back().acceleration;
                point.speed = std::min(static_cast<float>(CAR_MAX_SPEED), trajectory.points.back().speed + trajectory.points.back().acceleration * time_step);
                trajectory.points.emplace_back(std::move(point));
            }
        }
    }

    void draw_plan_trajectory(){
        TrajectoryPoint ego_traj_start_point;
        ego_traj_start_point.x = pos_x;
        ego_traj_start_point.y = pos_y;
        ego_traj_start_point.speed = v;

        InputData input_data;
        input_data.start_point = ego_traj_start_point;
        input_data.ref_lane = ref_lane;
        dp_planner.plan(input_data, trajectory);
        std::vector<Point> points;
        for(int i = 0; i < trajectory.points.size(); i++){
            points.emplace_back(Point(trajectory.points[i].x, trajectory.points[i].y));
        }
        qp_planner.optimize(points, pos_x + 350);
        trajectory.points.clear();
    }
};