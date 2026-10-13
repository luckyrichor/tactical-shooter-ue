# 最终验收与 CPU 采集

2026-10-13，Codex；原基线8322c82 + 本轮Windows工作树，实际文件SHA256见source.json（Windows原始字节，Git属性可能规范行尾）。UE5.6.1，MSVC14.44，Editor Win64 Development构建成功。Automation四项全部Succeeded，无SucceededWithWarnings/Failed/NotRun。

## 联机

scripts/check.ps1最终完整执行退出0：Listen Server与两个独立客户端，同机127.0.0.1；两次Join succeeded，服务器net=2/客户端net=3，Fire/技能RPC及客户端GameplayCue实际触发。服务器复制LabId，以身份关联最终Ammo/Health/Stamina/AIState，共8组客户端角色最终状态一致，三个进程退出0。不是每帧位移严格一致测试，不证明公网、丢包、重连或作弊防护。测试参数把敌人放到Sight之外，自动玩家输入五秒后停止；正常玩法不改变敌人位置。

## Unreal Insights

新Trace 5709761字节、CSV1897166字节；Trace使用唯一文件名，没有沿用旧文件。实际UnrealInsights.exe -NoUI -AutoQuit打开此次Trace并导出Lab_* TimingEvents，退出0。原始事件CSV/采集CSV、摘要及Trace SHA256均保留，Trace本体留.local避免二进制进普通Git。

| CPU scope | 样本 | 中位数 μs | 最大 μs |
|---|---:|---:|---:|
| Lab_AI | 2686 | 12.9 | 132.4 |
| Lab_Fire | 8 | 90.6 | 215.1 |
| Lab_Dash | 4 | 298.4 | 357.5 |
| Lab_Pulse | 2 | 978.700 | 1371.900 |

这些是含子调用/日志/Cue的inclusive时长，技能只有2–8个样本，不能据此估计尾延迟或排优化优先级。本次AI路径主要巡逻，不能把它当战斗负载。采集CSV有2570 个FrameTime，median16.67015 ms/max34.8027ms；t.MaxFPS=60主动限帧，null RHI无渲染，因此这些数值不能说明实际游戏达到60FPS。GameThreadTime/RenderThreadTime为0的字段不用于结论。

结论：权威业务链和可重复的CPU观测入口已打通；本夹具未提供性能瓶颈或优化收益证据。scripts/profile.ps1可重跑采集/Insights导出，参数-ReuseCapture仅重分析最近一次。人工操作手感、画面、美术/动画和GPU性能验收依授权保留。

## 失败与修正

启动前csvCaptureFrames触发引擎RHI初始化断言，客户端回退单机，不作为通过证据；改到地图启动后start、退出前stop。旧Trace已经存在导致引擎拒绝覆盖，改为唯一名称。Automation销毁测试世界未EndPlay的警告已修正。PowerShell延迟获取子进程句柄导致ExitCode为空，两个客户端启动时即保留Handle，最终完整重跑验证退出码。