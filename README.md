## 自动驾驶-规划算法
### 项目背景
30天，零基础入门自动驾驶规划算法，具备中级规划算法工程师的水平

### Day1 init project base tools
1. 创建空项目，并添加osqp、easy_x库
2. 可视化：实现车道线、车辆的静态绘制，支持平移和缩放

### Day2 define base data and refine visual
1. 定义如下数据结构：
    - point
    - car
    - lane
    - road
    - trajectory_point
    - trajectory
    - scene
2. 完善可视化，支持车辆匀加速运动，直线轨迹可视化

### Day3 path plan:Dp - sample and fit curve
1. 实现采样逻辑（基础版）
2. 基于五次多项式拟合曲线、计算多阶导数；

### Day4 path plan:Dp - add Path cost，finish best path select frame
1. 计算路径平滑度的损失
2. 完成动态规划选择最优路径的逻辑

### Day5 path plan:Qp - add Qp optimize path (to do:adjust params)
1. 新增二次规划优化DP给出的“粗糙”路径
2. 三次样条曲线工具类（jerk恒定），生成连续路径

### Day6 SimpleRefLine utils
1. 完成简易的xy ←→ sl坐标转换的工具类