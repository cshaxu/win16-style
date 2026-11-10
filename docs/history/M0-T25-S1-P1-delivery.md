# M0 T25 S1 P1：实验性 Win2000 接入准备

本交付建立 `ntw2ks32/64.dll`，`run16s --2k` 选择实验性路径，`--16` 和省略选项选择 Win16。Win2000 路径目前只对新建标准 Button、Edit、ComboBox、ListBox、ScrollBar 请求禁用视觉主题；不是完整的 Windows 2000 外观，标题栏、窗口边框、对话框和原版像素对照尚未实现。Win2000 的全局、UWP、已识别的默认 Explorer、Terminal 和控制台宿主入口明确拒绝。T25 S1 保持开放。

构建使用同一源码快照，快照清单 SHA-256 为 `6811bb310629bb9ba98a78ac7f967bd6190bd1a391fb5e2cfbf60c0ec8634907`；MinGW GCC 16.1.0、Ninja、ccache 4.13.6，x86/x64 干净构建分别 62.50 秒、80.34 秒。源码来源、原始资源、依赖边界和双位宽 ABI 门禁通过。

| 文件 | 架构 | SHA-256 |
| --- | --- | --- |
| run16s.exe | x64 | `b6e41f3229721454536a4371ecadc1bdeadd1de15e0831233ff925bbc8f9d83f` |
| demo.exe | x64 | `451243cffd7a5e49d0e5a699370108ae0b86afef95a5ecea5fcf57ddc67dd466` |
| ntw16s32.dll | x86 | `88596813157cb14bc858a32f96db4c1af88049a2daf645fd2e3961cdb2987a49` |
| ntw16s64.dll | x64 | `805ebe4c7c75713dc19d4e26867f0aa9054889d68f89462549a7a12a8c07d7ed` |
| ntw2ks32.dll | x86 | `881e46537ac7717e5c20863b18a45a1d03a45a2c37a5b91b7cebc2a6894b7558` |
| ntw2ks64.dll | x64 | `4c215b609cfeb3f6bf1d6dc881c194b06b0af86257c6e54d9a8d18d9dd7b0fdc` |

实际候选包定向测试共 24 项，其中 21 项通过，`demo-pair`、`automatic`、`propagation` 因受限环境创建私有桌面失败（错误 5）跳过；在授权的桌面权限下，另一次九项定向测试全部通过，包含这三项和滚动条视觉测试。新路径验证了双位宽加载、四种父子位宽传播、原生与 Win2000 路径的按钮主题句柄对照；它们不等于完整视觉验收。模拟 UAC 测试验证了参数保留；真实 UAC 未验证。

完整集成测试未通过：受限环境的 `scroll-frame-x86` 在屏幕抓取时返回 5，授权环境重测通过；授权环境的 `global-e2e` 在私有桌面创建托盘图标时失败（2147500037），使用已发布 T24 的同条件对照也出现相同失败。没有把失败或跳过计作通过，也没有声称完整回归通过。

同批六文件已按各自 SHA-256 发布到 `assets` 根目录及 `O:/winnt`。此次为实验性交付，完整集成失败及上述功能缺口继续属于开放事项。
