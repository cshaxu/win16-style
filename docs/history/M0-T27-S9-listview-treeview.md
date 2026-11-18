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
| `lvicon.c` | `3a4d03459268101f504e1fffe508d20a7e7af906` | `a1a35a4b0fc784730a30eac2d52c7e98089b1a617b67143e8da4deffa9fd2d23` |
| `treeview.c` | `d8ac6480227cb0863d9de4fc36a5e060a77b7d53` | `cc18f7c3ca7c1d1f4168617c3af93902d29294f2b6339632e91b0ab00b2a3829` |
| `tvpaint.c` | `ddaf12f5b26f4681aa09c5e483ec600978a3fb68` | `7dd6274f64e5146c2c550d635728cc7d72646e8eb75f0535db90001c9857ba0e` |

原版树的 TV_DrawItem 先区分活动选中、禁用和失焦选中，再让应用从
`NM_CUSTOMDRAW` 覆盖文本及背景。报告视图的列宽和顺序由 Header 提供，
网格线采用 `COLOR_3DFACE`。先共用 S6 已核对的 Windows Standard
选中色与文本色，在原生绘制回调中验证默认色映射和应用覆盖，之后再决定
哪些像素确实需要由年代呈现绘制。独立测试不复制系统布局或图标状态机。

首批增量回归登记 `build/t27-package/s9-palette-dev-01`。验证共用
Win2000 选中色表后的 ListBox、Combo 和 Tab 旧用例。

选中色表回归双宽 18/18。独立原生见证使用同一控件行为，变化仅是
清单和绘制通知处理。Win11 的 v6 ListView 在保持本次绘制快照的
`CDIS_SELECTED` 时，覆盖明确提交的背景色。v5 的非报告模式也有
同类现象。对普通文本项，解除的只是本次 `NM_CUSTOMDRAW` 的
绘制标记，保留原生 `LVIS_SELECTED`、输入、布局和应用拥有的数据。
图像列表、整行选中、边框选择、透明背景和复杂格式保持原生呈现。
TreeView 原生直接消费填入的颜色，继续拥有分支线和方形加减框。

内嵌 Header 的实际原生样式 `0x500000c2` 含列拖放及全拖动。
现有 S8 表头绘制可复用，列顺序、列宽和拖动仍由 ListView/Header
处理。首次展开后，原生 `TVIS_EXPANDEDONCE` 会抑制后续相同消息
的展开通知，探针分别检查首次通知、后续状态及加减框像素。
[TVM_EXPAND 原生契约](https://learn.microsoft.com/windows/win32/controls/tvm-expand)。

双宽合并回归 20/20。另用开发探针验证 v5/v6 两宽标准 `run16s --2k` 入口，
4/4。应用图像列表与自绘保持，树的加减框像素由原生绘制并经独立断言。
登记正式构建 s9-report-tree-release-01、发布 s9-report-tree-publish-01
及发布包验证 s9-report-tree-test-01。保留图像项、整行选择与复杂样式的原生
呈现，真实焦点切换另计。[开发实测证据](../etc/evidence/t27-s9-report-tree.json)。

正式双宽构建和同快照八件发布完成。先发布至 assets 根和 O:/winnt，再执行
108/108 进程用例。12 个新增用例覆盖 v5/v6、直接应用、DPI 感知及标准启动。
原有 36 项客户区打印子检查保留跳过。ListView 复杂选择按记录保留原生呈现，
真实前台焦点、物理拖列及所有状态图变体未验证。

P1 `2afaa0f6` 已推送并实际取回复核。29 个变更文件、1207 项构建输入
及八件产品校验完成。构建、发布备份、探针和下载临时资料共 428006415 字节列入收口清理。
保留稳定 Ninja 和 ccache。
