# M0 T25 S4 P73：标题与外框阶段复核

S4 的普通 Win32 标题和外框实现及有限技术验证完成：原版截图和只读源码对照见 [P59](M0-T25-S4-P59-physical-border.md)、[P60](M0-T25-S4-P60-caption-inset.md)、[P62](M0-T25-S4-P62-caption-separator.md)；活动/失活标题及图标见 [P63](M0-T25-S4-P63-window-activation.md)、[P68](M0-T25-S4-P68-real-dialog-icon.md)、[P69](M0-T25-S4-P69-dialog-custom-icon.md)；失活菜单的实际 Solitaire 模态场景见 [P70](M0-T25-S4-P70-inactive-menu.md)、[P71](M0-T25-S4-P71-solitaire-about.md)。x86/x64 普通、工具及对话框窗口的绘制和几何由 P70 同批包的 42 项聚焦回归覆盖；P72 补充了两种位宽的禁用按钮真实鼠标输入。

P70 纯 C17、MinGW GCC、Ninja、ccache 双位宽干净构建后，先发布再测试；发布的八文件和测试包摘要一致。复核时 assets 根目录、`O:/winnt` 与 P70 记录一致，实际远端主线为 `2dda66d455f09b2c158a1fb229a165e094a6d54e`，包含 P70 及后续验证。S4 按这些已完成的技术范围收口，T25 保持开放。

用户对当前外观的手动视觉验收尚未收到，保留至 T25 最终验收，不记作通过。其他显示器 DPI、跨屏、跨进程焦点、真实应用全矩阵、自绘菜单及 RTL 未证实；S5–S8 按既有计划继续，不以 S4 的聚焦通过替代这些范围。P66 曾有一次并行私有桌面释放失败，串行复测通过但原因未定。当前无产品二进制变更或新发布，也无本次临时构建。
