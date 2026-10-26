# M0 T17 S14：VB6 DriveListBox 样本审计

S14 只解决一个前提：是否已经有真实 MSVBVM60 DriveListBox，可以据此把现有
VB5 路径扩展为 VB6。结论是否定的，故没有产品源码、二进制或发布变化。

固定 Code Architects 提交中 `photoalbum/VB6/PhotoAlbum.frm` 的确声明
`VB.DriveListBox`；然而同提交的 `PhotoAlbum.exe` 固定 SHA 对应的实际导入是
`MSVBVM50.DLL`，此前原生观察也得到 `ThunderRT5DriveListBox`。源码目录名不能
推翻实际 PE/运行库身份，所以该样本仍只证明已交付的 VB5 路径。

复核固定 28 个 VB6 表单和 214 个 VB6 archive 后，没有第二个 DriveListBox 候选；
本地三个 runtime60 候选也没有该控件证据。因而不添加推测性的
`ThunderRT6DriveListBox` alias，不运行任意未知二进制，也不让缺少 UAC/显示会话成为
本次审计的假阻塞。完整固定身份、计数及接收条件见
[S14 证据](../etc/evidence/vb6-drivelistbox-audit.json)。

S14 关闭后，T17 顺序推进到 S15：Image/Shape/Line/Timer 等内建无窗口或非视觉对象的
真实承载/绘制边界审计。它们不会因为没有独立 HWND 而被错误地接管；M0/T17仍开放。
