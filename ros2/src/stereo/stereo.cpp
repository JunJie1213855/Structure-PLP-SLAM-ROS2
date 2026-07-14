#include <iostream>
#include "rclcpp/rclcpp.hpp"
#include "stereo-slam-node.hpp"
#include "PLPSLAM/config.h"

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::cerr << "Usage: ros2 run plpslam_ros2 stereo path_to_vocab path_to_config [use_line_tracking]" << std::endl;
        return 1;
    }

    rclcpp::init(argc, argv);

    std::string vocab_path = argv[1];
    std::string config_path = argv[2];
    bool use_line = (argc >= 4) ? std::stoi(argv[3]) : false;

    auto cfg = std::make_shared<PLPSLAM::config>(config_path);
    auto node = std::make_shared<StereoSlamNode>(cfg, vocab_path, use_line);

    rclcpp::spin(node);
    rclcpp::shutdown();

    return 0;
}
