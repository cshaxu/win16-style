# M0 T25 S1 P25：WinUI ToggleButton 阶段包

为默认样式来源的 WinUI ToggleButton 增加 Win16 风格模板，复用已有
控件发现、来源保护和恢复。选中状态呈按下边缘，不确定状态呈平起边缘。
S1/T25 保持开放。

GCC、Ninja、ccache 双位宽干净构建、来源资源检查与同批 13 项聚焦包
回归通过；同一快照自有 x64 WinUI 三态对象验证边缘切换及停止后默认
来源恢复，标准启动自有记事本动态创建控件并观察自动接入和会话清理。
首次启动探针误选隐藏 GDI+ 窗口失败，复测通过；测试选择器跳过隐藏
窗口后再次通过。真实鼠标、键盘、像素及 x86 WinUI 未验证。

四文件 SHA-256：

- run16s.exe `81520d492915208ad5adb272d6d56d4e2bef3d8075f0578e2afde24ca02e9d35`
- ntw16s32.dll `bf111b3d5ac9d0ce1fe057f095828d25ee261fee78452f59e419d3e283eb0551`
- ntw16s64.dll `1092264837e59c37debf1dcdf2284f3b3cc3a514642caa64bcad451c6854e8f8`
- demo.exe `90317238ae2960fe4adaec7bc10bb0dd8c16ba7e41d884fc793013000cd21061`

同批四文件已发布 assets 根目录和 O:/winnt，摘要与测试包一致。
