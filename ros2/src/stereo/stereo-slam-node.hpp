#ifndef PLPSLAM_ROS2_STEREO_SLAM_NODE_HPP
#define PLPSLAM_ROS2_STEREO_SLAM_NODE_HPP

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/image.hpp"
#include "message_filters/subscriber.h"
#include "message_filters/synchronizer.h"
#include "message_filters/sync_policies/approximate_time.h"
#include "cv_bridge/cv_bridge.h"

#include "PLPSLAM/system.h"
#include "utility.hpp"

class StereoSlamNode : public rclcpp::Node
{
public:
    StereoSlamNode(const std::shared_ptr<PLPSLAM::config>& cfg,
                   const std::string& vocab_file_path,
                   bool b_use_line_tracking = false);
    ~StereoSlamNode();

private:
    using ImageMsg = sensor_msgs::msg::Image;
    using approximate_sync_policy = message_filters::sync_policies::ApproximateTime<ImageMsg, ImageMsg>;

    void GrabStereo(const ImageMsg::SharedPtr msgLeft, const ImageMsg::SharedPtr msgRight);

    std::shared_ptr<PLPSLAM::system> m_SLAM;
    std::shared_ptr<message_filters::Subscriber<ImageMsg>> left_sub_;
    std::shared_ptr<message_filters::Subscriber<ImageMsg>> right_sub_;
    std::shared_ptr<message_filters::Synchronizer<approximate_sync_policy>> sync_;

    cv_bridge::CvImageConstPtr cv_ptrLeft_;
    cv_bridge::CvImageConstPtr cv_ptrRight_;
};

#endif
