# Project Status

## Current Work

M0 T25 S1 已准入：调研并准备 Windows 2000 经典窗口、控件和对话框样式，建立 ntw2ks-dll 并增加启动器样式选择，详见[研究方案](../proposals/windows-2000-classic.md)。目前只有实验性的标准控件处理，尚未实现完整 Win2000 样式；P1 已发布，完整回归未通过的原因和范围见[交付记录](../history/M0-T25-S1-P1-delivery.md)。T24 已有限收口，遗留见[收口记录](../history/M0-T24-closure.md)与[待办](TODO.md)。

## M0 T25 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New；T25 首个研究 S。 |
| Admission And Approval | 用户要求准入 T25，调研并为 Windows 2000 经典窗口、控件和对话框样式实现做准备。 |
| Objective | 确认原版参考和接口边界，建立双位宽 ntw2ks-dll 接入，并支持 run16s --2k、--16，默认 --16。 |
| Non-goals | 不宣称现已实现；不修改系统主题或注册表、目标程序文件，不擅自导入第三方资源或修改兄弟项目。 |
| Reference Baseline | main 4e59a867，T24 有限收口；Microsoft 官方 Win32 文档与可核验的原版样本。 |
| Candidate Proposal | docs/proposals/windows-2000-classic.md |
| Files And ABI Surface | 新增 ntw2ks-dll 双位宽及启动器样式选择，审计启动、跨位宽、UAC、传播和包格式；保留 ntw16s 现有 ABI。 |
| Applicable Rules | M0；EXECUTION、ARCHITECTURE、CODING、DOCUMENT；纯 C17、MinGW GCC、Ninja、ccache。 |
| Verification | 样本来源与可用性核验；必要的双位宽原生绘图对照；文档门禁；无法运行项明确 skip。 |
| Expected Markers | 原版参照来源、窗口/控件/对话框矩阵、复用边界、接口草案、优先级与验收标准。 |
| Asset Needs | 用户新要求覆盖原四文件约束：双位宽 ntw2ks 新增两个产品文件，共六文件；保留原资源与来源。 |
| Reporting Requirements | 区分官方文档、实际样本、推论与未验证项；不能将原型或颜色替换称为完整支持。 |
| Stop Conditions | 不更改系统安全设置或目标文件；样本缺失时说明边界，继续其他独立研究。 |
| Exit Criteria | 研究及可执行计划完成，按仓库交付规则验证、提交推送并复核；后续实现按活动包准入。 |
| Original Owner Request | 准入 T25，调研准备 Win2000 风格；建立 ntw2ks-dll，run16s 增加 --2k 和 --16，默认 --16。 |
| Similar-Issue Sweep | 对照 Win16、UWP 外框、对话框桥接及多主题格式，检查几何、字体、控件状态和恢复的共性与差异。 |

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 25
- Last allocated Td S: 19
- Last allocated T18 S: 64
- Last allocated T19 S: 3
- Last allocated T20 S: 19
- Last allocated T21 S: 44
- Last verified remote delivery: M0 T24 S1 P1，`a3c6610ff6a2fb85fdb43d17c76040883c92fa6f`。
- Open task: M0 T25 S1，研究与准备。

## Technical Baseline

已发布基线为三组件、四产品文件；纯 C17、MinGW GCC、Ninja 和 ccache。T25 活动包目标增加双位宽 ntw2ks，形成四组件、六文件，尚未完成发布。T24 四文件发布于项目 assets 根目录与 O:/winnt，摘要和验证范围见[交付记录](../history/M0-T24-delivery.md)。历史状态见[T24 工作快照](../history/M0-T24-S1-working-log.md)和[原状态快照](../history/M0-T24-current-state-archive.md)。
