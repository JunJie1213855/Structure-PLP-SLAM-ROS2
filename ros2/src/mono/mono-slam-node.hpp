#ifndef PLPSLAM_ROS2_MONO_SLAM_NODE_HPP
#define PLPSLAM_ROS2_MONO_SLAM_NODE_HPP

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/image.hpp"
#include "cv_bridge/cv_bridge.h"

#include "PLPSLAM/system.h"
#include "PLPSLAM/config.h"
#include "utility.hpp"

#ifdef USE_PANGOLIN_VIEWER
#include "pangolin_viewer/viewer.h"
#endif

#include <thread>
#include <memory>
#include <vector>
#include <fstream>
#include <algorithm>
#include <numeric>

class MonoSlamNode : public rclcpp::Node
{
public:
    MonoSlamNode(const std::shared_ptr<PLPSLAM::config>& cfg,
                 const std::string& vocab_file_path,
                 bool b_use_line_tracking,
                 bool eval_log = false,
                 const std::string& map_db_path = "",
                 const std::string& load_map_path = "",
                 bool mapping = true);
    ~MonoSlamNode();

private:
    using ImageMsg = sensor_msgs::msg::Image;
    void GrabMono(const ImageMsg::SharedPtr msg);

    std::shared_ptr<PLPSLAM::system> m_SLAM;
    rclcpp::Subscription<ImageMsg>::SharedPtr sub_image_;
    bool eval_log_;
    std::string map_db_path_;
    std::vector<double> track_times_;

#ifdef USE_PANGOLIN_VIEWER
    std::unique_ptr<pangolin_viewer::viewer> viewer_;
    std::unique_ptr<std::thread> viewer_thread_;
#endif
};

#endif
