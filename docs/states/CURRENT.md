# Project Status

## Current Work

用户决定以现有实际成果有限收口 T25，未完成的 S8 验证和 S10 最终审计不计通过；原 S9 去重已明确并入 T26，遗留范围见[T25 收口记录](../history/M0-T25-owner-closure.md)。准入 T26，抽离跨风格通用机制并整理样式库。

## M0 T26 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New；T25 按用户决定有限收口，T26 从 S1 开始。 |
| Admission And Approval | 用户要求把 Win16 中跨年代可用的外框、控件、对话框以及 Win32/UWP/WinUI 机制抽离至 lib-style，并准入下一 T。 |
| Objective | 冻结全部相关生产组件、调用与测试清单，区分风格无关机制及各年代专属行为，确定分批迁移和实际复用契约；按用户要求对已确认单元同步实施。 |
| Non-goals | 审计不宣称已支持未实现的主题或控件；不改变既有挂钩权限与用户体验。 |
| Reference Baseline | T25 S8 P13 已发布包及有限收口记录；保留既有 Win16 与 Win2000 行为。 |
| Candidate Proposal | docs/proposals/style-library-reorganization.md，结合本次明确扩展的窗口、控件、对话框和现代框架范围。 |
| Files And ABI Surface | 样式库、通用样式机制、产品适配、CMake、全部相应测试及来源声明；审计先不改公开 ABI。 |
| Applicable Rules | M0；纯 C17、MinGW GCC、Ninja、ccache；现有八文件同批交付。 |
| Verification | 静态生产依赖及调用方盘点、来源与测试映射；迁移后验证两种风格实际调用、双位宽及适用框架回归。 |
| Expected Markers | 每一项列明旧位置、分类、目标组件、调用方、迁移顺序及验证；未覆盖明确保留。 |
| Asset Needs | 保留原始图形资源及来源声明，不依赖或修改兄弟仓库。 |
| Reporting Requirements | 分别报告已共享、待迁移、风格专属和未验证能力；不把目录移动当作代码复用。 |
| Stop Conditions | 不创建或切换隔离桌面，不使用 computer-use；不改系统安全设置或兄弟仓库。 |
| Exit Criteria | 审计覆盖全部相关生产入口和测试，形成可执行的迁移次序、明确边界及 S 序列，完成规范验证与远端复核。 |
| Original Owner Request | 收口 T25；把 Win16 中可用于 Win2000、XP、Vista、Win7 的外框、控件、对话框及 Win32/UWP/WinUI 通用能力抽离到 lib-style。 |
| Similar-Issue Sweep | 检查现有 style-common、Win16/Win2000 样式模块、产品适配及共享进程入口，避免重复机制和风格反向依赖。 |

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 26
- Last allocated Td S: 19
- Last allocated T18 S: 64
- Last allocated T19 S: 3
- Last allocated T20 S: 19
- Last allocated T21 S: 44
- Last allocated T25 S: 8
- Last allocated T26 S: 1
- Last verified remote delivery: M0 T26 S1 P22，`838eca62`。
- Open task: M0 T26 S1；跨风格能力完整审计与迁移规划。

## Technical Baseline

T26 S1 已索引 28 个 Win16/Win2000 窗口与 UWP 源文件；直接引用及未完成的共享边界见[窗口索引](../history/M0-T26-S1-window-index.md)。

T26 S1 已移除窗口几何空包装并共享 XAML 子窗口识别；测试与限制见[记录](../history/M0-T26-S1-frame-xaml.md)。

T26 S1 已共享控件协议识别和绑定生命周期；属性命名变化、并存限制及发布后测试见[记录](../history/M0-T26-S1-control-protocol.md)。

T26 S1 已索引 55 个 Win16 控件源文件与直接测试引用，并验证现有控件协议清理；混合机制与未覆盖项见[控件索引](../history/M0-T26-S1-control-index.md)。

T26 S1 已共享经典文件路径、Shell 项及隐藏文件枚举，产品保留私有消息；见[记录](../history/M0-T26-S1-classic-file-helpers.md)。

T26 S1 已共享经典文件对话框显示、Shell 文件夹适配与活动控件同步；边界及验证见[记录](../history/M0-T26-S1-classic-bridge.md)。

T26 S1 已共享旧式文件对话框 COM 接口适配器，保留调用方的回退策略；验证与边界见[记录](../history/M0-T26-S1-dialog-interfaces.md)。

T26 S1 已迁移文件对话框事件、现代选择器及旧式 Shell 文件夹通用调用；范围、测试和剩余混合代码见[对话框审计](../history/M0-T26-S1-dialog-audit.md)。

