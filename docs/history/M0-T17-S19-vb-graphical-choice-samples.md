# M0 T17 S19：VB 图形选择真实图片样本审计

S19 对既有登记样本做了按 `Begin`/`End` 控件块边界的只读扫描。两个可运行的
Mouse 样本（VB5 与 VB6）各有一个普通 CheckBox、两个普通 OptionButton 和四个图形
CommandButton；选择控件没有 `Picture` 或图形 `Style`。可运行的 3Ddefline VB6 样本有
十个图片控件，但它们全部是 CommandButton。两个补充源码候选也只有普通 CheckBox，且
没有获认证的同源可运行观察。

因此没有把 S18 的自有 public-protocol bitmap/icon 模型虚构成真实 VB 图片选择控件支持。
本 S 无产品改动、无重建或发布；当前 assets 四文件仍是 S18 已测包。完整计数、样本路径和
未覆盖面见 [S19 证据](../etc/evidence/vb-graphical-choice-samples.json)。
