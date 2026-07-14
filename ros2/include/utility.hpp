#ifndef PLPSLAM_ROS2_UTILITY_HPP
#define PLPSLAM_ROS2_UTILITY_HPP

#include "rclcpp/rclcpp.hpp"

class Utility
{
public:
  static double StampToSec(builtin_interfaces::msg::Time stamp)
  {
    double seconds = stamp.sec + (stamp.nanosec * pow(10, -9));
    return seconds;
  }
};

#endif
