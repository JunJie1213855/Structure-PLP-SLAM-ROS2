#include "mono-slam-node.hpp"

using std::placeholders::_1;

MonoSlamNode::MonoSlamNode(const std::shared_ptr<PLPSLAM::config>& cfg,
                           const std::string& vocab_file_path,
                           bool b_use_line_tracking)
    : Node("PLPSLAM_Mono")
{
    m_SLAM = std::make_shared<PLPSLAM::system>(cfg, vocab_file_path, false, b_use_line_tracking);
    m_SLAM->startup();

    sub_image_ = this->create_subscription<ImageMsg>(
        "camera/image_raw", 10, std::bind(&MonoSlamNode::GrabMono, this, _1));

    RCLCPP_INFO(this->get_logger(), "PLP-SLAM Monocular node started");
}

MonoSlamNode::~MonoSlamNode()
{
    m_SLAM->shutdown();
    RCLCPP_INFO(this->get_logger(), "PLP-SLAM Monocular node shutdown");
}

void MonoSlamNode::GrabMono(const ImageMsg::SharedPtr msg)
{
    try
    {
        cv_ptr_ = cv_bridge::toCvShare(msg);
    }
    catch (cv_bridge::Exception& e)
    {
        RCLCPP_ERROR(this->get_logger(), "cv_bridge exception: %s", e.what());
        return;
    }

    m_SLAM->feed_monocular_frame(cv_ptr_->image, Utility::StampToSec(msg->header.stamp));
}
