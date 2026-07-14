# 建图、保存地图、重定位 命令指南

## 环境设置

```bash
cd /home/ros/lib/SLAM/VIO/Structure-PLP-SLAM
export LD_LIBRARY_PATH=$PWD/build/lib:$PWD/3rd/Pangolin/install/lib:$HOME/.local/lib:$LD_LIBRARY_PATH
```

---

## 1. 建图 (Mapping)

运行 SLAM 并保存地图数据库（`.msg` 文件）：

### TUM RGB-D 单目建图

```bash
./build/run_tum_rgbd_slam \
    -v ./orb_vocab/orb_vocab.dbow2 \
    -d /home/ros/dataset/Tum-RGBD/rgbd_dataset_freiburg2_360_hemisphere \
    -c ./example/tum_rgbd/TUM_RGBD_mono_2.yaml \
    --map-db mymap.msg \
    --auto-term
```

### TUM RGB-D 深度相机建图

```bash
./build/run_tum_rgbd_slam \
    -v ./orb_vocab/orb_vocab.dbow2 \
    -d /home/ros/dataset/Tum-RGBD/rgbd_dataset_freiburg2_360_hemisphere \
    -c ./example/tum_rgbd/TUM_RGBD_rgbd_2.yaml \
    --map-db mymap_rgbd.msg \
    --auto-term
```

### 点 + 线特征建图

```bash
./build/run_tum_rgbd_slam_with_line \
    -v ./orb_vocab/orb_vocab.dbow2 \
    -d /home/ros/dataset/Tum-RGBD/rgbd_dataset_freiburg2_360_hemisphere \
    -c ./example/tum_rgbd/TUM_RGBD_rgbd_2.yaml \
    --map-db mymap_line.msg \
    --auto-term
```

### KITTI 双目建图

```bash
./build/run_kitti_slam \
    -v ./orb_vocab/orb_vocab.dbow2 \
    -d /path/to/KITTI/odometry/dataset/sequences/00 \
    -c ./example/kitti/KITTI_stereo_00-02.yaml \
    --map-db kitti_00.msg \
    --auto-term
```

### EuRoC 单目建图

```bash
./build/run_euroc_slam \
    -v ./orb_vocab/orb_vocab.dbow2 \
    -d /path/to/EuRoC/MH_01_easy/mav0 \
    -c ./example/euroc/EuRoC_mono.yaml \
    --map-db euroc_mh01.msg \
    --auto-term
```

> `--auto-term`：跑完自动关闭 Viewer 并退出。去掉这个参数可以边跑边看。

---

## 2. 保存地图 (ROS2 方式)

### 用 ROS2 节点建图并保存

```bash
cd ros2
source /opt/ros/humble/setup.bash
source install/setup.bash
export LD_LIBRARY_PATH=$HOME/lib/SLAM/VIO/Structure-PLP-SLAM/build/lib:$HOME/lib/SLAM/VIO/Structure-PLP-SLAM/3rd/Pangolin/install/lib:$HOME/.local/lib:$LD_LIBRARY_PATH

# 第 6 个参数 = 地图保存路径（空字符串 = 不保存）
ros2 run plpslam_ros2 rgbd \
    ../orb_vocab/orb_vocab.dbow2 \
    ../example/tum_rgbd/TUM_RGBD_rgbd_2.yaml \
    0 \              # 0=点特征, 1=点+线
    0 \              # 0=不保存轨迹, 1=保存轨迹
    mymap.msg        # 地图数据库路径

# 在另一个终端发布数据
ros2 run plpslam_ros2 tum_publisher /home/ros/dataset/Tum-RGBD/rgbd_dataset_freiburg2_360_hemisphere --fast
```

---

## 3. 重定位 (Relocalization)

使用预先建好的地图进行重定位（不重新建图）：

### 单目图片重定位

```bash
./build/run_image_localization \
    -v ./orb_vocab/orb_vocab.dbow2 \
    -i /path/to/image/directory \
    -c ./example/tum_rgbd/TUM_RGBD_mono_2.yaml \
    -p mymap.msg
```

### 点 + 线特征重定位

```bash
./build/run_image_localization_point_line \
    -v ./orb_vocab/orb_vocab.dbow2 \
    -i /path/to/image/directory \
    -c ./example/tum_rgbd/TUM_RGBD_mono_2.yaml \
    -p mymap.msg
```

### 摄像头实时重定位

```bash
./build/run_camera_localization \
    -v ./orb_vocab/orb_vocab.dbow2 \
    -c ./example/tum_rgbd/TUM_RGBD_mono_2.yaml \
    -p mymap.msg \
    --camera-id 0
```

### 重定位 + 继续建图（mapping 模式）

```bash
./build/run_image_localization \
    -v ./orb_vocab/orb_vocab.dbow2 \
    -i /path/to/image/directory \
    -c ./example/tum_rgbd/TUM_RGBD_mono_2.yaml \
    -p mymap.msg \
    --mapping
```

加上 `--mapping` 标志后，SLAM 会在重定位的基础上继续扩展地图。

---

## 4. 保存轨迹

建图时可以同时保存轨迹用于评估：

```bash
./build/run_tum_rgbd_slam \
    -v ./orb_vocab/orb_vocab.dbow2 \
    -d /home/ros/dataset/Tum-RGBD/rgbd_dataset_freiburg2_360_hemisphere \
    -c ./example/tum_rgbd/TUM_RGBD_rgbd_2.yaml \
    --map-db mymap.msg \
    --eval-log
```

生成文件：
- `frame_trajectory.txt` — 每帧位姿
- `keyframe_trajectory.txt` — 关键帧位姿
- `track_times.txt` — 每帧跟踪耗时
- `mymap.msg` — 地图数据库

---

## 5. 典型工作流

```bash
# Step 1: 建图 + 保存
./build/run_tum_rgbd_slam \
    -v ./orb_vocab/orb_vocab.dbow2 \
    -d /home/ros/dataset/Tum-RGBD/rgbd_dataset_freiburg2_360_hemisphere \
    -c ./example/tum_rgbd/TUM_RGBD_rgbd_3.yaml \
    --map-db office.msg \
    --eval-log --auto-term

# Step 2: 重定位（使用同一场景的新图像）
./build/run_image_localization \
    -v ./orb_vocab/orb_vocab.dbow2 \
    -i /data/new_images \
    -c ./example/tum_rgbd/TUM_RGBD_mono_3.yaml \
    -p office.msg
```
