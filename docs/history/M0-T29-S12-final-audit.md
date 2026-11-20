# M0 T29 S12 经典主题迁移终审

T29 将两个年代已交付的可用主题数据迁入两对外部文件，双位宽执行路径收敛到同一
套中性运行时源码。范围依据[原规划](../proposals/msstyles-classic-runtime.md)和
[主题构建契约](../proposals/msstyles-build-contract.md)。

## 完成判据和覆盖账

冻结的 [S1 格式分类](M0-T29-S1-msstyles-audit.md)与
[S2 迁移台账](M0-T29-S2-theme-and-abi.md)均为 58 行。按年代有 3 项不适用，
逐项核对其余 113 项。S1 的 66 项标准、13 项扩展、30 项运行时和 4 项待证是
表达能力分类。下表记录已消费的数据、继续归原生的表面、实测边界及待证状态。

| 台账 | Win16 | Win2000 | 阶段证据 |
| --- | --- | --- | --- |
| 01 | 标准基础色已读回，实测改色。 | 标准基础色已读回，实测改色。 | [S5](M0-T29-S5-window-menus-fonts.md) 外框配色，[S6](M0-T29-S6-win32-controls.md) 控件 |
| 02 | 两种激活状态填色，实测改色。 | 两种激活状态渐变端点，实测改色。 | [S4](M0-T29-S4-representative-rendering.md) 标题色 |
| 03 | 标题字体描述及文字色实测，保留宿主字体回退。 | 标题字体描述及文字色实测，保留宿主字体回退。 | [S5](M0-T29-S5-window-menus-fonts.md) 原生字体 |
| 04 | 菜单字体描述实测，原生控件字体归原调用方。 | 已交付菜单/控件的字体描述与原有借用范围。 | [S5](M0-T29-S5-window-menus-fonts.md) 原生字体，[S6](M0-T29-S6-win32-controls.md) 控件 |
| 05 | 宿主 System 栅格与 advance 采样保留，独立历史字体来源及许可待定。 | 不适用。 | [S5](M0-T29-S5-window-menus-fonts.md) 收口 |
| 06 | 边色由主题提供，外框像素对照通过。 | 边色由主题提供，外框及最大化细线对照通过。 | [S5](M0-T29-S5-window-menus-fonts.md) 外框配色 |
| 07 | 主题度量进入布局和命中，两宽实测。 | 普通、工具窗度量实测，固定对话框遇到 About Notepad 特例时保留原边。 | [S4](M0-T29-S4-representative-rendering.md) 度量，[S10](M0-T29-S10-neutral-runtime.md) 固定框测量 |
| 08 | 标题按钮槽和原栅格、状态像素实测。 | 标题按钮槽和度量实测，保留原有 GDI 字形。 | [S4](M0-T29-S4-representative-rendering.md) 标题按钮 |
| 09 | 系统按钮的原始资源更换与命中实测。 | 应用提供的系统图标保持原有归属。 | [S4](M0-T29-S4-representative-rendering.md) 系统按钮 |
| 10 | 项目扩展承载成组间隔，使用双宽绘制、命中验证。 | 项目扩展承载成组间隔，使用双宽布局对照。 | [S4](M0-T29-S4-representative-rendering.md) 度量，[S9](M0-T29-S9-modern-frames.md) 现代宿主 |
| 11 | 最大化、无图标等能力由系统状态决定，主题度量进入现有变体。 | 最大化、无图标、不可调整使用现有窗口状态和相应度量，原生对照通过。 | [S5](M0-T29-S5-window-menus-fonts.md) 窗口组合 |
| 12 | 普通、禁用与选中菜单配色实测。 | 菜单配色及原生菜单边界实测。 | [S5](M0-T29-S5-window-menus-fonts.md) 菜单配色 |
| 13 | 原菜单栅格字节及实际像素已比对。 | 保留原菜单标记及关联系统路径。 | [S5](M0-T29-S5-window-menus-fonts.md) 栅格和原生菜单 |
| 14 | 主题扩展列距，改值后双宽布局实测。 | 不适用。 | [S5](M0-T29-S5-window-menus-fonts.md) 列度量 |
| 15 | 按钮正常、默认、按下、禁用实测。 | 同四态实测。 | [S4](M0-T29-S4-representative-rendering.md) 标准按钮 |
| 16 | 主题颜色及从原表生成的 PNG 实测。 | 主题状态颜色实测，Marlett 和原有半选网纹保留。 | [S6](M0-T29-S6-win32-controls.md) 选择按钮 |
| 17 | 静态文字与蚀刻边使用主题色，原生画刷和图像规则保持。 | 对应位置实测，父窗非默认文字颜色优先。 | [S6](M0-T29-S6-win32-controls.md) 静态与分组框 |
| 18 | 分组框字色、扁平边色换色实测。 | 分组框字色、蚀刻边色换色实测。 | [S6](M0-T29-S6-win32-controls.md) 静态与分组框 |
| 19 | 正常、只读与禁用的编辑框边实测，文本和编辑行为保留。 | 同三态的平边与凹边深色实测，客户区原生。 | [S6](M0-T29-S6-win32-controls.md) 标准编辑 |
| 20 | 主题背景、选择与边色实测，保留原生项目及制表位。 | 同类色值实测，保留应用画刷和自绘。 | [S6](M0-T29-S6-win32-controls.md) 列表框 |
| 21 | 组合框的普通、按下、禁用、边色实测，编辑子窗原生。 | 同类状态和色值实测，编辑子窗原生。 | [S6](M0-T29-S6-win32-controls.md) 组合框 |
| 22 | 横纵滚动区、箭头、滑块、轨迹与尺寸框验证，原始栅格保留。 | 同类区域实测，保留现有边层与原生箭头绘制。 | [S4](M0-T29-S4-representative-rendering.md) 滚动 |
| 23 | 度量从主题读取，现有非标准横截面保留。 | 度量应用和两个 DPI 实测，原生求解保留。 | [S4](M0-T29-S4-representative-rendering.md) 滚动 |
| 24 | 极短条由运行时压缩，主题参数与极值样本实测。 | 原有短条压缩及主题长度对照实测。 | [S4](M0-T29-S4-representative-rendering.md) 滚动 |
| 25 | 标签正常、选中、底色边线实测。 | 同类状态实测，切角与边层保持。 | [S6](M0-T29-S6-win32-controls.md) 标签与表头 |
| 26 | 表头普通、按下与释放实测。 | 同类状态实测，应用自绘通知及优先权保持。 | [S6](M0-T29-S6-win32-controls.md) 标签与表头 |
| 27 | 原生项目绘制保留，已交付字体、外框与滚动路径保持。 | 默认通知阶段选中、禁用和失焦色经原生样本实测。 | [S6](M0-T29-S6-win32-controls.md) 列表视图、树 |
| 28 | 横纵槽、填充、错误与暂停换色实测，现有填充比例保留。 | 横纵槽和填充实测，非正常态及旧版客户区原生。 | [S6](M0-T29-S6-win32-controls.md) 进度条 |
| 29 | 横纵轨道与滑块实测，刻度原生，专属运行库轨道保持既有边界。 | 横纵轨道与滑块实测，浅槽不可见底色和原生刻度均有区分。 | [S6](M0-T29-S6-win32-controls.md) 滑杆 |
| 30 | 两向箭头正常与禁用色实测。 | 同类颜色实测，箭头形状、位置和方向保留。 | [S6](M0-T29-S6-win32-controls.md) 微调，[S7](M0-T29-S7-composite-controls.md) 自有宿主 |
| 31 | 已接管的专属文字绘制器使用主题，标准 Toolbar 原生。 | 普通按钮、边色与分隔改色实测。 | [S6](M0-T29-S6-win32-controls.md) 工具栏，[S7](M0-T29-S7-composite-controls.md) 宿主范围 |
| 32 | 仅已支持面板边缘实测主题色，尺寸柄原生。 | 面板边及尺寸柄暗色实测。 | [S6](M0-T29-S6-win32-controls.md) 状态栏 |
| 33 | Rebar 原系统画刷和抓手边色保留。 | 已支持抓手面、边色实测。 | [S6](M0-T29-S6-win32-controls.md) Rebar |
| 34 | 已接管宿主日期按钮换色实测，保留其他宿主控件边界。 | 日期按钮实测，输入框边线原生。 | [S6](M0-T29-S6-win32-controls.md) 日期 |
| 35 | 已接管宿主月历标题和选中面实测。 | 原生与弹出月历主题色、应用覆盖和恢复实测。 | [S6](M0-T29-S6-win32-controls.md) 月历 |
| 36 | 既有识别范围的私有编辑边缘和原生退路保留。 | 非客户区边线实测，RichEdit20/HotKey 离屏客户区保持原夹具跳过。 | [S6](M0-T29-S6-win32-controls.md) 编辑边，[S7](M0-T29-S7-composite-controls.md) |
| 37 | 私有复合按钮、边线和图案等局部已验证，完整映射待证。 | 可确认的标准包装角色已验证，完整私有映射待证。 | [S7](M0-T29-S7-composite-controls.md) 私有模型与历史样本范围 |
| 38 | 已接管公共对话框配色与标准子控件实测。 | 标准框及控件沿原有呈现，宿主客户区保留。 | [S8](M0-T29-S8-common-dialogs.md) 公共对话框 |
| 39 | 既有文件和 Shell 模板原生，验证识别、选择和取消。 | Places 尺寸、工作区门槛与留白参数实测，兼容模板保留。 | [S8](M0-T29-S8-common-dialogs.md) 文件和目录 |
| 40 | 不适用。 | 三份 General 模板独立主题与基线逐字节相同，确认、取消及 Apply 实测。 | [S8](M0-T29-S8-common-dialogs.md) General |
| 41 | 现有原生几何保留，既有颜色和查找协议回归通过。 | 展开色板匹配与查找高度的主题值实测，模板识别失败保留原布局。 | [S8](M0-T29-S8-common-dialogs.md) 色板及查找 |
| 42 | 消息框当前外观及结果通过发布包矩阵，图标仍由原 API 持有。 | 同一主题路径的标准控件与字体实测，图标归原 API。 | [S8](M0-T29-S8-common-dialogs.md) 消息框 |
| 43 | 共享度量和实际 Calculator、Notepad 样本通过，全部现代宿主像素等价待证。 | 共享度量和实际 Calculator、Scan、Notepad 样本通过，全部现代宿主像素等价待证。 | [S9](M0-T29-S9-modern-frames.md)、[S10](M0-T29-S10-neutral-runtime.md)、[S11](M0-T29-S11-seven-file-package.md) |
| 44 | 原样式观察、抑制和恢复由现有事务持有，自有窗口恢复实测。 | 原样式恢复和几何实测，固定对话框压缩保留原规则。 | [S4](M0-T29-S4-representative-rendering.md) DWM 回归，[S9](M0-T29-S9-modern-frames.md) [S10](M0-T29-S10-neutral-runtime.md) |
| 45 | 现有窗口布局、度量、命中及恢复实测。 | 同类现有运行时几何和命中实测。 | [S4](M0-T29-S4-representative-rendering.md)、[S9](M0-T29-S9-modern-frames.md) |
| 46 | 宿主管理裁剪、重绘和提交，双宽自有宿主及真实 UWP 样本实测。 | 同类自有宿主和真实样本实测，WinUI 全场景像素恢复未测。 | [S9](M0-T29-S9-modern-frames.md)、[S10](M0-T29-S10-neutral-runtime.md)、[S11](M0-T29-S11-seven-file-package.md) |
| 47 | 菜单退出、项目数据恢复和命令路径实测，物理键鼠未纳入。 | 同类退出及恢复契约，保留原生菜单状态机。 | [S5](M0-T29-S5-window-menus-fonts.md) 菜单回归 |
| 48 | 原生选择、数据和通知保留，各控件范围见 15–36。 | 原生值、回调和应用测量优先权保留，按已支持控件验证。 | [S6](M0-T29-S6-win32-controls.md) 原生控件 |
| 49 | 两宽验证范围、拖动、捕获及取消。 | 两宽验证拖动、捕获及 32 位位置。 | [S4](M0-T29-S4-representative-rendering.md) 滚动回归 |
| 50 | 字体租约、释放、缓存回退实测，历史字体文件见 05。 | 字体描述和 GDI 生命周期实测。 | [S5](M0-T29-S5-window-menus-fonts.md) 字体 |
| 51 | 既有文件路径、筛选、缓冲区及回写的发布包回归。 | 两宽选定、取消、短缓冲及偏移实测。 | [S8](M0-T29-S8-common-dialogs.md) 文件选择 |
| 52 | 两宽 Shell A/W 和 COM 文件夹选择、取消及释放实测。 | 两宽 COM Open/Save 选择、取消，保留原 PIDL/对象归属。 | [S8](M0-T29-S8-common-dialogs.md) Shell、COM |
| 53 | 原生查找/替换、字体、颜色通知与回写保留，聚焦包验证。 | 两宽普通启动和自有全局接收器的选择、确认与通知实测。 | [S8](M0-T29-S8-common-dialogs.md) 通知与回写 |
| 54 | 保留原有打印/页面设置返回契约，实际打印未测。 | 双宽 General 确认/取消/Apply 实测，x64 A/W 得到 DC，x86 A/W 缺打印上下文；自定义打印页和域打印机维持批准跳过。 | [S8](M0-T29-S8-common-dialogs.md) 及 T28 记录 |
| 55 | 消息框按钮结果的双宽发布包矩阵实测。 | 消息框入口、选择和返回双宽实测。 | [S8](M0-T29-S8-common-dialogs.md) 消息框 |
| 56 | 自有 VB/VCL 模型身份、裁剪、拒绝路径、异常与撤销实测。 | 可确认的原生模型保留内容和状态，伪类及自绘原生退路实测。 | [S7](M0-T29-S7-composite-controls.md) 已有角色 |
| 57 | 普通启动、已运行表面及自有全局接收测试。 | 同类自有范围，八组 Win2K 三代传播与主题缺失拒绝。 | [S10](M0-T29-S10-neutral-runtime.md)、[S11](M0-T29-S11-seven-file-package.md)；管理员托盘依批准以代码和单测为准 |
| 58 | 中性 DLL 保有旧命名导出，helper 序号按实际调用契约保留。 | 兼容命名入口迁入同一中性 DLL，旧 DLL 路径及任意旧序号并非对外承诺。 | S2、[S10](M0-T29-S10-neutral-runtime.md) ABI、[S11](M0-T29-S11-seven-file-package.md) 七件实包 |

