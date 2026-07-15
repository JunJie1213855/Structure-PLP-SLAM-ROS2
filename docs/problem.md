# 问题排查与解决记录（Viewer 可视化）

> 日期：2026-07-15 · 环境：Ubuntu (Linux 6.8), GCC + `-O3 -ffast-math`, 系统 Eigen (/usr/include/eigen3), 内置 Pangolin (3rd/Pangolin) · 数据集：TUM RGB-D freiburg1_room

---

## 问题 1：第一次点击 "Follow Camera" 切到错误视角，第二/三次点击才正常

### 现象
- 首次勾选 Follow Camera，视角跳到地图原点附近的特写（"另一个地图视角"），看不到当前相机；
- 取消再重新勾选（第 2、3 次点击）后，跟随视角恢复正常。

### 根本原因
Pangolin `OpenGlRenderState::Follow()`（`3rd/Pangolin/src/display/opengl_render_state.cpp:340`）的语义：

- 跟随状态下，实际视图 = `modelview × T_cw`，即 **`modelview` 被解释为"渲染视点相对被跟随相机的偏移"**；
- 仅在 follow 标志 **false→true 翻转时**，`Follow()` 会执行 `modelview := modelview × T_wc`，把当前绝对视图转换为相机相对偏移。

原代码在状态转换分支里先 `SetModelViewMatrix(lookat)` 再 `Follow()`：首次勾选时（follow 标志翻转）刚设置的绝对 lookat 被错误地转换成锚定在**世界原点**的偏移。第 2 次点击只清内部标志、不调 `Unfollow()`（Pangolin follow 标志保持 true），因此第 3 次点击时翻转不再发生，lookat 被直接当作相机相对偏移使用——歪打正着地正确了。

ORB_SLAM3 同样的写法没问题，是因为其菜单默认勾选，翻转发生在第 1 帧、相机位姿≈单位阵时，偏移被"碰巧"正确捕获。

### 解决方案（`src/pangolin_viewer/viewer.cc` `follow_camera()`）
状态转换分支中**调换调用顺序**：先 `Follow(view_matrix)`（让翻转发生、旧 modelview 转偏移后随即被覆盖），再 `SetModelViewMatrix(lookat)`（此时 lookat 被正确解释为相机相对偏移）。同时把 `viewer.h` 中 `follow_camera_` 初值从 `true` 改为 `false`，与菜单默认值一致。

### 验证
用工程链接的同一 `libpangolin.so` 做数值验证（相机位于 (3,1,2) 时首次勾选）：

```
EXPECTED (follow view):   render-eye = ( 3.000,  0.200,  0.500)
OLD order (buggy):        render-eye = (-0.000, -0.800, -1.500)  ← 锚定世界原点
NEW order (fixed):        render-eye = ( 3.000,  0.200,  0.500)  ← 首次勾选即正确
NEW order (re-engage):    render-eye = ( 3.000,  0.200,  0.500)  ← 重复勾选路径一致
```

---

## 问题 2：跟随模式下"地图移动方向与相机相反"——确认为正常行为，非 bug

### 现象
相机位姿向右/上移动时，地图画面向左/下滑动，怀疑方向反了。

### 排查结论
数值模拟（真实 libpangolin，相机逐帧 +x/-y 平移）证明**渲染视点与相机严格同向 1:1 移动**：

```
frame |  相机位置              |  渲染视点位置
  0   |  ( 0.00, -0.00, 0.00) |  (-0.00, -0.80, -1.50)
  5   |  ( 1.00, -0.50, 0.00) |  ( 1.00, -1.30, -1.50)
```

视点跟着相机走，屏幕上的地图内容必然反向滑动（车窗效应）；红色相机框恒定钉在屏幕固定位置（`lookat × T_cw × T_wc ≡ lookat`）。与 ORB_SLAM3 / OpenVSLAM 行为一致。**保持现状，未改代码。**

---

## 问题 3：当前帧相机线框不显示（表象为"最新位姿不是红色"）⭐ 最深的坑

### 现象
设置了红色后当前相机仍"看不到红色"。截图像素级分析证实：**地图区域 0 个红色像素——当前相机线框从未被渲染过**（此前误以为的"红色"实为局部地图点的旧配色 `{1.0,0.1,0.1}`）。

### 排查过程（三层）
1. **静态分析**：位姿链路 `tracker (T_cw) → map_publisher → viewer.inverse() (T_wc) → glMultMatrixd` 与上游一致，无误；
2. **运行时探针**：传入 OpenGL 的 T_wc 矩阵损坏——对角线 `(0.997, -0.998, 0.999, -1.000)`，`m[15] = -1` 导致视锥所有顶点裁剪空间 w<0，被 GPU 整体裁剪；
3. **逐级隔离**：`map_publisher` 返回的 `cam_pose_cw` 是完好的刚体变换（diag≈1,1,1,1），损坏发生在 **`cam_pose_cw.inverse()`** 这一步。

### 根本原因
**全局编译选项 `-ffast-math`（CMakeLists.txt:69-70）导致 Eigen `Matrix4d::inverse()` 返回错误结果**。独立最小复现：

```
--- g++ -O3 -ffast-math（项目选项）---
inv diag = (0.999, -0.999, 1.000, -1.000)    ‖T·inv − I‖ = 2.8       ← 完全错误
--- g++ -O3 ---
inv diag = (0.999,  0.999, 1.000,  1.000)    ‖T·inv − I‖ = 9.8e-18   ← 正确
```

