# M0 T25 S7 P13：真实程序回归有限收口

S7 覆盖 Solitaire、扫雷、旧版记事本、demo、字符映射表、Delphi 样本及系统版本窗口的有限外框验证，详见 [P1](M0-T25-S7-P1-real-apps.md)、[P5](M0-T25-S7-P5-apps-and-restore.md)、[P7](M0-T25-S7-P7-charmap.md)、[P8](M0-T25-S7-P8-delphi-window.md) 和 [P12](M0-T25-S7-P12-winver-window.md)。[P9](M0-T25-S7-P9-propagated-frames.md)验证四种父子位宽组合各自的附着和标题采样；[P2](M0-T25-S7-P2-cli-rollback.md) 至 [P4](M0-T25-S7-P4-cli-process-cleanup.md)检查失败回滚和目标退出；[P10](M0-T25-S7-P10-physical-caption-buttons.md)、[P11](M0-T25-S7-P11-physical-frame-drag.md)覆盖自有双位宽窗口的真实鼠标按钮、移动和拉伸。

上述为有限技术收口，不代表 T25 已完成。Win2000 运行中样式移除尚未实现；真实 Alt+Space/Escape、真实菜单切换、连续拉伸的瞬时闪烁、跨屏 DPI、完整包矩阵和用户视觉验收仍未通过或未覆盖，交 S8 逐项审计，不得按通过报告。用户禁止隔离桌面，完整包中相应测试未运行。[TODO](../states/TODO.md) 保留这些缺口。

产品二进制未在 S7 更改，assets 与 `O:/winnt` 的八文件摘要一致，发布基线仍为 [S5 P9](M0-T25-S5-P9-caption-command-order.md)。本记录只关闭 S7 有限回归并准入 S8 最终审计，不关闭 T25；下一个 T 的样式库重组仍在队列。工作区另有未提交 proposal 修改，本次不覆盖或提交。
