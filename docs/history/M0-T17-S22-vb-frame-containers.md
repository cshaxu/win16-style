# M0 T17 S22：VB Frame 与嵌套容器变体

S22 核实 Frame 是独立的 runtime-owned 容器 HWND，而 Frame 中的 Label 仍为父 DC
绘制。真实 Mouse 样本包含带 caption 的 Frame、TextBox 与 Label；来源中的 PhotoAlbum
Frame 还嵌套文件控件，但其可运行身份是已记录的 VB5 mismatch，不能外推成 VB6 运行时
结论。

现有 renderer 只将精确 runtime Frame 接入既有 group-box painter，子 HWND 仍各自分类，
无窗口 Label 继续走父 DC 文本范围。它不强制传播可见性/禁用状态，从而保留 VB runtime
的容器语义。S4/S9 的既有原生证据覆盖这一有限边界，因此没有产品二进制改动。详见
[S22 证据](../etc/evidence/vb-frame-containers.json)。
