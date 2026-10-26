# M0 T17 S13：图形选择控件

S13 接纳的范围是已附着 VB 容器中的精确 runtime-owned `BS_PUSHLIKE`
CheckBox/OptionButton。旧路径刻意拒绝这些控件，导致运行库的自动选中状态仍能
工作、但按钮面没有采用本项目经典绘制。

实现保留原有 runtime/owner、公共 BUTTON 类型、`BS_PUSHLIKE`、无 `BS_ICON`/
`BS_BITMAP` 的全部门槛，再复用既有 face painter 与 `BM_GETCHECK`。没有读取 VB
私有对象，也没有接管 Value、分组、Click、键盘、图像或未知 role 变化。移除样式
后控件恢复原生路径。

纯 C 原生 `core-vb-graphical-choice` 在 x86/x64 均验证 graphical checkbox 的
选中与 retirement，以及 option 的 `a1=1,b1=0 -> a2=0,b2=1` 原生分组状态。
两个宽度的同批候选四文件通过 package checker 和 package-driven focused probe。
该候选的 SHA-256、发布位置和限制见
[S13 证据](../etc/evidence/vb-graphical-choices.json)。x64 是协议模型而非不存在的
x64 VB 运行库声明；实际屏幕/UI 矩阵因执行会话权限未覆盖，也没有被伪装为通过。

P1、P2、P3 已推送并实际远端复核。P3 将已测同一四文件包同步到 `assets/` 根目录及
`O:\\winnt\\system32`。S13 至此关闭，自动顺序准入 S14 做真实 VB6 DriveListBox
样本与身份审计；复杂图片/位图/owner-draw 选择变体与最终 148 单位审计仍由后续 S
接收，M0/T17 保持开放。
