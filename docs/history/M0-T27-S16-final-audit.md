# M0 T27 S16 全任务终审

以 S1 冻结的 24 个标准组、8 个运行库和复合组为审计全集。逐项核对
当前代码、原版源码/图像、实际包标记和限制。原版可见状态来自真实图，
未在图中见证的输入与回调行为以原版源码及现有验证为准。

用 S15 发布的八件和同摘要用例作最终基线。取回并复核所有相关交付提交
与当前发布物。终审只调整文档，保留八件、来源、测试和构建输入对应关系。
真实托盘、物理输入、私有桌面、其他控件年代和未验证复杂变体分别留有明确记录。

完成后收口 S16 与 T27，M0 保持固定。

## 逐组处置

按 S1 顺序核对 24 个标准组。每行的来源和实测边界见所列交付证据。

| 控件 | Demo 入口 | 结果和范围 | 证据 |
| --- | --- | --- | --- |
| push-button | Buttons and states | 有限呈现。普通/默认、按下、禁用、焦点、原生图文借用；应用自绘和独立的现代图片列表路径保持原样。 | [S2](../etc/evidence/t27-s2-buttons.json) |
| checkbox | Buttons and states | 有限呈现。原版状态字形、按下/禁用和 push-like；原生复选值、通知及父背景。 | [S3](../etc/evidence/t27-s3-choice-controls.json) |
| radio-button | Buttons and states | 有限呈现。普通/自动单选、原版权重与字形；原生分组、键盘和消息。 | [S3](../etc/evidence/t27-s3-choice-controls.json) |
| three-state | Buttons and states | 有限呈现。空、勾选、半选及正常/按下/禁用；保持原生三态。 | [S3](../etc/evidence/t27-s3-choice-controls.json) |
| static | Containers and drawing | 有限呈现。普通文字、色块、边线和蚀刻；原生图标、位图、应用自绘与复杂组合按其专属路径。 | [S4](../etc/evidence/t27-s4-static-group.json) |
| group-box | Containers and drawing | 有限呈现。普通分组线和标题间断，父背景、借用字体和对齐；应用自绘容器保留。 | [S4](../etc/evidence/t27-s4-static-group.json) |
| edit | Editing and lists | 有限呈现。标准边缘、默认画刷、只读、密码与禁用；文字、选择、输入法、应用刷与数据继续原生。 | [S5](../etc/evidence/t27-s5-edit-controls.json) |
| listbox | Editing and lists / Files and scrollbars | 有限呈现。普通列表及验证的 TAB stops、选择和边框；owner-draw 由应用处理。 | [S6](../etc/evidence/t27-s6-list-combo.json) |
| combobox | Editing and lists | 有限呈现。普通 SIMPLE / DROPDOWN / DROPDOWNLIST 与原生子窗；数据、通知和特殊绘制保留。 | [S6](../etc/evidence/t27-s6-list-combo.json) |
| scrollbar | Files and scrollbars | 有限呈现。独立横纵、非客户区、滚动轨迹和 sizebox；镜像非客户布局保留原生。 | [S7](../etc/evidence/t27-s7-scroll.json) |
| tab | Lists and toolbars | 有限呈现。普通标签及文图、选中、焦点和热点；非标准方向、自绘与高级变体保持原生。 | [S8](../etc/evidence/t27-s8-tab-header.json) |
| header | Lists and toolbars | 有限呈现。普通表头尽量使用原样式边线和背景；复杂排序、过滤、调整列宽和数据由原生/应用处理。 | [S8](../etc/evidence/t27-s8-tab-header.json) |
| listview | Lists and toolbars | 有限呈现。普通文本项及验证的选择状态；复杂图像、整行选择、透明、owner-data、自绘保持原生。 | [S9](../etc/evidence/t27-s9-report-tree.json) |
| treeview | Lists and toolbars | 有限呈现。普通树的颜色与边缘；层级、展开、图像、连接线和展开方框由原生拥有。 | [S9](../etc/evidence/t27-s9-report-tree.json) |
| progress | Ranges and dates | 有限呈现。v6 普通分段/平滑、横纵和颜色；v5 仅经典边缘，动态模式原生保留。 | [S10](../etc/evidence/t27-s10-progress.json) |
| trackbar | Ranges and dates | 有限呈现。原版通道、滑块与刻度；特殊几何、方向变体及未验证的选择通道保留原生。 | [S11](../etc/evidence/t27-s11-track-spin.json) |
| updown | Ranges and dates | 有限呈现。正常、按下、禁用箭头与框线；伙伴控件、数值和通知留在原生。 | [S11](../etc/evidence/t27-s11-track-spin.json) |
| toolbar | Lists and toolbars | 有限呈现。普通或平面按钮和平面分隔，保留应用文字、图像、分裂按钮等复杂路径。 | [S12](../etc/evidence/t27-s12-bars.json) |
| statusbar | Lists and toolbars | 有限呈现。普通分区、凸起、无边框与 sizegrip；原生分区和所有者数据保留。 | [S12](../etc/evidence/t27-s12-bars.json) |
| rebar | Lists and toolbars | 有限呈现。普通水平抓手；原生布局、伙伴、带文字和带图、自定背景保留。 | [S12](../etc/evidence/t27-s12-bars.json) |
| datetime | Ranges and dates | 有限呈现。普通下拉按钮、弹层与默认原版角色；时间微调、复选框和自定义解析保留原生。 | [S13](../etc/evidence/t27-s13-dates.json) |
| monthcalendar | Ranges and dates | 有限呈现。普通单选月历的标题、选择，应用自设颜色及导航保留；多选继续原生。 | [S13](../etc/evidence/t27-s13-dates.json) |
| richtext-and-hotkey | Text and special objects | 仅经典边缘。RichEdit 20A/20W、50W 及原生热键的边缘/已有编辑协议；富文本内容、格式、快捷键状态由原生拥有。部分原生离屏客户打印跳过。 | [S5](../etc/evidence/t27-s5-edit-controls.json) |
| syslink-modern-only | — | 原生保留。Win2000 无对应 SysLink。沿用现代原生链接显示和输入，本行无年代复刻。 | [S4](../etc/evidence/t27-s4-static-group.json) |

