# M0 T25 S8 P2：普通 Win32 外框逐项审计

依据 [P1 冻结清单](M0-T25-S8-P1-audit-scope.md)，核对当前生产入口及原始验证记录；“通过”均限所列条件。

| 单位 | 生产入口及证据 | 结论和剩余范围 |
| --- | --- | --- |
| 原版参考 | `win2k-window/caption.c`、`frame.c`；[S4 P59](M0-T25-S4-P59-physical-border.md)、[P60](M0-T25-S4-P60-caption-inset.md)、[P67](M0-T25-S4-P67-dialog-caption-reference.md)对照用户截图和只读源码包 | 按参考调整过边线、按钮与图标；截图缩放和配色不能证明全部像素一致，最终用户视觉验收仍缺。 |
| 启动与传播 | `run16s-exe/main.c` 的 `--2k`，`ntw2ks-dll/controls.c` 附着；[S7 P2](M0-T25-S7-P2-cli-rollback.md)、[P9](M0-T25-S7-P9-propagated-frames.md) | 自有 x86/x64 目标的失败回滚及四种父子位宽窗口附着与有限标题采样通过；真实 UAC 未覆盖。 |
| 窗口和对话框 | `win2k_frame_attach` 和 `frame.c` 的非客户区尺寸、命中、重绘及 DPI 消息；[S4 P49](M0-T25-S4-P49-tool-resize.md)、[P66](M0-T25-S4-P66-aware-dpi.md)、[P68](M0-T25-S4-P68-real-dialog-icon.md) | 普通、工具窗口和部分系统对话框在所列测试条件下通过；不是所有 DPI 或全部程序。 |
| 标题按钮和菜单 | `frame.c` 的按钮、菜单路径；[S7 P10](M0-T25-S7-P10-physical-caption-buttons.md)、[S5 P9](M0-T25-S5-P9-caption-command-order.md) | 双位宽自有窗口真实鼠标按钮通过；真实 Alt+Space/Escape 及近期鼠标菜单切换探针未通过有效对照，仍未验证。`compact_menu` 对所有者绘制、特殊菜单布局和位图回退原生路径；RTL 不走紧凑路径。 |
| 移动和尺寸 | `frame.c` 的命中与尺寸消息；[S6 P4](M0-T25-S6-P4-natural-resize.md)、[S7 P11](M0-T25-S7-P11-physical-frame-drag.md) | 双位宽自有窗口真实移动及一次拉伸通过，连续过程闪烁、跨屏 DPI 和其他应用真实拖动未证明。 |
| 实际程序 | [S7 P1](M0-T25-S7-P1-real-apps.md)、[P5](M0-T25-S7-P5-apps-and-restore.md)、[P7](M0-T25-S7-P7-charmap.md)、[P8](M0-T25-S7-P8-delphi-window.md)、[P12](M0-T25-S7-P12-winver-window.md) | 所列实际程序有限消息和像素检查通过；不代表用户已验收全部视觉，也不代表所有 Win32 程序。 |
| 生命周期和 Win16 | `win2k-window/frame.h` 仅有附着入口；[S7 P2](M0-T25-S7-P2-cli-rollback.md)、[P6](M0-T25-S7-P6-restore-boundary.md) | 默认和显式 Win16 有聚焦回归；Win2000 运行中移除/恢复尚未实现。`--2k --global` 明确拒绝，不能称为支持全局切换。 |
| 构建和发布 | [S5 P9](M0-T25-S5-P9-caption-command-order.md)干净双位宽构建和发布后聚焦测试；S7 P13 八文件摘要复核 | 两个发布位置与已测试批次摘要一致；完整包矩阵含用户禁止的隔离桌面，未执行，不计通过。 |
| 验收及遗留 | [S7 P13](M0-T25-S7-P13-closure.md)、[TODO](../states/TODO.md) | 以上未覆盖项继续开放。用户原版截图和后续反馈是设计依据，但不代表最新发布版本已完成手动视觉验收。 |

本次为只读源码与证据审计，无产品二进制修改、无新发布，亦未创建隔离桌面。S8 和 T25 保持开放；需继续完成交付与缺口审计，不把本表视为目标完成证明。
