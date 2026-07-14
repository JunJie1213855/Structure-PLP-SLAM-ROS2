import os
from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration

def generate_launch_description():
    # PLP-SLAM paths
    plpslam_dir = os.path.expanduser('~/lib/SLAM/VIO/Structure-PLP-SLAM')

    vocab_path = os.path.join(plpslam_dir, 'orb_vocab/orb_vocab.dbow2')
    config_path = os.path.join(plpslam_dir, 'example/tum_rgbd/TUM_RGBD_mono_1.yaml')

    return LaunchDescription([
        DeclareLaunchArgument('vocab_path', default_value=vocab_path,
                              description='Vocabulary file path'),
        DeclareLaunchArgument('config_path', default_value=config_path,
                              description='Config YAML file path'),
        DeclareLaunchArgument('use_line', default_value='0',
                              description='Enable line tracking (0/1)'),

        Node(
            package='plpslam_ros2',
            executable='mono',
            name='plpslam_mono',
            output='screen',
            arguments=[LaunchConfiguration('vocab_path'),
                       LaunchConfiguration('config_path'),
                       LaunchConfiguration('use_line')],
            parameters=[{'use_sim_time': False}],
        ),
    ])
