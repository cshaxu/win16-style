# M0 T17 S23：VB CommandButton 状态变体

S23 将真实 VB5/VB6 `Default=True`/`Cancel=True` 源码与现有公开 BUTTON 路径分开。
仅 `BS_DEFPUSHBUTTON` 是安全、可观测的绘制输入；Cancel 保持 VB/dialog 的命令路由。一个
`Appearance=Flat` 的真实源样本同时是 `Style=Graphical` 且有 Picture，因此继续由 S10 的
受限 owner-draw/DC 路径处理，不能假造 `BS_FLAT` 支持。

自有 C17 runtime 模型新增精确 ThunderRT6CommandButton default witness。实际 x64/x86 注入
均确认附着、默认边框、运行时 style 转换、禁用、Space、两次 `BN_CLICKED` 通知和拆卸；
裸模型仍由原生 BUTTON 处理。生产源和四个发布二进制均未改变，详见
[S23 证据](../etc/evidence/vb-command-button-variants.json)。
