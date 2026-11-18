# M0 T27 S15 Demo 控件目录与风格切换

已有 demo 的 Controls 菜单有九个原生页面。保留按钮、编辑、文件列表、
应用绘图、MDI 和对话框页固定顺序，使用现有页面和控件状态验证本 T。
导入来源及原版图像继续使用 S2–S14 记录。Demo 只使用 Windows 原生
控件和应用自有示例数据，所有风格效果由正常启动路径应用。

首轮目录审查发现 Lists and toolbars 已有标准 Tree、ListView、Header、
Tab、Toolbar、Rebar、StatusBar，尚缺一条可看见的 Toolbar 平面分隔和
不同状态栏分区。Ranges and dates 已有连续 Progress、TrackBar、
UpDown、日期选择器和月历，增加同一按钮可推进的分块 Progress。Text
and special objects 已有 RichEdit，补原生 HotKey。其它覆盖通过原有
按钮、编辑、列表及文件滚动页面。数据、应用图片、第三方运行库外壳、
Win2000 无对应样式的现代控件按实际范围记录，保留原有示例边界。

沿用 tests/demo-exe/observe.c 的公开控件、DPI、PrintWindow 与资源
观测，扩展已发布 run16s --2k demo 的少量对照，保持已有 Win3.x、原生
示例验证。使用其 worker 入口与外部桌面驱动的 --direct 运行，避免历史
私有桌面默认入口。九页实际枚举和组合样式另检查典型控件原生值及着色。

六方向使用现有 global_style_bridge 和生产共享生命周期，在自有可控
Win32 窗口上核验 Original、Win3.x、Win2000，保留已有/新建子控件、
控件状态或字体、恢复后的旧色和现场。普通启动路径与托盘管理员
端到端分别列证据；当前会话无法执行的交互明确标注。保留原图形资源和
来源，不改变整机配置。每轮开发用增量，合批完整八件先发布再测试。
