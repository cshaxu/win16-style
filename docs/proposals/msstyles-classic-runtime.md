# 用 MSStyles 定义 Win16 与 Win2000 风格

用户批准准入。目标是让 Win16 和 Win2000 成为可选的主题数据，并从产品包移除两套按年代分发的 DLL。S1 已完成格式与产品边界调研。用户确认继续在本 T 内完成两种主题整理和旧年代 DLL 的退出，实施按下列 S 顺序准入。

## 初步结论

可把风格数据迁到真实的 `.msstyles` 资源容器，并让两套年代外观由同一个运行时消费。移除 `ntw16s32/64.dll` 和 `ntw2ks32/64.dll` 的前提是把其中现在仍有生产调用方的可执行职责迁出。原生 UxTheme 的公开 API 可查询和绘制当前视觉样式。公开的 `SetWindowTheme` 参数是子应用、子 ID 列表，`OpenThemeData` 参数是窗口与类列表，这些入口不提供为单个窗口传入任意主题文件路径的契约。[Microsoft OpenThemeData](https://learn.microsoft.com/en-us/windows/win32/api/uxtheme/nf-uxtheme-openthemedata) · [Microsoft SetWindowTheme](https://learn.microsoft.com/en-us/windows/win32/api/uxtheme/nf-uxtheme-setwindowtheme)

`.theme` 是系统桌面主题描述，VisualStyles 段可以引用 `.msstyles`。微软给出的 Windows Classic 示例没有独立的视觉样式文件路径，因此 Win16 和 Win2000 的数据文件需要根据现有实现和原版参考来编制。微软的视觉样式 API 定义了类、部件、状态、边距和 DPI 绘制等消费方式。[Microsoft Theme File Format](https://learn.microsoft.com/en-us/windows/win32/controls/themesfileformat-overview) · [Microsoft Visual Styles Reference](https://learn.microsoft.com/en-us/windows/win32/controls/uxctl-ref)

## 格式实证

| 样本 | 只读取证 | 尚待实测 |
| --- | --- | --- |
| 本机 Windows Aero 与 Aero Lite | 以 PE 文件读取及资源映射枚举，x64 资源文件均只有 .rdata 和 .rsrc；均有 CMAP、VMAP、RMAP、VARIANT、IMAGE、STREAM。未执行文件代码。 | 选定类状态的属性与图形解码、版本边界与失败行为。 |
| 本地早期 XP 源树 Luna | `shell/themes/themedir/luna` 中的构建描述把三种配色的 INI 和主题索引交给 packthem，目标后缀是 .mst。未找到该树中的出货 `Luna.msstyles`。 | 使用有来源的出货 Luna 文件对照资源类型、版本和状态映射，早期源树样本不代替出货文件。 |

两份本机样本的 PACKTHEM_VERSION 均为小端 4；从 CMAP 资源实际读到 Button、ScrollBar、Window，VMAP 含 Normal、NormalSize、NormalColor。此结果只证明目标部件在类表中存在，尚未验证具体 VARIANT 属性。

Aero 样本 SHA-256 为 `ed9515aea9a98fdda8a4a37655b701e74bf8e5bfc97428cff04a60276d4e34d9`，Aero Lite 为 `4d3934ed39f7604c9fdcb1a5a6ad4bee064105788ac66556fa4ed159b28a131b`。本地 Luna 构建描述 `makefile.inc` 摘要为 `7fb9b3feb5c7209b017b327aa8df77059cd4776b87cb74d002f40fcc7468970c`，主题索引 `themes.ini` 为 `d3d71664c8f9b81709c8a3b6bd782037d11442c3bbadc0489a9bafd603d087ea`。样本及第三方实现均未导入。

Wine 的解析实现展示了另一条 INI 资源路线，读取 themes_ini、配色和尺寸名称。后期格式的类映射与二进制属性表由 msstyleEditor 作者记录。由这些实现和本机样本推断，XP 和后期文件应独立识别和验证，再归一到公共数据模型。本机文件容器、资源类型和三个部件类的公开接口对照见 [S1 审计台账](../history/M0-T29-S1-msstyles-audit.md)。[Wine 解析源码](https://github.com/wine-mirror/wine/blob/master/dlls/uxtheme/msstyles.c) · [msstyleEditor 格式记录](https://github.com/nptr/msstyleEditor/wiki/.msstyles-Format)

## 迁移边界

| 内容 | 建议归属 |
| --- | --- |
| 配色、字体描述、边框尺寸、状态映射、图像和边距 | 优先映射到标准类、部件、状态属性与图像资源。以现有已验收参数及原始栅格为基准。 |
| Win16 的系统菜单按钮、Win2000 标题按钮组、特有滚动条几何 | 通用布局与命中算法保持可执行。先测标准属性可表达的几何，确有缺口再考虑有版本的最小数据扩展，并明确其由本项目运行时解释。 |
| 窗口附着与恢复、菜单输入及控件通知 | 留在共享运行时和 application 层，保持现有操作及生命周期。 |
| 原生 API 与 COM 对话框转换、打印事务、模板与回退 | 共享运行时执行。主题只提供适用的外观、度量和资源；程序调用契约不写入主题。 |
| Win32、UWP 与 WinUI | 各自适配 HWND、窗口状态、合成提交或客户区边界，共用同一主题数据与呈现接口。 |

启动器目前根据风格选择传统模块名，运行中及现代宿主还引用 Win16 命名的入口。全局启动用 Win16 组件作为接收端，选 Win2000 时另核验其配套模块，接收端用后者的窗口附着、查询与移除导出。两套 DLL 也共享公共对话框库。停用与恢复需保留现有语义。证据见 [启动器](../../src/run16s-exe/main.c)、[全局装配](../../src/run16s-exe/global.c)、[现代窗口调用方](../../src/run16s-exe/window_target.c)、[跨年代窗口分派](../../src/ntw16s-dll/global/window.c)及[构建装配](../../CMakeLists.txt)。

## T29 目标目录

保留现有三层。主题提供数据，共享代码决定布局、绘制和行为，产品组件负责装配。

| 目标位置 | 职责 |
| --- | --- |
| `src/lib-win32-style/access` | 现有启动、运行时、全局模式的入口与生命周期。 |
| `src/lib-win32-style/application` | 窗口、菜单、控件、对话框的状态与事务；Win32/UWP/WinUI 的宿主适配共用呈现。 |
| `src/lib-win32-style/appearance/msstyles` | 有界读取标准主题资源和有版本的项目扩展，验证后生成统一只读描述。 |
| `src/lib-win32-style/appearance/render` | 窗口、菜单、控件和对话框的共享绘制。布局、绘制与命中共用计算结果。 |
| `src/lib-win32-style/appearance/themes/win3x`、`win2000` | 各自的可编辑主题源、原始资源和需要随风格切换的参数。 |
| `src/ntwshk-dll` | 双位宽主题运行时装配与兼容导出。共用机制及绘制继续由公共库实现。 |

两套主题源分别生成真实的 `.msstyles` 和配套 `.theme` 描述。主题数据由双宽共用。正式包共七件，包含启动器、两份 `ntwshk` 以及 Win16 和 Win2K 的主题文件。发布者只交付这七件，演示程序作为验收夹具。详见[主题构建契约](msstyles-build-contract.md)。

运行时切换先迁启动器对旧年代 DLL 的文件名依赖，再清理链接和包清单。
通用 `ntwshk` 与两个旧年代 DLL 都使用 ordinal 1，迁移时按真实调用方区分
内部辅助入口与已发布的命名接口。旧 DLL 文件名随七件包移除，已约定保留的命名
调用契约由新运行时承接。七件目录的验证隔离旧安装和构建树的搜索路径，并记录
实际加载模块与主题文件来源。

## S 规划

S1 已交付 113 单元的分类与三类部件探针。以下 S2–S12 是建议实施顺序，准入编号与活动包由 CURRENT 记录。

| S | 交付 | 收口判据 |
| --- | --- | --- |
| S2 主题与 ABI 契约 | 冻结主题文件支持版本、默认值、资源所有权、13 项扩展的最小边界，登记 39 个 Win16、5 个 Win2000 导出的实际消费者。 | 113 项均有迁移接收者和可观察验收项，每个兼容导出有保留、转接或退役依据。固定 4 项待证的样本和验证归属。 |
| S3 MSStyles 读写基础 | 建立首批支持格式的资源读取、可重复生成和统一描述，验证标准属性与扩展的版本边界。 | 两份最小主题和配套描述可生成并通过独立解析与引用核对；无效版本、越界资源、缺失部件与未知可选扩展有明确结果，原始栅格可逐项核对。 |
| S4 首批闭环 | 标题栏、普通按钮、滚动条，从主题文件走到共有布局、绘制和命中，在自有 Win32 样本中对照。 | 两风格 x86/x64、96/144 DPI、普通与禁用／按压等关键状态，比较区域、像素、命中和恢复。明确标准字段与必须扩展字段的实测分界。 |
| S5 窗口、菜单与字体 | 迁各类标题按钮、系统菜单、普通菜单、Win16 字形及字体配方。 | 核对正常、最大化、非激活、带图标及无图标，菜单首帧及键鼠取消，字体借用、度量和恢复；确认弹出菜单列布局。 |
| S6 Win32 标准控件 | 迁按钮族、编辑与列表、组合框、标签与表头、范围及日期类等已交付标准表面。 | 按审计范围逐组对照常见状态、数据及原生通知，回归已支持的 Common Controls 版本和 DPI。应用自绘遵循原归属。 |
| S7 VB/VCL 及复合控件 | 用既有适配器把已验证角色接入同一主题部件和源码路径。 | 复用原实机样本和 role 对照，分别记录实机、模型、未知专用状态。涉及两项私有外观待证单元，保留证据或已批准的适用范围。 |
| S8 公共对话框 | 将可见面和已交付布局迁入主题与扩展，接入现有共享 Win32/COM/打印行为。 | 两宽 A/W 的普通和自有全局接收路径对照布局、结果、回调、所有权及回退；保留 T28 用户批准的两项验证债务。 |
| S9 UWP/WinUI 外框 | 三种宿主消费同一主题与 renderer，沿用现有坐标和合成适配。 | 自有样本覆盖标题按钮、焦点、重排、最大化和恢复。对照已约定的 DPI 与同像素范围，记录剩余两项现代宿主待证单元。 |
| S10 中性运行时替换 | 将旧年代 DLL 的可执行职责接到 `ntwshk` 的双位宽装配，迁移旧入口及实际调用方。 | 风格选择由外部主题身份驱动。双宽覆盖普通、运行中和全局模式及样式撤销，逐项核对已发布调用契约。 |
| S11 独立候选与包切换 | 候选目录严格使用七件清单，更新包门禁、来源和整包回滚办法。 | 双宽干净构建和独立目录验证。实际加载记录及依赖检查确认七件可独立运行，新包同批发布并实包回归。 |
| S12 全 T 终审 | 逐项对照 113 单元、旧导出、资源及实际发布。汇总移除的生产路径与测试结果。 | 每项注明已实测、原生保留、明确批准跳过或未覆盖；实际远端、assets 和 O:/winnt 逐件复核后收口 T29。 |

S2–S4 验证主题格式与共享呈现是否立得住。随后按功能整批迁移现有行为，S11 保留正式切换和回滚窗口，S12 核对最终产物及覆盖。任何改变产品二进制的交付都遵循仓库同包发布规则。Luna 出货样本可作为格式对照，Luna 和 Aero 实现另行规划。

主题采用项目内的只读加载。既有 Windows 系统主题、注册表、权限、适用目标、资源来源及应用回退仍按既有契约处理。
