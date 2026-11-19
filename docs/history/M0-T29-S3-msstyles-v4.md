# M0 T29 S3 MSStyles v4 读写

## P1 标准属性记录

`style-msstyles` 以 C17 读写固定 32 字节的 v4 属性记录头。内联值按八字节对齐，短记录保留资源 ID 与声明长度。实现对长度、边界、类索引和保留字段做检查。负例覆盖截断的记录头、数据和填充，以及无效类索引、保留字段和缓冲区不足。

原生测试只读加载本机系统主题的 PE 资源，使用 x86 和 x64 遍历 `VARIANT/NORMAL` 并逐条重写，对照原始字节。`CMAP` 跳过 UTF-16 终止符之间的空填充，两份样本均得到 374 个类名。

| 样本 | 记录数 | `VARIANT/NORMAL` 字节数 | SHA-256 |
| --- | ---: | ---: | --- |
| Aero | 11096 | 449072 | `9168d28f0efc7a53b702b6d631bcac763158e59191ad3d2e3c6cb572c21d5a96` |
| Aero Lite | 10313 | 428616 | `61386cf29c5422056824ecf2aae5c0fa1df0db6eee9d9a4c7838ed534f5893bd` |

测试的常规入口使用合成记录，系统样本由测试程序的可选路径参数提供。记录结构参考 [msstyleEditor 作者的格式说明](https://github.com/nptr/msstyleEditor/wiki/.msstyles-Format)，实现和测试均为项目原创。

P1 交付属性记录层。完整主题编译仍需实现类表和变体映射、资源打包、配套 `.theme` 描述及独立校验。产品执行件继续使用 P45 发布包。

## P2 类表和变体

两份本机样本的 `CMAP` 都是 18,760 字节。374 个类名分别以 UTF-16LE 储存，包含终止符后对齐至八字节。通过对每一项编码并比较原始字节确认了对齐和索引。

两份样本的 `VMAP` 都是 76 字节。每项顺序是记录名、尺寸名、配色名。每个名称前的四字节字段统计含终止符的 UTF-16LE 单元数，名称对齐至四字节。样本分别读出 `Normal`、`NormalSize` 和 `NormalColor`，与配套主题描述中使用的尺寸和配色一致。

x86、x64 独立通过合成负例，以及 Aero、Aero Lite 的类名、变体与全部属性记录字节级往返。合成负例覆盖截断的名称、越界长度、嵌入终止符、非零类名填充和输出空间不足。

## P3 配套主题描述

描述写入器按 Microsoft 的格式提供 `[Control Panel\Desktop]`、`[VisualStyles]` 和 `[MasterThemeSelector]`。其中 `Size` 与 `ColorStyle` 直接取自生成 `VMAP` 所用的变体对象。项目便携布局使用同目录的样式文件名，并拒绝目录穿越和换行注入。[Microsoft Theme File Format](https://learn.microsoft.com/en-us/windows/win32/controls/themesfileformat-overview)

x86、x64 用 Windows `GetPrivateProfileStringW` 独立复读生成文件的六个字段，拒绝的输入未写出字节，测试文件随后删除。此验证覆盖 INI 语法和字段绑定。交付布局的实际文件引用仍需用两份完整候选核对，系统个性化对相对路径的行为尚未验证。

## P4 同源生成两对文件

`tools/theme/classic_source.h` 提供两份可编辑的代表部件数据。x64 Ninja 的 `classic-themes` 目标调用 C17 生成器、MinGW windres 和链接器，输出无入口、无可执行段的 PE 资源文件和它的同目录描述。四件放在 x64 构建目录的 `themes/Win16` 和 `themes/Win2000` 下。启用测试构建时可使用 `check-classic-themes` 同时编译并校验。

独立 C17 验证器未链接项目 MSStyles 库。x86 和 x64 都打开 canonical x64 PE，使用 Windows 资源与 INI API，核对文件引用、变体和标题栏、按钮与滚动条的填充色。单独暂存的两对文件也通过，删去同批样式文件或把描述中的尺寸改成不存在的变体均被拒绝。两位宽 C17 生成器的资源和描述字节一致。同工具链重复链接两次，四件摘要一致。

| 相对于 `themes` | SHA-256 |
| --- | --- |
| `Win16/Win16.msstyles` | `d358c444c6f7536f898573a978e129672ba6a7ca5f51181174728e1c1cf06980` |
| `Win16/Win16.theme` | `0dcbf80bb67ea283a81265c670ae6c6c07ee776b8ac2449c0a05b0818f2984a8` |
| `Win2000/Win2000.msstyles` | `78d01226421c71d0b5b5f3a1aec9561eb4b352d6aff3995f69c1c2dd9a705a19` |
| `Win2000/Win2000.theme` | `2872cff7d762ffd97519b54bec4eb99441c72a736edefa0e228cf622526a5f7e` |

四件供继续验证格式，当前覆盖代表部件的标准属性。图像引用、项目扩展、真实运行时消费和系统个性化加载各自仍待验证。发布二进制沿用 P45。
