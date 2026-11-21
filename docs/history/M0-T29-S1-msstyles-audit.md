# M0 T29 S1 MSStyles 可迁移性审计

冻结基线为 `f9b54871a71710d08f5b23d6593aa263104c08c3`。取两年代的当前呈现与公开契约，结合 [Win16 运行库控件盘点](M0-T18-S64-control-inventory.md)、[Win16 对话框审计](M0-T20-closure-audit.md)、[Win2000 控件终审](M0-T27-S16-final-audit.md)、[T28 总验收](M0-T28-closure-audit.md)及当前生产源码。使用同一个审计定义分别计数，每行每年代最多一个单元。多个状态、参数组合、位宽和宿主样本不按次数叠加。私有复合控件只按已知角色成组审计，全部私有实现、程序自绘、未适配控件、未交付功能和其他年代主题不计入。分母为功能与外观单元，不能换算为文件数、总代码量或可删代码比例。

## 结果

| 风格 | 标准 S | 扩展 E | 运行时 R | 待证 ? | 适用合计 | 外观合计 |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Win16 | 33 | 7 | 15 | 2 | 57 | 42 |
| Win2000 | 33 | 6 | 15 | 2 | 56 | 41 |
| 合计 | 66 | 13 | 30 | 4 | 113 | 83 |

- **标准 S** 包含公开属性或标准状态下的图形负载。按台账，标准格式能表达 66/83 项外观数据（79.5%）。按整体验收单元是 66/113（58.4%）。这里的「能表达」表示存在适用的标准槽或图形资源表达，未制作两份年代文件来验证像素对等。
- **扩展 E** 表示若该数据还要随主题切换，需要由本项目定义的版本字段或附加资源。标准格式加扩展合计 79/83 项外观数据（95.2%），占全部 79/113（69.9%）。扩展可随 PE 容器保存，只有识别它的消费者能够使用。
- **运行时 R** 属于行为、状态或调用契约，共 30 个按年代计的单元，对应 15 项成对职责。
- **待证 ?** 有 4 个按年代计的单元。现有局部实机证据不足以确认完整呈现的映射。表内 `—` 是 3 个不适用单元，未计入 113。

## 逐项台账