关键帧显示一直正常，因为 `keyframe::get_cam_pose_inv()` 用解析逆（R^T | −R^T·t），不经过通用 `.inverse()`。

### 解决方案（`src/pangolin_viewer/viewer.cc` `get_current_cam_pose()`）
改用 SE(3) 解析逆，绕开被 `-ffast-math` 破坏的通用 4×4 求逆：

```cpp
const Mat33_t rot_wc = cam_pose_cw.block<3,3>(0,0).transpose();
const Vec3_t trans_wc = -rot_wc * cam_pose_cw.block<3,1>(0,3);
// 组装 T_wc = [rot_wc | trans_wc; 0 0 0 1]
```

### 验证
修复后截图：地图区域出现 631 个红色像素、40×28px 紧凑线框结构，随位姿移动。

### ⚠️ 遗留风险（待决策）
`-ffast-math` 影响所有对 4×4 位姿调用 `.inverse()` 的代码，目前已知：

- `src/PLPSLAM/io/trajectory_io.cc` —— **保存的轨迹可能有误（直接影响 ATE 评估！）**
- `src/PLPSLAM/data/frame_statistics.cc`
- `src/PLPSLAM/optimize/g2o/se3/shot_vertex.cc` —— 优化顶点

建议从 `CMakeLists.txt:69-70` 移除 `-ffast-math`（SLAM 精度类项目本不该用该选项，性能收益通常很小），或逐处替换为解析逆。

---

## 配色调整汇总（最终约定）

| 元素 | 颜色 | 位置 |
|---|---|---|
| 当前帧相机线框 | 红 `(1.0, 0.0, 0.0)` | `viewer.cc draw_current_cam_pose()` + `color_scheme.cc curr_cam_` |
| 匹配（局部地图）路标点 | 红 `(1.0, 0.1, 0.1)` | `color_scheme.cc local_lm_`（black 方案） |
| 关键帧线框 | 绿（black 方案） | `color_scheme.cc kf_line_` |
| 普通路标点 | 灰白 | `color_scheme.cc lm_` |
| 2D 帧窗口跟踪特征点 | 绿 `BGR(0,255,0)` | `frame_publisher.h mapping_color_` |

另清理了此前调试遗留：当前相机的 8 倍放大、硬编码线宽、每秒位置日志、红色圆点标记（`draw_current_cam_pose` 恢复为简洁实现）。`map_publisher.cc set_current_cam_pose()` 中还残留一条只打印前 3 次的调试日志，待清理。

---

## 问题 4：ROS2 节点加载地图重定位后立即段错误（Segmentation fault）

### 现象
`ros2 run plpslam_ros2 rgbd ... mymap_rgbd.msg 1`（加载地图 + 重定位 + 建图模式），重定位成功、跟踪约 3 帧后进程段错误。

### 排查
gdb 复现抓到崩溃栈（mapping 线程）：

```
#0 PLPSLAM::module::two_view_triangulator_line::triangulate(...)
#1 PLPSLAM::mapping_module::triangulate_line_with_two_keyframes(...)
```

### 根本原因
地图序列化格式**不包含 `_keyline_functions`**（每条 2D 线特征的直线方程 ax+by+c=0）。从 msg 文件恢复关键帧的构造函数（`keyframe.cc` 点+线版本）恢复了 `_keylsd`、深度、LBD 描述子等，但 `_keyline_functions` 保持空 vector。

重定位+建图模式下，mapping 模块把**新关键帧与加载的旧关键帧**做线三角化，`two_view_triangulator_line::triangulate()` 对空 vector 执行 `_keyline_functions[idx]` 越界读（空 vector data 指针为 null）→ 段错误。纯建图模式（不加载地图）不会触发，因此此前未暴露。

### 解决方案
1. **根因修复**（`src/PLPSLAM/data/keyframe.cc`）：地图加载构造函数在初始化列表中用 `compute_keyline_functions(keylines)` 重建线方程——公式与 `line_extractor.cc` 一致：`f = (sp × ep) / √(a²+b²)`（成员为 const，须在初始化列表中构造）。
2. **纵深防御**（`src/PLPSLAM/module/two_view_triangulator_line.cc`）：`triangulate()` 入口对两个关键帧的 `_keylsd`/`_keyline_functions`/`_stereo_x_right_cooresponding_to_keylines` 做边界检查，越界直接返回 false。

### 验证
gdb 下用相同参数 + freiburg2_large_with_loop 数据复跑：修复前首帧后 <1 秒崩溃；修复后 120 秒无崩溃，重定位成功 73 次，"3D lines found" 线三角化正常输出。

### 备注
更彻底的做法是把 `_keyline_functions` 加入地图序列化（`map_database_io`），但该字段可由 keylines 确定性重建，重建方案不破坏已有 msg 文件的兼容性，故采用重建。

---

## 经验教训


1. **Pangolin `Follow()` 的 modelview 是"相机相对偏移"**，不是绝对视角；`Follow()` 必须先于 `SetModelViewMatrix()` 调用。
2. **`-ffast-math` 会静默破坏 Eigen 的 4×4 通用求逆**——症状极其隐蔽（几何体直接消失而非画错位置）。对刚体变换永远优先用解析逆。
3. 排查渲染问题时，**截图 + 像素统计**（xwd + PIL）是无 GUI 自动化工具时的有效验证手段；"看不见"要先区分"画错了位置/颜色"还是"根本没画"。
4. 用户报告"A 不是红色"时，先验证 A 是否被渲染，再查颜色——本例中"不是红色"的真因是"根本没画出来"+"别的东西恰好是红的"。
