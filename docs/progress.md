# 进度记录

最后更新：2026-10-02（北京时间）

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
