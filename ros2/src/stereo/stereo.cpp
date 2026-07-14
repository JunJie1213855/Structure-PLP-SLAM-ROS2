#include <iostream>
#include "rclcpp/rclcpp.hpp"
#include "stereo-slam-node.hpp"
#include "PLPSLAM/config.h"

int main(int argc, char** argv)
{
    if (argc < 3) {
        std::cerr << "Usage: ros2 run plpslam_ros2 stereo VOCAB CONFIG "
                     "[use_line=0] [eval_log=0] [save_map=\"\"] [load_map=\"\"] [mapping=1]" << std::endl;
        return 1;
    }
    rclcpp::init(argc, argv);
    auto cfg = std::make_shared<PLPSLAM::config>(argv[2]);
    auto node = std::make_shared<StereoSlamNode>(
        cfg, argv[1],
        (argc>=4) ? std::stoi(argv[3]) : 0,
        (argc>=5) ? std::stoi(argv[4]) : 0,
        (argc>=6) ? argv[5] : "",
        (argc>=7) ? argv[6] : "",
        (argc>=8) ? std::stoi(argv[7]) : 1
    );
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
