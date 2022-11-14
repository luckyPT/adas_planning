#pragma once
#include <vector>
#include "../../libs/osqp/osqp.h"
#include "../data/point.hpp"
#include "../math/piecewise_jerk_trajectory1d.hpp"
class QpPlan{
    public:
    double w_x = 0;
    double w_x_ref = 0;
    double w_dx = 10000;
    double w_ddx = 100000;
    double w_dddx = 1000000;
    double w_end_x = 35;
    double w_end_dx = 100;
    double w_end_ddx = 100;
    double dddx_bound_lower = -5;
    double dddx_bound_upper = 5;
    double delta_s = 500; 

    template <typename T>
    T* CopyData(const std::vector<T>& vec) {
        T* data = new T[vec.size()];
        memcpy(data, vec.data(), sizeof(T) * vec.size());
        return data;
    }

    void calculate_cost_kernel(std::vector<Point>& sample_points,
                                double standard_x,
                                std::vector<c_float>* P_data,
                                std::vector<c_int>* P_indptr,
                                std::vector<c_int>* P_indices){
        int value_index = 0;
        const int n = sample_points.size();
        const int num_of_variables = 3 * n; // l, l', l"
        std::vector<std::vector<std::pair<c_int, c_float>>> columns(num_of_variables);
        // x(i)^2 * (w_x + w_x_ref[i])
        for (int i = 0; i < n - 1; ++i) {
            columns[i].emplace_back(i, (w_x + w_x_ref)); // 不同位置可以设置不同的w_x_ref
            ++value_index;
        }
        // 末态单独处理  x(n-1)^2 * (w_x + w_x_ref[n-1] + w_end_x)
        columns[n - 1].emplace_back(n - 1, (w_x + w_x_ref + w_end_x));
        ++value_index;

        // x(i)' ^2 * w_dx
        for (int i = 0; i < n - 1; ++i) {
            columns[n + i].emplace_back(n + i, w_dx);
            ++value_index;
        }
        columns[2 * n - 1].emplace_back(2 * n - 1, w_end_dx);
        ++value_index;

        // 在计算 l'' ^2 的权重时， 考虑了  l''' ^2 的权重
        // 其中 l''' = ( l''(i)-l''(i-1) ) / delta_s
        // l'''^2 = l''(i)^2 /delta_s^2   +  l''(i-1)^2 /delta_s^2  -  2 * l''(i)*l''(i-1) /delta_s^2
        auto delta_s_square = delta_s * delta_s;
        // x(i)'' ^2  * (w_ddx + 2 * w_dddx / delta_s^2)
        // 所以每个 l''(i)^2 的权重有两部分组成，一个是w_ddx， 一个是w_dddx
        columns[2 * n].emplace_back(2 * n, (w_ddx + w_dddx / delta_s_square));
        ++value_index;

        for (int i = 1; i < n - 1; ++i) {
            columns[2 * n + i].emplace_back(
                2 * n + i, (w_ddx + 2.0 * w_dddx / delta_s_square));
            ++value_index;
        }
        columns[3 * n - 1].emplace_back(
            3 * n - 1, (w_ddx + w_dddx / delta_s_square + w_x_ref));
        ++value_index;

        // -2 * w_dddx / delta_s^2 * x(i)'' * x(i + 1)''
        // hession矩阵的 右下角这个 n*n的矩阵，除了对角线元素，它左下侧还有一排元素
        /***       |    o                  | 
         *         |         o             |
         *         |              o        |
         *         |              o  o     |
         *         |                 o  0  |
         * ***/ 
        for (int i = 0; i < n - 1; ++i) {
            columns[2 * n + i].emplace_back(2 * n + i + 1, (-2.0 * w_dddx / delta_s_square));
            ++value_index;
        }

        // 转换成csc_matrix的形式
        int ind_p = 0;
        for (int i = 0; i < num_of_variables; ++i) {
            P_indptr->push_back(ind_p);
            for (const auto& row_data_pair : columns[i]) {
            P_data->push_back(row_data_pair.second * 2.0); // P_data来记录 hession矩阵的元素        
            P_indices->push_back(row_data_pair.first);     // P_indices来记录各个元素所在列的行号               
            ++ind_p;                                                                                           
            }                                                                                                    
        }                                                                                                      
        P_indptr->push_back(ind_p);  
    }

    void calculate_cost_offset(std::vector<Point>& sample_points, 
                                double standard_x,
                                std::vector<c_float>* q){
        const int n = static_cast<int>(sample_points.size());
        const int kNumParam = 3 * n;
        q->resize(kNumParam, 0.0);
        double x_ref[n];
        // todo：计算x_ref[n]
        for(int i = 0; i < n; ++i){
            x_ref[i] = sample_points[i].x;
        }
        // l^2+(l-ref)^2 拆开项中的 -2ref*i
        for (int i = 0; i < n; ++i) {
            q->at(i) += -2.0 * w_x_ref * x_ref[i];
        }

        q->at(n - 1) +=
            -2.0 * w_end_x * x_ref[n-1];
        q->at(2 * n - 1) +=
            -2.0 * w_end_dx * 0;
        q->at(3 * n - 1) +=
            -2.0 * w_end_ddx * 0;
    }

