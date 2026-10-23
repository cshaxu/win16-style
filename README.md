# Win16 Style

将 ntvdm64 的 `run16 --win16-style-all` 独立为 Windows 原生应用的
Win16 窗口外观启动器。项目目标与组件契约见
[目标](docs/design/GOAL.md)及[架构](docs/design/ARCHITECTURE.md)。

| 组件 | 产物 | 架构 |
| --- | --- | --- |
| run16s-exe | run16s.exe | x64 |
| ntw16s-dll | ntw16s32.dll、ntw16s64.dll | x86、x64 |
| demo-exe | demo.exe | x64 |

预期用法（启动器与 demo 尚未实现，核心 DLL 已可构建）：

```powershell
.\demo.exe                 # 普通 Win32 窗口外观
.\run16s.exe .\demo.exe    # Win16 窗口外观
.\run16s.exe <app.exe> [arguments]
```

T1 已完成；T2 正在导入并验证双位宽样式核心。当前还没有完整可运行产品包。
文档入口为 [docs/README.md](docs/README.md)，
进度以 [CURRENT](docs/states/CURRENT.md) 为准。所有工作固定在 milestone0，
使用 MTSP 管理任务及交付。源代码、构建和运行验收将在后续导入任务进行。

治理检查：

```powershell
pwsh -NoProfile -File tools/Verify-Governance.ps1 -SelfTest
python tools/verify_source_manifest.py --self-test
git diff --check
```
