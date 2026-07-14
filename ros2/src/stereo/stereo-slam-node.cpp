#include "stereo-slam-node.hpp"

using std::placeholders::_1;

StereoSlamNode::StereoSlamNode(const std::shared_ptr<PLPSLAM::config>& cfg,
                               const std::string& vocab_file_path,
                               bool b_use_line_tracking)
    : Node("PLPSLAM_Stereo")
{
    m_SLAM = std::make_shared<PLPSLAM::system>(cfg, vocab_file_path, false, b_use_line_tracking);
    m_SLAM->startup();

    left_sub_ = std::make_shared<message_filters::Subscriber<ImageMsg>>(
        std::shared_ptr<rclcpp::Node>(this), "camera/left");
    right_sub_ = std::make_shared<message_filters::Subscriber<ImageMsg>>(
        std::shared_ptr<rclcpp::Node>(this), "camera/right");

    sync_ = std::make_shared<message_filters::Synchronizer<approximate_sync_policy>>(
        approximate_sync_policy(10), *left_sub_, *right_sub_);
    sync_->registerCallback(&StereoSlamNode::GrabStereo, this);

    RCLCPP_INFO(this->get_logger(), "PLP-SLAM Stereo node started");
}

StereoSlamNode::~StereoSlamNode()
{
    m_SLAM->shutdown();
    RCLCPP_INFO(this->get_logger(), "PLP-SLAM Stereo node shutdown");
}

void StereoSlamNode::GrabStereo(const ImageMsg::SharedPtr msgLeft, const ImageMsg::SharedPtr msgRight)
{
    try
    {
        cv_ptrLeft_ = cv_bridge::toCvShare(msgLeft);
    }
    catch (cv_bridge::Exception& e)
    {
        RCLCPP_ERROR(this->get_logger(), "cv_bridge exception: %s", e.what());
        return;
    }

    try
    {
        cv_ptrRight_ = cv_bridge::toCvShare(msgRight);
    }
    catch (cv_bridge::Exception& e)
    {
        RCLCPP_ERROR(this->get_logger(), "cv_bridge exception: %s", e.what());
        return;
    }

    m_SLAM->feed_stereo_frame(cv_ptrLeft_->image, cv_ptrRight_->image,
                              Utility::StampToSec(msgLeft->header.stamp));
}
