#include <iostream>
#include "rclcpp/rclcpp.hpp"
#include "mono-slam-node.hpp"
#include "PLPSLAM/config.h"

int main(int argc, char** argv)
{
    // Usage: mono vocab config [use_line] [eval_log] [map_db_save] [map_db_load] [mapping]
    if (argc < 3) {
        std::cerr << "Usage: ros2 run plpslam_ros2 mono VOCAB CONFIG "
                     "[use_line=0] [eval_log=0] [save_map=\"\"] [load_map=\"\"] [mapping=1]" << std::endl;
        std::cerr << "  load_map: path to pre-built .msg file for relocalization" << std::endl;
        std::cerr << "  mapping=1: continue mapping (with load_map), =0: localization only" << std::endl;
        return 1;
    }
    rclcpp::init(argc, argv);
    auto cfg = std::make_shared<PLPSLAM::config>(argv[2]);
    auto node = std::make_shared<MonoSlamNode>(
        cfg, argv[1],
        (argc>=4) ? std::stoi(argv[3]) : 0,   // use_line
        (argc>=5) ? std::stoi(argv[4]) : 0,   // eval_log
        (argc>=6) ? argv[5] : "",              // save_map
        (argc>=7) ? argv[6] : "",              // load_map
        (argc>=8) ? std::stoi(argv[7]) : 1     // mapping
    );
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
