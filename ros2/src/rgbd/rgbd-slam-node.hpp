#ifndef PLPSLAM_ROS2_RGBD_SLAM_NODE_HPP
#define PLPSLAM_ROS2_RGBD_SLAM_NODE_HPP

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/image.hpp"
#include "message_filters/subscriber.h"
#include "message_filters/synchronizer.h"
#include "message_filters/sync_policies/approximate_time.h"
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

class RgbdSlamNode : public rclcpp::Node
{
public:
    RgbdSlamNode(const std::shared_ptr<PLPSLAM::config>& cfg,
                 const std::string& vocab_file_path,
                 bool b_use_line_tracking,
                 bool eval_log = false,
                 const std::string& map_db_path = "",
                 const std::string& load_map_path = "",
                 bool mapping = true);
    ~RgbdSlamNode();

private:
    using ImageMsg = sensor_msgs::msg::Image;
    using approximate_sync_policy = message_filters::sync_policies::ApproximateTime<ImageMsg, ImageMsg>;

    void GrabFrame(const ImageMsg::SharedPtr msg1, const ImageMsg::SharedPtr msg2);

    std::shared_ptr<PLPSLAM::system> m_SLAM;
    std::shared_ptr<message_filters::Subscriber<ImageMsg>> sub1_, sub2_;
    std::shared_ptr<message_filters::Synchronizer<approximate_sync_policy>> sync_;
    bool eval_log_;
    std::string map_db_path_;
    std::vector<double> track_times_;

#ifdef USE_PANGOLIN_VIEWER
    std::unique_ptr<pangolin_viewer::viewer> viewer_;
    std::unique_ptr<std::thread> viewer_thread_;
#endif
};

#endif
