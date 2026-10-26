# M0 T17 S24：VB TextBox 特殊变体

S24 固定真实 VB5/VB6 多行双滚动条 TextBox 源码，以及 ImageProcessor 的 Locked TextBox
语义。精确 runtime-owned TextBox 继续走既有 EDIT 路径；多行窗口才启用已有的经典滚动条
承载，文本与输入协议不重写。

自有 C17 runtime witness 在 x86/x64 注入下验证多行、水平/垂直滚动样式、Undo、只读状态
下的键盘拒绝、密码字符、居中、字体/像素退役；裸路径保持原生 EDIT。Windows 允许
程序化 `EM_REPLACESEL` 写入只读 EDIT，故测试不将其误判为用户输入拒绝。生产源与四个
发布二进制没有改动，详见 [S24 证据](../etc/evidence/vb-textbox-variants.json)。
