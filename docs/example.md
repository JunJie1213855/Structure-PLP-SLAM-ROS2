# Structure PLP-SLAM 运行命令

## 环境设置

```bash
cd /home/ros/lib/SLAM/VIO/Structure-PLP-SLAM

export LD_LIBRARY_PATH=$PWD/build/lib:$PWD/3rd/Pangolin/install/lib:$HOME/.local/lib:$LD_LIBRARY_PATH
```

---

## TUM RGB-D 数据集

### 单目 (Monocular)

**点特征：**
```bash
./build/run_tum_rgbd_slam \
    -v ./orb_vocab/orb_vocab.dbow2 \
    -d /home/ros/dataset/Tum-RGBD/rgbd_dataset_freiburg2_360_hemisphere \
    -c ./example/tum_rgbd/TUM_RGBD_mono_2.yaml
```

**点 + 线特征：**
```bash
./build/run_tum_rgbd_slam_with_line \
    -v ./orb_vocab/orb_vocab.dbow2 \
    -d /home/ros/dataset/Tum-RGBD/rgbd_dataset_freiburg2_360_hemisphere \
    -c ./example/tum_rgbd/TUM_RGBD_mono_2.yaml
```

### RGB-D（深度相机）

**点特征：**
```bash
./build/run_tum_rgbd_slam \
    -v ./orb_vocab/orb_vocab.dbow2 \
    -d /home/ros/dataset/Tum-RGBD/rgbd_dataset_freiburg2_360_hemisphere \
    -c ./example/tum_rgbd/TUM_RGBD_rgbd_2.yaml
```

**点 + 线特征：**
```bash
./build/run_tum_rgbd_slam_with_line \
    -v ./orb_vocab/orb_vocab.dbow2 \
    -d /home/ros/dataset/Tum-RGBD/rgbd_dataset_freiburg2_360_hemisphere \
    -c ./example/tum_rgbd/TUM_RGBD_rgbd_2.yaml
```

**点 + 线 + 面特征（RGB-D + 平面分割）：**  `run_slam_planeSeg`（已修改，同时启用线特征和面特征）
```bash
./build/run_slam_planeSeg \
    -v ./orb_vocab/orb_vocab.dbow2 \
    -d /home/ros/dataset/Tum-RGBD/rgbd_dataset_freiburg2_360_hemisphere_with_masks \
    -c ./example/tum_rgbd/TUM_RGBD_rgbd_2.yaml
```

> **配置文件说明：** `_1` 对应 fr1 系列，`_2` 对应 fr2，`_3` 对应 fr3。

---

## KITTI 数据集

### 单目 (Monocular)

**点特征：**
```bash
./build/run_kitti_slam \
    -v ./orb_vocab/orb_vocab.dbow2 \
    -d /path/to/KITTI/odometry/dataset/sequences/00 \
    -c ./example/kitti/KITTI_mono_00-02.yaml
```

**点 + 线特征：**
```bash
./build/run_kitti_slam_with_line \
    -v ./orb_vocab/orb_vocab.dbow2 \
    -d /path/to/KITTI/odometry/dataset/sequences/00 \
    -c ./example/kitti/KITTI_mono_00-02.yaml
```

### 双目 (Stereo)

**点特征：**
```bash
./build/run_kitti_slam \
    -v ./orb_vocab/orb_vocab.dbow2 \
    -d /path/to/KITTI/odometry/dataset/sequences/00 \
    -c ./example/kitti/KITTI_stereo_00-02.yaml
```

**点 + 线特征：**
```bash
./build/run_kitti_slam_with_line \
    -v ./orb_vocab/orb_vocab.dbow2 \
    -d /path/to/KITTI/odometry/dataset/sequences/00 \
    -c ./example/kitti/KITTI_stereo_00-02.yaml
```

> **配置文件说明：** `_00-02` 对应序列 00-02，`_03` 对应序列 03，`_04-12` 对应序列 04-12。

