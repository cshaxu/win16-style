# M0 T17 S27：VB 滚动条状态变体

S27 固定真实 VB5/VB6 H/VScrollBar 与 ImageProcessor 动态 Max/Visible 来源。新增自有
runtime x86/x64 witness 直接验证 H/V 附着、范围、页大小、位置和禁用状态。私有桌面
API probe 因 ERROR_ACCESS_DENIED (5) 跳过，未用于通过结论；renderer 不保存或解释应用
业务值。生产源和四个发布二进制没有改动，详见 [S27 证据](../etc/evidence/vb-scrollbar-variants.json)。
