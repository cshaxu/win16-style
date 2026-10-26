# Supporting Material Index

- [VB图片变体](evidence/vb-picture-variants.json)：T17 S11真实十按钮/四状态像素、
  同批包、许可与原生观察器限制；不作为图形Check/Option或全集支持声明。

- [VB观察器与生命周期](evidence/vb-observer-lifetime.json)：T17 S12有限GDI平台期、
  临时IAT观察器严格诊断及非交互桌面边界；不作为全VB应用零增长或真实样本根因声明。

- [VB图形选择控件](evidence/vb-graphical-choices.json)：T17 S13精确图形
  CheckBox/OptionButton的公开协议、双宽测试、同批四文件发布与保留变体；不作为
  所有 VB owner-draw 或图片按钮支持声明。

- [VB图形选择图片/图标变体](evidence/vb-graphical-choice-variants.json)：T17 S18
  精确公开 BM_GETIMAGE/BM_SETIMAGE 路径的双宽状态、资源和退役证据；不作为任意
  VB 图片属性或 owner-draw 支持声明。

- [VB图形选择真实图片样本](evidence/vb-graphical-choice-samples.json)：T17 S19
  已登记 VB5/VB6 图片样本的控件块审计；确认没有真实图片 CheckBox/OptionButton，
  不将模型证明外推为真实控件支持。

- [VB控件数组](evidence/vb-control-arrays.json)：T17 S20真实 VB5/VB6 控件数组
  源码依据及双宽独立 HWND 销毁/重建/退役回归；不作为 VB 私有集合支持声明。

- [VB Label变体](evidence/vb-label-variants.json)：T17 S21真实边框/对齐/数组
  Label 与运行时无窗口 DC 承载边界；不作为独立 STATIC 控件支持声明。

- [VB Frame容器](evidence/vb-frame-containers.json)：T17 S22真实 Frame/嵌套
  子控件边界与现有有限 renderer 路径；不作为私有容器状态支持声明。

- [VB CommandButton状态变体](evidence/vb-command-button-variants.json)：T17 S23
  default/cancel/flat 的真实公共边界与双宽 runtime witness；不作为私有对话框路由支持声明。

- [VB TextBox特殊变体](evidence/vb-textbox-variants.json)：T17 S24多行、只读、密码和
  对齐的真实公共边界与双宽 runtime witness；不作为富文本或IME完整支持声明。

- [VB ComboBox变体](evidence/vb-combo-variants.json)：T17 S25三种公共组合框样式的
  真实来源边界与双宽 runtime witness；不作为owner-draw支持声明。

- [VB ListBox变体](evidence/vb-listbox-variants.json)：T17 S26真实样本与多选/排序
  证据缺口的边界；不作为owner-draw或未见样式支持声明。

- [VB ScrollBar变体](evidence/vb-scrollbar-variants.json)：T17 S27真实动态范围/可见
  性边界与既有API回归；不作为物理拖动验收声明。

- [VB6 DriveListBox审计](evidence/vb6-drivelistbox-audit.json)：T17 S14固定
  源码/PE/runtime 身份与有界样本缺失结论；不作为VB6 DriveListBox支持声明。

- [VB无窗口内建控件审计](evidence/vb-windowless-controls.json)：T17 S15对
  Image/Shape/Line/Timer的应用绘制或非视觉边界；不作为宿主全量换肤声明。

- [VB特殊内建宿主审计](evidence/vb-special-hosts-audit.json)：T17 S16对Data、
  OLE、MDIForm、UserControl的数据/嵌入对象/宿主边界；不作为这些对象的换肤声明。

- [VB扩展OCX审计](evidence/vb-extension-ocx-audit.json)：T17 S17对冻结官方扩展
  模块/控件行与本机模块缺失的有界结论；不作为任意OCX支持声明。

