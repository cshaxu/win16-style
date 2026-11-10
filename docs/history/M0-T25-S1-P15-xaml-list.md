# M0 T25 S1 P15：WinUI ListBox 阶段包

为默认样式来源的 ListBox 和 ListBoxItem 增加 Win16 风格，列表项复用
ComboBoxItem 的项目模板；调用方现有本地样式不被覆盖。S1/T25 保持开放。

GCC、Ninja、ccache 双位宽干净构建及来源、资源、ABI、包检查通过；同批
四文件13项聚焦实包回归通过。自有 x64 WinUI 对象验证项目生成、程序化
单选和多选、项目文本与状态颜色、恢复及本地样式保护；共用项目模板后的
ComboBox 对象回归通过。标准启动自有记事本验证动态列表及首项自动接入与
会话清理。一次探针先命中隐藏 GDI+ 窗口失败，重试命中主窗口后通过。
真实键鼠、多选交互、滚动、像素、其他运行时和 x86 WinUI 尚未验证，不
代表用户验收通过。

四文件 SHA-256：

- run16s.exe `fa234ba00169610a409efcb9a47ed43338ea098b62436aa1e5d2f990476fad68`
- ntw16s32.dll `bb7899005b942db73004c5a95b99ad4b41a2aa6b609fedfd2d54da18b4f26a5d`
- ntw16s64.dll `f3aba921377fb31204755d9325c4dedba392b268ecc8b8a1efd9431e71d634da`
- demo.exe `9ac02a6314a5cbc446f76c634298171e78a1c664d4290b66ee7339edfea4a080`

同批文件已发布 assets 根目录及 O:/winnt，两个位置摘要一致。
