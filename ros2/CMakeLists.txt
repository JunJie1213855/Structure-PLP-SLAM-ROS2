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

# ---- PLPSLAM library (linked directly from build tree) ----
set(PLPSLAM_ROOT "$ENV{HOME}/lib/SLAM/VIO/Structure-PLP-SLAM")
set(PLPSLAM_LIB "${PLPSLAM_ROOT}/build/lib/libPLPSLAM.so")
set(_3rd "${PLPSLAM_ROOT}/3rd")
set(PLPSLAM_INCLUDE_DIR "${PLPSLAM_ROOT}/src"
                        "${_3rd}/json/include"
                        "${_3rd}/spdlog/include")

# ---- Common compile definitions ----
set(PLPSLAM_DEFS USE_DBOW2)

# ---- Monocular ----
add_executable(mono
  src/mono/mono.cpp
  src/mono/mono-slam-node.cpp
)
ament_target_dependencies(mono rclcpp sensor_msgs cv_bridge Pangolin)
target_include_directories(mono PRIVATE include ${PLPSLAM_INCLUDE_DIR} ${DBoW2_INCLUDE_DIRS})
target_compile_definitions(mono PRIVATE ${PLPSLAM_DEFS})
target_link_libraries(mono
  ${PLPSLAM_LIB}
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
target_include_directories(rgbd PRIVATE include ${PLPSLAM_INCLUDE_DIR} ${DBoW2_INCLUDE_DIRS})
target_compile_definitions(rgbd PRIVATE ${PLPSLAM_DEFS})
target_link_libraries(rgbd
  ${PLPSLAM_LIB}
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
target_include_directories(stereo PRIVATE include ${PLPSLAM_INCLUDE_DIR} ${DBoW2_INCLUDE_DIRS})
target_compile_definitions(stereo PRIVATE ${PLPSLAM_DEFS})
target_link_libraries(stereo
  ${PLPSLAM_LIB}
  ${Pangolin_LIBRARIES}
  ${OpenCV_LIBS}
  Eigen3::Eigen
  yaml-cpp
  ${DBoW2_LIBS}
)

install(TARGETS mono rgbd stereo
  DESTINATION lib/${PROJECT_NAME})

install(DIRECTORY launch/
  DESTINATION share/${PROJECT_NAME}/launch)

ament_package()
