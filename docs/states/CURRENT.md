# Project Status

## Current Work

M0 T25 S3 已完成首版 Win2000 普通 Win32 窗口接入、发布与远端复核；验证范围和未覆盖见[S3 P1](../history/M0-T25-S3-P1-delivery.md)。S4 P3 修复用户发现的 P2 重复图标回归，见[S4 P3](../history/M0-T25-S4-P3-delivery.md)。用户随后报告颜色不合格；S4 P4 根据参考图调整配色，见[S4 P4](../history/M0-T25-S4-P4-delivery.md)，S4 P6 修复按住按钮时系统颜色回退，见[S4 P6](../history/M0-T25-S4-P6-button-press.md)；S4 P8 完善边框底色，见[S4 P8](../history/M0-T25-S4-P8-frame-band.md)；S4 P9 调整工具标题字号，见[S4 P9](../history/M0-T25-S4-P9-tool-font.md)；S4 P10 调整标题按钮默认尺寸及命中，见[S4 P10](../history/M0-T25-S4-P10-button-geometry.md)；S4 P12 调整无菜单窗口标题高度，见[S4 P12](../history/M0-T25-S4-P12-bare-caption.md)；S4 P14 处理普通菜单窗口几何，见[S4 P14](../history/M0-T25-S4-P14-menu-geometry.md)；S4 P15 接入对话框窗口，见[S4 P15](../history/M0-T25-S4-P15-dialog-frame.md)；S4 P16 修复空对话框过程的窗口附着，见[S4 P16](../history/M0-T25-S4-P16-null-dialog.md)；S4 P17 补充资源对话框回归，见[S4 P17](../history/M0-T25-S4-P17-dialog-resources.md)；S4 P18 接入系统菜单经典配色，见[S4 P18](../history/M0-T25-S4-P18-system-menu.md)；S4 P19 修复系统菜单图标，见[S4 P19](../history/M0-T25-S4-P19-menu-glyphs.md)；S4 P20 修复默认菜单项字重，见[S4 P20](../history/M0-T25-S4-P20-menu-default.md)；S4 P21 记录菜单离屏和录音机对照，见[S4 P21](../history/M0-T25-S4-P21-visual-audit.md)；S4 P22 记录菜单边框和命令探针，见[S4 P22](../history/M0-T25-S4-P22-menu-border-audit.md)；S4 P23 验证菜单实际客户区，见[S4 P23](../history/M0-T25-S4-P23-live-menu-audit.md)；S4 P24 接入标准消息框外框，见[S4 P24](../history/M0-T25-S4-P24-message-frame.md)；继续 S4，T25 仍开放。

## M0 T25 S4 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New；T25 内下一 S。 |
| Admission And Approval | 用户要求继续实现普通 Win32 程序的 Windows 2000 外框及标题栏，并授权自行规划 S。 |
| Objective | 完善普通 Win32 标题栏活动/失活、文字、图标、渐变、窗口边缘及重绘，基于可取得的原版参考明确几何与颜色。 |
| Non-goals | 不声称 UWP、WinUI、自绘客户区、控件内部或 Win2000 全局模式已支持。 |
| Reference Baseline | main 8e7f487d；S3 P1 八文件已发布并远端复核；Win2000 源码包只读参考。 |
| Candidate Proposal | docs/proposals/windows-2000-classic.md 的 S4；完整目标保留至 S8。 |
| Files And ABI Surface | win2k-window、ntw2ks 接入、对应同名测试与包测试；保持 Win16 及启动行为。 |
| Applicable Rules | M0；纯 C17、MinGW GCC、Ninja、ccache；八文件同批交付。 |
| Verification | 双位宽真实普通窗口验证活动/失活、文字图标、标题颜色、重复绘制和 resize；默认 Win16、启动传播及资源边界回归。 |
| Expected Markers | 区分标题实际画面、消息与命中，列出颜色/尺寸参考及仍不支持状态。 |
| Asset Needs | 保留原有资源和来源声明，不复制未经许可的参考源码或图形。 |
| Reporting Requirements | 区分产品包测试、原版对照、物理输入和用户手动验收。 |
| Stop Conditions | 不改变系统安全设置、目标文件或兄弟仓库；受限验证记录真实原因。 |
| Exit Criteria | 目标状态实测、双位宽干净构建、同批包测试与发布、提交推送及远端复核；限制不得算通过。 |
| Original Owner Request | 实现普通 Win32 程序的 Windows 2000 经典窗口样式。 |
| Similar-Issue Sweep | 窗口创建、不同标题/图标、激活/失活、最大化/还原、窗口边缘、主题切换和 Win16 默认路径。 |

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 25
- Last allocated Td S: 19
- Last allocated T18 S: 64
- Last allocated T19 S: 3
- Last allocated T20 S: 19
- Last allocated T21 S: 44
- Last allocated T25 S: 4
- Last verified remote delivery: M0 T25 S4 P23，`2513be3b0fe88d0468472b3ad93916bb6c3f9e4a`。
- Open task: M0 T25 S4；Win2000 普通 Win32 标题与外框绘制。

## Technical Baseline

已发布基线为五组件、八产品文件；纯 C17、MinGW GCC、Ninja 和 ccache。T25 S4 P24 八文件已发布于项目 assets 根目录与 O:/winnt，摘要和验证范围见[交付记录](../history/M0-T25-S4-P24-message-frame.md)。历史状态见[T24 工作快照](../history/M0-T24-S1-working-log.md)和[原状态快照](../history/M0-T24-current-state-archive.md)。