    void calculate_constraint(std::vector<Point>& sample_points,
                                double standard_x,
                                std::vector<c_float>* A_data, std::vector<c_int>* A_indices,
                                std::vector<c_int>* A_indptr, std::vector<c_float>* lower_bounds,
                                std::vector<c_float>* upper_bounds){
        // 3N个    上下界约束
        // 3(N-1)  连续性约束
        // 3个     初始约束
        const int n = static_cast<int>(sample_points.size());
        const int num_of_variables = 3 * n;                                // 决策变量个数
        const int num_of_constraints = num_of_variables + 3 * (n - 1) + 3; // 约束条件的个数
        lower_bounds->resize(num_of_constraints);
        upper_bounds->resize(num_of_constraints);
        std::vector<std::vector<std::pair<c_int, c_float>>> variables(num_of_variables);
        int constraint_index = 0;

        // set x, x', x'' bounds
        for (int i = 0; i < num_of_variables; ++i) {
            if (i < n) {
                variables[i].emplace_back(constraint_index, 1.0);
                lower_bounds->at(constraint_index) = sample_points[i].x - 50;
                upper_bounds->at(constraint_index) = sample_points[i].x + 50;
            } else if (i < 2 * n) {
                variables[i].emplace_back(constraint_index, 1.0);
                lower_bounds->at(constraint_index) = -10;
                upper_bounds->at(constraint_index) = 10;
            } else {
                variables[i].emplace_back(constraint_index, 1.0);
                lower_bounds->at(constraint_index) = -10;
                upper_bounds->at(constraint_index) = 10;
            }
            ++constraint_index;
        }

        // x" 加速度约束 dddx约束
        // x(i->i+1)''' = (x(i+1)'' - x(i)'') / delta_s
        for (int i = 0; i + 1 < n; ++i) {
            variables[2 * n + i].emplace_back(constraint_index, -1.0);
            variables[2 * n + i + 1].emplace_back(constraint_index, 1.0);
            lower_bounds->at(constraint_index) = dddx_bound_lower * delta_s;
            upper_bounds->at(constraint_index) = dddx_bound_upper * delta_s;
            ++constraint_index;
        }

        // x' 速度连续性约束
        // x(i+1)' - x(i)' -  0.5*delta_s *x(i)'' -  0.5*delta_s *x(i+1)'' = 0
        for (int i = 0; i + 1 < n; ++i) {
            // 这里应用了缩放因子
            variables[n + i    ].emplace_back(constraint_index, -1.0);
            variables[n + i + 1].emplace_back(constraint_index,  1.0);
            variables[2 * n + i].emplace_back(constraint_index, -0.5 * delta_s);
            variables[2*n + i+1].emplace_back(constraint_index, -0.5 * delta_s);
            lower_bounds->at(constraint_index) = 0;
            upper_bounds->at(constraint_index) = 0.00000001;
            ++constraint_index;
        }

        // x 位置连续性约束
        // x(i+1) =  x(i) + delta_s * x(i)' + 1/3* delta_s^2 * x(i)'' + 1/6* delta_s^2 * x(i+1)''
        auto delta_s_sq_ = delta_s * delta_s;
        for (int i = 0; i + 1 < n; ++i) {
            variables[i        ].emplace_back(constraint_index, -1.0);
            variables[i   + 1  ].emplace_back(constraint_index,  1.0);
            variables[n   + i  ].emplace_back(constraint_index, -delta_s);
            variables[2*n + i  ].emplace_back(constraint_index, -delta_s_sq_ / 3.0);
            variables[2*n + i+1].emplace_back(constraint_index, -delta_s_sq_ / 6.0);
            lower_bounds->at(constraint_index) = 0;
            upper_bounds->at(constraint_index) = 0.00000001;
            ++constraint_index;
        }

        // 初始状态约束
        // constrain on x_init、x'_init、x"_init
        variables[0].emplace_back(constraint_index, 1.0);
        lower_bounds->at(constraint_index) = sample_points[0].x - 0.001;
        upper_bounds->at(constraint_index) = sample_points[0].x + 0.001;
        ++constraint_index;

        variables[n].emplace_back(constraint_index, 1.0);
        lower_bounds->at(constraint_index) = -0.000001;
        upper_bounds->at(constraint_index) = 0.000001;
        ++constraint_index;

        variables[2 * n].emplace_back(constraint_index, 1.0);
        lower_bounds->at(constraint_index) = -0.000001;
        upper_bounds->at(constraint_index) = 0.000001;
        ++constraint_index;

        int ind_p = 0;
        for (int i = 0; i < num_of_variables; ++i) {
            A_indptr->push_back(ind_p);
            for (const auto& variable_nz : variables[i]) {
            // coefficient
            A_data->push_back(variable_nz.second);

            // constraint index
            A_indices->push_back(variable_nz.first);
            ++ind_p;
            }
        }
        // We indeed need this line because of
        // https://github.com/oxfordcontrol/osqp/blob/master/src/cs.c#L255
        A_indptr->push_back(ind_p);
    }

