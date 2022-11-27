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
            // 绘制采样点
            for(std::vector<Point>& pts : sample_points){
                for(Point p:pts){
                    p.draw();
                }
            }
            // 动态规划求解最优路径
            float ref_line_x = input_data.ref_lane.points[0].x;
            double dp_cost[sample_points.size()-1][sample_points[1].size()][2];
            for(int i = 0; i < sample_points.size()-1; i++){
                for( int j = 0; j < sample_points[1].size(); j++){
                    dp_cost[i][j][0] = 1e10;
                }
            }
            for(int i = 1; i < sample_points.size(); i++){
                for(int j = 0; j < sample_points[i].size(); j++){
                    Point cur_point = sample_points[i][j];
                    for(int k = 0; k < sample_points[i-1].size(); k++){
                        Point pre_point = sample_points[i-1][k];
                        QuinticPolynomialCurve1d curve(ref_line_x - pre_point.x, 0, 0,
                                        ref_line_x - cur_point.x, 0.0, 0.0,
                                        sample_points[i][0].y - sample_points[i-1][0].y);
                        double path_cost = calculate_path_cost(curve, 0, sample_points[i][0].y - sample_points[i-1][0].y);
                        double cost = path_cost + (i == 1 ? 0 : dp_cost[i-2][k][0]);
                        if(dp_cost[i-1][j][0] > cost){
                            dp_cost[i-1][j][0] = cost;
                            dp_cost[i-1][j][1] = k; 
                        }
                        // 绘制
                        // debug_draw(sample_points, i, j, ref_line_x, curve);
                    }
                }
            }
            // 回溯，找到最小路径
            int indexes[sample_points.size()-1];
            int min_cost_index = 0;
            for(int j = 1; j < sample_points[1].size(); j++){
                if(dp_cost[sample_points.size()-2][j][0] < dp_cost[sample_points.size()-2][min_cost_index][0]){
                    min_cost_index = j;
                }
            }
            indexes[sample_points.size() - 2] = min_cost_index;
            for(int i = sample_points.size()-3; i>=0; i--){
                indexes[i] = dp_cost[i+1][indexes[i+1]][1];
            }
            // 绘制最短路径
            POINT* pts = new POINT[9];
            *(pts + 0) = {static_cast<int>(input_data.start_point.x), 
                          static_cast<int>(input_data.start_point.y)};
            TrajectoryPoint  tmp = TrajectoryPoint();
            tmp.x = input_data.start_point.x;
            tmp.y = input_data.start_point.y;
            trajectory.points.emplace_back(tmp);
            for(int i = 1; i < 9; i++){
                *(pts + i) = {static_cast<long>(sample_points[i][indexes[i-1]].x), static_cast<long>(sample_points[i][indexes[i-1]].y)};
                TrajectoryPoint  tmp = TrajectoryPoint();
                tmp.x = sample_points[i][indexes[i-1]].x;
                tmp.y = sample_points[i][indexes[i-1]].y;
                trajectory.points.emplace_back(std::move(tmp));
            }
            setlinestyle(PS_DASH, 10);
            setlinecolor(BLUE);
            polyline(pts, 9);
            delete[] pts;
        }

        double calculate_path_cost(QuinticPolynomialCurve1d& curve, double start_s, double end_s){
            double path_cost = 0.0;
            double l_cost = 1;
            double dl_cost = 1;
            double ddl_cost = 1;
            int interpolation_step = 50;
            for (double curve_s = 0.0; curve_s < (end_s - start_s); curve_s += interpolation_step){
                const double l = curve.Evaluate(0, curve_s) / 350;
                const double dl = std::fabs(curve.Evaluate(1, curve_s));
                const double ddl = std::fabs(curve.Evaluate(2, curve_s));
                path_cost += l_cost * std::pow(l, 2) + 
                             dl_cost * std::pow(dl, 2) + 
                             ddl_cost * std::pow(ddl, 2);

            }
            return path_cost;
        }
        void debug_draw(std::vector< std::vector<Point>>& sample_points, int i, int j, 
                        float ref_line_x,
                        QuinticPolynomialCurve1d& curve){
            int interpolation_step = 50;
            int interpolation_count = 1 + static_cast<int>(sample_points[1][0].y - sample_points[0][0].y) / interpolation_step;
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
};