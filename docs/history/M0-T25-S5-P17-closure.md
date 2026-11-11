# M0 T25 S5 P17：标题按钮与系统菜单有限技术收口

标题按钮禁用后的命令检查和同步发送由 [P5](M0-T25-S5-P5-disabled-close-command.md)、[P9](M0-T25-S5-P9-caption-command-order.md)修复；P9 的双位宽干净构建按先发布后测试交付，按钮命令、拖离取消、帮助、禁用和关闭通过聚焦测试。[P14](M0-T25-S5-P14-button-capture-loss.md)补充捕获丢失/取消，[P8](M0-T25-S5-P8-menu-cancel.md)验证程序取消消息；[P2](M0-T25-S5-P2-menu-keyboard.md)、[P7](M0-T25-S5-P7-caption-context-menu.md)覆盖系统菜单消息路径。[P13](M0-T25-S5-P13-solitaire-dialog.md)检查实际 Solitaire 活动对话框和失活主窗口。此前双位宽禁用按钮真实鼠标覆盖见 [S4 P72](M0-T25-S4-P72-disabled-buttons.md)。

本 S 仅按上述已验证技术范围收口，不声称全量通过。真实 Alt+Space/Escape 仍未验证：[P1](M0-T25-S5-P1-system-menu-baseline.md)套样式测试超时的原因未知；程序消息及 [P4](M0-T25-S5-P4-escape-input-control.md)的无效原生负对照不能替代真实按键。另有 [P15](M0-T25-S5-P15-menu-switch.md)应用菜单方向键用例首次超时、复测通过但根因未知。用户禁止隔离桌面，完整包矩阵未运行；其他 DPI、用户视觉验收均未完成。这些继续由 T25 后续工作和最终审计承担，不计作通过。

当前发布包仍是 P9，八文件已发布 assets 与 `O:/winnt`；S5 后续仅测试、文档和队列变更。T25 保持开放，按已授权序列准入 S6；下一个 T 的样式库重组仍在队列，尚未准入。
