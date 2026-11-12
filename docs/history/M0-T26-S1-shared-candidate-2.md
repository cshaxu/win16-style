# T26 S1 第二批共享机制候选

同一干净快照的双位宽 MinGW GCC C17、Ninja、ccache 全目标构建和 ABI
测试通过。快照清单 SHA-256：
`aa196a891f5b70a7488311791701f37920f6730611bced52feaceafefdaca769`。
来源、原始资源、治理、包身份检查通过；每个位宽的四个公共进程库和
十三个公共样式库通过二进制依赖边界检查。构建时间分别为 x86 102 秒、
x64 129 秒，不包括快照和检查。

按先发布后测试的要求，以下同批文件已发布至 `assets` 根目录与
`O:\winnt`，发布脚本核对两处摘要与候选一致：

| 文件 | SHA-256 |
| --- | --- |
| demo.exe | 9756caea786a81357dfbaf561083bdc27409f1abe35d6866d1fa7364815f2299 |
| ntw16s32.dll | 2802c2ce1b73c94233c66b38fae99a47758d1f16ecbcf59b986a55c3bf5954c3 |
| ntw16s64.dll | 6a2ea689a140bcc2a21dd4efdfd66087a62d43ac290af7a7a163a704675b3efd |
| ntw2ks32.dll | 2745a6c8e60a2424fdcf24a42641f918fc3ab093f9581837f25fdd61e406117a |
| ntw2ks64.dll | 1d8f4bbe3a7f4c87fac886ddca763cd5a0cbbbfabecbb515b4d62a750564998d |
| ntwshk32.dll | cc726f0f572b91d3d85847f5845008ac5b0e5269ca356870b5e11fad5aaec41f |
| ntwshk64.dll | dad6a09b44599268d7265c1f2e4b149cafdb280061aa630a474089e8d088b011 |
| run16s.exe | 77d962cc72c876386ec1ac20f797335cad5bd9b7ad71e852ad63d9271b2b359f |

发布后，使用 `O:\winnt\run16s.exe` 与同快照测试目标，Win16 原生字符串
系统菜单和 Win2000 系统菜单的 x86、x64 探针均退出 0，输出符合绘制及
选择标记。双位宽公共菜单项、UWP 菜单、控件协议和 UWP 原生组件测试
均退出 0。

UWP 分发测试两个位宽均退出 6，返回错误 126；Win2000 外框直接生命周期
测试两个位宽均退出 3，原始 DWM 状态未恢复。两项均为失败。完整发布包
矩阵依旧含用户禁止的私有桌面用例，未运行；真实 UWP 视觉、完整传播和
交互式对话框未覆盖。该候选已发布但交付未完成，不得据此收口 S 或 T。
