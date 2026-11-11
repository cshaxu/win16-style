# Project Status

## Current Work

M0 T25 S4 已按有限技术范围完成，详见[收口记录](../history/M0-T25-S4-P73-closure.md)；用户视觉验收和未覆盖项保留至 T25 后续工作。T25 保持开放，现进入 S5 标题按钮及系统菜单。

## M0 T25 S5 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New；T25 内下一 S。 |
| Admission And Approval | 用户要求实现普通 Win32 的 Windows 2000 窗口外框并授权自行规划 S；S5 属于原定序列。 |
| Objective | 完善标题按钮和系统菜单的外观、可用状态、鼠标与键盘输入和系统命令。 |
| Non-goals | 不声称 UWP、WinUI、应用自绘客户区、控件内部或 Win2000 全局模式已支持。 |
| Reference Baseline | S4 P73 技术收口；P70 八文件发布及测试，用户视觉验收待 T25 最终验收。 |
| Candidate Proposal | docs/proposals/windows-2000-classic.md 中用户收窄后的 S5。 |
| Files And ABI Surface | win2k-window 标题按钮及系统菜单、对应测试和包测试；不改变公开 ABI。 |
| Applicable Rules | M0；纯 C17、MinGW GCC、Ninja、ccache；八文件同批交付。 |
| Verification | 双位宽检查关闭、最小化、最大化/还原、帮助、禁用状态，系统菜单鼠标/键盘、命令和取消；回归 Win16 与普通启动。 |
| Expected Markers | 分别报告实际绘制、命中、真实输入、命令及菜单清理；与原版可取得证据对照。 |
| Asset Needs | 保留现有图形资源和来源声明，不复制未获许可的原版资源。 |
| Reporting Requirements | 分开说明程序消息、真实键鼠、实际应用、未覆盖的 DPI 和用户视觉验收。 |
| Stop Conditions | 用户要求不使用隔离桌面；不改变系统安全设置、目标文件或兄弟仓库，环境限制如实记录。 |
| Exit Criteria | 完成范围内状态和交互验证，构建、必要发布、测试、提交推送与远端复核；限制不得算通过。 |
| Original Owner Request | 实现普通 Win32 程序的 Windows 2000 经典窗口样式。 |
| Similar-Issue Sweep | 比较普通和工具窗口、禁用和动态变化状态、菜单打开取消、Alt+Space 及焦点切换。 |

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 25
- Last allocated Td S: 19
- Last allocated T18 S: 64
- Last allocated T19 S: 3
- Last allocated T20 S: 19
- Last allocated T21 S: 44
- Last allocated T25 S: 5
- Last verified remote delivery: M0 T25 S4 P73，`826503cf6fb16f2da7fad130a829118d65a6a4c3`。
- Open task: M0 T25 S5；Win2000 标题按钮与系统菜单。

## Technical Baseline

已发布基线为五组件、八产品文件；纯 C17、MinGW GCC、Ninja 和 ccache。T25 S5 P9 八文件已发布于项目 assets 根目录与 O:/winnt，摘要和验证范围见[交付记录](../history/M0-T25-S5-P9-caption-command-order.md)。历史状态见[T24 工作快照](../history/M0-T24-S1-working-log.md)和[原状态快照](../history/M0-T24-current-state-archive.md)。

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

S4 P71 在实际 Solitaire About 场景验证失活菜单，见[记录](../history/M0-T25-S4-P71-solitaire-about.md)。

S4 P72 用真实鼠标验证禁用的标题按钮，见[记录](../history/M0-T25-S4-P72-disabled-buttons.md)。

S5 P1 验证系统菜单基线，记录真实键盘超时及用户禁止隔离桌面的约束，见[记录](../history/M0-T25-S5-P1-system-menu-baseline.md)。

S5 P2 在当前桌面验证程序键盘消息驱动的系统菜单选中绘制，见[记录](../history/M0-T25-S5-P2-menu-keyboard.md)。

S5 P3 排除用简单投递 Escape 消息替代真实键盘的无效对照，见[记录](../history/M0-T25-S5-P3-escape-message-control.md)。

S5 P4 记录当前桌面原生按键探针的无效负对照，见[记录](../history/M0-T25-S5-P4-escape-input-control.md)。

S5 P5 修正按钮按下期间关闭命令被禁用时的误发送，见[交付记录](../history/M0-T25-S5-P5-disabled-close-command.md)。

S5 P6 在不使用隔离桌面的条件下验证双位宽加载与传播，见[记录](../history/M0-T25-S5-P6-direct-launch-regression.md)。

S5 P7 验证标题栏上下文菜单消息入口，见[记录](../history/M0-T25-S5-P7-caption-context-menu.md)。

S5 P8 验证系统菜单取消消息，见[记录](../history/M0-T25-S5-P8-menu-cancel.md)。

S5 P9 修复标题按钮系统命令的同步顺序，见[交付记录](../history/M0-T25-S5-P9-caption-command-order.md)。

S5 P10 复核实际应用及创建失败，见[记录](../history/M0-T25-S5-P10-real-app-regression.md)。

S5 P11 对照启动环境并验证实际 Solitaire，见[记录](../history/M0-T25-S5-P11-solitaire-environment.md)。

S5 P12 修正实际程序采样的裁剪和 DPI 上下文，见[记录](../history/M0-T25-S5-P12-real-app-dpi.md)。

S5 P13 检查 Solitaire 关于对话框与失活主窗口，见[记录](../history/M0-T25-S5-P13-solitaire-dialog.md)。