T26 S1 已冻结首批 272 个相关文件路径，见[初始清单](../history/M0-T26-S1-initial-inventory.md)；逐项依赖、调用方、测试和迁移归属审计仍在进行，未宣称完成。

T26 S1 首批共享基础已发布，双位宽聚焦测试通过，但 Win2000 DWM 生命周期
测试两侧失败，完整回归及视觉验收未完成；证据见[记录](../history/M0-T26-S1-shared-foundation.md)。

T26 S1 样式库及组件测试已迁至 `lib-style`，同批包发布后的双位宽 22 项
聚焦检查通过；实际 UWP 程序、物理输入和旧有 DWM 恢复问题仍未覆盖，见[记录](../history/M0-T26-S1-style-layout.md)。

T26 S1 已共享 Win16/Win2000 菜单助记键、键盘动作及指针切换判断；发布后
双位宽 18 项消息测试通过，完整跟踪器及物理交互仍未统一或验收，见[记录](../history/M0-T26-S1-menu-input.md)。

T26 S1 已抽离 UWP 原生标题选择与全屏 UIA 服务；发布后 11 项聚焦检查通过，
真实 UWP 激活和全屏调用尚未验证，见[记录](../history/M0-T26-S1-uwp-native.md)。

T26 S1 已抽离 UWP 窗口发现和关联校验，发布后 13 项聚焦检查通过；真实
UWP 激活与实际 WinUI 应用仍未覆盖，见[记录](../history/M0-T26-S1-uwp-target.md)。

T26 S1 已共享经典文件对话框契约和 Shell 结果转换；发布后双位宽 14 项
聚焦检查通过，Win2000 对话框尚未实现，见[记录](../history/M0-T26-S1-classic-file.md)。

T26 S1 已统一 Win16 和 Win2000 的弹出菜单方向键判定；发布后双位宽
18 项聚焦检查通过，完整菜单跟踪器仍未合并，见[记录](../history/M0-T26-S1-menu-arrows.md)。

T26 S1 已抽离缓冲呈现和窗口打印遍历；发布后双位宽 14 项聚焦检查通过，
仪器化真实绘制测试因禁止隔离桌面未运行，见[记录](../history/M0-T26-S1-present.md)。

T26 S1 已抽离同线程主题状态、覆盖及恢复机制；发布后双位宽 14 项聚焦检查
通过，Win2000 尚未接入，历史主题恢复有限制，见[记录](../history/M0-T26-S1-theme-state.md)。

T26 S1 已将风格模块定位、校验与入口解析移入 `lib-win32/style`；发布后双位宽
12 项聚焦检查通过，包含正式 DLL 的窗口切换桥接，见[记录](../history/M0-T26-S1-style-companion.md)。

T26 S1 已抽离窗口线程派发机制；发布后双位宽 12 项聚焦检查通过，包括
自建跨进程目标，真实 UWP 应用未覆盖，见[记录](../history/M0-T26-S1-window-dispatch.md)。

T26 S1 已共享 UWP 宿主坐标与位图缓存；双位宽聚焦检查通过，完整外框
生命周期与旧版均以错误 5 失败，原因未明，见[记录](../history/M0-T26-S1-uwp-surface.md)。

T26 S1 已共享 UWP 分层表面提交；双位宽 16 项聚焦检查通过。完整外框
测试仍失败，已定位到跨进程相邻窗口的 `SetWindowPos` 返回错误 5，原因
未明，见[记录](../history/M0-T26-S1-uwp-present.md)。

S8 P13 对齐 Win16 的原生绘制消息防护并修复 Win2000 重绘覆盖，双位宽受控
消息测试及发布后直接探针通过；状态栏曾出现一次未解释偏差，真实鼠标和用户
视觉验收未完成。共享菜单跟踪及可配置 Win2000 滚动条仍未完成，见[记录](../history/M0-T25-S8-P13-shared-frame-redraw.md)。

S8 P12 复核 Solitaire 底部状态栏：一次跨进程采样出现 19 像素差距，五轮重复未
复现；双位宽受控窗口通过，底部视觉效果仍待用户确认。见[记录](../history/M0-T25-S8-P12-status-layout-audit.md)。

S8 P11 修复 Windows 2000 重绘时原生菜单覆盖客户区；正式 Solitaire 像素对照和
双位宽回归通过，真实鼠标回归受自动审批限制，等待用户验收。见[记录](../history/M0-T25-S8-P11-menu-redraw.md)。

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