| 序号 | 分组 | 审计单元 | Win16 | Win2000 | 映射与边界 | 生产及范围证据 |
| --- | --- | --- | --- | --- | --- | --- |
| 01 | 外观 | 外框与控件基础配色 | S | S | SysMetrics 色槽与部件色值 | [1](../../src/lib-win32-style/application/window/palette.c) [2](../../src/lib-win32-style/appearance/msstyles/classic_theme.h) |
| 02 | 外观 | 活动／非活动标题底色与 Win2000 渐变端点 | S | S | WINDOW 状态和 TMT_GRADIENTCOLOR1/2 或对应背景图 | [1](../../src/lib-win32-style/application/window/palette.c) [2](../../src/lib-win32-style/appearance/msstyles/classic_theme.h) |
| 03 | 外观 | 标题文字颜色及字体描述 | S | S | TMT_CAPTIONFONT、TMT_TEXTCOLOR；字体可用性另计 | [1](../../src/lib-win32-style/appearance/win3x/window/frame_renderer.c) [2](../../src/lib-win32-style/appearance/win2000/window/caption.c) |
| 04 | 外观 | 控件与菜单的标准字体描述 | S | S | LOGFONT 对应的系统字体属性，应用借用与替代另计 | [1](../../src/lib-win32-style/appearance/msstyles/raster_font.c) [2](../../src/lib-win32-style/appearance/win2000/window/caption.c) |
| 05 | 外观 | Win16 System 原位图字体的字符字形及 advance | E | — | 标准 LOGFONT 可引用字体，不能随文件定义 CP1252 栅格与每字 advance；需授权的附加数据 | [1](../../src/lib-win32-style/appearance/msstyles/raster_font.c) [2](../../src/lib-win32-style/appearance/win2000/window/caption.c) |
| 06 | 外观 | 普通与最大化外框边缘的绘制数据 | S | S | WINDOW 边框、标题状态与图像切片 | [1](../../src/lib-win32-style/appearance/win3x/window/frame_layout.c) [2](../../src/lib-win32-style/appearance/win2000/window/frame.c) |
| 07 | 外观 | 标题栏和窗口边框的基本度量 | S | S | 标题栏及边框系统度量，按 DPI 的定位规则另计 | [1](../../src/lib-win32-style/appearance/win3x/window/frame_layout.c) [2](../../src/lib-win32-style/appearance/win2000/window/frame.c) |
| 08 | 外观 | 标题按钮各状态的图形与内边距 | S | S | WINDOW 标准按钮部件、状态和图像边距；保留现有栅格来源 | [1](../../src/lib-win32-style/appearance/win3x/window/frame_renderer.c) [2](../../src/lib-win32-style/appearance/win2000/window/caption.c) |
| 09 | 外观 | 系统按钮的可见图形 | S | S | 公开 WP_SYSBUTTON 部件可表示；本机 Aero 该部件没有可绘制数据 | [1](../../src/lib-win32-style/appearance/win3x/window/frame_renderer.c) [2](../../src/lib-win32-style/appearance/win2000/window/caption.c) |
| 10 | 外观 | 标题按钮组排列、间隔和 Win16 左侧按钮几何 | E | E | 每个部件尺寸可标准化；成组关系和环境分支需项目布局字段 | [1](../../src/lib-win32-style/appearance/win3x/window/frame_layout.c) [2](../../src/lib-win32-style/appearance/win2000/window/frame.c) |
| 11 | 外观 | 最大化、不可调整、无图标等组合布局选择 | E | E | 部件与度量有标准槽；当前组合选择需显式规则参数 | [1](../../src/lib-win32-style/appearance/win3x/window/frame_layout.c) [2](../../src/lib-win32-style/appearance/win2000/window/frame.c) |
| 12 | 外观 | 菜单栏与弹出菜单背景、项目、禁用与选中配色 | S | S | MENU 菜单栏／弹出部件及状态 | [1](../../src/lib-win32-style/appearance/win3x/menu/menu_paint.c) [2](../../src/lib-win32-style/application/menu/bar_controller.c) |
| 13 | 外观 | 菜单勾选、子菜单箭头和分隔部件图像 | S | S | MENU 对应部件状态；图像负载须保留来源 | [1](../../src/lib-win32-style/appearance/win3x/resources/frame_buttons.h) [2](../../src/lib-win32-style/appearance/win3x/resources/control_glyphs.h) [3](../../src/lib-win32-style/appearance/win3x/resources/combo_glyph.h) [4](../../src/lib-win32-style/appearance/win3x/resources/menu_glyphs.h) [5](../../src/lib-win32-style/appearance/win3x/resources/scroll_glyphs.h) |
| 14 | 外观 | Win16 弹出菜单标签、快捷键和箭头的列间距 | E | — | 组件专属列宽参数；实际测量和跟踪另计 | [1](../../src/lib-win32-style/appearance/win3x/menu/menu_paint.c) [2](../../src/lib-win32-style/application/menu/bar_controller.c) |
| 15 | 外观 | 普通按钮、默认边缘、按下和禁用绘制 | S | S | BUTTON.BP_PUSHBUTTON 对应状态 | [1](../../src/lib-win32-style/appearance/win3x/control/basic_paint.c) [2](../../src/lib-win32-style/appearance/win2000/control/button_paint.c) |
| 16 | 外观 | 复选、半选与单选的标记和状态 | S | S | BUTTON.BP_CHECKBOX/BP_RADIOBUTTON；状态选择留代码 | [1](../../src/lib-win32-style/appearance/win3x/control/choice_paint.c) [2](../../src/lib-win32-style/appearance/win2000/control/button_paint.c) |
| 17 | 外观 | 静态文字、蚀刻边和框线色值 | S | S | 颜色及 BORDERFILL/IMAGE 数据；绘制角色另由匹配器识别 | [1](../../src/lib-win32-style/appearance/win3x/control/basic_paint.c) [2](../../src/lib-win32-style/appearance/win2000/control/static_paint.c) |
| 18 | 外观 | 分组框背景、边和标题字样 | S | S | BUTTON.BP_GROUPBOX 及颜色、背景属性 | [1](../../src/lib-win32-style/appearance/win3x/control/basic_paint.c) [2](../../src/lib-win32-style/appearance/win2000/control/static_paint.c) |
| 19 | 外观 | 普通、只读、禁用编辑框的边缘及颜色 | S | S | EDIT 部件状态及系统颜色 | [1](../../src/lib-win32-style/appearance/win3x/control/basic_paint.c) [2](../../src/lib-win32-style/appearance/win2000/control/edit_paint.c) |
| 20 | 外观 | 普通列表选中、背景色与边缘 | S | S | LISTBOX 和系统色值；内容仍由列表提供 | [1](../../src/lib-win32-style/appearance/win3x/control/list_paint.c) [2](../../src/lib-win32-style/appearance/win2000/control/list_combo.c) |
| 21 | 外观 | 组合框下拉按钮、边缘与禁用外观 | S | S | COMBOBOX 各部件和状态；子窗口归属另计 | [1](../../src/lib-win32-style/appearance/win3x/control/combo_paint.c) [2](../../src/lib-win32-style/appearance/win2000/control/list_combo.c) |
| 22 | 外观 | 横纵滚动条箭头、滑块、轨道和尺寸框 | S | S | SCROLLBAR 箭头、方向、状态图和 size box | [1](../../src/lib-win32-style/appearance/win3x/control/scroll_paint.c) [2](../../src/lib-win32-style/appearance/win2000/control/scroll.c) |
| 23 | 外观 | 滚动条的基本厚度和尺寸 | S | S | SysMetrics 滚动条度量或部件大小 | [1](../../src/lib-win32-style/appearance/win3x/control/scroll_paint.c) [2](../../src/lib-win32-style/appearance/win2000/control/scroll.c) |
| 24 | 外观 | 极短条及非标准厚度的裁决参数 | E | E | 标准部件大小可存；保留当前极值规则需额外参数，实际求解另计 | [1](../../src/lib-win32-style/appearance/win3x/control/scroll_paint.c) [2](../../src/lib-win32-style/appearance/win2000/control/scroll.c) |
| 25 | 外观 | 标签页状态及边缘 | S | S | TAB 项目和 pane；复杂自绘仍由应用 | [当前共享绘制](../../src/lib-win32-style/application/control/tab_snapshot.c) |
| 26 | 外观 | 表头边框、普通与按压表面 | S | S | HEADER 部件和状态 | [1](../../src/lib-win32-style/appearance/win3x/control/header_paint.c) [2](../../src/lib-win32-style/appearance/win2000/control/header_paint.c) |
| 27 | 外观 | 普通列表视图与树视图的边缘和选择色 | S | S | LISTVIEW/TREEVIEW 及系统色；只计已支持表面 | [1](../../src/lib-win32-style/appearance/win3x/control/control_dispatch.c) [2](../../src/lib-win32-style/appearance/win2000/control/control.c) |
| 28 | 外观 | 进度条槽、普通分段或平滑可见面 | S | S | PROGRESS 底槽与填充部件；进度数据留原生 | [1](../../src/lib-win32-style/appearance/win3x/control/progress_paint.c) [2](../../src/lib-win32-style/appearance/win2000/control/progress_paint.c) |
| 29 | 外观 | 滑杆轨道、滑块和刻度 | S | S | TRACKBAR 标准部件与状态 | [1](../../src/lib-win32-style/appearance/win3x/control/trackbar_paint.c) [2](../../src/lib-win32-style/appearance/win2000/control/trackbar_paint.c) |
| 30 | 外观 | 微调箭头及边线 | S | S | SPIN 对应状态与方向 | [UpDown](../../src/lib-win32-style/application/control/updown.c)（原 Win3x 独立绘制已并入此文件） |
| 31 | 外观 | 工具栏普通按钮和分隔线 | S | S | TOOLBAR 状态、图形与分隔部件 | [1](../../src/lib-win32-style/appearance/win3x/control/toolbar_paint.c) [2](../../src/lib-win32-style/appearance/win2000/control/toolbar_paint.c) |
| 32 | 外观 | 状态栏普通面、分隔和尺寸柄 | S | S | STATUS 部件及边距 | [1](../../src/lib-win32-style/appearance/win3x/control/control_dispatch.c) [2](../../src/lib-win32-style/appearance/win2000/control/control.c) |
| 33 | 外观 | Rebar 普通抓手 | S | S | REBAR.RP_GRIPPER；只计现有支持状态 | [1](../../src/lib-win32-style/appearance/win3x/control/control_dispatch.c) [2](../../src/lib-win32-style/appearance/win2000/control/control.c) |
| 34 | 外观 | 日期选择器的边及下拉按钮 | S | S | DATEPICKER 部件及状态 | [1](../../src/lib-win32-style/appearance/win3x/control/control_dispatch.c) [2](../../src/lib-win32-style/appearance/win2000/control/control.c) |
| 35 | 外观 | 月历普通标题、日期与选中面 | S | S | MONTHCAL 或标准文本/颜色属性，以 v4 为研究范围 | [1](../../src/lib-win32-style/appearance/win3x/control/month_paint.c) [2](../../src/lib-win32-style/appearance/win2000/control/date_paint.c) |
| 36 | 外观 | RichEdit 和热键的经典边缘 | S | S | 只迁移边缘画笔或切片，不计富文本内容 | [1](../../src/lib-win32-style/appearance/win3x/control/control_dispatch.c) [2](../../src/lib-win32-style/appearance/win2000/control/control.c) |
| 37 | 外观 | VB/VCL 私有复合控件的完整可见状态映射 | ? | ? | 已有局部原生角色对照，完整自绘状态不足以定标准部件 | [1](../history/M0-T18-S64-control-inventory.md) [2](../history/M0-T27-S16-final-audit.md) |
| 38 | 外观 | 公共对话框外框及标准子控件可见面 | S | S | WINDOW.WP_DIALOG 加对应标准控件部件 | [1](../history/M0-T20-closure-audit.md) [2](../history/M0-T28-S3-common-dialogs.md) |
| 39 | 外观 | 文件夹／文件对话框的历史控件布局与模板标识 | E | E | 坐标表、模板引用和触发条件需项目配置；API 转接另计 | [1](../history/M0-T20-closure-audit.md) [2](../history/M0-T28-S3-common-dialogs.md) |
| 40 | 外观 | 打印扩展 General 页的历史内容布局 | — | E | Win2000 的紧凑／标准／大模板为独立模板数据 | [1](../../src/lib-win32-style/application/dialog/print_extended.c) [2](../../src/lib-win32-style/appearance/win2000/dialog/print_general.c) |
| 41 | 外观 | 色板和查找框的历史展开、压缩几何参数 | E | E | 资源角色与逻辑尺寸可入扩展，实际重排另计 | [1](../../src/ntw16s-dll/injection/window_titles.c) [2](../../src/ntw2ks-dll/controls.c) |
| 42 | 外观 | 消息框中历史图标、按钮边距与字体样式 | S | S | 映射标题/边距与标准 BUTTON 属性，图标按原有来源借用 | [1](../history/M0-T20-closure-audit.md) [2](../history/M0-T28-S3-common-dialogs.md) |
| 43 | 外观 | 现代宿主与普通外框最终同像素的完整覆盖 | ? | ? | T28 合成对照是有限宿主和 DPI，尚无主题数据候选 | [1](../../src/lib-win32-style/appearance/win3x/window/modern_frame.c) [2](../../src/lib-win32-style/appearance/win2000/window/modern_frame.c) |
| 44 | 行为 | 原主题观察、抑制与还原事务 | R | R | 状态及句柄生命周期 | [1](../../src/lib-win32-style/application/lifecycle/theme_track.c) [2](../../src/lib-win32-style/application/window/frame.h) |
| 45 | 行为 | 窗口尺寸求解、非客户区和命中测试 | R | R | 使用已解析数据与实际客户区、窗口状态 | [1](../../src/lib-win32-style/appearance/win3x/window/frame_layout.c) [2](../../src/lib-win32-style/appearance/win2000/window/frame.c) |
| 46 | 行为 | UWP/WinUI 宿主坐标、重绘及合成提交 | R | R | 由宿主适配器执行 | [1](../../src/lib-win32-style/appearance/win3x/window/modern_frame.c) [2](../../src/lib-win32-style/appearance/win2000/window/modern_frame.c) |
| 47 | 行为 | 菜单焦点、跟踪、键盘、取消及 owner 通知 | R | R | 状态机与 HWND 消息 | [1](../../src/lib-win32-style/appearance/win3x/menu/menu_paint.c) [2](../../src/lib-win32-style/application/menu/bar_controller.c) |
| 48 | 行为 | 控件真实状态、应用返回值及回调所有权 | R | R | 原生处理、子窗和重入 | [1](../../src/lib-win32-style/appearance/win3x/control/control_dispatch.c) [2](../../src/lib-win32-style/appearance/win2000/control/control.c) |
| 49 | 行为 | 滚动范围计算、捕获、拖拽与通知 | R | R | 交互算法和消息 | [1](../../src/lib-win32-style/appearance/win3x/control/scroll_paint.c) [2](../../src/lib-win32-style/appearance/win2000/control/scroll.c) |
| 50 | 行为 | 应用字体借用、位图字形适配及释放 | R | R | 运行期字体对象及恢复 | [1](../../src/lib-win32-style/appearance/msstyles/raster_font.c) [2](../../src/lib-win32-style/appearance/win2000/window/caption.c) |
| 51 | 行为 | Win32 文件参数转换和选定结果回写 | R | R | 调用契约及错误码 | [1](../history/M0-T20-closure-audit.md) [2](../history/M0-T28-S3-common-dialogs.md) |
| 52 | 行为 | Shell 目录树 PIDL 与 COM 对象代理、选择和释放 | R | R | 接口标识和签名不写入主题 | [1](../../src/lib-win32-style/application/dialog/classic_com_show.c) [2](../../src/ntw16s-dll/injection/folder_dialogs.c) |
| 53 | 行为 | 查找／替换的通知及字体、颜色回调 | R | R | 模型无关消息、A/W 原生数据和调用方所有权 | [1](../../src/ntw16s-dll/injection/window_titles.c) [2](../../src/ntw2ks-dll/controls.c) |
| 54 | 行为 | 打印结果事务、驱动、设置句柄与对话框回退 | R | R | PrintDlgEx／PageSetup 等应用契约 | [1](../../src/lib-win32-style/application/dialog/print_extended.c) [2](../../src/lib-win32-style/appearance/win2000/dialog/print_general.c) |
| 55 | 行为 | 消息框按钮命令、帮助及返回值 | R | R | 调用原生 API 并返回应用 | [1](../../src/ntw16s-dll/injection/window_titles.c) [2](../../src/ntw2ks-dll/controls.c) |
| 56 | 行为 | VB/VCL 角色发现、应用自绘判定和原生退路 | R | R | 共享适配器运行路径 | [1](../history/M0-T18-S64-control-inventory.md) [2](../history/M0-T27-S16-final-audit.md) |
| 57 | 行为 | CLI 模式、会话选择和挂载恢复编排 | R | R | 公共产品装配，与配色资源无关 | [1](../../src/run16s-exe/main.c) [2](../../src/run16s-exe/global.c) |
| 58 | 行为 | 兼容导出、双位宽合约和跨模块分派 | R | R | 发布和实际调用方合约 | [1](../../src/ntw16s-dll/ntw16s64.def) [2](../../src/ntw2ks-dll/ntw2ks.def) [3](../../src/ntw16s-dll/global/window.c) |

