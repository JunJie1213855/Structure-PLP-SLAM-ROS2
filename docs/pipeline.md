# Structure PLP-SLAM Pipeline: 建图 → 保存 → 重定位

## 环境

```bash
cd /home/ros/lib/SLAM/VIO/Structure-PLP-SLAM
export LD_LIBRARY_PATH=$PWD/build/lib:$PWD/3rd/Pangolin/install/lib:$HOME/.local/lib:$LD_LIBRARY_PATH
```

---

## 1. 建图 + 保存地图

`--map-db` 指定保存路径，`--eval-log` 同时保存轨迹用于精度评估。

### TUM RGB-D 单目

```bash
./build/run_tum_rgbd_slam \
    -v ./orb_vocab/orb_vocab.dbow2 \
    -d /home/ros/dataset/Tum-RGBD/rgbd_dataset_freiburg2_360_hemisphere \
    -c ./example/tum_rgbd/TUM_RGBD_mono_2.yaml \
    --map-db mymap.msg \
    --eval-log --auto-term
```

### TUM RGB-D 深度相机

```bash
./build/run_tum_rgbd_slam \
    -v ./orb_vocab/orb_vocab.dbow2 \
    -d /home/ros/dataset/Tum-RGBD/rgbd_dataset_freiburg2_360_hemisphere \
    -c ./example/tum_rgbd/TUM_RGBD_rgbd_2.yaml \
    --map-db mymap_rgbd.msg \
    --eval-log --auto-term
```

### 深度相机 + 点 + 线特征

```bash
./build/run_tum_rgbd_slam_with_line \
    -v ./orb_vocab/orb_vocab.dbow2 \
    -d /home/ros/dataset/Tum-RGBD/rgbd_dataset_freiburg2_360_hemisphere \
    -c ./example/tum_rgbd/TUM_RGBD_rgbd_2.yaml \
    --map-db mymap_line.msg \
    --eval-log --auto-term
```

### KITTI 双目

```bash
./build/run_kitti_slam \
    -v ./orb_vocab/orb_vocab.dbow2 \
    -d /path/to/KITTI/odometry/dataset/sequences/00 \
    -c ./example/kitti/KITTI_stereo_00-02.yaml \
    --map-db kitti_00.msg \
    --eval-log --auto-term
```

### EuRoC 单目

```bash
./build/run_euroc_slam \
    -v ./orb_vocab/orb_vocab.dbow2 \
    -d /path/to/EuRoC/MH_01_easy/mav0 \
    -c ./example/euroc/EuRoC_mono.yaml \
    --map-db euroc_mh01.msg \
    --eval-log --auto-term
```

### ROS2 节点建图

```bash
source /opt/ros/humble/setup.bash
source ros2/install/setup.bash

# 终端 1: SLAM 建图 + 保存
ros2 run plpslam_ros2 rgbd \
    ./orb_vocab/orb_vocab.dbow2 \
    ./example/tum_rgbd/TUM_RGBD_rgbd_2.yaml \
    1 1 mymap.msg

# 终端 2: 高速发布数据
ros2 run plpslam_ros2 tum_publisher /home/ros/dataset/Tum-RGBD/rgbd_dataset_freiburg2_360_hemisphere --rate 30
```

> ROS2 参数 (8 个)：`<vocab> <config> <use_line> <eval_log> <save_map> <load_map> <mapping>`

### ROS2 重定位（加载已有地图）

```bash
# 纯重定位模式（不修改地图）
ros2 run plpslam_ros2 rgbd \
    ./orb_vocab/orb_vocab.dbow2 \
    ./example/tum_rgbd/TUM_RGBD_rgbd_2.yaml \
    1 0 "" mymap.msg 0
```

```bash
# 重定位 + 继续建图
ros2 run plpslam_ros2 rgbd \
    ../orb_vocab/orb_vocab.dbow2 \
    ../example/tum_rgbd/TUM_RGBD_rgbd_2.yaml \
    1 0 "" mymap.msg 1
```

> 参数 6 (`load_map`)：已有地图路径；参数 7 (`mapping`)：`0`=仅重定位，`1`=重定位+建图

### 生成文件

| 文件 | 说明 |
|------|------|
| `*.msg` | 地图数据库（关键帧、地图点、BoW、图结构） |
| `frame_trajectory.txt` | 每帧相机位姿（TUM 格式） |
| `keyframe_trajectory.txt` | 关键帧位姿 |
| `track_times.txt` | 每帧跟踪耗时 |

---

## 2. 重定位（加载已有地图）

用预先建好的地图在新图像上定位，**不修改地图**。

### 图片序列重定位

