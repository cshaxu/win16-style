# M0 T27 S12 Windows 2000 工具栏、可重排栏和状态栏

固定原版 toolbar.c、rebar.c、status.c，覆盖普通按钮、分隔、工具栏/可重排栏
背景、抓手、状态栏分区和缩放角。以原生图像列表、字体、文本、回调、通知和
伙伴子窗口为边界。状态、定位与布局由控件保留，应用自绘和自设颜色优先。

Toolbar_DrawButton 与相关阶段确定软凸/凹边、平面按钮和按下状态。应用
TBCDRF_NOEDGES 等标记保留。Rebar 的原版抓手采用 BDR_RAISEDINNER，
使用真实 band 头坐标，背景位图和带内容布局继续原生。Status 的普通分区
采用 BDR_SUNKENOUTER，SBT_POPOUT 采用 BDR_RAISEDINNER，SBT_NOBORDERS
保留间距，所有者文本和图标不复绘。先盘点公用通知、rebar 几何、status 快照
及 Win2k Marlett 缩放角接口，再选择最小呈现改动。

已查看原版 [WordPad](https://guidebookgallery.org/pics/gui/applications/office/texteditor/win2000pro.png)，
SHA-256 92cbef51a3ba92b5a3dc6dc7f9cf8f93fa0fd53de9cb04225a176f9489287e0c。
上方标准工具栏包含图像、分隔、禁用与按下；底部状态栏含独立右侧分区和缩放角。
S9 已查看 [My Computer](https://guidebookgallery.org/pics/gui/system/managers/filemanager/win2000pro.png)；
源码 browseui 的 bandsite.cpp、browbs.cpp、itbar.cpp 确认使用 REBARCLASSNAME。
两者按原版源码解释，图片只作实际外观对照。

验收使用 v5/v6 双位宽自有实例。分别采样边线与图像/文本保持、组和分区变化、
通知、借用 DC、移除和普通启动。合批构建八件、先发布再测、推送取回与清理后准入 S13。

原版提交 fd809a81098565b33f52d0f65925159de8f4c337。

| 文件 | Git blob | 原工作副本 SHA-256 |
| --- | --- | --- |
| private/shell/comctl32/toolbar.c | b303689c5618a6e1a143ef60620b1beaf415772e | 63da0c594e63811a0da1bd2bf27c5992b1022d3f427b7f8ba5439d2bb7fc213c |
| private/shell/comctl32/rebar.c | ac07d2daf1df31e6dedd79b745b74e4b9819ce15 | 40b5177147747d969f718dcfeb9a2db0efa5880720d183976e7dcb30aafe71cf |
| private/shell/comctl32/status.c | 985043f6ad397ab314d4c61ea494065ae7c14eec | e6a09a7b1399ce8ad31c0cdf389b3aab04d13730537829967807c657a3832d96 |

已用 Win11 两种原生清单的自有实例确认，Toolbar 与 Rebar 均收到
CDDS_PREPAINT / CDDS_ITEMPREPAINT。Rebar 公共结构应使用项目既有
REBARBANDINFOW_V3_SIZE，v5 拒绝最新结构尺寸。StatusBar 无 NM_CUSTOMDRAW，
其首个分区可由 SB_GETRECT 取得，实测边界分别为 v5 (0,2,95,22) 和
v6 (0,2,95,23)。先按这些实测入口实施，原生客户背景特殊值予以保护。

工具栏在原生绘制完成后覆绘普通按钮及平面分隔的边缘，保留字体、图像列表、
按键通知和应用自己的 NM_CUSTOMDRAW 阶段。Rebar 以当前原生 RB_HITTEST
及 band ID 确认抓手，仅在普通背景绘制原版单层凸边，应用底图和客户子窗保留。
状态栏在原生客户区绘制后通过 SB_GETRECT 覆绘普通与凸起分区，保留无边框、
自绘、文字和背景。缩放角仍由原生布局及命中管理，优先复用原生图形，只在
已确认独立区域内映射阴影色。简单模式绘制保持原生。

第一轮双位宽 v5/v6 与两种 DPI awareness 合计 8 个用例通过。加验发现
两种清单的缩放角已有正确命中和白色亮面，阴影仍用原生系统灰。x86 实测
未声明 DPI 时最后一分区止于 x362，DPI aware 时止于 x353，客户区均为
380 像素宽。修复按原生保留区映射，不按固定 DPI 扩盖分区。简单模式文本
通过现有 GetWindowTextW 路径读取。修复后 8 个增量用例及 4 个普通启动
用例均通过，随后加验 Rebar 自设背景与状态栏借用 DC 裁剪。
Rebar 的自设背景由公开接口读回并验证抓手采用原生路径。当前系统的
WM_PRINTCLIENT 没有填入自设色，项目继续保留原生行为。最终候选的
x86、x64 共 8 个增量用例通过，包含状态栏借用 DC 的像素裁剪检查。

效率检查点采用一组三控件探针覆盖公共通知、图形与普通启动。初始通过后，
针对失败只加缩放角采样和直接文本读取，各轮增量构建同时覆盖两位宽。
停止重复尝试固定坐标，改用真实公开布局。开发临时目录在档案记录结果后回收。

首次正式构建在 x86 停止。公共通知结构增加可选后绘制回调后，Win3.x
和公共测试仍有位置初始化，触发 GCC 的缺失字段告警。统一所有旧消费者
使用具名初始化，保留零值可选回调，并将两风格与公共通知覆盖纳入增量检查。
增量构建覆盖两个 DLL 和公共通知探针，双位宽共 10/10 通过。失败的
源码快照未发布，回收后以新名称启动正式构建。

复杂 Toolbar 嵌套、分裂按钮、垂直或多行分隔、应用自定义背景、Rebar
竖直布局和位图及状态栏简单模式均保留原生绘制。栏背景空白区仍由系统或
应用绘制。未覆盖高对比度、RTL、特殊打印设备或真实鼠标拖动。