源文件用于定位现有逻辑，公开部件名称和属性用 [Microsoft 部件与状态](https://learn.microsoft.com/en-us/windows/win32/controls/parts-and-states)、[Visual Styles Reference](https://learn.microsoft.com/en-us/windows/win32/controls/uxctl-ref)与本机 MinGW 的 `vsstyle.h`、`vssym32.h` 核对。个别只有后期格式公开支持的部件按 v4 研究范围计入。

## 只读部件探针

对本机当前 `Aero.msstyles` 执行 `GetCurrentThemeName` 确认文件和变体，文件 SHA-256 为 `ed9515aea9a98fdda8a4a37655b701e74bf8e5bfc97428cff04a60276d4e34d9`，变体为 NormalColor／NormalSize。对三个类依次调用 `OpenThemeData(NULL, class)`、`GetThemePartSize(TS_TRUE)`、`GetThemeMargins(TMT_SIZINGMARGINS)` 和 `DrawThemeBackground`。绘制目标是 120×32、32 位内存 DIB，初始 RGB 为 85/85/85，`GdiFlush` 后计数 RGB 有变化的像素。未调用主题设置接口，未创建目标应用窗口。

| 类 | 部件／状态 | 部件 | 原始大小 | 拉伸边距（左/右/上/下） | HRESULT | 像素变化 |
| --- | --- | --- | --- | --- | --- | ---: |
| WINDOW | 1/1 | caption-active | 66×29 | 28/35/9/17 | 00000000 | 3824 |
| WINDOW | 1/2 | caption-inactive | 66×29 | 28/35/9/17 | 00000000 | 3824 |
| WINDOW | 15/1 | minimize | 5×17 | 2/2/7/7 | 00000000 | 3840 |
| WINDOW | 17/1 | maximize | 5×17 | 2/2/7/7 | 00000000 | 3840 |
| WINDOW | 18/1 | close | 28×17 | 2/2/7/7 | 00000000 | 3840 |
| WINDOW | 18/3 | close-pressed | 28×17 | 2/2/7/7 | 00000000 | 3840 |
| WINDOW | 13/1 | sysbutton | 0×0 | — | 80004005 | 0 |
| BUTTON | 1/1 | push-normal | 13×11 | 6/6/5/5 | 00000000 | 3536 |
| BUTTON | 1/3 | push-pressed | 13×11 | 6/6/5/5 | 00000000 | 3536 |
| BUTTON | 1/4 | push-disabled | 13×11 | 6/6/5/5 | 00000000 | 3536 |
| BUTTON | 1/5 | push-defaulted | 13×11 | 6/6/5/5 | 00000000 | 3536 |
| BUTTON | 3/9 | checkbox-mixed | 13×13 | — | 00000000 | 165 |
| BUTTON | 2/5 | radio-selected | 13×13 | — | 00000000 | 145 |
| SCROLLBAR | 1/1 | arrow-up | 17×17 | 8/8/8/8 | 00000000 | 3840 |
| SCROLLBAR | 1/13 | arrow-right | 17×17 | 8/8/8/8 | 00000000 | 3840 |
| SCROLLBAR | 2/1 | thumb-horizontal | 20×17 | 11/8/8/8 | 00000000 | 3840 |
| SCROLLBAR | 3/1 | thumb-vertical | 17×11 | 8/8/5/5 | 00000000 | 3840 |
| SCROLLBAR | 4/1 | lower-track-horizontal | 1×8 | 0/0/4/3 | 00000000 | 3840 |
| SCROLLBAR | 6/1 | lower-track-vertical | 14×1 | 10/3/0/0 | 00000000 | 3840 |
| SCROLLBAR | 10/1 | size-box | 16×16 | — | 00000000 | 24 |

十九个部件状态返回成功且有像素变化。系统按钮在本机样本上返回 `0x80004005`、零像素，公开 `WP_SYSBUTTON` 部件的存在仍可为 Win16 系统图形提供具名槽。部分状态即使 `IsThemePartDefined` 返回 FALSE，绘制仍可通过主题继承得到数据，不能据布尔值直接判定文件缺少映射。[OpenThemeData](https://learn.microsoft.com/en-us/windows/win32/api/uxtheme/nf-uxtheme-openthemedata) · [GetThemePartSize](https://learn.microsoft.com/en-us/windows/win32/api/uxtheme/nf-uxtheme-getthemepartsize)

Win16 和 Win2000 的标题色及激活状态、标准按钮状态和滚动条图形可选上表对应的标准部件。Win16 左侧系统按钮及右侧布局、Win2000 按钮组间隔和状态组合仍需当前几何逻辑。源字形按现有身份保留，资源存储不证明已通过像素、布局或命中对照。下一实现阶段需用两年代真实候选在 96/144 DPI 下分别测试正常、非激活、按压、最大化和恢复。样本版本边界及出货 Luna 缺失见[调研提案](../proposals/msstyles-classic-runtime.md)。

## DLL 退场与发布门槛

生产读审计覆盖 [CLI](../../src/run16s-exe/main.c)、[全局风格装配](../../src/run16s-exe/global.c)、[UWP](../../src/run16s-exe/uwp.c)、[WinUI/运行中窗口](../../src/run16s-exe/window_target.c)、[窗口分派](../../src/ntw16s-dll/global/window.c)、[CMake 消费者](../../CMakeLists.txt)和[当前包契约](../etc/product-contract.json)。Win16 每个位宽有 39 个导出序号，Win2000 每个位宽有 5 个，其中各有一个序号入口。后续逐一核对是否有生产消费者以及是否需要兼容转接，不能凭合计删导出。对话框、状态恢复、宿主输入和接入的可执行部分进入共享代码与中性运行时。

优先制作独立的中性双位宽运行时候选。两份主题文件替代旧年代二进制的外观角色，保持独立基础运行时的方案共八件。把样式代码移入基础运行时可变成六件，进入实施前对比其基础路径 GUI 直接依赖、包字节数、导出消费者、无样式启动以及恢复回归。P45 八件作为回滚基线。先在独立候选目录完成 Win32/UWP/WinUI、对话框、两宽和三种接入方式的原功能回归，再在完全缺少旧 DLL 的候选目录复测。在更新正式包契约前保留 P45 八件摘要，候选回归失败使用现有整包恢复。切换只以完整同快照包进行。

本轮为只读探针、源码审计与文档交付。未导入系统样本、未生成替代资源、未变更产品二进制。
