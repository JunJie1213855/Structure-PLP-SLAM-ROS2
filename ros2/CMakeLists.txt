cmake_minimum_required(VERSION 3.8)
project(plpslam_ros2)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(message_filters REQUIRED)
find_package(Pangolin REQUIRED)
find_package(OpenCV REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(DBoW2 REQUIRED)

# ---- Common include dirs ----
include_directories(
  include
  /opt/ros/humble/include
  $ENV{HOME}/lib/SLAM/VIO/Structure-PLP-SLAM/src
  $ENV{HOME}/lib/SLAM/VIO/Structure-PLP-SLAM/src/pangolin_viewer
  $ENV{HOME}/lib/SLAM/VIO/Structure-PLP-SLAM/3rd/json/include
  $ENV{HOME}/lib/SLAM/VIO/Structure-PLP-SLAM/3rd/spdlog/include
  ${DBoW2_INCLUDE_DIRS}
)

# ---- PLPSLAM libraries ----
set(PLPSLAM_ROOT "$ENV{HOME}/lib/SLAM/VIO/Structure-PLP-SLAM")
set(PLPSLAM_LIB "${PLPSLAM_ROOT}/build/lib/libPLPSLAM.so")
set(PLPSLAM_PANGOLIN_VIEWER_LIB "${PLPSLAM_ROOT}/build/lib/libpangolin_viewer.so")

# ---- Common compile definitions ----
add_definitions(-DUSE_DBOW2 -DUSE_PANGOLIN_VIEWER)

# ---- Monocular ----
add_executable(mono
  src/mono/mono.cpp
  src/mono/mono-slam-node.cpp
)
ament_target_dependencies(mono rclcpp sensor_msgs cv_bridge Pangolin)
target_link_libraries(mono
  ${PLPSLAM_LIB}
  ${PLPSLAM_PANGOLIN_VIEWER_LIB}
  ${Pangolin_LIBRARIES}
  ${OpenCV_LIBS}
  Eigen3::Eigen
  yaml-cpp
  ${DBoW2_LIBS}
)

# ---- RGB-D ----
add_executable(rgbd
  src/rgbd/rgbd.cpp
  src/rgbd/rgbd-slam-node.cpp
)
ament_target_dependencies(rgbd rclcpp sensor_msgs cv_bridge message_filters Pangolin)
target_link_libraries(rgbd
  ${PLPSLAM_LIB}
  ${PLPSLAM_PANGOLIN_VIEWER_LIB}
  ${Pangolin_LIBRARIES}
  ${OpenCV_LIBS}
  Eigen3::Eigen
  yaml-cpp
  ${DBoW2_LIBS}
)

# ---- Stereo ----
add_executable(stereo
  src/stereo/stereo.cpp
  src/stereo/stereo-slam-node.cpp
)
ament_target_dependencies(stereo rclcpp sensor_msgs cv_bridge message_filters Pangolin)
target_link_libraries(stereo
  ${PLPSLAM_LIB}
  ${PLPSLAM_PANGOLIN_VIEWER_LIB}
  ${Pangolin_LIBRARIES}
  ${OpenCV_LIBS}
  Eigen3::Eigen
  yaml-cpp
  ${DBoW2_LIBS}
)

# ---- TUM Dataset Publisher ----
add_executable(tum_publisher src/publisher/tum_publisher.cpp)
ament_target_dependencies(tum_publisher rclcpp sensor_msgs cv_bridge)
target_link_libraries(tum_publisher ${OpenCV_LIBS})

install(TARGETS mono rgbd stereo tum_publisher
  DESTINATION lib/${PROJECT_NAME})

install(DIRECTORY launch/
  DESTINATION share/${PROJECT_NAME}/launch)

ament_package()
