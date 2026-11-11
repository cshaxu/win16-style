# M0 T25 S4 P72：禁用按钮真实输入

只读原版 `ntuser/kernel/caption.c` 的 `xxxCalcCaptionButton` 后，分别以不含 `WS_MINIMIZEBOX` 或 `WS_MAXIMIZEBOX` 的窗口样式表示禁用对应按钮，以系统菜单灰显 `SC_CLOSE` 表示关闭不可用。对 [P70](M0-T25-S4-P70-inactive-menu.md) 发布版，在可见私有桌面用 `SendInput` 点击自有窗口的禁用按钮和关闭按钮：x86/x64 两种按钮组合均观察到 unavailable 状态，真实鼠标点击未最小化、最大化或关闭窗口。结果只覆盖此窗口和本机 DPI，不等同所有应用和状态。

最初探针错误地假设单独灰显系统菜单中的最小化/最大化命令会使标题按钮的 accessibility 状态变为 unavailable；当前 Windows 实际报告这两个状态仍可用。合成非客户区按下进入原生跟踪，探针两次超时并由自建 job 清理；该测试既不能证明产品缺陷，也不能算通过。随后按照窗口样式设置和实际鼠标输入重新测试，四项通过。

只修改双位宽自有测试和来源摘要；增量构建、来源清单和治理检查通过。产品二进制未变化，assets 与 `O:/winnt` 八文件摘要与 P70 一致；未运行完整包矩阵。仅使用保留的 `build/.ninja`，没有临时构建副本。S4、T25 保持开放。
