# M0 T25 S1 P9：WinUI ComboBox 阶段包

为默认来源的 ComboBox 和生成的 ComboBoxItem 增加 Win16 风格模板；保留
下拉、选中项、可编辑文本、标题及滚动属性接线，现有本地样式不会被覆盖。
S1 和 T25 保持开放。

GCC、Ninja、ccache 双位宽干净构建及来源、资源、ABI、包检查通过；同批
四文件13项聚焦实包回归通过。自有 x64 WinUI 对象验证下拉项生成、选中
显示、程序化编辑绑定、列表文字及选中/禁用文字颜色、来源保护和恢复。
标准启动的自有记事本验证动态 ComboBox 和下拉首项自动接入以及会话清理。
曾有一次探针创建未返回，后续对照通过；真实鼠标键盘、滚动、像素、不同
DPI、其他运行时及 x86 WinUI 未覆盖，不能视为用户验收。

四文件 SHA-256：

- run16s.exe `3e4c9deb4a40a968eff2b819edc1db6b20b05a2f8bb53b39508494690b7442b0`
- ntw16s32.dll `4d29741a95866da39725eece8697b35d7a69024f133b682e1cebd7d273fc7244`
- ntw16s64.dll `c4243f055e3fc697ebd06e925ca04b7a4219806b5a9ef262f222bf59965a4ddb`
- demo.exe `9ca36ab62df6db5767145740c4b51dc0dce18e4f45cd2318a3f6d9937852fabb`

同批文件已发布 assets 根目录及 O:/winnt，两个位置逐项核对摘要一致。
