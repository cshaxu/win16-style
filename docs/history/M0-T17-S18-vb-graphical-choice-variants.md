# M0 T17 S18：VB 图形选择图片与图标变体

S18 将精确 runtime-owned、`BS_PUSHLIKE` 的 VB CheckBox/OptionButton 图片和图标
公共路径纳入既有 BUTTON painter。此前 S13 一律拒绝 `BS_BITMAP`/`BS_ICON`，即使
既有 painter 已经只借用 `BM_GETIMAGE` 取得的句柄。此处仅取消该两位的拒绝；运行时
仍拥有图片、`Value`、分组、事件、字体和布局，renderer 不替换或删除句柄。

双位宽 C17 原生夹具验证了 bitmap/icon 的 `BM_SETIMAGE`/`BM_GETIMAGE` 身份、点击、
禁用、退役和资源释放；x64 launcher 注入 x64 与 x86 目标均通过。既有 `core-vb`
回归也通过，并把销毁回调用显式 `WM_SETFONT` 驱动，避免把 `SetWindowLongPtr` 误当成
消息广播。完整证据、包摘要和明确未覆盖面见
[S18 证据](../etc/evidence/vb-graphical-choice-variants.json)。

没有真实 VB 图片属性样本，因此不把模型证明扩张为任意 VB 图片控件；owner-draw、复杂
变体和所谓 default CheckBox/OptionButton 仍保持原生。用户未确认的 UAC/显示观察也不
阻塞本 S，且没有被计为通过。
