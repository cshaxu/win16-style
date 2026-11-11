# Project Status

## Current Work

M0 T25 S6 按已验证的有限技术范围收口，详见[收口记录](../history/M0-T25-S6-P5-closure.md)；真实拖动/缩放、瞬时闪烁、跨屏 DPI 和部分前台焦点仍未证明，转交真实程序回归及最终审计，不记通过。T25 保持开放，现进入 S7 多进程与真实应用回归。

## M0 T25 S7 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New；T25 内下一 S。 |
| Admission And Approval | 用户要求继续完成普通 Win32 的 Windows 2000 外框并授权自行规划 S；S7 属于原定序列。 |
| Objective | 用实际普通 Win32 程序和自有双位宽目标检查启动、传播、标题外框、菜单与窗口行为，核查 Win16 路径及失败清理。 |
| Non-goals | 不声称 UWP、WinUI、应用自绘客户区、控件内部或 Win2000 全局模式已支持。 |
| Reference Baseline | S5 P9 八文件发布包；S6 P5 有限技术收口，真实输入、完整包矩阵和视觉验收仍未完成。 |
| Candidate Proposal | docs/proposals/windows-2000-classic.md 用户收窄后的 S7。 |
| Files And ABI Surface | run16s、ntw2ks、共享进程能力、win2k-window 及对应测试；必要变更保持 ABI 和既有样式行为。 |
| Applicable Rules | M0；纯 C17、MinGW GCC、Ninja、ccache；八文件同批交付。 |
| Verification | 双位宽启动/传播及清理、实际 32/64 位程序外框、对话框和菜单、默认 Win16 回归；明确真实输入及 DPI 范围。 |
| Expected Markers | 分开报告启动、加载、附着、绘制、交互、退出及资源清理；可用时对照原版。 |
| Asset Needs | 保留图形资源与来源声明；实际应用只读使用，不复制或修改样本。 |
| Reporting Requirements | 区分消息、真实输入、实际程序、前台焦点、各 DPI、用户视觉验收与未覆盖范围。 |
| Stop Conditions | 不创建或切换隔离桌面；不改变系统安全设置、目标文件或兄弟仓库。 |
| Exit Criteria | 已准入范围实现及实际验证，按规范构建、必要发布、测试、提交推送与远端复核；受限项不计通过并交最终审计。 |
| Original Owner Request | 实现普通 Win32 程序的 Windows 2000 经典窗口样式。 |
| Similar-Issue Sweep | 对照真实程序与自有模型，检查位宽、进程传播、启动顺序、窗口交互和正常退出。 |

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 25
- Last allocated Td S: 19
- Last allocated T18 S: 64
- Last allocated T19 S: 3
- Last allocated T20 S: 19
- Last allocated T21 S: 44
- Last allocated T25 S: 7
- Last verified remote delivery: M0 T25 S4 P73，`826503cf6fb16f2da7fad130a829118d65a6a4c3`。
- Open task: M0 T25 S7；Win2000 多进程与真实应用回归。

## Technical Baseline

S7 P1 检查实际扫雷、Solitaire 和双位宽传播，见[记录](../history/M0-T25-S7-P1-real-apps.md)。

S7 P2 验证两种样式的 CLI 失败回滚及 Win16 回归，见[记录](../history/M0-T25-S7-P2-cli-rollback.md)。

S7 P3 验证启动事务失败后目标退出，见[记录](../history/M0-T25-S7-P3-process-cleanup.md)。

S7 P4 在真实 CLI 失败后检查目标进程快照，见[记录](../history/M0-T25-S7-P4-cli-process-cleanup.md)。

S7 P5 检查实际记事本与 demo 并审计样式移除边界，见[记录](../history/M0-T25-S7-P5-apps-and-restore.md)。

S7 P6 审计 Win2000 恢复及后续共享生命周期边界，见[记录](../history/M0-T25-S7-P6-restore-boundary.md)。

S7 P7 验证 64 位无菜单实际程序，见[记录](../history/M0-T25-S7-P7-charmap.md)。

S7 P8 验证实际 Delphi 程序的窗口外框，见[记录](../history/M0-T25-S7-P8-delphi-window.md)。

S6 P1 验证双位宽连续尺寸变化的几何与有限采样，见[记录](../history/M0-T25-S6-P1-resize-cycle.md)。

S6 P2 验证重复最大化和还原，见[记录](../history/M0-T25-S6-P2-zoom-cycle.md)。

S6 P3 区分线程激活与前台窗口，见[记录](../history/M0-T25-S6-P3-activation-foreground.md)。

S6 P4 验证尺寸变化后的自然消息重绘，见[记录](../history/M0-T25-S6-P4-natural-resize.md)。

S6 P5 按有限技术范围收口并准入 S7，见[记录](../history/M0-T25-S6-P5-closure.md)。

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

S5 P14 验证标题按钮捕获丢失及取消，见[记录](../history/M0-T25-S5-P14-button-capture-loss.md)。

S5 P15 区分应用菜单切换与真实 Escape，见[记录](../history/M0-T25-S5-P15-menu-switch.md)。

S5 P17 按有限技术范围收口并准入 S6，见[记录](../history/M0-T25-S5-P17-closure.md)。
