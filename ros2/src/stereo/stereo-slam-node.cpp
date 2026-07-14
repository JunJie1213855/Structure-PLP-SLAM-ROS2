#include "stereo-slam-node.hpp"

using std::placeholders::_1;

StereoSlamNode::StereoSlamNode(const std::shared_ptr<PLPSLAM::config>& cfg,
                           const std::string& vocab_file_path,
                           bool b_use_line_tracking,
                           bool eval_log,
                           const std::string& map_db_path,
                           const std::string& load_map_path,
                           bool mapping)
    : Node("PLPSLAM_STEREO"), eval_log_(eval_log), map_db_path_(map_db_path)
{
    m_SLAM = std::make_shared<PLPSLAM::system>(cfg, vocab_file_path, false, b_use_line_tracking);

    if (!load_map_path.empty())
    {
        RCLCPP_INFO(get_logger(), "Loading map: %s", load_map_path.c_str());
        m_SLAM->load_map_database(load_map_path);
        m_SLAM->startup(false);
        if (mapping) {
            m_SLAM->enable_mapping_module();
            RCLCPP_INFO(get_logger(), "Relocalization + mapping mode");
        } else {
            RCLCPP_INFO(get_logger(), "Relocalization-only mode");
        }
    }
    else
    {
        m_SLAM->startup();
    }

#ifdef USE_PANGOLIN_VIEWER
    viewer_ = std::make_unique<pangolin_viewer::viewer>(
        cfg, m_SLAM.get(), m_SLAM->get_frame_publisher(), m_SLAM->get_map_publisher());
    viewer_thread_ = std::make_unique<std::thread>([this]() {
        viewer_->run();
        if (m_SLAM->terminate_is_requested()) {
            while (m_SLAM->loop_BA_is_running())
                std::this_thread::sleep_for(std::chrono::microseconds(5000));
            rclcpp::shutdown();
        }
    });
#endif

    sub1_ = std::make_shared<message_filters::Subscriber<ImageMsg>>(this, "camera/left");
    sub2_ = std::make_shared<message_filters::Subscriber<ImageMsg>>(this, "camera/right");
    sync_ = std::make_shared<message_filters::Synchronizer<approximate_sync_policy>>(
        approximate_sync_policy(10), *sub1_, *sub2_);
    sync_->registerCallback(&StereoSlamNode::GrabFrame, this);

    RCLCPP_INFO(get_logger(), "PLP-SLAM Stereo node started");
}

StereoSlamNode::~StereoSlamNode()
{
#ifdef USE_PANGOLIN_VIEWER
    if (viewer_) viewer_->request_terminate();
    if (viewer_thread_ && viewer_thread_->joinable()) viewer_thread_->join();
#endif
    if (eval_log_) {
        m_SLAM->save_frame_trajectory("frame_trajectory.txt", "TUM");
        m_SLAM->save_keyframe_trajectory("keyframe_trajectory.txt", "TUM");
        std::ofstream ofs("track_times.txt");
        for (auto t : track_times_) ofs << t << std::endl;
    }
    if (!map_db_path_.empty()) m_SLAM->save_map_database(map_db_path_);
    if (!track_times_.empty()) {
        std::sort(track_times_.begin(), track_times_.end());
        auto total = std::accumulate(track_times_.begin(), track_times_.end(), 0.0);
        std::cout << "median tracking time: " << track_times_.at(track_times_.size()/2) << " [s]" << std::endl;
        std::cout << "mean tracking time: " << total/track_times_.size() << " [s]" << std::endl;
    }
    m_SLAM->shutdown();
    RCLCPP_INFO(get_logger(), "PLP-SLAM Stereo node shutdown");
}

void StereoSlamNode::GrabFrame(const ImageMsg::SharedPtr msg1, const ImageMsg::SharedPtr msg2)
{
    auto tp_1 = std::chrono::steady_clock::now();
    try {
        auto cv_ptr1 = cv_bridge::toCvShare(msg1, "bgr8");
        auto cv_ptr2 = cv_bridge::toCvShare(msg2);
        m_SLAM->feed_stereo_frame(cv_ptr1->image, cv_ptr2->image, Utility::StampToSec(msg1->header.stamp));
    } catch (cv_bridge::Exception& e) {
        RCLCPP_ERROR(get_logger(), "cv_bridge: %s", e.what());
        return;
    }
    auto tp_2 = std::chrono::steady_clock::now();
    track_times_.push_back(
        std::chrono::duration_cast<std::chrono::duration<double>>(tp_2 - tp_1).count());
}
