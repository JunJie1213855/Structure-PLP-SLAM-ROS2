#ifndef PLPSLAM_ROS2_MONO_SLAM_NODE_HPP
#define PLPSLAM_ROS2_MONO_SLAM_NODE_HPP

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/image.hpp"
#include "cv_bridge/cv_bridge.h"

#include "PLPSLAM/system.h"
#include "utility.hpp"

class MonoSlamNode : public rclcpp::Node
{
public:
    MonoSlamNode(const std::shared_ptr<PLPSLAM::config>& cfg,
                 const std::string& vocab_file_path,
                 bool b_use_line_tracking = false);
    ~MonoSlamNode();

private:
    using ImageMsg = sensor_msgs::msg::Image;

    void GrabMono(const ImageMsg::SharedPtr msg);

    std::shared_ptr<PLPSLAM::system> m_SLAM;
    rclcpp::Subscription<ImageMsg>::SharedPtr sub_image_;
    cv_bridge::CvImageConstPtr cv_ptr_;
};

#endif
