#pragma once
#include <vector>
#include "input_data.hpp"
#include "../data/trajectory.hpp"
#include "../math/quintic_polynomial_curve1d.hpp"
#include "../data/point.hpp"
class DpPlan{
    public:
        void sample_way_point(TrajectoryPoint& start_point, std::vector< std::vector<Point>>& points){
            double delta_s = std::max(500.0F, start_point.speed); // 5m
            double delta_l = 100; //1m
            int sample_lengthways_count = 8;
            int sample_horizontal_count = 9;
            points.clear();
            std::vector<Point> start{Point(start_point.x, start_point.y)};
            points.emplace_back(start);
            double cur_y = start_point.y;
            for (size_t i = 0; i < sample_lengthways_count; i++){
                cur_y += delta_s;
                std::vector<Point> sample_points;
                for (int j = -sample_horizontal_count / 2; j <= sample_horizontal_count / 2; j++){
                    sample_points.emplace_back(Point(start_point.x + j * delta_l, cur_y));
                }
                points.emplace_back(std::move(sample_points));
            }
        }

        void plan(InputData& input_data, Trajectory& trajectory){
            std::vector< std::vector<Point>> sample_points;
            sample_way_point(input_data.start_point, sample_points);
            for(std::vector<Point>& pts : sample_points){
                for(Point p:pts){
                    p.draw();
                }
            }
            float ref_line_x = input_data.ref_lane.points[0].x;
            for(int i = 1; i < sample_points.size(); i++){
                for(int j = 0; j < sample_points[i].size(); j++){
                    Point cur_point = sample_points[i][j];
                    for(int k = 0; k < sample_points[i-1].size(); k++){
                        Point pre_point = sample_points[i-1][k];
                        QuinticPolynomialCurve1d curve(ref_line_x - pre_point.x, 0, 0,
                                        ref_line_x - cur_point.x, 0.0, 0.0,
                                        sample_points[i][0].y - sample_points[i-1][0].y);
                        // 绘制
                        int interpolation_step = 50;
                        int interpolation_count = 1 + static_cast<int>(sample_points[i][0].y - sample_points[i-1][0].y) / interpolation_step;
                        POINT* pts = new POINT[interpolation_count];
                        for(float dy = 0; dy < sample_points[i][0].y - sample_points[i-1][0].y; dy += interpolation_step){
                            float dx = curve.Evaluate(0, dy);
                            *(pts + static_cast<int>(dy) / interpolation_step ) = {static_cast<long>(ref_line_x - dx), static_cast<long>(sample_points[i-1][0].y + dy)};
                        }
                        *(pts + interpolation_count - 1) = {static_cast<int>(sample_points[i][j].x), static_cast<int>(sample_points[i][j].y)};
                        setlinestyle(PS_DASH, 1);
                        setlinecolor(WHITE);
                        polyline(pts, interpolation_count);
                        delete[] pts;
                    }
                }
            }
        }
};