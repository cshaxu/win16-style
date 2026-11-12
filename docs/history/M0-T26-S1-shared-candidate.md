# T26 S1 样式公共机制候选包

同一干净快照的 x86/x64 MinGW GCC C17、Ninja、ccache 全目标构建及 ABI
测试通过；快照清单 SHA-256 为
`a19d462052f4c208b414e954b01c26407f4a7f899726f34a311953552492d373`。
源码来源、资源身份、进程库边界、治理及包检查通过。按要求先发布到
`assets` 根目录和 `O:\winnt`，发布后与候选逐项 SHA-256 一致：

| 文件 | SHA-256 |
| --- | --- |
| run16s.exe | 0390432dd490804c281c55a2407d9794907bcfbd68d5e69aa0a427410421ef8e |
| demo.exe | 144dbe773f4bfe50527a833761daace0c32594e265d14c43d53d337a454f636a |
| ntw16s32.dll | 78d883c06be3c69e14979924249bf93e62e67dd08bee39dc43363ba79033a8aa |
| ntw16s64.dll | 3443a143f1aacda63f01ec04decb8ade2ee28e0ac5dbacf3c8e570509a39d3e3 |
| ntw2ks32.dll | bd47dc97e6355f48ccf89b8234223034a1dbb533f230da9a9464d53629413806 |
| ntw2ks64.dll | aaa4001f0f63818a0bf9e217ae4593b58f4cf0a9c4ad0dbaafa3d91489e26363 |
| ntwshk32.dll | c90f895743ca022349ea2fa14dd49fc0feaa86ef7f34d3093006bab3b338cc2c |
| ntwshk64.dll | de64be6fbe64b0e5df9df0e0da31832b085bbc0da474e73f42144ad87378b30c |

发布后，同快照构建的八组公共呈现、观察、打印、对话框测试在两个位宽
通过；`O:\winnt\run16s --wait` 的 Win16 原生字符串系统菜单及 `--2k`
系统菜单探针在两个位宽通过。UWP 分发测试 x86/x64 均退出 6，返回错误
126；Win2000 外框直接生命周期测试 x86/x64 均退出 3，DWM 恢复未通过。
两项均为失败，不能称作通过或跳过。

完整 `Test-Package.ps1` 未运行：其必选的 `demo-pair` 和 `automatic` 等
用例会创建私有桌面，与用户当前禁令冲突。交互式文件/打印对话框、真实
鼠标、UWP 与完整传播等验证未完成。本候选已发布但未完成交付，不能
提交为通过或关闭 S；需修复/解决限制后重新构建、发布和验证。
