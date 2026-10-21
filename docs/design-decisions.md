# 设计取舍

最后更新：2026-10-21（北京时间），Codex。

## 工程与武器

UE 5.6.1 原生C++模块；运行时用引擎Entry地图和Cube构造占位地面/角色，避免依赖未提供的美术资产。CharacterMovement承担移动复制；Camera/SpringArm实现第三人称镜头。服务器控制射击间隔0.15秒、弹药30、换弹1秒、死亡限制与射线伤害；客户端经拥有角色的RPC提出动作，不能直接写权威属性。命中表现为不可靠Multicast调试线，允许丢失，不承担伤害判定。无连射输入保持、动画、声音或成品武器手感。

## GAS技能链

冲刺与范围攻击为ServerOnly Ability，分别提交25/30 Stamina成本GameplayEffect；范围600内目标获得-20 Health Effect，Health/Stamina夹在0..100。成本Effect触发原生GameplayCue调试球；ASC/属性复制，角色BeginPlay在两端注册Cue。冷却2/3秒由服务器角色验证，未使用预测式客户端Ability或Cooldown GameplayEffect；这是原型取舍，不能称完整GAS战斗框架。

## AI

AI Perception Sight更新Blackboard Target；原生BehaviorTree序列里的长期Task切换Patrol/Chase/Attack，并在服务器射击。采用直线转向而不是NavMesh路径规划，复杂障碍下会卡住。Automation状态转换测试显式设置Blackboard，不等于完整视觉感知测试；实际运行日志另可观测Sight引出的状态。

## 联机与采集

Listen Server + 两个同机客户端，权威属性/Ammo/Reloading/AIState和移动复制。LabId是服务器生成并复制的诊断身份，防止不同进程的对象名称被误当稳定身份。网络验证夹具LabNetworkProbe把敌人放到视野外，让客户端有时间执行动作；LabAutoPlayer仅驱动本地玩家，五秒后停止输入以检查最终状态。正常运行不启用这些参数。未测试公网、恶意RPC洪泛、重连、预测或专用服务器构建。

启动期-csvCaptureFrames在本引擎-nullrhi下触发RHI未初始化断言；改为地图启动后ExecCmds csvprofile start，退出前stop落盘。trace文件每次唯一命名，拒绝沿用旧文件。Unreal Insights可按Lab_AI/Fire/Dash/Pulse事件分析；null RHI只支持此夹具的CPU测量，不代表实际渲染帧率或优化收益。

用户授权无法自动推进的UE/Unity部分跳过：人工视觉、手感和编辑器美术/动画验收仍需人工，不用自动测试冒充。

## 基础场景照明（2026-10-21）

空 Entry 没有光源，默认材质物体在 Lit 模式全黑。主方向光和弱补光作为自定义 GameState 默认组件，在服务器与客户端各自创建，避免仅服务器 GameMode 有光。显式清除 AInfo 默认隐藏标志。保留正常 Lit 模式；回归检查实际 GameMode 选用的 GameState 有可见非零光源，另用真实D3D离屏截图覆盖 null RHI 的视觉盲区。天空仍是占位黑背景，没有增加天空/美术系统。


## 鼠标与死亡可见性（2026-10-21）

释放鼠标绑定放在PlayerController而非Pawn，死亡后仍可Esc解锁。GameAndUI/DoNotLock/NoCapture与可见鼠标配合，保留Esc恢复；释放时移动与镜头忽略状态成对切换，玩法入口另拦截误射击/技能。AI仍走服务器攻击路径。HUD提供可见的生命值、弹药、体力与死亡提示，Enter只走UE已有的单机RestartLevel，不声称实现多人重生。敌人初始放在视野之外，给用户先操作的机会，未改变感知半径或伤害规则。