## 构建、包与运行路径

- [S3 读写证据](M0-T29-S3-msstyles-v4.md)核对本机出货 Aero/Aero Lite 的类表、
  变体和属性字节往返，并验证截断、越界、错误引用和必需扩展拒绝。生成器从
  [主题源](../../tools/theme/classic_source.h)产生描述与资源。发布的两个 PE
  均为只读数据容器，包含标准类、变体及部件属性。标题布局、滚动、菜单及对话框
  中已采纳的额外度量使用有版本的扩展，解释由本项目运行时负责。Windows 个性化
  的签名接受与安装路径属于 S2 明示排除项，本任务未做系统主题安装。
- [S11 干净构建](M0-T29-S11-seven-file-package.md)使用 C17、MinGW GCC、Ninja
  和 ccache。三运行件和四主题文件来自同一快照。实际 GitHub 发布提交
  a810d5b242ea10fc330e834b23ef41975becabdb 与两个发布位置逐件核对。
  1,281 个构建输入已对照实际提交，其中 8 项为 Git 换行规范化差异。
  [发行摘要](../etc/release.json)记录七个 SHA-256 及五个已退役的路径。
- 本终审重新对两个发布位置计算七件摘要，并从 O:/winnt 拷贝出严格七件目录复跑
  [验包门禁](../../tools/check_package.py)。门禁直接读 PE 头、导入、导出及
  只读主题引用，两位宽各核实原始 18 张栅格表、图形来源声明和许可。
  五份原图头及来源声明与 T29 准入提交 `f9b54871` 逐件相同。
  两个目录分别用两位宽的独立读回、扩展边界及主题模型探针验证，12/12 通过。
  通过 Windows 只读资源映射复核两份 `CMAP`、`VMAP`、`VARIANT/NORMAL`
  及 `CLASSIC_STYLE` 均位于外部主题，三份打印 General 模板仅位于
  `Win2K.msstyles`。两个中性 DLL 的资源表均没有上述主题资源或模板。
