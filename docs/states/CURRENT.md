# Project Status

## Current Work

M0 T25 S4 已按有限技术范围完成，详见[收口记录](../history/M0-T25-S4-P73-closure.md)；用户视觉验收和未覆盖项保留至 T25 后续工作。T25 保持开放，下一步为 S5 标题按钮及系统菜单。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 25
- Last allocated Td S: 19
- Last allocated T18 S: 64
- Last allocated T19 S: 3
- Last allocated T20 S: 19
- Last allocated T21 S: 44
- Last allocated T25 S: 4
- Last verified remote delivery: M0 T25 S4 P72，`2dda66d455f09b2c158a1fb229a165e094a6d54e`。
- Open task: 无活动任务包；T25 保持开放，下一步 S5。

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

S4 P71 在实际 Solitaire About 场景验证失活菜单，见[记录](../history/M0-T25-S4-P71-solitaire-about.md)。

S4 P72 用真实鼠标验证禁用的标题按钮，见[记录](../history/M0-T25-S4-P72-disabled-buttons.md)。
