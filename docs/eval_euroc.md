# EuRoC 轨迹评估流程

> 实测验证于 2026-07-15（V1_01_easy 立体，ATE RMSE 0.0935 m）。
> 前置：`trajectory_io.cc` 的 `-ffast-math`/`inverse()` 问题已修复（见 [problem.md](problem.md) 问题 3 遗留风险），修复前保存的轨迹文件不可信，需重新生成。

## 1. 跑图并保存轨迹

```bash
cd /home/ros/lib/SLAM/VIO/Structure-PLP-SLAM

./build/run_euroc_slam \
  -v ./orb_vocab/orb_vocab.dbow2 \
  -d /home/ros/dataset/Euroc/vicon_room1/V1_01_easy/mav0 \
  -c ./example/euroc/EuRoC_stereo.yaml \
  --eval-log --auto-term
```

- `--eval-log`：在**当前目录**生成
  - `keyframe_trajectory.txt` —— 关键帧位姿
  - `frame_trajectory.txt` —— 每帧位姿
  - 格式均为 TUM：`timestamp tx ty tz qx qy qz qw`
- `--auto-term`：序列结束后自动退出 viewer
- `--no-sleep`：不按实时速率、全速处理（可选）
- `-p mymap_euroc.msg`：同时保存地图数据库（可选）
- 单目：`-c ./example/euroc/EuRoC_mono.yaml`
- 点线模式：`./build/run_euroc_slam_with_line`；点面模式：`./build/run_euroc_slam_planeSeg`（需分割掩码，见 README）

## 2. 评估（evo_ape）

```bash
# 关键帧轨迹 ATE
bash scripts/eval_euroc.sh ./keyframe_trajectory.txt \
  /home/ros/dataset/Euroc/vicon_room1/V1_01_easy/mav0/ --plot --align

# 每帧轨迹 ATE
bash scripts/eval_euroc.sh ./frame_trajectory.txt \
  /home/ros/dataset/Euroc/vicon_room1/V1_01_easy/mav0/ --plot --align
```

- 第 2 个参数是数据集的 **mav0 目录**（脚本从中读取 `state_groundtruth_estimate0` 真值并转 TUM 格式）
- `--align`：评估前做 SE(3) Umeyama 对齐（单目还需尺度对齐时在脚本内调整）
- 输出：终端打印 ATE 统计（rmse/mean/median/max），结果与图表保存到 `../results/`：
  - `<场景>_ape.zip`、`<场景>_ape_raw.png`、`<场景>_ape_map.png`

## 3. 参考结果

| 序列 | 模式 | ATE RMSE | 备注 |
|---|---|---|---|
| V1_01_easy | 立体 | 0.0935 m | 2026-07-15，trajectory_io 修复后 |

## 常见坑

1. **时间戳对不上（`found no matching timestamps`）**：估计轨迹不是这个数据集跑出来的。EuRoC 时间戳形如 `1403715273...`（2014-06）；若看到 `1311874903...`（2011-07）说明是 TUM freiburg2 的旧轨迹残留。跑新序列前先挪走旧的 `*_trajectory.txt`。
2. **必须在仓库根目录运行**评估脚本与 SLAM（配置内相对路径依赖 cwd）。
3. `--eval-log` 只在正常退出（`--auto-term` 或手动 Terminate）时写文件，`Ctrl+C` 强杀可能拿不到轨迹。