    bool optimize(std::vector<Point>& sample_points, double standard_x){
        std::vector<c_float> P_data;
        std::vector<c_int> P_indices;
        std::vector<c_int> P_indptr;
        calculate_cost_kernel(sample_points, standard_x, &P_data, &P_indices, &P_indptr);
        
        // calculate offset
        std::vector<c_float> q;
        calculate_cost_offset(sample_points, standard_x, &q);

        // calculate affine constraints
        std::vector<c_float> A_data;
        std::vector<c_int> A_indices;
        std::vector<c_int> A_indptr;
        std::vector<c_float> lower_bounds;
        std::vector<c_float> upper_bounds;
        calculate_constraint(sample_points, standard_x, &A_data, &A_indices, &A_indptr, &lower_bounds,
                            &upper_bounds);

        size_t kernel_dim = 3 * sample_points.size();
        int num_affine_constraint = lower_bounds.size();
        OSQPData* data = reinterpret_cast<OSQPData*>(c_malloc(sizeof(OSQPData)));
        data->n = kernel_dim;
        data->m = num_affine_constraint;
        data->P = csc_matrix(kernel_dim, kernel_dim, P_data.size(), CopyData(P_data),
                            CopyData(P_indices), CopyData(P_indptr));
        data->q = CopyData(q);
        data->A =
            csc_matrix(num_affine_constraint, kernel_dim, A_data.size(),
                        CopyData(A_data), CopyData(A_indices), CopyData(A_indptr));
        data->l = CopyData(lower_bounds);
        data->u = CopyData(upper_bounds);
        // osqp
        OSQPSettings* settings =
        reinterpret_cast<OSQPSettings*>(c_malloc(sizeof(OSQPSettings)));
        osqp_set_default_settings(settings);
        settings->polish = true;
        settings->verbose = false;
        settings->scaled_termination = true;
        settings->max_iter = 1000000;
        OSQPWorkspace *work;
        osqp_setup(&work, data, settings);
        osqp_solve(work);
        auto status = work->info->status_val;
        if (status < 0 || (status != 1 && status != 2)) {
            //ERROR
            //throw "OSQP ERROR";
            return false;
        }else if(work->solution == nullptr){
            //ERROR
            //throw "OSQP ERROR";
            return false;
        }
        // extract primal results
         // output
        std::vector<double> x_;
        std::vector<double> dx_;
        std::vector<double> ddx_;
        x_.resize(sample_points.size());
        dx_.resize(sample_points.size());
        ddx_.resize(sample_points.size());
        for (size_t i = 0; i < sample_points.size(); ++i) {
            x_.at(i) = work->solution->x[i];
            dx_.at(i) = work->solution->x[i + sample_points.size()];
            ddx_.at(i) = work->solution->x[i + 2 * sample_points.size()];
        }

        // Cleanup
        osqp_cleanup(work);
        FreeData(data);
        c_free(settings);
        ToPiecewiseJerkPath(sample_points[0], x_, dx_, ddx_);
        return true;
    }

    void ToPiecewiseJerkPath(Point& start_point, 
                                std::vector<double>& x,
                                std::vector<double>& dx,
                                std::vector<double>& ddx){
        PiecewiseJerkTrajectory1d piecewise_jerk_traj(x.front(), dx.front(),
                                                ddx.front());
        for (std::size_t i = 1; i < x.size(); ++i) {
            const auto dddl = (ddx[i] - ddx[i - 1]) / delta_s;
            piecewise_jerk_traj.AppendSegment(dddl, delta_s);
        }
        // 生成轨迹，并绘制
        POINT* pts = new POINT[80];
        int i = 0;
        double accumulated_s = 0.0;
        double accumulated_y = start_point.y;
        while (accumulated_s < piecewise_jerk_traj.ParamLength()) {
            double l = piecewise_jerk_traj.Evaluate(0, accumulated_s);
            double dl = piecewise_jerk_traj.Evaluate(1, accumulated_s);
            accumulated_y += 100;
            double ddl = piecewise_jerk_traj.Evaluate(2, accumulated_s);
            *(pts + i) = {static_cast<int>(l), static_cast<int>(accumulated_y)};
            i += 1;
            accumulated_s += 100;
        }
        setlinestyle(PS_DASH, 20);
        setlinecolor(RED);
        polyline(pts, 80);
        delete[] pts;

    }

    void FreeData(OSQPData* data) {
        delete[] data->q;
        delete[] data->l;
        delete[] data->u;

        delete[] data->P->i;
        delete[] data->P->p;
        delete[] data->P->x;

        delete[] data->A->i;
        delete[] data->A->p;
        delete[] data->A->x;
    }
};