| 路径 | 责任、用途及保留条件 |
| --- | --- |
| [evidence/outline-original.json](evidence/outline-original.json) | T16原始Win3.1 USER/GDI/VGA身份、调用地址、USER与VGA例程CPU执行参数；不包含系统二进制，不冒称完整VGA硬件/调色板运行 |
| [screenshots/README.md](screenshots/README.md) | 用户明确要求公开的精选demo/真实XP Solitaire截图及英文来源/摘要；非原始测试抓图 |
| [evidence/vdm-propagation-diagnosis.json](evidence/vdm-propagation-diagnosis.json) | 原生命令传播、冷/热/历史与双钩子独立诊断的精选状态/原始摘要；不替代产品包回归 |
| [evidence/native-cmd-staging.json](evidence/native-cmd-staging.json) | T9真实嵌套cmd、外部导入表正负例及脱敏进程图/模块SHA；完整身份不公开 |
| [evidence/test-efficiency.json](evidence/test-efficiency.json) | Td S9同集合串/并行、热增量、共享cache、完整回归和实际包耗时；仅公开计数/原始摘要 |
| [evidence/uac-elevation.json](evidence/uac-elevation.json) | T11双宽UAC、实际高令牌/窗口/子孙、负例、回归及未定位回滚计数失败；原始身份不公开 |
| [evidence/xaml-research.json](evidence/xaml-research.json) | 用户取消的XAML研究事实及原始摘要；无换肤或产品支持声明 |
| [evidence/legacy-iat-staging.json](evidence/legacy-iat-staging.json) | T12缺失IAT原生断点、双宽负例、真实DoubleKiller、创建对象身份判据及包验证证据 |
| [evidence/leaderboard-font.json](evidence/leaderboard-font.json) | Td S10双宽英雄榜同资源字体/墨迹/DPI对照及卸载恢复；无产品修改 |
| [evidence/multilingual-font.json](evidence/multilingual-font.json) | 已被T13 S2撤回的历史多语言控件字体保护：71项逐项覆盖、25实际包、旧DLL负例、真实扫雷/标题、发布及授权占用处置 |
| [evidence/multilingual-font-revert.json](evidence/multilingual-font-revert.json) | T13 S2原样撤回的源身份、加载映像对照、双宽回归及实际包/双发布证据 |
| [evidence/system-shell-launch.json](evidence/system-shell-launch.json) | 已由S2撤回的T14 S1历史系统UI方案及交付证据 |
| [evidence/legacy-retry-cleanup.json](evidence/legacy-retry-cleanup.json) | T14 S2清理二次Shell、首次stderr/193、原生cmd和自有树清理及发布审计 |
| [evidence/console-error-routing.json](evidence/console-error-routing.json) | T14 S3按控制台分流的原生UI/首次诊断、双旧版负例及双发布/审计证据 |
| [evidence/madshow-controls.json](evidence/madshow-controls.json) | Td S11真实VB6应用的控件分类/原生状态/附着断点、有限候选及产品未变证据 |
| [evidence/vb6-adaptation.json](evidence/vb6-adaptation.json) | T15 VB6分类、客户端字体/边框、真实MadShow对照、资源所有权、实际包及双发布证据 |
| [evidence/doublekiller-controls.json](evidence/doublekiller-controls.json) | T15 S2原生超类、VCL初始化/暂存主题、真实绘制/旧包负例、资源所有权及实际包/双发布证据 |
| [evidence/md5summer-controls.json](evidence/md5summer-controls.json) | Td S12真实MD5summer原生协议/附着/截图对照及识别缺口、TreeView边界；无产品变更 |
| [evidence/hdtune-controls.json](evidence/hdtune-controls.json) | Td S13 HD Tune Pro真实提权主窗/标准控件附着缺口、740及关闭失败；无产品修复 |
| [evidence/framework-completion.json](evidence/framework-completion.json) | T15 S3原生菜单栏/对话框/隐藏状态修复、旧包负例、41实际包/12聚焦、真实三应用及双发布证据；后续状态见CURRENT |
| [release.json](release.json) | 当前开发包的输入/工具链/四文件哈希与验收范围，历史版本从对应Git提交取回 |
| [evidence/control-protocols.json](evidence/control-protocols.json) | T15 S4初始化后框架协议角色、原生图像/PushLike/client-edge、实际应用及失败原型/发布证据 |
| [evidence/tab-controls.json](evidence/tab-controls.json) | T15 S5原生/框架页签、原生布局/通知、字体同步、安装取消所有权与实际包验证；状态见CURRENT |
| [evidence/header-controls.json](evidence/header-controls.json) | T15 S6原生文字表头协议/实际列与像素、通知/取消、回调与内存/DC所有权、有限保留模式及包验证 |
| [evidence/report-list.json](evidence/report-list.json) | T15 S7受控报告列表文字、真实DK custom-draw/小字形、独立位图/负例/所有权及实际包发布证据 |
| [evidence/tree-controls.json](evidence/tree-controls.json) | T15 S8原生/框架树的协议、实际字形/边框、通知/资源及早显frame精确所有权，真实MD5与负例/包证据 |
| [evidence/progress-controls.json](evidence/progress-controls.json) | T15 S9受控原生树键盘夹具、有限v6进度协议/实际像素/状态/取消和v5/特殊模式原生保留及交付证据 |
| [evidence/trackbar-controls.json](evidence/trackbar-controls.json) | T15 S10滑块原生部件/状态、应用绘制保留、缓存差异与树并发超时对照；当前结果见CURRENT |
| [evidence/updown-controls.json](evidence/updown-controls.json) | T15 S11微调原生API状态、准确DC归属/释放、共享主题拒绝、旧包负例及实际包交付证据 |
| [evidence/framework-audit.json](evidence/framework-audit.json) | T15 S12冻结18单位逐项审计、独立同批四文件/实际包/双发布与有限边界；状态见CURRENT |
| [product-contract.json](product-contract.json) | 架构权威的机器映射；组件/产物变化必须同步审核 |
| [import](import/README.md) | 来源基线与 Git blob 清单；只作导入对照，不能成为构建输入 |
| [operations/source-policy.md](operations/source-policy.md) | 导入来源处置及用户明确的原资源身份要求；完整保留原声明 |
| [evidence/m0-t445-s4-scrollbar-fidelity-audit.md](evidence/m0-t445-s4-scrollbar-fidelity-audit.md) | 原resources.md历史链接的来源定位，不作为本项目验收证据 |
| [operations/UAC.md](operations/UAC.md) | 按需提升方法、原生测试入口、环境/控制台边界及验收范围 |
| [operations/BUILD.md](operations/BUILD.md) | 本地构建、干净候选包、验证结果和使用/支持限制 |

