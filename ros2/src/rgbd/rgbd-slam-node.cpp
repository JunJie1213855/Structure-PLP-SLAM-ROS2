#include "rgbd-slam-node.hpp"

using std::placeholders::_1;

RgbdSlamNode::RgbdSlamNode(const std::shared_ptr<PLPSLAM::config>& cfg,
                           const std::string& vocab_file_path,
                           bool b_use_line_tracking)
    : Node("PLPSLAM_RGBD")
{
    m_SLAM = std::make_shared<PLPSLAM::system>(cfg, vocab_file_path, false, b_use_line_tracking);
    m_SLAM->startup();

    rgb_sub_ = std::make_shared<message_filters::Subscriber<ImageMsg>>(
        std::shared_ptr<rclcpp::Node>(this), "camera/rgb");
    depth_sub_ = std::make_shared<message_filters::Subscriber<ImageMsg>>(
        std::shared_ptr<rclcpp::Node>(this), "camera/depth");

    sync_ = std::make_shared<message_filters::Synchronizer<approximate_sync_policy>>(
        approximate_sync_policy(10), *rgb_sub_, *depth_sub_);
    sync_->registerCallback(&RgbdSlamNode::GrabRGBD, this);

    RCLCPP_INFO(this->get_logger(), "PLP-SLAM RGB-D node started");
}

RgbdSlamNode::~RgbdSlamNode()
{
    m_SLAM->shutdown();
    RCLCPP_INFO(this->get_logger(), "PLP-SLAM RGB-D node shutdown");
}

void RgbdSlamNode::GrabRGBD(const ImageMsg::SharedPtr msgRGB, const ImageMsg::SharedPtr msgD)
{
    try
    {
        cv_ptrRGB_ = cv_bridge::toCvShare(msgRGB);
    }
    catch (cv_bridge::Exception& e)
    {
        RCLCPP_ERROR(this->get_logger(), "cv_bridge exception: %s", e.what());
        return;
    }

    try
    {
        cv_ptrD_ = cv_bridge::toCvShare(msgD);
    }
    catch (cv_bridge::Exception& e)
    {
        RCLCPP_ERROR(this->get_logger(), "cv_bridge exception: %s", e.what());
        return;
    }

    m_SLAM->feed_RGBD_frame(cv_ptrRGB_->image, cv_ptrD_->image,
                            Utility::StampToSec(msgRGB->header.stamp));
}
