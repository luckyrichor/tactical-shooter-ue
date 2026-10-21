# 运行与复验

Windows：UE 5.6.1（D:\UE\UE_5.6），Visual Studio 2022。打开CombatLab.uproject；场景为引擎Entry，GameMode运行时生成占位角色、地面、敌人。WASD移动，鼠标相机，左键射击，R换弹，Space冲刺，F范围攻击。

一键构建、六项Automation、Listen Server双客户端与采集检查：

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/check.ps1
```

可传-SkipNetwork只跑编译与Automation。脚本等待真实进程退出，拒绝零测试、入服失败、单机回退、无技能RPC/Cue、最终复制值不一致或缺少新采集文件。产物在.local和Saved/Profiling/CSV；不要提交生成的二进制/缓存。

手动本地Listen Server（启动一个实例）：

```powershell
& 'D:\UE\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe' "$PWD\CombatLab.uproject" '/Engine/Maps/Entry?listen' -game -port=7777
```

客户端把地图参数换成127.0.0.1:7777。此例只监听本机验证；没有配置公网防火墙、账户认证或专用服务器发行包。正常玩法不要启用-LabAutoPlayer/-LabNetworkProbe。
Unreal Insights无UI导出：powershell -NoProfile -ExecutionPolicy Bypass -File scripts/profile.ps1。默认先重新采集；-ReuseCapture重分析已确认的最近一次，结果在.local/profile-summary.json。


鼠标与状态：左上角显示HP、Ammo、Stamina。按Esc显示鼠标并解除窗口限制，可移出窗口点击关闭；回到游戏窗口再按Esc恢复游戏控制。释放期间镜头和玩法输入停止，但世界不暂停。HP为0时角色不能移动/攻击，会出现死亡提示；单人场景按Enter重开，多人重生尚未实现。正常开局敌人在初始感知范围外，可先测试WASD和左键；左键单击后Ammo应减少1。
