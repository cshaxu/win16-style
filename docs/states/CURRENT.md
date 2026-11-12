# Project Status

## Current Work

M0 T25 S7 按有限范围收口，见[记录](../history/M0-T25-S7-P13-closure.md)。用户明确纠正：S8 是全局风格选择，S9 是 Win16/Win2000 重复代码合并，S10 才是最终审计。此前误将 S8 作为最终审计，P1–P4 保留为前期证据，未完成的键盘、连续闪烁、跨屏 DPI、完整包矩阵及样式恢复不得计通过。T25 保持开放，现按用户确认继续 S8。

## M0 T25 S8 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation；用户更正仍在进行的 S8 范围，保留已使用的 P 序号。 |
| Admission And Approval | 用户明确指定 S8 为 --global 风格选择，S9 为两套样式去重，S10 为最终审计。 |
| Objective | 在全局托盘提供互斥、可取消的 Win16 和 Win2000 样式选择，并实现对应启用、切换、停用及尽力恢复。 |
| Non-goals | 不扩展 UWP、WinUI、应用自绘客户区或控件内部的 Win2000 支持；不把停用称为物理卸载。 |
| Reference Baseline | S5 P9 八文件发布包；S7 P13 有限技术收口及其列出的遗留项。 |
| Candidate Proposal | docs/proposals/windows-2000-classic.md 中用户确认的 S8 全局选择，S9 去重、S10 审计排在后续。 |
| Files And ABI Surface | run16s 托盘与全局控制器、ntw16s/ntw2ks 全局适配及窗口恢复、共享进程能力和对应测试。 |
| Applicable Rules | M0；纯 C17、MinGW GCC、Ninja、ccache；八文件同批交付。 |
| Verification | 验证互斥选择、重复点击停用、切换、现有与新目标、双位宽、失败和清理；普通启动与 Win16 全局行为回归。 |
| Expected Markers | 分开报告选择、全局启用、接入、样式应用、停用、恢复和实际卸载能力；失败与跳过明确记录。 |
| Asset Needs | 保留图形资源与来源声明；实际应用只读使用，不复制或修改样本。 |
| Reporting Requirements | 区分消息、真实输入、实际程序、前台焦点、各 DPI、用户视觉验收与未覆盖范围。 |
| Stop Conditions | 不创建或切换隔离桌面；不改变系统安全设置、目标文件或兄弟仓库。 |
| Exit Criteria | 实现并验证两种全局风格的选择、切换和停用，按规范构建、发布、测试、提交、推送和远端复核；恢复与卸载限制如实报告。 |
| Original Owner Request | 实现 Windows 2000 经典窗口样式；用户随后明确 S8 全局选择、S9 去重和 S10 最终审计。 |
| Similar-Issue Sweep | 检查现有托盘状态机、全局会话、目标接入、双位宽传播、样式附着和恢复，不建立重复控制路径。 |

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 25
- Last allocated Td S: 19
- Last allocated T18 S: 64
- Last allocated T19 S: 3
- Last allocated T20 S: 19
- Last allocated T21 S: 44
- Last allocated T25 S: 8
- Last verified remote delivery: M0 T25 S4 P73，`826503cf6fb16f2da7fad130a829118d65a6a4c3`。
- Open task: M0 T25 S8；全局 Win16/Win2000 风格选择。

## Technical Baseline

S8 P10 缩短停止时对未返回加载器的等待；发布包的双位宽六方向和快速改选
测试通过，实际耗时和遗留限制见[交付记录](../history/M0-T25-S8-P10-switch-latency.md)。
用户手动验收仍待完成。

S8 P9 修复旧全局样式仍附着时直接拒绝切换的问题，六方向发布包附着与移除回归
通过；Win2000 → Win16 后的像素复测和用户验收仍待完成，见[交付记录](../history/M0-T25-S8-P9-style-transitions.md)。

S8 P8 修复全局切换时托盘选项变灰：三项始终可用，后台串行收敛到用户最后选择。
双位宽同批构建已发布并完成正式包辅助功能切换回归；实际鼠标连续点击及用户验收
待完成，恢复和失败范围见[交付记录](../history/M0-T25-S8-P8-tray-always-enabled.md)。

S8 P7 候选八文件已发布，发布后的双位宽受控检查通过；正式包托盘交互、真实全局
切换和 DWM 恢复仍未通过，范围和测试目录见[交付记录](../history/M0-T25-S8-P7-global-style-candidate.md)。
此前管理员全局切换测试使用混合候选，不能代替正式包验证。首次干净构建的 x86
测试替身链接遗漏已修复；第二次双位宽干净构建通过。S8 与 T25 继续开放。

S8 P5 按用户纠正全局选择、样式去重和最终审计的顺序，见[记录](../history/M0-T25-S8-P5-plan-correction.md)。

S8 P6 审计现有全局流程及 Win2000 恢复缺口，见[记录](../history/M0-T25-S8-P6-global-path-audit.md)。

S8 P1 冻结最终审计覆盖清单，见[记录](../history/M0-T25-S8-P1-audit-scope.md)。

S8 P2 核对外框实现与证据，见[记录](../history/M0-T25-S8-P2-frame-audit.md)。

S8 P3 验证自有双位宽窗口真实鼠标菜单切换，见[记录](../history/M0-T25-S8-P3-physical-menu.md)。

S8 P4 核对遗留项范围与实际鼠标验证证据，见[记录](../history/M0-T25-S8-P4-debt-scope.md)。

S7 P13 按有限范围收口并准入 S8，见[记录](../history/M0-T25-S7-P13-closure.md)。

S7 P1 检查实际扫雷、Solitaire 和双位宽传播，见[记录](../history/M0-T25-S7-P1-real-apps.md)。

S7 P2 验证两种样式的 CLI 失败回滚及 Win16 回归，见[记录](../history/M0-T25-S7-P2-cli-rollback.md)。

S7 P3 验证启动事务失败后目标退出，见[记录](../history/M0-T25-S7-P3-process-cleanup.md)。

S7 P4 在真实 CLI 失败后检查目标进程快照，见[记录](../history/M0-T25-S7-P4-cli-process-cleanup.md)。

S7 P5 检查实际记事本与 demo 并审计样式移除边界，见[记录](../history/M0-T25-S7-P5-apps-and-restore.md)。

S7 P6 审计 Win2000 恢复及后续共享生命周期边界，见[记录](../history/M0-T25-S7-P6-restore-boundary.md)。

S7 P7 验证 64 位无菜单实际程序，见[记录](../history/M0-T25-S7-P7-charmap.md)。

S7 P8 验证实际 Delphi 程序的窗口外框，见[记录](../history/M0-T25-S7-P8-delphi-window.md)。

S7 P9 验证跨位宽父子窗口实际附着与标题绘制，见[记录](../history/M0-T25-S7-P9-propagated-frames.md)。

S7 P10 在当前桌面验证双位宽真实鼠标标题按钮，见[记录](../history/M0-T25-S7-P10-physical-caption-buttons.md)。

S7 P11 在当前桌面验证双位宽真实鼠标移动和拉伸，见[记录](../history/M0-T25-S7-P11-physical-frame-drag.md)。

S7 P12 验证系统版本信息窗口的外框，见[记录](../history/M0-T25-S7-P12-winver-window.md)。

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