任务完成证据进入 history；原始日志及测试抓图保留在任务 scratch，不提交。用户明确要求的精选README展示图作为文档资源按上表索引，原始诊断材料仍不提交。

- [T16轮廓移动验证登记](evidence/outline-validation.md)：冻结覆盖单位、离屏/显示分界与待补条件。

- [Actor VB6复选框诊断](evidence/actor-checkbox.md)：实际类/状态/模块与分类缺口；未实施控件修复。

- [T16实现与任务审计](../history/M0-T16-S1-outline-move.md)：16单位、原始依据、有限移动/原生边界及交付闭环。

- [T16交付与审计证据](evidence/outline-delivery.json)：完整包、安装路径、视觉/资源保留及实际交付复核。

- [T16 S2轮廓撤回](evidence/outline-withdrawal.json)：短计时、源码/测试原样撤回、恢复包及实际交付；S1资料仅作历史。

- [T17 VB控件首轮审计](evidence/vb-controls-audit.md)及[逐项矩阵](evidence/vb-controls-audit.json)：内建双版本、微软扩展、模块/样本和未完成目录证据；不是已支持声明。

- [T17 S1审计交付](../history/M0-T17-S1-vb-control-audit.md)：148单位目录、真实类型库/源码缺口、三项短基线包验证及后续交接。