8 个运行库或复合组依据原有公用协议及 [S14 样本与模型](../etc/evidence/t27-s14-runtime.json)核对。

| 运行库组 | 结果和范围 |
| --- | --- |
| vb-native | 有限呈现。角色匹配标准控件后复用相应年代呈现。已观察原作者 VB5/6 各 20 个控件，Actor x86 的 5 个按钮/复选框。64 位为模型。 |
| vb-containers-and-owner-roles | 有限呈现。标准 Frame 和实际原生子窗由共同协议接收；文件列表、Checklist 和窗口外绘制交给应用。 |
| vcl-native-wrappers | 有限呈现。已确认标准公开协议与来源的包装。同一实现验证 DoubleKiller x86 的 10 个按钮；MD5summer 主窗跳过。 |
| vcl-text-and-containers | 部分保留。可确认的原生 STATIC 或 BUTTON 子窗沿用标准呈现；专用 Panel、Group 和 ControlBar 外壳保留应用绘制。 |
| vcl-grid-file-inplace | 部分保留。已确认的标准编辑/列表/组合子窗可应用；表格、文件驱动器、就地业务和自绘按原控件处理。 |
| vcl-data-navigation-media | 部分保留。原生标准子窗适用；导航、媒体、图表、位图和应用定义内容由应用拥有。 |
| vcl-special-composites | 部分保留。公开协议证实的公用控件可应用；专用色盒、日期、状态栏及富编辑包装无独立见证时保留。 |
| ocx-and-unsupported | 原生保留。仅沿用已证实同模块原生子窗协议，未知或非可视的 OCX、windowless ActiveX、ownerdraw 无新增认证。 |

## 交付复核

原版固定提交 `fd809a81098565b33f52d0f65925159de8f4c337` 的 32 个冻结来源按原有 SHA-256 核查。图像仅覆盖各阶段可见状态。图标、程序图片和字体的所有权及缺失的原资源一并留在分批证据。产品始终使用既有五个原图形头及来源声明。

年代绘制从 ntw2ks-dll 的 controls 装配进入 win2000/control；控件身份、事件、原生状态、共用生命周期在 application 层。Demo 保持纯原生，不链接样式库。程序文件和注册表无本任务新增依赖。

S15 P1 `73f60c5a` 的八件继续在 assets 根和 O:/winnt，以 SHA-256 对照实际远端。当前 1228 项构建输入及八件沿用同一快照。S1 至 S15 的已取回产品字节再次从实际对象核查，终审未作二次构建或发布。

正式包聚焦回归 185/185。既有 36 项客户区打印子检查跳过。Win2000、Win3.x 和原生各观察九页；六方向均以 x86 和 x64 自有窗口检查附着、标准按钮颜色、输入值及恢复像素。真实管理员托盘、物理输入、私有桌面和完整集成矩阵未执行。

MD5summer 主窗、广泛真实 VCL/OCX、用户自绘、Win2000 公共对话框转换及 UWP/WinUI 客户区保持在各自证据或后续债务边界。用户可用 `O:/winnt/run16s.exe --2k O:/winnt/demo.exe` 手验收。证据台账含原件摘要和所有对应阶段。
