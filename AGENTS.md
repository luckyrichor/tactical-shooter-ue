# AGENTS.md

本文件为在本仓库工作的编码 agent 提供指引（Claude Code 读 `CLAUDE.md`、Codex 读 `AGENTS.md`，两者都指向这里）。

## 这是什么

UE C++ 战术射击原型：武器系统、技能、敌人 AI（行为树 + 感知）、简化版多人同步、性能分析。

三个月求职计划六项目之一（原编号 ①），对应岗位 **03、04**，兼顾 **12**。总计划见 [workplan-docs](https://github.com/luckyrichor/workplan-docs)。

**当前状态：未开工，环境已就绪。** 建工程的分步操作见 [`docs/操作手册-工程骨架.md`](docs/操作手册-工程骨架.md)，那份是给用户在 Windows 前照着做的，**不要把它的内容复制到本文件**。

## 引擎版本：UE 5.6.1（已安装）

```
引擎路径  D:\UE\UE_5.6
版本      5.6.1   CL-44394996   ++UE5+Release-5.6
占用      25.4 GB（精简安装，未含调试符号）    D 盘剩余 176.9 GB
```

**使用 UE `5.6`，不要擅自改版本。** 选型依据：

- 5.6 的**推荐** VS 版本正是本机的 VS 2022 17.14（5.8 只把 17.14 作为最低要求）
- 5.6 完全不支持 VS 2026，编译路径唯一清晰；5.7 将 VS 2026 标为实验性、5.8 推荐 VS 2026，本机两个 VS 并存时更易挑错
- 社区资料覆盖充分，卡住时搜得到对口答案
- 岗位 03/04/12 考的是 gameplay 框架、GAS、行为树、网络同步，这些在 5.x 内稳定，没有岗位要求特定版本

**API 细节一律按 5.6.1 对齐**，不要照搬其他版本的写法。

### 已验证的环境［实测 2026-09-20］

| 项 | 状态 |
|---|---|
| `UnrealEditor.exe` / `UnrealEditor-Cmd.exe` | 有 |
| `UnrealBuildTool.exe` | 有，可运行（.NET 运行时正常） |
| `Build.bat` | 有 |
| `GenerateProjectFiles.bat` | **无——Launcher 二进制版本来就没有**，靠右键 `.uproject` 生成工程文件，不是缺东西 |
| 引擎源码 `Runtime` | 有 |
| `AIModule` / `NavigationSystem` / `GameplayTasks` | 有（行为树与感知内置于引擎，不是插件） |
| `GameplayAbilities`（GAS） | 有 |
| `ReplicationGraph` | 有 |
| C++ 模板 | `TP_Blank` / `TP_FirstPerson` / `TP_ThirdPerson` / `TP_TopDown` / `TP_VehicleAdv` / `TP_SIM_Blank` |

### 工具链已完整验证［实测 2026-09-20］

在 `D:\UEProbe`（一次性目录，验证后已删）建最小 C++ 工程编译 `UEProbeEditor Win64 Development`：

| 项 | 结果 |
|---|---|
| 退出码 | **0，Succeeded** |
| 耗时 | 首次 82 秒；清中间产物后重编 38 秒 |
| 产物 | `UnrealEditor-UEProbe.dll` / `.pdb` / `.target` |
| 链接的模块 | `GameplayAbilities`、`GameplayTags`、`GameplayTasks`、`AIModule`、`NavigationSystem` **全部链接成功** |
| Windows SDK | 实际使用 10.0.22621.0 |

**82 秒／38 秒是"改一处代码后重编"的量级参考**（引擎是预编译的）。真正变慢会发生在自有代码积累起来之后。

### BuildConfiguration.xml（已配置）

位于 `%USERPROFILE%\Documents\Unreal Engine\UnrealBuildTool\BuildConfiguration.xml`：

- `<Compiler>VisualStudio2022</Compiler>` **加** `<CompilerVersion>14.44.35207</CompilerVersion>`

  **只写 `<Compiler>` 不够**［实测踩过］：UBT 把它当**编译器系列**处理，然后在该系列里挑版本号最高的工具链。本机三个 x64 工具链里最高的是 VS 2026 的 **14.51**，于是它挑了 VS 2026 的编译器，日志里还把它标成"Visual Studio 2022 14.51"——极具迷惑性。加上 `<CompilerVersion>` 才真正锁住。

  锁定后实际使用 `C:\Program Files (x86)\...\2022\BuildTools\VC\Tools\MSVC\14.44.35207`，而不是 `F:\VS\IDE` 那份。**两份是同一工具链版本，编译器二进制相同，无差别**；游戏开发工作负载提供的是 IDE 集成与调试器，不是编译器。

  ［预警］UE 5.6 的偏好版本是 **MSVC 14.38.33130**，本机只有 14.44 和 14.51，因此日志会一直有 `not a preferred version` 警告。**这是 warning 不是 error，构建正常通过**。要消掉需用 VS 安装器补装 14.38 工具链，收益很小，暂不处理。
- `<MaxParallelActions>12</MaxParallelActions>` 与 `<MaxProcessorCount>12</MaxProcessorCount>` —— 32 核但仅 13.7G 内存，并行编译时内存先爆。**编译报内存相关错误时先调这两个值，别急着怀疑代码**
- 注释用纯 ASCII：PowerShell 5.1 以 GBK 显示无 BOM 的 UTF-8 文件会乱码，中文注释会造成误判

在 UE 装好之前，本项目只能推进不依赖引擎的部分（技术方案、系统设计）。**不要把无法编译的代码算作进展。**

## 环境：只在 Windows 机器上开发

UE 不在 Mac 或 Linux 上部署。开发机是 `luowindows`，工作区 `E:\WorkPlan`。

安装 UE 时**建议装 D 盘**（203G 空闲），不要装 E 盘——E 盘要留给工作区，UE 单版本加 DerivedDataCache 轻松吃掉 100G+。

杀软方面：Defender 已被火绒接管并完全停用（三项状态全 False），**不要给 Defender 加排除目录，无效**。实际在跑的只有火绒一个（安全中心里那条腾讯电脑管家是卸载残留的僵尸注册）。是否给火绒加信任区，等首次完整编译实测耗时后再决定。

该机 32 核但**只有 13.7G 内存**，编 UE C++ 时内存会先于 CPU 成为瓶颈，可能要在 `BuildConfiguration.xml` 限制 `MaxParallelActions`。**编译失败先排查内存，别急着怀疑代码。**

已配置：`LongPathsEnabled=1`（系统层）、`core.longpaths=true`（git 层，两者是独立开关，都要开）、Git LFS 已 install。

### 编译器：必须用 VS 2022，不是 2026

机器上有三套 VS，**只有 Community 2022 装了「使用 C++ 的游戏开发」工作负载**，而且它装在**非常规位置 `F:\VS\IDE`**（不在 C 盘，容易找不到）：

| 实例 | 路径 | MSVC |
|---|---|---|
| **Community 2022 ← 用这个** | `F:\VS\IDE` | 14.44.35207 |
| Community 2026 | `C:\Program Files\Microsoft Visual Studio\18\Community` | 14.51.36231 |
| 生成工具 2022 | `C:\Program Files (x86)\...\BuildTools` | 14.44.35207 |

### UE 与 Visual Studio 的官方兼容性（2026-09-19 查证自 Epic 官方文档）

| UE 版本 | VS 2022 | VS 2026 |
|---|---|---|
| 5.8 | **17.14 或更高** | 18.0+（官方推荐用于常规开发） |
| 5.7 | 17.8+，推荐 17.14 | 实验性 |
| 5.6 | 17.8+，**推荐 17.14** | 不支持 |
| 5.5 | 17.8+，推荐 17.10 | 不支持 |
| 5.4 | 17.4+，推荐 17.8 | 不支持 |

本机 VS 2022 是 **17.14.37628.2**，因此 **5.6 / 5.7 / 5.8 都可用现有 VS 2022 编译**，不需要 VS 2026。

**订正**：本文档此前写「2026 的 MSVC 14.51 太新，UnrealBuildTool 大概率不认」——**这是错的**，UE 5.8 官方反而推荐 VS 2026。但本机 VS 2026 未装「使用 C++ 的游戏开发」工作负载，而 VS 2022 装了，所以仍应使用 VS 2022。

UE 要求的工作负载与组件**本机 VS 2022 已全部具备**［实测］：使用 C++ 的游戏开发、使用 C++ 的桌面开发、.NET 桌面开发、C++ 分析工具、C++ AddressSanitizer；Windows SDK 有 10.0.22621 与 10.0.26100（要求 ≥10.0.18362）；.NET SDK 9.0.318 与 10.0.401。**装 UE 前不需要再动 VS。**

［预警］官方推荐内存 **32 GB**，本机 13.7 GB，远低于推荐值。这是 ① 的主要风险，不是阻塞项但会影响编译与编辑器体验。

**两个 VS 并存时 UE 可能挑错**，生成工程前先在 `BuildConfiguration.xml` 显式锁定：

```xml
<Configuration>
  <WindowsPlatform>
    <Compiler>VisualStudio2022</Compiler>
  </WindowsPlatform>
</Configuration>
```

该文件位于 `%USERPROFILE%\Documents\Unreal Engine\UnrealBuildTool\BuildConfiguration.xml`。内存只有 13.7G，同一文件里也可以顺便限制并行度。

## 分工模式：混合

- **Claude**：通过 ssh 写 C++ 源码、调 UBT 编译、跑 UE Automation Test、读编译与测试日志
- **用户**：在编辑器里做场景搭建、蓝图连线、动画状态机配置、Unreal Insights 分析和肉眼验证

这不是分工偷懒，是 UE 的固有形态——编辑器操作无法通过 ssh 完成。岗位 03/04/12 主要考察 C++ gameplay 能力，这部分由代码承载。

## Git LFS

`.uasset` / `.umap` / `.fbx` / 贴图 / 音频走 LFS，规则已写在 `.gitattributes`。克隆后先 `git lfs install`。**不要把二进制资产直接提交进 git**，仓库会撑爆且无法 diff。

## 所有项目共同的约束

这些是用户明确要求的，优先级高于一般工程习惯：

- **区分事实与推断**：JD 原文明确写的、与「行业常见要求／补充推断」必须显式区分。允许联网补充，但要标注来源性质。
- **不夸大**：不把规划写成已完成成果，不把本地测试数据包装成生产规模，不把加分项改写成硬性门槛。
- **不预设降级**：即使用户当下没时间反馈，也按既定方向持续推进产出，不因「时间可能不够」提前砍掉或缩水——取舍由用户自己做。
- **不安排模型算法、训练、微调、推理引擎优化方向的学习。**
- 不需要重新确认用户背景：Python 最熟练，C++/C#/Go 有基础，Agent 开发已较熟悉。直接进入框架层和有深度的工程问题，不安排语言零基础或入门教程。
- **AI 产出的代码不等于可以写进简历。** 用户要能在面试里讲清每个关键设计取舍，偏产出的项目要同步维护 `docs/design-decisions.md`。

## 多机协作

三台机器：Mac、`tx`（Ubuntu 服务器，常开）、`luowindows`（Windows，引擎唯一机器）。GitHub 是唯一事实源。

- **开工 `git pull`，收工 `git push`**，不留未推送的提交过夜
- **同一个项目同一时间只在一台机器上改**
- 密钥永不进仓库，放仓库内已 gitignore 的 `.local/`

**动环境之前先读 [`workplan-docs/环境与踩坑记录.md`](https://github.com/luckyrichor/workplan-docs/blob/main/环境与踩坑记录.md)** —— 三台的规格、三种不同的代理机制、以及十条已经踩过的坑都在那里。其中两条最容易再犯：三台文件系统大小写敏感性不一致（tx 敏感，另外两台不敏感）；PowerShell 5.1 的 BOM 规则（读 `.ps1` 必须带 BOM，写文件绝不能带）。

## 进度记录

进展写在 `docs/progress.md`：**只写已发生的事，不写计划**；失败和返工也要记，那是面试时最有料的部分。汇总到 `workplan-docs/进度总览.md`。

## Docker 运行位置（用户明确要求）

**需要 Docker 时默认跑在 `tx` 服务器上**，不要在 Mac 或 Windows 上起容器。

确有必要在本地跑时，**必须先征得用户同意**，不要自行决定。

tx 上 `ubuntu` **已加入 `docker` 组**（2026-09-19），docker 命令不需要 `sudo`，Testcontainers 也可直接用。该机内存只有 7.5G，多个服务同时跑之前先看 `free -h`。

### 在 tx 上跑 Docker／测试前，必须先同步代码

本地改完直接去 tx 跑，而 tx 上还是旧代码 —— 结果是「测试通过」测的其实是旧版本，**这种假通过比失败更危险**。

用 `workplan-docs/scripts/run-on-tx.sh` 代替手工 ssh，它强制执行正确顺序：

```bash
bash workplan-docs/scripts/run-on-tx.sh agent-memory 'uv run pytest -q'
bash workplan-docs/scripts/run-on-tx.sh agent-memory 'docker compose up -d postgres'
```

它会：本地有未提交改动就拒绝 → push → tx pull → **校验两边哈希一致** → 才执行。哈希比对是最终判据，不是「命令有没有报错」。脚本还顺带剥掉了腾讯云登录横幅。

## 记录规范：新内容写到哪儿

**本仓库的正文只维护 `AGENTS.md` 一份**，`CLAUDE.md` 永远只是几行指路。两份都写正文必然漂移，**不要往 `CLAUDE.md` 里加任何内容**。

| 内容类型 | 去处 |
|---|---|
| 本仓库的指引、约定、边界、分工方式 | 本仓库 `AGENTS.md` |
| 环境、机器、工具链、跨平台的坑 | `workplan-docs/环境与踩坑记录.md` |
| 已经发生的进展、验证结果、失败与返工 | 本仓库 `docs/progress.md` |
| 关键设计取舍及其理由（面试要讲的） | 本仓库 `docs/design-decisions.md` |
| 排期、工时预算、里程碑 | `workplan-docs/总节奏表.md` |
| 岗位要求本身 | `workplan-docs/岗位要求原文.md`（唯一事实来源，不要在别处改写） |

写进「环境与踩坑记录」时，用 **［实测］**／**［预警］** 标注区分「已验证的事实」和「未触发的已知风险」，不要把推断写成结论。
