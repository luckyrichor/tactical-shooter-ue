# AGENTS.md

本文件为在本仓库工作的编码 agent 提供指引（Claude Code 读 `CLAUDE.md`、Codex 读 `AGENTS.md`，两者都指向这里）。

## 这是什么

UE C++ 战术射击原型：武器系统、技能、敌人 AI（行为树 + 感知）、简化版多人同步、性能分析。

三个月求职计划六项目之一（原编号 ①），对应岗位 **03、04**，兼顾 **12**。总计划见 [workplan-docs](https://github.com/luckyrichor/workplan-docs)。

**当前状态：未开工，且环境未就绪 —— UE 尚未安装。**

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
