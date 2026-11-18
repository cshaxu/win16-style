# M0 T27 S9 Windows 2000 列表视图和树

从 Win3.x 有证据的有限域起步。列表视图盘点 Report、Small Icons、Icons、List，
以及选中/失焦、图标、列宽顺序、回调与边框。树视图盘点选中/失焦、展开器、分支线、
复选框/状态图、编辑标签、滚动通知和应用回调。Owner draw、owner data、分组、RTL
及应用指定颜色逐项判断保留或适配，记录未验证的组合。

固定 Windows 2000 基线的 listview.c、lvrept.c、treeview.c、tvpaint.c。
Task Manager Processes 提供 Report 样式对照，树至少一张真实原版截图。
原生控件继续拥有布局、焦点、数据、图标、通知和输入。S8 Header 与 S7 滚动条
复用实际路径。自绘通知及生命周期先盘点已有 report、notify 和清理契约，再扩展
必需的接口。应用明确给出的颜色与自绘决定优先。

验收在自有原生控件上分别断言默认与应用覆盖、画面、选中/展开/回调、借用资源
和移除；双宽两套 comctl32 清单及标准启动。聚合同一 S 的覆盖再正式双宽构建，
先发布八件，再跑本批和既有控件的包回归。完成取回核验和清理后准入 S10。

已实际查看原版图片：[Task Manager Processes](https://guidebookgallery.org/pics/gui/system/managers/tasks/win2000pro-1-2.png)（标准报告表头），[My Computer](https://guidebookgallery.org/pics/gui/system/managers/filemanager/win2000pro.png)（大图标与焦点框，SHA-256 `ecd165ff0f688f74b746d9fdb79427cf4cf480ffc45c6be5c451d85fdcdaaaf2`），以及 [Advantech TPC-1260 手册 Figure C-2](https://advdownload.advantech.com/productfile/Downloadfile3/1-17MK9T/TPC-1260_manual_final%28ed.2%29.pdf#page=65)（原图树线和方形加减，PDF SHA-256 `40766b6f0743b5d5af658a2000885461b81f0a09bb42ab9633a6aadae94a523c`）。手册截图有缩放，只作形状对照。

| Windows 2000 comctl32 文件 | 固定 Git blob | 原工作副本 SHA-256 |
| --- | --- | --- |
| `listview.c` | `eddeee5122b5474f2033b67efe8f08e69d40035a` | `cfb0ee3d6489b22255aa3648e237bc7d0e4c55a404ebd0c044ea714622fb0ea6` |
| `lvrept.c` | `241dd7a5bf270061850b196b33d6c0a6d797b11f` | `62833080508b44cbd0546948506a33b673ae9a0b937e49bca206f28ad6ced339` |
| `treeview.c` | `d8ac6480227cb0863d9de4fc36a5e060a77b7d53` | `cc18f7c3ca7c1d1f4168617c3af93902d29294f2b6339632e91b0ab00b2a3829` |
| `tvpaint.c` | `ddaf12f5b26f4681aa09c5e483ec600978a3fb68` | `7dd6274f64e5146c2c550d635728cc7d72646e8eb75f0535db90001c9857ba0e` |
