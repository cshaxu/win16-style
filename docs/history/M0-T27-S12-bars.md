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