---

## EuRoC MAV 数据集

### 单目 (Monocular)

**点特征：**
```bash
./build/run_euroc_slam \
    -v ./orb_vocab/orb_vocab.dbow2 \
    -d /path/to/EuRoC/MH_01_easy/mav0 \
    -c ./example/euroc/EuRoC_mono.yaml
```

**点 + 线特征：**
```bash
./build/run_euroc_slam_with_line \
    -v ./orb_vocab/orb_vocab.dbow2 \
    -d /path/to/EuRoC/MH_01_easy/mav0 \
    -c ./example/euroc/EuRoC_mono.yaml
```

### 双目 (Stereo)

**点特征：**
```bash
./build/run_euroc_slam \
    -v ./orb_vocab/orb_vocab.dbow2 \
    -d /path/to/EuRoC/MH_01_easy/mav0 \
    -c ./example/euroc/EuRoC_stereo.yaml
```

**点 + 线特征：**
```bash
./build/run_euroc_slam_with_line \
    -v ./orb_vocab/orb_vocab.dbow2 \
    -d /path/to/EuRoC/MH_01_easy/mav0 \
    -c ./example/euroc/EuRoC_stereo.yaml
```

**点 + 面特征（EuRoC + 平面分割）：**
```bash
./build/run_euroc_slam_planeSeg \
    -v ./orb_vocab/orb_vocab.dbow2 \
    -d /path/to/EuRoC_with_masks/MH_01_easy/mav0 \
    -c ./example/euroc/EuRoC_mono.yaml
```

---

## 通用选项

| 选项 | 说明 |
|------|------|
| `-v, --vocab` | 词汇文件路径 |
| `-d, --data-dir` | 数据集目录 |
| `-c, --config` | 配置文件路径 |
| `--frame-skip` | 跳帧间隔 (默认 1) |
| `--no-sleep` | 不等待下一帧（快速运行） |
| `--auto-term` | 完成后自动关闭 Viewer |
| `--debug` | 调试模式 |
| `--eval-log` | 保存轨迹 (`frame_trajectory.txt`, `keyframe_trajectory.txt`) |
| `-p, --map-db` | SLAM 结束后保存地图数据库 |

---

## 保存位姿轨迹

在命令末尾加 `--eval-log`，运行结束后会生成 TUM 格式的轨迹文件：

```bash
./build/run_tum_rgbd_slam \
    -v ./orb_vocab/orb_vocab.dbow2 \
    -d /home/ros/dataset/Tum-RGBD/rgbd_dataset_freiburg2_360_hemisphere \
    -c ./example/tum_rgbd/TUM_RGBD_rgbd_2.yaml \
    --eval-log
```

生成文件：
- `frame_trajectory.txt` — 每帧相机位姿
- `keyframe_trajectory.txt` — 关键帧位姿

格式：`timestamp tx ty tz qx qy qz qw` (TUM RGB-D 标准)

---

## 平面特征（Plane）数据集要求

> **重要：** 运行平面 SLAM 前，必须先对数据集的所有图像运行 PlaneRecNet 生成实例分割 mask。

### 数据集目录结构

```
rgbd_dataset_with_masks/
├── rgb.txt              # RGB 图像时间戳（TUM 标准格式）
├── depth.txt            # 深度图像时间戳（TUM 标准格式）
├── mask.txt             # 分割 mask 时间戳（与 rgb.txt 格式相同）
├── rgb/                 # RGB 图像
├── depth/               # 深度图像
└── mask/                # 实例分割 mask 图像（PNG 格式）
```

### mask.txt 格式

与 `rgb.txt` 格式相同，每行：`timestamp mask/filename.png`

```
1341847980.723020 mask/1341847980.723020.png
1341847980.756032 mask/1341847980.756032.png
...
```

### mask 图像要求

