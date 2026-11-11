# Project Status

## Current Work

M0 T25 S3 已完成首版 Win2000 普通 Win32 窗口接入、发布与远端复核；验证范围和未覆盖见[S3 P1](../history/M0-T25-S3-P1-delivery.md)。S4 P3 修复用户发现的 P2 重复图标回归，见[S4 P3](../history/M0-T25-S4-P3-delivery.md)。用户随后报告颜色不合格；S4 P4 根据参考图调整配色，见[S4 P4](../history/M0-T25-S4-P4-delivery.md)，S4 P6 修复按住按钮时系统颜色回退，见[S4 P6](../history/M0-T25-S4-P6-button-press.md)；S4 P8 完善边框底色，见[S4 P8](../history/M0-T25-S4-P8-frame-band.md)；S4 P9 调整工具标题字号，见[S4 P9](../history/M0-T25-S4-P9-tool-font.md)；S4 P10 调整标题按钮默认尺寸及命中，见[S4 P10](../history/M0-T25-S4-P10-button-geometry.md)；S4 P12 调整无菜单窗口标题高度，见[S4 P12](../history/M0-T25-S4-P12-bare-caption.md)；S4 P14 处理普通菜单窗口几何，见[S4 P14](../history/M0-T25-S4-P14-menu-geometry.md)；S4 P15 接入对话框窗口，见[S4 P15](../history/M0-T25-S4-P15-dialog-frame.md)；S4 P16 修复空对话框过程的窗口附着，见[S4 P16](../history/M0-T25-S4-P16-null-dialog.md)；S4 P17 补充资源对话框回归，见[S4 P17](../history/M0-T25-S4-P17-dialog-resources.md)；S4 P18 接入系统菜单经典配色，见[S4 P18](../history/M0-T25-S4-P18-system-menu.md)；S4 P19 修复系统菜单图标，见[S4 P19](../history/M0-T25-S4-P19-menu-glyphs.md)；S4 P20 修复默认菜单项字重，见[S4 P20](../history/M0-T25-S4-P20-menu-default.md)；S4 P21 记录菜单离屏和录音机对照，见[S4 P21](../history/M0-T25-S4-P21-visual-audit.md)；S4 P22 记录菜单边框和命令探针，见[S4 P22](../history/M0-T25-S4-P22-menu-border-audit.md)；S4 P23 验证菜单实际客户区，见[S4 P23](../history/M0-T25-S4-P23-live-menu-audit.md)；S4 P24 接入标准消息框外框，见[S4 P24](../history/M0-T25-S4-P24-message-frame.md)；S4 P25 接入查找和替换窗口外框，见[S4 P25](../history/M0-T25-S4-P25-find-frame.md)；S4 P26 接入旧式文件窗口外框，见[S4 P26](../history/M0-T25-S4-P26-file-frame.md)；S4 P27 验证文件窗口选择返回，见[S4 P27](../history/M0-T25-S4-P27-file-selection.md)；S4 P28 接入颜色选择窗口外框，见[S4 P28](../history/M0-T25-S4-P28-color-frame.md)；S4 P29 完成实际程序窗口单次回归，见[S4 P29](../history/M0-T25-S4-P29-real-window.md)；S4 P30 完成实际程序单次位置与尺寸检查，见[S4 P30](../history/M0-T25-S4-P30-window-position.md)；S4 P31 接入页面设置窗口外框，见[S4 P31](../history/M0-T25-S4-P31-page-setup.md)；继续 S4，T25 仍开放。

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
- Last verified remote delivery: M0 T25 S4 P63，`a48cd88d9a93e618fc9f731e83122b9334a11044`。
- Open task: M0 T25 S4；Win2000 普通 Win32 标题与外框绘制。

## Technical Baseline

已发布基线为五组件、八产品文件；纯 C17、MinGW GCC、Ninja 和 ccache。T25 S4 P70 八文件已发布于项目 assets 根目录与 O:/winnt，摘要和验证范围见[交付记录](../history/M0-T25-S4-P70-inactive-menu.md)。历史状态见[T24 工作快照](../history/M0-T24-S1-working-log.md)和[原状态快照](../history/M0-T24-current-state-archive.md)。

S4 P32 补充扩展打印窗口双位宽取消路径验证，见[审计记录](../history/M0-T25-S4-P32-print-ex-audit.md)；没有产品二进制变更。

S4 P33 补充普通窗口动态移除和恢复标题栏验证，见[审计记录](../history/M0-T25-S4-P33-style-transition.md)。

S4 P34 接入初始无标题、运行时添加标题的顶层窗口，见[交付记录](../history/M0-T25-S4-P34-caption-added.md)。

S4 P35 以发布版复测两个实际程序窗口，见[审计记录](../history/M0-T25-S4-P35-published-real-window.md)。

S4 P36 补充实际窗口模拟激活与失焦颜色采样，见[审计记录](../history/M0-T25-S4-P36-real-activation.md)。

S4 P37 准备双窗口激活探针；可见桌面切换受限，未运行真实输入检查，见[记录](../history/M0-T25-S4-P37-activation-environment.md)。

S4 P38 验证标题帮助按钮命中与命令转发，见[记录](../history/M0-T25-S4-P38-help-button.md)。

