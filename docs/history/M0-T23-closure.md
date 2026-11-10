# M0 T23 有限收口

用户明确要求在未进行 S7 手动验收的情况下收口。T23 是有限外框适配，不能据此声称所有 Win32、UWP 或 WinUI 程序均已支持。

- Notepad、Explorer、Chrome 已获用户验收；Terminal/cmd/conhost 基本验收，Terminal 原生按钮重复仍在 TODO。
- Task Manager 有限收口，重复图标、白条和拖动丢帧仍在 TODO；Codex Desktop 按用户要求跳过。
- Paint、截图工具及设置的观察范围与设备管理器未获提权的限制见 [S7 交付记录](M0-T23-S7-P1-system-apps.md)。用户未进行 S7 手动验收；短命令别名错误 1920 保留 TODO。
- S7 P1 同批四文件已发布并测试，源码提交 `8eee38b8`，后续复核记录 `2221d875`。本次不修改产品二进制，不把此前有限测试扩大为完整交互或任务级全覆盖。