```bash
./build/run_image_localization \
    -v ./orb_vocab/orb_vocab.dbow2 \
    -i /home/ros/dataset/Tum-RGBD/rgbd_dataset_freiburg2_360_hemisphere/rgb \
    -c ./example/tum_rgbd/TUM_RGBD_mono_2.yaml \
    -p mymap.msg
```

### 点 + 线重定位

```bash
./build/run_image_localization_point_line \
    -v ./orb_vocab/orb_vocab.dbow2 \
    -i /home/ros/dataset/Tum-RGBD/rgbd_dataset_freiburg2_360_hemisphere/rgb \
    -c ./example/tum_rgbd/TUM_RGBD_mono_2.yaml \
    -p mymap.msg
```

### 实时摄像头重定位

```bash
./build/run_camera_localization \
    -v ./orb_vocab/orb_vocab.dbow2 \
    -c ./example/tum_rgbd/TUM_RGBD_mono_2.yaml \
    -p mymap.msg \
    --camera-id 0
```

### 重定位 + 继续建图

加上 `--mapping` 标志，在重定位的基础上同时扩展地图：

```bash
./build/run_image_localization \
    -v ./orb_vocab/orb_vocab.dbow2 \
    -i /home/ros/dataset/Tum-RGBD/rgbd_dataset_freiburg2_360_hemisphere/rgb \
    -c ./example/tum_rgbd/TUM_RGBD_mono_2.yaml \
    -p mymap.msg \
    --mapping
```

---

## 3. 完整工作流示例

```bash
# Step 1: 建立地图
./build/run_tum_rgbd_slam \
    -v ./orb_vocab/orb_vocab.dbow2 \
    -d /home/ros/dataset/Tum-RGBD/rgbd_dataset_freiburg2_360_hemisphere \
    -c ./example/tum_rgbd/TUM_RGBD_rgbd_3.yaml \
    --map-db office.msg \
    --eval-log --auto-term
# 输出: office.msg, frame_trajectory.txt, keyframe_trajectory.txt

# Step 2: 在新数据上重定位（不修改地图）
./build/run_image_localization \
    -v ./orb_vocab/orb_vocab.dbow2 \
    -i /home/ros/dataset/Tum-RGBD/rgbd_dataset_freiburg2_360_hemisphere/rgb \
    -c ./example/tum_rgbd/TUM_RGBD_mono_3.yaml \
    -p office.msg
```

```bash
./build/run_image_localization \
    -v ./orb_vocab/orb_vocab.dbow2 \
    -i /data/TUM_RGBD/rgbd_dataset_freiburg2_360_hemisphere/rgb \
    -c ./example/tum_rgbd/TUM_RGBD_mono_3.yaml \
    -p office.msg



./build/run_image_localization \
    -v ./orb_vocab/orb_vocab.dbow2 \
    -i /home/ros/dataset/Tum-RGBD/rgbd_dataset_freiburg2_360_hemisphere/rgb  \
    -c ./example/tum_rgbd/TUM_RGBD_mono_2.yaml \
    -p room_map.msg \
    --mapping
```



---

## 4. 选项速查

| 选项 | 说明 |
|------|------|
| `--map-db <path>` | 保存地图到文件 |
| `-p, --map-db <path>` | (定位模式) 加载已有地图 |
| `--eval-log` | 保存轨迹 + 跟踪耗时 |
| `--auto-term` | 完成后自动退出 |
| `--mapping` | 重定位时同时扩展地图 |
| `--camera-id <n>` | 摄像头设备 ID（`/dev/video<n>`） |


---

## 5. ROS2 参数速查

所有节点 (`mono` / `rgbd` / `stereo`) 共用相同的 8 个位置参数：

| 位置 | 参数名 | 默认值 | 说明 |
|------|--------|--------|------|
| 1 | `vocab` | 必填 | ORB 词汇文件路径 |
| 2 | `config` | 必填 | YAML 配置文件路径 |
| 3 | `use_line` | `0` | `0`=仅点特征，`1`=点+线特征 |
| 4 | `eval_log` | `0` | `1`=保存轨迹 (`frame_trajectory.txt` 等) |
| 5 | `save_map` | `""` | 退出时保存地图的路径 |
| 6 | `load_map` | `""` | 启动时加载已有地图的路径 |
| 7 | `mapping` | `1` | 仅在 `load_map` 非空时有效：`0`=纯重定位，`1`=重定位+建图 |

### 示例

```bash
# 建图
ros2 run plpslam_ros2 rgbd vocab orb_vocab config.yaml 1 1 mymap.msg

# 重定位（不建图）
ros2 run plpslam_ros2 rgbd vocab orb_vocab config.yaml 0 0 "" mymap.msg 0

# 重定位 + 继续建图
ros2 run plpslam_ros2 rgbd vocab orb_vocab config.yaml 0 0 "" mymap.msg 1
```