S4 P39 检查发布版自建普通窗口的 DC 画面，见[记录](../history/M0-T25-S4-P39-visual-window.md)。

S4 P40 发现常见默认配置下可缩放外框度量差距，后续几何修复须协调客户区、菜单和命中，见[记录](../history/M0-T25-S4-P40-frame-metric.md)。

S4 P41 测量双位宽固定与可缩放窗口的边框及菜单几何，见[记录](../history/M0-T25-S4-P41-frame-baseline.md)。

S4 P42 修正普通可缩放窗口的经典外框几何，见[交付记录](../history/M0-T25-S4-P42-frame-geometry.md)。

S4 P43 修正移动后系统图标边缘的命中回归，见[交付记录](../history/M0-T25-S4-P43-system-icon-hit.md)。

S4 P44 修正系统图标覆盖菜单顶部，系统菜单实时选中绘制及部分输入测试仍有未解决结果，见[交付记录](../history/M0-T25-S4-P44-icon-clip.md)。

S4 P45 修正私有桌面系统菜单首次实时选中色，真实输入和完整 DPI 仍待验证，见[交付记录](../history/M0-T25-S4-P45-system-menu-idle.md)。

S4 P46 验证程序触发的系统菜单选中项切换，见[记录](../history/M0-T25-S4-P46-menu-selection.md)。

S4 P47 验证带横纵滚动条窗口的外框位置与命中，见[记录](../history/M0-T25-S4-P47-scroll-geometry.md)。

S4 P48 验证标题按钮消息路径的拖离取消及后续最大化，见[记录](../history/M0-T25-S4-P48-button-cancel.md)。

S4 P49 验证可缩放工具窗口的标题、边框和命中，见[记录](../history/M0-T25-S4-P49-tool-resize.md)。

S4 P50 检查计算器、录音机与扫雷的实际窗口；录音机退出超时未计通过，见[记录](../history/M0-T25-S4-P50-real-dialog-apps.md)。

S4 P51 修复应用自定义透明／错误／无命中结果被覆盖，见[交付记录](../history/M0-T25-S4-P51-custom-hit.md)。

S4 P52 修复 Solitaire 创建窗口期间尺寸消息过早导致的启动崩溃，见[交付记录](../history/M0-T25-S4-P52-solitaire-startup.md)。

S4 P53 调整标题图标背景并请求启动后的客户区重绘；用户环境的菜单残影待复验，见[交付记录](../history/M0-T25-S4-P53-caption-icon-repaint.md)。

S4 P54 修复菜单悬停覆盖和标题右侧残影；用户提供的原版配色及按钮比例仍待对齐，见[交付记录](../history/M0-T25-S4-P54-menu-overlap.md)。

S4 P55 根据用户提供的原版截图调整按钮尺寸、位置及配色；真实鼠标最大化仍需调查，见[交付记录](../history/M0-T25-S4-P55-reference-caption.md)。

S4 P56 处理按钮快速再次点击产生的双击消息，真实鼠标已复测；一次探针首次按下未命中仍未解释，见[交付记录](../history/M0-T25-S4-P56-caption-double-click.md)。

S4 P57 借鉴 Win16 的中性菜单命中处理，阻止系统原生菜单浮现，见[交付记录](../history/M0-T25-S4-P57-neutral-menu-hit.md)。

S4 P58 将两种样式共用的菜单输入协议提取至 style-common，见[交付记录](../history/M0-T25-S4-P58-style-common-menu.md)。

S4 P59 依据原版源码与用户对物理宽度的选择调整边框，见[交付记录](../history/M0-T25-S4-P59-physical-border.md)。

S4 P60 按原版截图及源码调整标题按钮上、右间距，见[交付记录](../history/M0-T25-S4-P60-caption-inset.md)。

S4 P61 用发布版复核双位宽标题按钮真实鼠标输入，见[记录](../history/M0-T25-S4-P61-caption-input.md)。

S4 P62 对照原版修正标题渐变下的分隔线，见[交付记录](../history/M0-T25-S4-P62-caption-separator.md)。

S4 P63 验证双位宽同线程窗口实际激活及标题颜色，见[记录](../history/M0-T25-S4-P63-window-activation.md)。

S4 P64 按活动包要求审计已覆盖范围与限制，见[记录](../history/M0-T25-S4-P64-scope-audit.md)。

S4 P65 检查双位宽标题命令与系统菜单选中状态，见[记录](../history/M0-T25-S4-P65-system-commands.md)。

S4 P66 增加 DPI 感知窗口测试并在 144 DPI 实测，见[记录](../history/M0-T25-S4-P66-aware-dpi.md)。

S4 P67 对照无图标对话框和失活窗口标题，见[记录](../history/M0-T25-S4-P67-dialog-caption-reference.md)。

S4 P68 补充系统对话框类和普通窗口失活图标验证，见[记录](../history/M0-T25-S4-P68-real-dialog-icon.md)。

S4 P69 验证系统对话框显式提供专用图标的情况，见[记录](../history/M0-T25-S4-P69-dialog-custom-icon.md)。

S4 P70 修正窗口失活时的菜单文字颜色，见[交付记录](../history/M0-T25-S4-P70-inactive-menu.md)。
