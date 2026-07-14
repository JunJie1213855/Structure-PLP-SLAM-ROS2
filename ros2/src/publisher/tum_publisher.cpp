#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <cv_bridge/cv_bridge.h>
#include <opencv2/opencv.hpp>
#include <fstream>
#include <sstream>
#include <thread>
#include <atomic>

class TumRgbdPublisher : public rclcpp::Node
{
    struct Frame { double rgb_ts, depth_ts; std::string rgb_path, depth_path; };

public:
    TumRgbdPublisher(const std::string& dataset_path, double rate_hz = 0.0)
        : Node("tum_rgbd_publisher"), rate_hz_(rate_hz)
    {
        rgb_pub_ = create_publisher<sensor_msgs::msg::Image>("camera/rgb", rclcpp::QoS(100));
        depth_pub_ = create_publisher<sensor_msgs::msg::Image>("camera/depth", rclcpp::QoS(100));

        std::ifstream assoc(dataset_path + "/associate.txt");
        std::string line;
        while (std::getline(assoc, line)) {
            std::istringstream iss(line);
            double rts, dts; std::string rp, dp;
            if (iss >> rts >> rp >> dts >> dp)
                frames_.push_back({rts, dts, dataset_path + "/" + rp, dataset_path + "/" + dp});
        }

        if (rate_hz_ > 0)
            RCLCPP_INFO(get_logger(), "Loaded %zu frames, publishing @ %.1f Hz", frames_.size(), rate_hz_);
        else if (rate_hz_ < 0)
            RCLCPP_INFO(get_logger(), "Loaded %zu frames, publishing MAX speed", frames_.size());
        else
            RCLCPP_INFO(get_logger(), "Loaded %zu frames, publishing realtime (30fps)", frames_.size());
    }

    void run()
    {
        using clock = std::chrono::steady_clock;
        auto start = clock::now();
        double ts0 = frames_[0].rgb_ts;
        double interval = (rate_hz_ > 0) ? (1.0 / rate_hz_) : 0.0;

        for (size_t i = 0; i < frames_.size() && rclcpp::ok(); ++i) {
            auto loop_start = clock::now();

            auto& f = frames_[i];
            cv::Mat rgb = cv::imread(f.rgb_path, cv::IMREAD_COLOR);
            cv::Mat depth = cv::imread(f.depth_path, cv::IMREAD_UNCHANGED);
            if (rgb.empty() || depth.empty()) continue;

            auto make_msg = [](const cv::Mat& img, double ts, const std::string& fid, const std::string& enc) {
                auto msg = cv_bridge::CvImage(std_msgs::msg::Header(), enc, img).toImageMsg();
                msg->header.stamp.sec = (int32_t)ts;
                msg->header.stamp.nanosec = (uint32_t)((ts - (int32_t)ts) * 1e9);
                msg->header.frame_id = fid;
                return msg;
            };

            rgb_pub_->publish(*make_msg(rgb, f.rgb_ts, "camera_rgb", "bgr8"));
            depth_pub_->publish(*make_msg(depth, f.depth_ts, "camera_depth", "mono16"));

            if (i % 200 == 0)
                RCLCPP_INFO(get_logger(), "%zu/%zu", i+1, frames_.size());

            // Rate control
            if (rate_hz_ > 0) {
                // Fixed rate mode: sleep until next publish time
                auto elapsed = std::chrono::duration<double>(clock::now() - loop_start).count();
                double sleep_t = interval - elapsed;
                if (sleep_t > 0)
                    std::this_thread::sleep_for(std::chrono::duration<double>(sleep_t));
            } else if (rate_hz_ == 0.0) {
                // Realtime mode: follow original timestamps
                double target = (f.rgb_ts - ts0);
                auto elapsed = std::chrono::duration<double>(clock::now() - start).count();
                double sleep_t = target - elapsed;
                if (sleep_t > 0)
                    std::this_thread::sleep_for(std::chrono::duration<double>(sleep_t));
            }
            // rate_hz_ < 0: max speed, no sleep
        }
        RCLCPP_INFO(get_logger(), "Done: %zu frames", frames_.size());
    }

private:
    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr rgb_pub_, depth_pub_;
    std::vector<Frame> frames_;
    double rate_hz_;  // 0=realtime, <0=max speed, >0=fixed Hz
};

int main(int argc, char** argv)
{
    rclcpp::init(argc, argv);
    if (argc < 2) {
        std::cerr << "Usage: tum_publisher DATASET [--fast | --rate N]" << std::endl
                  << "  (default)  realtime (follow original timestamps, ~30fps)" << std::endl
                  << "  --fast     max speed (no rate limit)" << std::endl
                  << "  --rate N   fixed rate in Hz (e.g. --rate 10)" << std::endl;
        return 1;
    }

    double rate = 0.0;  // default: realtime
    if (argc >= 3) {
        std::string opt = argv[2];
        if (opt == "--fast") rate = -1.0;
        else if (opt == "--rate" && argc >= 4) rate = std::stod(argv[3]);
    }

    auto node = std::make_shared<TumRgbdPublisher>(argv[1], rate);
    node->run();
    rclcpp::shutdown();
    return 0;
}
