# M0 T17 S25：VB ComboBox 变体

S25 将真实 VB5/VB6 的 editable ComboBox 和 Dropdown List 源码与公共 COMBOBOX 样式
对应，并补充 `CBS_SIMPLE` 自有 runtime witness。x86/x64 注入均确认三种类型的附着、
选择、弹出/取消和禁用状态；裸路径保持原生协议。生产源与四个发布二进制没有改动，
详见 [S25 证据](../etc/evidence/vb-combo-variants.json)。
