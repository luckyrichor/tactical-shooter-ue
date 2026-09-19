# tactical-shooter-ue

UE C++ 战术射击原型：武器系统、技能、敌人 AI（行为树 + 感知）、简化版多人同步、性能分析。

**状态：未开工，且环境未就绪**（2026-09-19）

## 对应岗位

03 腾讯《三角洲行动》客户端开发、04 腾讯《暗区突围》端游 gameplay，兼顾 12 米哈游 Unreal 客户端（AI）。岗位原文见 [workplan-docs](https://github.com/luckyrichor/workplan-docs)。

## 环境

**只在 Windows 机器（`luowindows`）上开发**，UE 不在 Mac 或 Linux 上部署。

**UE 尚未安装**，用户后续会装。在此之前本项目只能推进不依赖引擎的部分（技术方案、系统设计），**不把无法编译的代码算作进展**。

## 分工

- Claude 通过 ssh 写 C++ 源码、调 UBT 编译、跑 UE Automation Test、读编译与测试日志
- 用户在编辑器里做场景搭建、蓝图连线、动画状态机配置、Unreal Insights 分析和肉眼验证

这不是分工偷懒，是 UE 的固有形态：编辑器操作无法通过 ssh 完成。岗位 03/04/12 主要考察 C++ gameplay 能力，这部分由代码承载。

## Git LFS

本仓库启用 LFS 管理 `.uasset` / `.umap` 等二进制资产。克隆后先 `git lfs install`。
