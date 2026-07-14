export LD_LIBRARY_PATH=./build/lib/:./3rd/Pangolin/install/lib/:$LD_LIBRARY_PATH 
./build/run_tum_rgbd_slam \
-v ./orb_vocab/orb_vocab.dbow2 \
-d /home/ros/dataset/Tum-RGBD/rgbd_dataset_freiburg1_room \
-c ./example/tum_rgbd/TUM_RGBD_mono_1.yaml

export LD_LIBRARY_PATH=./build/lib/:./3rd/Pangolin/install/lib/:$LD_LIBRARY_PATH ./build/run_tum_rgbd_slam_with_line \
-v ./orb_vocab/orb_vocab.dbow2 \
-d /home/ros/dataset/Tum-RGBD/rgbd_dataset_freiburg1_room \
-c ./example/tum_rgbd/TUM_RGBD_rgbd_1.yaml