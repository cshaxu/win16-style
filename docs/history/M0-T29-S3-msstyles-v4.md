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

描述文件生成按 Microsoft 要求提供 `[Control Panel\Desktop]`、`[VisualStyles]` 和 `[MasterThemeSelector]`。其中 `Size` 与 `ColorStyle` 由同源 `VMAP` 绑定。[Microsoft Theme File Format](https://learn.microsoft.com/en-us/windows/win32/controls/themesfileformat-overview)