- [普通启动](../../src/run16s-exe/main.c)在创建目标前验证主题，使用公共
  路径解析选择中性模块。[全局路径](../../src/run16s-exe/global.c)使用同一
  路径解析，[托盘切换](../../src/run16s-exe/tray_controller.c)在提交前验证
  待切主题。[中性链接](../../CMakeLists.txt#L435)设置 NTWSHK_UNIFIED，
  两年代呈现按源文件链接进同一个 DLL 目标，并输出两个位宽。
  [样式切换](../../src/ntw16s-dll/global/window.c)在中性分支直接调用两种呈现，
  [会话绑定](../../src/ntw16s-dll/global/session.c)使用中性模块名。
  [直接 UWP](../../src/ntw16s-dll/uwp/hook.c)从包内描述取主题，按已限定
  的宿主范围走同一模型。[主题装载](../../src/lib-win32-style/appearance/msstyles/classic_theme.c)
  解析两组外部文件并约束引用位置。默认绘制常量继续供既有回退使用，旧测试
  装配仍可读取其嵌入资源。源代码中的旧模块名只用于非中性兼容
  构建或测试。演示、旧 DLL 的测试目标及其样本结果单列。
- 实包的 17 项聚焦回归、Win2K 的 8 条三代传播、4 条描述缺失或无效的
  启动拒绝、两宽 UWP 缺主题保留旧表面，以及五个真实应用样本均见 S11。
  Win16 与 Win2K 的生产路径和原先停用、恢复事务的额外对照见
  [S10 中性运行时](M0-T29-S10-neutral-runtime.md)。
  自有接收端测试范围限于测试目标。管理员整桌面按用户批准采用代码与单测证据，
  不计作真实管理员托盘端到端通过。

## 旧 ABI 逐项核验

从两个已发布 DLL 的 PE 导出目录复核了 S2 的全部 44 项。42 个旧命名符号
在中性 x86、x64 继续按名称提供。两个旧无名 helper 迁到共同的中性无名序号 1，
表内以“1（中性）”标示。中性 x86 与 x64 分别有 94、95 个命名导出。
两者均无旧年代 DLL 的 PE 导入。表中的新序号是当前构建的观测值。
按旧 DLL 文件名绑定、指定旧命名符号的历史数字序号或未登记模块的外部消费者，
须按 S2 兼容边界使用旧发行或迁到中性库的命名 API。旧兼容目标继续验证新旧
调用语义所需的历史输入。

| 年代 | 旧序号 | S2 符号 | 中性 x86 | 中性 x64 |
| --- | ---: | --- | ---: | ---: |
| Win16 | 1 | `Ntw16sFinishHelper` | 1（中性） | 1（中性） |
| Win16 | 2 | `Ntw16sCreationBegin` | 2 | 2 |
| Win16 | 3 | `Ntw16sCreationEnd` | 3 | 3 |
| Win16 | 4 | `Ntw16sGetAbiVersion` | 4 | 4 |
| Win16 | 5 | `Ntw16sScrollChanged` | 8 | 8 |
| Win16 | 6 | `Ntw16sSetWindowTheme` | 9 | 9 |
| Win16 | 7 | `Ntw16sWindowAttached` | 13 | 13 |
| Win16 | 8 | `Ntw16sWindowRemove` | 14 | 14 |
| Win16 | 9 | `Ntw16sWindowTerminal` | 15 | 15 |
| Win16 | 10 | `win16_style_control_install` | 73 | 74 |
| Win16 | 11 | `win16_style_control_remove` | 74 | 75 |
| Win16 | 12 | `win16_style_controls_begin` | 75 | 76 |
| Win16 | 13 | `win16_style_controls_end` | 76 | 77 |
| Win16 | 14 | `win16_style_default_palette` | 77 | 78 |
| Win16 | 15 | `win16_style_edit_install` | 78 | 79 |
| Win16 | 16 | `win16_style_edit_scrollbars_enable` | 79 | 80 |
| Win16 | 17 | `win16_style_font_acquire` | 80 | 81 |
| Win16 | 18 | `win16_style_font_release` | 81 | 82 |
| Win16 | 19 | `win16_style_frame_scrollbars_install` | 82 | 83 |
| Win16 | 20 | `win16_style_frame_scrollbars_palette` | 83 | 84 |
| Win16 | 21 | `win16_style_frame_scrollbars_remove` | 84 | 85 |
| Win16 | 22 | `win16_style_get_scroll_info` | 85 | 86 |
| Win16 | 23 | `win16_style_hit_test` | 86 | 87 |
| Win16 | 24 | `win16_style_install` | 87 | 88 |
| Win16 | 25 | `win16_style_list_scrollbar_enable` | 88 | 89 |
| Win16 | 26 | `win16_style_menu_measure` | 89 | 90 |
| Win16 | 27 | `win16_style_menu_text` | 90 | 91 |
| Win16 | 28 | `win16_style_remove` | 91 | 92 |
| Win16 | 29 | `win16_style_scrollbars_paint` | 92 | 93 |
| Win16 | 30 | `win16_style_set_palette` | 93 | 94 |
| Win16 | 31 | `win16_style_set_scroll_info` | 94 | 95 |
| Win16 | 32 | `win16_style_tabbed_list_install` | 95 | 96 |
| Win16 | 33 | `Ntw16sShowModernFileDialog` | 10 | 10 |
| Win16 | 34 | `Ntw16sGlobalHook` | 5 | 5 |
| Win16 | 35 | `Ntw16sGlobalHost` | 6 | 6 |
| Win16 | 36 | `Ntw16sWindowAttach` | 12 | 12 |
| Win16 | 37 | `Ntw16sGlobalThread` | 7 | 7 |
| Win16 | 38 | `Ntw16sUwpFrameHook` | 11 | 11 |
| Win16 | 39 | `Ntw16sWindowThreadHook` | 16 | 16 |
| Win2000 | 1 | `Ntw2ksFinishHelper` | 1（中性） | 1（中性） |
| Win2000 | 2 | `Ntw2ksDrawButton` | 17 | 17 |
| Win2000 | 3 | `Ntw2ksWindowAttach` | 18 | 18 |
| Win2000 | 4 | `Ntw2ksWindowAttached` | 19 | 19 |
| Win2000 | 5 | `Ntw2ksWindowRemove` | 20 | 20 |

## 验收限制与遗留

- 第 05 项沿用宿主字体采样。独立历史字体仍待来源与许可，已在
  [TODO](../states/TODO.md)登记。
- 第 37 项两个年代都有已实测的 VB/VCL 模型与已有角色，完整私有状态映射待证。
  第 43 项两个年代都通过实际宿主和可测采样点，所有 UWP/WinUI 宿主及 DPI
  的逐像素等价仍待证。现代客户区、超出现有角色的控件和完整私有应用覆盖不在
  本次迁移验收范围。
- [T28 批准跳过的打印项目](M0-T28-closure-audit.md)保持原有结论。T29
  S8 x64 A/W 带 DC 的正例通过，x86 A/W 缺少原生打印上下文，保留跳过。
  RichEdit20 和 HotKey 的离屏客户区保留原夹具跳过。完整矩阵和物理输入没有
  在最终发行重复验证，阶段证据保留各自覆盖范围。
- About Notepad 的固定边框仍按现有规则，144 DPI 对照的左右各 11 个物理像素，
  与原生对话框相同，见 [S10 实测](M0-T29-S10-neutral-runtime.md#交付前回归)。
  样式撤销、停止传播、线程钩子解除按各自事务核对。旧 DLL 物理卸载无新增保证。
- S11 已删除干净构建和回归工作目录。三个历史映射暂存见
  [S11 清理记录](M0-T29-S11-seven-file-package.md)，仅在任务暂存区。
  S12 验包副本按探针生命周期回收。

本次 S12 只补终审证据，七件摘要等于 S11 P1。T29 的实际关闭状态以
[CURRENT](../states/CURRENT.md)的远端复核记录为准。
