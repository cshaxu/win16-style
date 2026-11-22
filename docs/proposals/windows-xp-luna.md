# Windows XP Luna 主题加载

T30 从用户指定的 Windows XP Professional SP3 VL x86 安装介质读取原版
Luna。目标是在自有主题目录中读取未改动的 .theme 和 .msstyles，通过
已有启动器选择主题。系统主题设置和原版文件保持原样。

## 组件边界

| 位置 | 职责 |
| --- | --- |
| appearance/msstyles | 解析主题描述、确定同目录资源文件、识别格式、读取 v3 变体与文本属性。返回借用的资源或明确的加载错误。 |
| appearance/msstyles/v3_frame | 依据真实样本映射 Luna 的窗口和代表性控件资源、状态和度量。 |
| application | 复用已有窗口、控件与图像提交，保持命中、布局、恢复及资源所有权。 |
| run16s-exe、ntwshk | 沿用主题目录选择、会话和调用入口。按主题声明与格式能力进入对应执行器。 |

源描述的系统主题路径只用于识别本地资源名，始终打开选定目录里的文件。
微软原版二进制和文字仅作为用户提供的本地样本，保留原字节，不进正式包、
仓库或必需构建输入。Luna 控件超出明确支持的部件时保持原生。
Vista、Win7、后续 v4 和 Aero 独立保留在候选队列规划。

## 配色和字号

本地测试样本保存在 assets/Luna，目录内只有原版 Luna.theme 与 Luna.msstyles。
原版描述选择 NormalColor 和 NormalSize。用户可在同目录的
Luna.theme 中设置 [VisualStyles] 的 ColorStyle 与 Size，不需要增加主题目录。

| 配色 | ColorStyle |
| --- | --- |
| 默认蓝色 | NormalColor |
| 银灰 | Metallic |
| 橄榄绿 | HomeStead |

Size 支持 NormalSize、LargeFonts、ExtraLargeFonts。配色和字号由资源表
解析。主题加载不改写描述或样式文件。原版证据记档时以未编辑的初始 SHA-256 为准。

## 单批交付

读取实现、Win32 外框／普通按钮与双位宽验证共享同一次正式七件构建。
原版样本仅本地放置，安装目录与样式目录仍沿用现有启动方式。
Win32 窗口的高度对照采用同一窗口和同一 DPI 分别读 Win2000 与 Luna。

## 实施顺序

以下三个验证阶段在 S1 合批执行。完成记录见 [T30 交付](../history/M0-T30-S1-luna-source.md)。

| S | 交付及退出证据 |
| --- | --- |
| S1 原版与读取 | 固定介质和两个原版主题文件的摘要。实现同目录描述与 v3 只读变体选择，验证九个组合、命名资源和失败路径。 |
| S2 正式入口 | 按格式接上主题发现、单程序加载及共享窗口呈现，先跑 Win32 标题、按钮和外框。验证两位宽、三配色、已声明度量、实际原版像素及撤销。 |
| S3 聚焦覆盖和交付 | 验证明确支持的代表性控件和原生回退，复验 Win16/Win2000 原七件。两处发布和实际远端复核后按范围收口。其他宿主分别报告。 |

格式参考 [微软 .theme 描述](https://learn.microsoft.com/en-us/windows/win32/controls/themesfileformat-overview)
及 [Wine 的 v3 读取实现](https://github.com/wine-mirror/wine/blob/master/dlls/uxtheme/msstyles.c)。
实现依据用户提供的样本和原生数据，独立编写。
