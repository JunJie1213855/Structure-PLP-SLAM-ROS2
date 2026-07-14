# Structure PLP-SLAM 编译指南 (Ubuntu 22.04 + ROS2 Humble)

## 系统依赖

```bash
# 基础工具
sudo apt install -y build-essential cmake git libeigen3-dev libyaml-cpp-dev

# OpenCV (带 contrib)
sudo apt install -y libopencv-dev libopencv-contrib-dev

# SuiteSparse + CXSparse (g2o 依赖)
sudo apt install -y libsuitesparse-dev libcxsparse-dev liblapack-dev libblas-dev

# OpenGL (Pangolin 依赖)
sudo apt install -y libgl1-mesa-dev libglew-dev libegl-dev libwayland-dev

# ffmpeg (Pangolin 可选)
sudo apt install -y libavcodec-dev libavformat-dev libavutil-dev libswscale-dev

# ROS2 Humble g2o
sudo apt install -y ros-humble-libg2o
```

---

## 第三方库编译

### DBoW2（安装到 `~/.local/`）

```bash
cd /tmp
git clone --depth=1 https://github.com/dorian3d/DBoW2.git
mkdir DBoW2/build && cd DBoW2/build

cmake .. \
    -DCMAKE_INSTALL_PREFIX=$HOME/.local \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5

make -j$(nproc)
cmake --install .
```

> **注意：** 本项目 fork 的 DBoW2 在 `TemplatedVocabulary.h` 中添加了 `saveToBinaryFile()` / `loadFromBinaryFile()` 方法以支持二进制 ORB 词汇文件 `orb_vocab.dbow2`。如果使用原始 DBoW2，需修改 `system.cc` 中 `loadFromBinaryFile` → `load`，并将词汇转为 YAML 格式。

### Pangolin **v0.6**（`3rd/Pangolin/`，已包含在仓库中）

> **关键：** 必须使用 **v0.6**（仓库已自带），不能使用 v0.8+。v0.8 的 `Follow()` 不在每帧更新视图矩阵，会导致可视化异常。

Pangolin v0.6 源码已包含在 `3rd/Pangolin/` 目录中，预编译的 `install/` 也已在仓库内。如需重新编译：

```bash
cd 3rd/Pangolin
rm -rf build install && mkdir build && cd build

cmake .. \
    -DCMAKE_INSTALL_PREFIX=$PWD/../install \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_EXAMPLES=OFF \
    -DBUILD_TESTS=OFF \
    -DBUILD_TOOLS=OFF \
    -DBUILD_PYTHON=OFF \
    -DBUILD_PANGOLIN_PYTHON=OFF

make -j$(nproc)
cmake --install .
```

> 如果仓库自带的 `install/` 库文件因系统差异无法使用，按上述步骤重新编译即可。

---

## 编译 PLP-SLAM

```bash
cd /home/ros/lib/SLAM/VIO/Structure-PLP-SLAM
mkdir -p build && cd build

cmake .. \
    -DDBoW2_DIR=$HOME/.local/lib/cmake/DBoW2 \
    -DPangolin_DIR=$PWD/../3rd/Pangolin/install/lib/cmake/Pangolin \
    -DCMAKE_BUILD_TYPE=Release \
    -DUSE_PANGOLIN_VIEWER=ON \
    -DBUILD_EXAMPLES=ON \
    -DBUILD_WITH_MARCH_NATIVE=OFF

make -j$(nproc)
```

### CMake 选项

| 选项 | 默认值 | 说明 |
|------|--------|------|
| `DBoW2_DIR` | - | DBoW2 cmake config 路径 |
| `Pangolin_DIR` | - | Pangolin cmake config 路径 |
| `USE_PANGOLIN_VIEWER` | ON | 启用 Pangolin 可视化 |
| `BUILD_EXAMPLES` | ON | 编译示例可执行文件 |
| `BUILD_WITH_MARCH_NATIVE` | OFF | `-march=native` 优化 |
| `USE_OPENMP` | OFF | OpenMP 并行加速 |
| `CMAKE_BUILD_TYPE` | Release | Debug / Release |

---

## 编译 ROS2 包

```bash
cd ros2
source /opt/ros/humble/setup.bash

colcon build --symlink-install --cmake-args \
    -DDBoW2_DIR=$HOME/.local/lib/cmake/DBoW2 \
    -DPangolin_DIR=$PWD/../3rd/Pangolin/install/lib/cmake/Pangolin \
    -DCMAKE_BUILD_TYPE=Release
```

---

## 关于源码修改

本项目在原始代码基础上做了以下修改以支持 Ubuntu 22.04 + OpenCV 4.x + Pangolin v0.6：

| 文件 | 修改 |
|------|------|
| `CMakeLists.txt` | 添加 `CMP0052 OLD` 策略，允许源码路径出现在 INTERFACE_INCLUDE_DIRECTORIES |
| `src/PLPSLAM/CMakeLists.txt` | 修复 DBoW2 头文件列表分号问题 |
| `src/PLPSLAM/feature/line_extractor.cc` | `CV_INTER_LINEAR` → `cv::INTER_LINEAR` (OpenCV 4.x) |
| `src/PLPSLAM/publish/frame_publisher.cc` | 添加 `#include <iomanip>` |
| `src/pangolin_viewer/color_scheme.cc` | 添加 `#include <stdexcept>` |
| `src/pangolin_viewer/CMakeLists.txt` | 适配 Pangolin v0.6 `find_package` |
| `src/pangolin_viewer/viewer.cc` | `follow_camera_` 初始值改为 `false`，匹配 ORB-SLAM3 状态机 |
| `example/run_slam_planeSeg.cc` | 启用线特征 (`b_use_line_tracking=true`) |
