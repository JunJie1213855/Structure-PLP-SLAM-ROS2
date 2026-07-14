#ifndef PLPSLAM_ROS2_RGBD_SLAM_NODE_HPP
#define PLPSLAM_ROS2_RGBD_SLAM_NODE_HPP

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/image.hpp"
#include "message_filters/subscriber.h"
#include "message_filters/synchronizer.h"
#include "message_filters/sync_policies/approximate_time.h"
#include "cv_bridge/cv_bridge.h"

#include "PLPSLAM/system.h"
#include "utility.hpp"

class RgbdSlamNode : public rclcpp::Node
{
public:
    RgbdSlamNode(const std::shared_ptr<PLPSLAM::config>& cfg,
                 const std::string& vocab_file_path,
                 bool b_use_line_tracking = false);
    ~RgbdSlamNode();

private:
    using ImageMsg = sensor_msgs::msg::Image;
    using approximate_sync_policy = message_filters::sync_policies::ApproximateTime<ImageMsg, ImageMsg>;

    void GrabRGBD(const ImageMsg::SharedPtr msgRGB, const ImageMsg::SharedPtr msgD);

    std::shared_ptr<PLPSLAM::system> m_SLAM;
    std::shared_ptr<message_filters::Subscriber<ImageMsg>> rgb_sub_;
    std::shared_ptr<message_filters::Subscriber<ImageMsg>> depth_sub_;
    std::shared_ptr<message_filters::Synchronizer<approximate_sync_policy>> sync_;

    cv_bridge::CvImageConstPtr cv_ptrRGB_;
    cv_bridge::CvImageConstPtr cv_ptrD_;
};

#endif