- [T17 S2原生demo目录](evidence/vb-demo-gallery.json)：148审计单位的对应页/明确缺失项、有限原生展示及实际包交付；不等同VB/OCX适配。

- [T17 S3选择控件](evidence/vb-selection-controls.json)：准确VB6普通CheckBox、真实Actor/独立C负例、VB5注册断点和明确变体边界。

- [T17 S4真实内建控件](evidence/vb-intrinsic-controls.json)：固定原作者VB5/6样本、精确runtime/类/公开协议、有限按钮/Frame/Edit实现及下一S接收面。

- [T17 S5列表与滚动包装层](evidence/vb-composite-controls.json)：真实VB5/6精确类、Combo原生编辑子控件字体所有权、原glyph像素与有限公开协议。

- [T17 S6原生输入](evidence/vb-input-controls.json)：真实VB消息队列/通知与双宽自包含见证正负例，普通输入范围和未认证变体。

- [T17 S7文件控件](evidence/vb-file-controls.json)：真实VB5文件/目录/驱动器列表、准确owner-draw DC、图像所有权与双宽生命周期验证；VB6未认证范围另列。

- [T17 S8真实VB6文件组](evidence/vb6-file-controls.json)：准确FileListBox/DirListBox、可选免注册原作者样本与双版本/双宽模型；VB6驱动器列表仍未认证。

- [T17 S9 PictureBox承载](evidence/vb-container-controls.json)：真实双版本无窗口标签、应用Print保持及双宽自有资源/输入模型；未完成面明确保留。

- [T17 S10图形命令按钮](evidence/vb-graphical-controls.json)：真实双版本owner-draw通知/primitive，原生返回/图像/应用绘制保持与有限覆盖。

- [T17 S13图形选择控件](evidence/vb-graphical-choices.json)：精确 VB 图形
  CheckBox/OptionButton 的状态/分组、退役与发布证据；复杂变体仍由后续 S 接收。

- [T17 S18图形选择图片/图标](evidence/vb-graphical-choice-variants.json)：精确
  图形 CheckBox/OptionButton 的公共位图/图标借用、状态、退役与双宽包证据；真实
  图片属性样本、owner-draw 与复杂变体仍保留后续接收。

- [T17 S19图形选择真实图片样本](evidence/vb-graphical-choice-samples.json)：已登记
  可运行和源码候选的控件块扫描；没有真实图片选择控件，S18模型边界保持不变。

- [T17 S20控件数组](evidence/vb-control-arrays.json)：真实控件数组源码依据和自有
  双宽 HWND 独立生命周期回归；私有集合与未获样本保持后续接收。

- [T17 S21 Label变体](evidence/vb-label-variants.json)：Label 的真实边框/对齐
  源码和无独立 HWND 运行时边界；保持父 DC 绘制，不作猜测性 STATIC 附着。

- [T17 S22 Frame容器](evidence/vb-frame-containers.json)：Frame caption/嵌套
  子控件的真实协议边界；保留 runtime 可见性和禁用语义。

- [T17 S14 VB6 DriveListBox审计](evidence/vb6-drivelistbox-audit.json)：唯一源码
  匹配实际为VB5运行时的反证与固定目录样本缺失；没有产品实现。

- [T17 S15无窗口内建控件审计](evidence/vb-windowless-controls.json)：Image、
  Shape、Line保留应用绘制，Timer保留非视觉业务语义；没有产品实现。

- [T17 S16特殊内建宿主审计](evidence/vb-special-hosts-audit.json)：Data/OLE/MDI/
  UserControl保持数据、对象和客户端责任；没有产品实现。

- [T17 S17扩展OCX审计](evidence/vb-extension-ocx-audit.json)：官方扩展目录、
  公共控件已有路径与缺少真实wrapper协议的边界；没有产品实现。
