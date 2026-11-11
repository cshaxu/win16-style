# Project Status

## Current Work

M0 T25 S3 已按用户要求准入，T25 收窄为普通 Win32 程序的 Windows 2000 窗体外框和标题栏；S3–S8 序列见[方案](../proposals/windows-2000-classic.md)。S3 P1 外框基础已本地发布、重点测试通过，待提交推送及远端复核；范围与限制见[S3 交付记录](../history/M0-T25-S3-P1-delivery.md)。S2 已交付、远端复核，但用户尚未对其手动验收，既有全局托盘和 UWP 测试限制见 [S2 交付记录](../history/M0-T25-S2-P1-delivery.md)。

## M0 T25 S3 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New；T25 内新 S。 |
| Admission And Approval | 用户要求开始 Windows 2000 普通 Win32 窗体外框并自行规划 S。 |
| Objective | 审核 Win2000 参考及现有接入，完成普通顶层窗口外框基础及可验证的双位宽产品接入。 |
| Non-goals | 不处理 UWP、WinUI、自绘客户区、控件内部、对话框布局或 Win2000 全局模式。 |
| Reference Baseline | main 24a0f644，S2 八文件已发布；原版源码包只读参考，不复制实现或资源。 |
| Candidate Proposal | docs/proposals/windows-2000-classic.md，用户收窄 T25 后 S3–S8 序列。 |
| Files And ABI Surface | ntw2ks-dll、必要公共窗口库、demo、构建及对应测试；维护 ntwshk 和 Win16 行为。 |
| Applicable Rules | M0；纯 C17、MinGW GCC、Ninja、ccache；用户批准的八文件包。 |
| Verification | x86/x64 实际启动和窗口附着，标题绘制和原生系统命令、失败清理及 Win16 对照；真实桌面及未覆盖状态分别记录。 |
| Expected Markers | 明确原版参考、窗口类和状态范围；观察实际外框而非仅 DLL 加载。 |
| Asset Needs | 保留原有图形和来源声明，不复制未经许可的参考源码或资源。 |
| Reporting Requirements | 区分已实现、后续 S 和未验证；自动截图不替代用户视觉验收。 |
| Stop Conditions | 不修改系统安全设置、目标文件或兄弟仓库；环境限制记录并继续可做工作。 |
| Exit Criteria | 基础接入实测、双位宽构建及同批实际包测试、发布、提交推送和远端复核；待验收与限制如实记录。 |
| Original Owner Request | 开始做 Win2000 窗体外框与标题栏，本 T 专注 Win32，自行规划 S。 |
| Similar-Issue Sweep | 比较普通与工具窗口、标题按钮、菜单、最大化、DPI、重绘、传播及默认 Win16 路径。 |

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 25
- Last allocated Td S: 19
- Last allocated T18 S: 64
- Last allocated T19 S: 3
- Last allocated T20 S: 19
- Last allocated T21 S: 44
- Last allocated T25 S: 3
- Last verified remote delivery: M0 T25 S2 P1，`9aecf5305714a8105072790afab2464c5bf4e4bc`。
- Open task: M0 T25 S3；Win2000 普通 Win32 外框基础。

## Technical Baseline

已发布基线为五组件、八产品文件；纯 C17、MinGW GCC、Ninja 和 ccache。T25 S2 P1 八文件已发布于项目 assets 根目录与 O:/winnt，摘要和验证范围见[交付记录](../history/M0-T25-S2-P1-delivery.md)。历史状态见[T24 工作快照](../history/M0-T24-S1-working-log.md)和[原状态快照](../history/M0-T24-current-state-archive.md)。
