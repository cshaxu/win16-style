# M0 T17 S21：VB Label 边框、对齐与无窗口承载

S21 确认了一个不能用普通控件附着解决的边界。已登记 VB5/VB6 源码拥有固定单线、
Inside Solid、左/右/居中对齐和控件数组 Label；但真实 S4 运行时窗口枚举并不包含
Label HWND。Label 由其 FormDC/PictureBox 父对象在运行时 DC 上绘制。

现有 `vb_text` 已在精确 runtime 父绘制范围内借用 System 字形，并以 label-only 屏障
避免穿透到不相关的嵌套子控件。对 Label 另猜一个 `STATIC` HWND 或重画其边框会破坏
运行时的对齐、边框、图片邻接内容和 DC 所有权，因此本 S 不作这种伪修复。没有产品
二进制变化；完整边界见 [S21 证据](../etc/evidence/vb-label-variants.json)。