| 项目 | 要求 |
|------|------|
| 格式 | PNG，`cv::IMREAD_UNCHANGED` 读取 |
| 尺寸 | 与对应 RGB 图像完全相同 |
| 通道 | 单通道灰度图 |
| 像素值 | 每个像素值为平面实例 ID（0 = 背景/无平面，1,2,3... = 不同平面实例） |
| 数量 | 与 RGB 图像一一对应 |
| 生成工具 | [PlaneRecNet](https://github.com/EryiXie/PlaneRecNet) |

---

## 图片定位 (Image Localization)

**点特征：**
```bash
./build/run_image_localization \
    -v ./orb_vocab/orb_vocab.dbow2 \
    -i /path/to/image/directory \
    -c ./example/tum_rgbd/TUM_RGBD_mono_2.yaml \
    -p ./map.msg
```

**点 + 线特征：**
```bash
./build/run_image_localization_point_line \
    -v ./orb_vocab/orb_vocab.dbow2 \
    -i /path/to/image/directory \
    -c ./example/tum_rgbd/TUM_RGBD_mono_2.yaml \
    -p ./map.msg
```


---

## ROS2 运行

### 环境设置

```bash
cd /home/ros/lib/SLAM/VIO/Structure-PLP-SLAM/ros2

source /opt/ros/humble/setup.bash
source install/setup.bash

export LD_LIBRARY_PATH=$HOME/lib/SLAM/VIO/Structure-PLP-SLAM/build/lib:$HOME/lib/SLAM/VIO/Structure-PLP-SLAM/3rd/Pangolin/install/lib:$HOME/.local/lib:$LD_LIBRARY_PATH
```

### 单目 (Monocular)

**点特征：**
```bash
ros2 run plpslam_ros2 mono \
    ../orb_vocab/orb_vocab.dbow2 \
    ../example/tum_rgbd/TUM_RGBD_mono_2.yaml \
    0
```

**点 + 线特征：**
```bash
ros2 run plpslam_ros2 mono \
    ../orb_vocab/orb_vocab.dbow2 \
    ../example/tum_rgbd/TUM_RGBD_mono_2.yaml \
    1
```

### RGB-D

**点特征：**
```bash
ros2 run plpslam_ros2 rgbd \
    ../orb_vocab/orb_vocab.dbow2 \
    ../example/tum_rgbd/TUM_RGBD_rgbd_2.yaml \
    0
```

**点 + 线特征：**
```bash
ros2 run plpslam_ros2 rgbd \
    ../orb_vocab/orb_vocab.dbow2 \
    ../example/tum_rgbd/TUM_RGBD_rgbd_2.yaml \
    1
```

### 双目 (Stereo)

**点特征：**
```bash
ros2 run plpslam_ros2 stereo \
    ../orb_vocab/orb_vocab.dbow2 \
    ../example/kitti/KITTI_stereo_00-02.yaml \
    0
```

**点 + 线特征：**
```bash
ros2 run plpslam_ros2 stereo \
    ../orb_vocab/orb_vocab.dbow2 \
    ../example/kitti/KITTI_stereo_00-02.yaml \
    1
```

> 第 4 个参数：`0` = 仅点特征，`1` = 点 + 线特征

### 用 rosbag 播放

```bash
# 终端 1: 启动 SLAM
ros2 run plpslam_ros2 mono ../orb_vocab/orb_vocab.dbow2 ../example/tum_rgbd/TUM_RGBD_mono_2.yaml 0

# 终端 2: 播放 rosbag
ros2 bag play your_dataset_bag --clock
```

### 订阅话题

| 可执行文件 | 话题 | 消息类型 |
|-----------|------|---------|
| `mono` | `camera/image_raw` | `sensor_msgs/Image` |
| `rgbd` | `camera/rgb` + `camera/depth` | `sensor_msgs/Image` |
| `stereo` | `camera/left` + `camera/right` | `sensor_msgs/Image` |
