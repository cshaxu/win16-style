# M0 T17 S20：VB 控件数组重复实例生命周期

真实 Mouse VB5/VB6 源码已证明 `OptionButton optTest(0/1)` 和多个图形 CommandButton
是控件数组：共同的控件名不代表共享 HWND 或共享 renderer 状态。S20 为现有精确 runtime
夹具加入三成员图形 CheckBox 阵列：分别设值、销毁中间成员、重建相同 ID，并显式退役
一个兄弟成员。

x64/x86 注入与无注入对照均通过，验证幸存成员状态独立、重建成员重新附着、单成员退役
不会影响兄弟。产品 renderer 已按 HWND 管理，故本 S 只新增 C17 回归和来源回执；当前
发布四文件保持 S18 哈希，不重复发布。完整边界见
[S20 证据](../etc/evidence/vb-control-arrays.json)。
