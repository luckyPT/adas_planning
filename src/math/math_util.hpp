#pragma once
#include <math.h>
class math_util{
    public:
    /**
     * @brief 
     * 
     * @param center_x 
     * @param center_y 
     * @param theta theta > 0
     * @param origin_x 
     * @param origin_y 
     * @param x 
     * @param y 
     */
    void static rotate_point(float center_x, float center_y, float theta,
                      float origin_x, float origin_y,
                      float& x, float& y){
        x= (origin_x - center_x)*std::cos(theta) - (origin_y - center_y)*std::sin(theta) + center_x ;
        y= (origin_x - center_x)*std::sin(theta) + (origin_y - center_y)*std::cos(theta) + center_y ;
    }

    double static calculate_angle(float vec1_x, float vec1_y, float vec2_x, float vec2_y){
        return std::acos((vec1_x * vec2_x + vec1_y * vec2_y) / 
                            (std::sqrt(vec1_x * vec1_x + vec1_y * vec1_y) * std::sqrt(vec2_x * vec2_x + vec2_y * vec2_y)));
    }

    /**
     * @brief 判断向量1是否在向量2的右边
     * 
     * @param vec1_x 
     * @param vec1_y 
     * @param vec2_x 
     * @param vec2_y 
     * @return 
     * -1 - 向量1在向量2的右边
     *  0 - 两个向量重合
     *  1 - 向量1在向量2的左边
     */
    int static calculate_relative_pos(float vec1_x, float vec1_y, float vec2_x, float vec2_y){
        float result = vec1_x * vec2_y -vec2_x * vec1_y;
        if(std::abs(result) < 1e-5){
            return 0;
        }
        return result > 0 ? -1 : 1;
    }
};