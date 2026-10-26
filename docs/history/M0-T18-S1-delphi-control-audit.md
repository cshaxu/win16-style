# M0 T18 S1：Delphi Win32 控件全貌审计

S1 冻结 74 个 VCL UI 家族种子并逐项映射到五类证据接收者；集合校验为 74 个计划项、
74 个种子项、零重复、零遗漏。它区分 legacy VCL、现代静态 VCL 和 FireMonkey：相同的
`TButton` 等名称不构成相同的 native HWND 或绘制协议。

本机固定五个候选：DoubleKiller、MD5summer、JoyToKey、cnc-ddraw config 与 HD Tune Pro。
前四者中，前三/前四的具体 VCL 身份分别由 Borland 运行时资料或 `Vcl.Forms`、
`Vcl.Controls`、`Vcl.Themes` 及命名资源证明；HD Tune 没有 Delphi 静态身份，不能充当
Delphi 样本。JoyToKey 的 17 个命名表单资源与 cnc-ddraw config 的 `TConfigForm` 仅证明
VCL 表单资源存在，不能推导内部控件或样式协议。

现有 private-desktop 原生 probe 在本会话返回精确 `PRIVATE_DESKTOP_CREATE_FAIL error=5`；
这被记录为显示观察 skip，不用当前用户桌面替代，也不算实际绘制通过。S1 无产品源码或
二进制变化，四文件保持原摘要。S1 关闭并顺序准入 S2：只对已证明可映射的 USER/COMCTL
控件建立 demo 对照；T18/M0 保持开放。
