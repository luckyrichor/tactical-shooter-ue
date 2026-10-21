# 进度记录

最后更新：2026-10-21（北京时间），Codex。

本文件是 `tactical-shooter-ue` 的进度事实源，汇总到 `workplan-docs/进度总览.md`。

格式：每条记录写明日期、做了什么、验证方式与结果、遇到的问题。**不写计划，只写已发生的事**；失败和返工也要记，那是面试时最有料的部分。

---

## 2026-10-01 W2 M1 / W4 维持：Windows 连接阻塞（Codex）

只读取 tx 仓库 tactical-shooter-ue@098202c（开工 clean、pull --ff-only 已同步）与工程指引/操作手册，没有在 tx 写入引擎源码。

实际 tx ~/.ssh/config 仅 GitHub host；ssh -G luowindows 展开为 hostname=luowindows/user=ubuntu（没有别名配置，指引 Windows 用户为 shnh2）；ssh -o BatchMode=yes -o ConnectTimeout=8 luowindows hostname 退出 255：Could not resolve hostname luowindows: Temporary failure in name resolution。文档没有可连接 IP/替代入口，未猜地址、未改 SSH 或凭据。

W2 骨架/移动/射击/武器状态机未开发编译，不能验收；W4 维持也未完成，不把这条阻塞记录算作工程进展。接续：用户提供 tx 可用的 Windows SSH HostName/User/公钥授权或在 Windows 会话执行；先查 Windows git status 与运行进程，确认无同仓并行修改后 pull --ff-only，再按操作手册创建/编译工程。编译通过后还需用户编辑器角色移动/射击/武器状态肉眼验收。

只有 docs/progress.md 增加本条事实记录，未改代码、未提交/推送。
## 2026-10-02 W5 / W7（Codex）

来源 tactical-shooter-ue@906115f，代码未修改。用户授权提前推进 W5–W7，并允许暂不可做项跳过。本次 pull --ff-only 无新增提交；再次 ssh luowindows hostname 返回域名无法解析，未连接Windows或核实其工作树。

W5 M2 GAS 两个技能的 Ability/Effect/Cue 链路，以及 W7 独立维护，按授权暂跳过；没有实现/编译/编辑器验收，不标完成。需先恢复 Windows 入口及 M1 工程，再完成技能资产配置与编辑器运行。没有在 tx 写无法验证的UE源码。此前章节为历史记录。


## 2026-10-07 Codex：W8–W10实际执行

本轮执行W8–W10范围核对后按用户授权跳过不可执行项。2026-10-07 TX实测ssh -o BatchMode=yes -o ConnectTimeout=5 luowindows hostname退出255，Temporary failure in name resolution；无法访问唯一UE工作机。未运行UE编译、行为树/感知测试或编辑器操作。W9 M3敌人AI及W10维护未完成，M1/GAS等前置缺口仍存在；没有以无法编译源码替代验收。恢复工作机入口后核实M1、补前置再做AI。此次只有进度记录更新。

## 2026-10-13 Windows恢复后的M1–M4工程验收（Codex）
基线8322c82及本轮工作树：CombatLab Editor构建成功，4项Automation全Succeeded无警告。真实Listen Server+双客户端两次入服、技能RPC与Cue、6组最终角色关键状态一致；三个进程退出0。Unreal Insights实际分析新5709761字节Trace并导出Lab_*事件，frame CSV有2570 帧，完整报告/源码哈希见measurements/2026-10-13。RHI启动采集断言、旧Trace拒绝覆盖、测试World EndPlay与PowerShell退出码均修正后重跑。null RHI与限帧不构成渲染/生产性能，手感/画面人工验收按授权保留；此前跳过记录均为历史。

正常场景8秒运行（不启用网络夹具或自动玩家）退出0，实际Sight驱动巡逻/追击/攻击均有日志，补足M3真实感知验收；证据见ai-runtime-evidence.log。Windows GitHub SSH两次失败，已提交变更将原样通过tx转发推送，不在tx修改本项目。


## 2026-10-21 正常渲染黑屏修复（Codex）

来源 5d96cae + 本次 Windows 工作树。10月20日用户正常启动后全黑，切换 viewmode unlit 后可见；实时日志显示场景和 AI 运行。原因是空 Entry 场景只有地面和角色，没有光源。此前 null RHI 验收未覆盖实际画面。

自定义复制 GameState 构造主方向光与弱补光，各客户端本地创建组件，并关闭 AInfo 默认隐藏状态。新增照明回归：旧代码0通过/1失败；10月20日最终全套5项通过、0警告，双客户端6组关键状态一致、三进程退出0。独立审查未发现正确性问题。

10月21日使用真实 D3D、Lit 模式和 RenderOffscreen 生成1280x720截图，已检查地面、角色和阴影可见，进程退出0。SSH非交互窗口交换链曾返回 DXGI_ERROR_NOT_CURRENTLY_AVAILABLE；离屏避免创建桌面交换链，不是用户桌面重启验收，也不是GPU性能或手感验收。原启动命令可继续使用，不需要切换Unlit。证据摘要见 docs/measurements/2026-10-21-lighting.json。

编译因约3GB可用内存持续等待，单次改用 -NoUBA -MaxParallelActions=1，未改全局配置；MSVC非首选版本警告仍存在。网络脚本摘要的date字段仍保留历史硬编码，实际执行日期以本次日志与trace文件名为准。


## 2026-10-21 鼠标释放与死亡反馈修复（Codex）

来源7c077e4及Windows工作树。用户截图在限帧后约60FPS、Frame16.67ms，Game/Draw/GPU约1.8/2.0/6.0ms；只说明该时刻帧时间，不认定持续性能或卡顿根因。旧运行日志玩家health=0、弹药未消耗；死亡后移动和攻击被拒绝，而旧界面没有死亡提示。另核实默认控制器没有Esc释放操作。

新增本地PlayerController：Esc切换鼠标显示、解锁和捕获，释放时禁止移动/转镜头和玩法输入；死亡后也可操作。Enter重开只适用于单人。HUD显示HP/Ammo/Stamina、操作提示和死亡信息。常规敌人从x700移至x1500，玩家从x-500开始，初始距离2000超过Sight1500，网络探针仍保持x5000。

新输入回归先在旧代码失败（缺少Esc绑定），修复后6项Automation全部通过且0警告；Listen Server两个真实客户端，6组最终角色状态一致、三个进程退出0。正常D3D离屏Lit截图已检查HUD显示HP100/Ammo30/Stamina100，8秒观察玩家仍存活。证据摘要见 docs/measurements/2026-10-21-input.json。独立审查未发现正确性问题；实际Windows窗口失焦、鼠标移出和重新捕获仍需用户交互确认，不以nullRHI测试替代。
