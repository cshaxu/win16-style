# 用 MSStyles 定义 Win16 与 Win2000 风格

用户批准准入。目标是让 Win16 和 Win2000 成为可选的主题数据，并从产品包移除两套按年代分发的 DLL。本阶段先做格式与产品边界调研。实现和切换包契约等后续阶段，按调研结果准入。

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

## 包装候选与验证顺序

1. S1 先验证三组现有外观样本的资源可表达性，以标题栏、按钮和滚动条作第一组。冻结样本摘要和可迁移属性清单，并审计启动、运行中、全局、现代窗口和兼容导出的实际生产消费者。对未找到的出货 Luna 样本保留范围。
2. 后续优先验证一对按位宽发布的中性样式运行时，保留 `ntwshk` 当前的基础职责。该方案用共享运行时和两份数据文件替代四个年代 DLL，按独立主题文件计算仍为八件。另比较把样式运行时并入 `ntwshk` 的六件方案，按无样式路径的 GUI 依赖、模块体积、兼容入口和实际测试结果决定。
3. 候选原型先在自有 Win32 窗口以同一 renderer 比较现有资源和主题资源的标题、按钮、滚动条；再验证菜单和对话框，并复用现代窗口合成对照。验收覆盖 96/144 DPI、激活和按压、命中、恢复及失败回退。
4. 包契约变更前列清兼容导出的所有消费者、旧产物淘汰与回滚办法。最终双宽干净构建、核验原资源与已发布功能、在候选目录验证缺少旧 DLL 仍能运行，之后才按用户批准范围修改正式分发。Luna 在本阶段用于格式对照。

主题候选采用项目私有数据读取路径。研究验证不修改 Windows 系统主题、文件或注册表。数据读取按 PE 资源边界做尺寸和版本校验，不作为可执行 DLL 加载。Win16 图形源及已冻结的栅格身份继续遵循[资源来源规则](../etc/operations/source-policy.md)。本阶段不改变现有产品契约、ABI、发布文件、权限或目标范围。
