#!/usr/bin/env python3
"""
TUM RGB-D Dataset Publisher for ORB_SLAM3 ROS2

用法:
    # 终端1: 启动 rgbd-inertial
    source install/setup.sh
    ros2 run orbslam3 rgbd-inertial VOCAB SETTINGS

    # 终端2: 发布数据集
    python3 scripts/publish_tum_rgbd.py ~/dataset/Tum-RGBD/rgbd_dataset_freiburg1_room

发布的 topics:
    /camera/rgb    — sensor_msgs/Image (彩色 BGR8)
    /camera/depth  — sensor_msgs/Image (16-bit 单通道)
    /imu           — sensor_msgs/Imu (仅加速度计，无陀螺仪)
"""

import sys
import time
import argparse
from pathlib import Path

import rclpy
from rclpy.node import Node

import cv2
import numpy as np
import numpy as np
from sensor_msgs.msg import Image, Imu
from builtin_interfaces.msg import Time


class TumRgbdPublisher(Node):
    def __init__(self, dataset_dir: str, speed: float = 1.0):
        super().__init__('tum_rgbd_publisher')

        self.dataset_dir = Path(dataset_dir)
        self.speed = speed
        pass  # cv_bridge broken, using manual numpy conversion

        self.pub_rgb = self.create_publisher(Image, 'camera/rgb', 10)
        self.pub_depth = self.create_publisher(Image, 'camera/depth', 10)
        self.pub_imu = self.create_publisher(Imu, 'imu', 1000)

        self.rgb_data = self._load_txt('rgb.txt')
        self.depth_data = self._load_txt('depth.txt')
        self.accel_data = self._load_txt('accelerometer.txt')

        self.get_logger().info(
            f'已加载 rgb: {len(self.rgb_data)}, depth: {len(self.depth_data)}, accel: {len(self.accel_data)}')

    def _load_txt(self, filename: str) -> list:
        path = self.dataset_dir / filename
        rows = []
        with open(path, 'r') as f:
            for line in f:
                line = line.strip()
                if not line or line.startswith('#'):
                    continue
                parts = line.split()
                rows.append(parts)
        return rows

    def _sec_to_ros_time(self, ts_sec: float) -> Time:
        t = Time()
        t.sec = int(ts_sec)
        t.nanosec = int((ts_sec - int(ts_sec)) * 1e9)
        return t

    def publish(self):
        events = []

        for row in self.rgb_data:
            ts = float(row[0])
            events.append(('rgb', ts, row[1]))

        for row in self.depth_data:
            ts = float(row[0])
            events.append(('depth', ts, row[1]))

        for row in self.accel_data:
            ts = float(row[0])
            events.append(('imu', ts, (float(row[1]), float(row[2]), float(row[3]))))

        events.sort(key=lambda e: e[1])

        t0 = events[0][1]
        t0_wall = time.time()
        rgb_count = 0
        depth_count = 0

        for evt_type, ts, payload in events:
            elapsed_dataset = ts - t0
            elapsed_wall = time.time() - t0_wall
            sleep_time = (elapsed_dataset / self.speed) - elapsed_wall

            if sleep_time > 0:
                time.sleep(sleep_time)

            if evt_type == 'rgb':
                msg = self._make_image_msg(payload, ts)
                self.pub_rgb.publish(msg)
                rgb_count += 1
                if rgb_count % 100 == 0:
                    self.get_logger().info(f'已发布 rgb: {rgb_count}')

            elif evt_type == 'depth':
                msg = self._make_depth_msg(payload, ts)
                self.pub_depth.publish(msg)
                depth_count += 1

            elif evt_type == 'imu':
                msg = self._make_imu_msg(payload, ts)
                self.pub_imu.publish(msg)

        self.get_logger().info(f'完成 — rgb: {rgb_count}, depth: {depth_count}')

    def _numpy_to_imgmsg(self, img, encoding):
        msg = Image()
        msg.height, msg.width = img.shape[:2]
        msg.encoding = encoding
        msg.is_bigendian = False
        msg.step = img.strides[0] if len(img.shape) > 1 else img.shape[1] * img.dtype.itemsize
        msg.data = img.tobytes()
        return msg

    def _make_image_msg(self, filename: str, ts: float) -> Image:
        path = self.dataset_dir / filename
        cv_img = cv2.imread(str(path), cv2.IMREAD_COLOR)
        if cv_img is None:
            self.get_logger().error(f'无法读取: {path}')
            return Image()

        msg = self._numpy_to_imgmsg(cv_img, encoding='bgr8')
        msg.header.stamp = self._sec_to_ros_time(ts)
        msg.header.frame_id = 'camera_rgb'
        return msg

    def _make_depth_msg(self, filename: str, ts: float) -> Image:
        path = self.dataset_dir / filename
        cv_img = cv2.imread(str(path), cv2.IMREAD_UNCHANGED)
        if cv_img is None:
            self.get_logger().error(f'无法读取: {path}')
            return Image()

        msg = self._numpy_to_imgmsg(cv_img, encoding='mono16')
        msg.header.stamp = self._sec_to_ros_time(ts)
        msg.header.frame_id = 'camera_depth'
        return msg

    def _make_imu_msg(self, accel: tuple, ts: float) -> Imu:
        ax, ay, az = accel

        msg = Imu()
        msg.header.stamp = self._sec_to_ros_time(ts)
        msg.header.frame_id = 'imu'

        msg.linear_acceleration.x = ax
        msg.linear_acceleration.y = ay
        msg.linear_acceleration.z = az

        # TUM 数据集无陀螺仪 → 设为零
        msg.angular_velocity.x = 0.0
        msg.angular_velocity.y = 0.0
        msg.angular_velocity.z = 0.0

        for i in range(9):
            msg.orientation_covariance[i] = -1.0
            msg.angular_velocity_covariance[i] = -1.0
            msg.linear_acceleration_covariance[i] = -1.0

        return msg


def main():
    parser = argparse.ArgumentParser(description='TUM RGB-D 数据集 ROS2 发布器')
    parser.add_argument('dataset_dir', help='数据集目录路径')
    parser.add_argument('--speed', type=float, default=1.0,
                        help='播放速度倍率 (默认: 1.0)')
    args = parser.parse_args()

    rclpy.init()
    node = TumRgbdPublisher(args.dataset_dir, speed=args.speed)

    try:
        node.publish()
    except KeyboardInterrupt:
        node.get_logger().info('用户中断')
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()
