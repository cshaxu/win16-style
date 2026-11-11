# M0 T25 S4 P51：保留应用自定义命中结果

普通窗口应用可通过 `WM_NCHITTEST` 返回鼠标穿透等结果。P45 发布包的 x64 负例中，应用在标题和最大化按钮位置返回 `HTTRANSPARENT`，样式层却分别改成 `HTCAPTION` 和 `HTMAXBUTTON`。现保留应用返回的 `HTTRANSPARENT`、`HTERROR` 和 `HTNOWHERE`，其他命中继续走既有几何与按钮处理；不按程序名称特判。

双位宽私有桌面候选测试中，上述三种结果在标题、按钮和客户区均保持不变；系统图标、菜单几何及普通标题按钮的增量回归通过。纯 C17、MinGW GCC 16.1.0、Ninja 和 ccache 双位宽干净构建，源码快照清单 SHA-256 `388ce4bb4abd6794f5e0a459fd3c48c8b7d83e14b90d2364f148059149bd289e`；同批包 64 项重点回归通过、无跳过。已发布至 assets 根目录及 `O:/winnt`，两处与测试包摘要一致：

| 文件 | SHA-256 |
| --- | --- |
| demo.exe | a29c6aaeb88eea31486d06197ac04ef854af2d5d9e4ba5299beaeefb0b345aeb |
| ntw16s32.dll | 334caa17e363820fd4fc5b384adfa21b8d94ba8901ec7858a8e78bf1b2a9be84 |
| ntw16s64.dll | 080ff1c8f342e5034e840091b76bd1fee012964744b5e826ad5e854a6c29336c |
| ntw2ks32.dll | 83b9b0573e5bcac03290e342944f94dcedf52b62213551cdfc7fdd630ce9db78 |
| ntw2ks64.dll | f0fcc3a072d3c752bc496681c5dff3a2571154f161d87b7a2d128f3d3ec00e98 |
| ntwshk32.dll | 6b3f861e5930dcc6f2221443c62ae1a832f95b679bc685bfbc4855fba27ae581 |
| ntwshk64.dll | 117fba02c23aec0142196c5c14659ad71fdd7264f2708980516fb0a5b3baaac1 |
| run16s.exe | 313894a06c94689704de944a4e6904fb295709d5ffa7d786c6fed8603bb7af44 |

该检查不证明所有应用自定义命中值、全部 DPI 或物理鼠标行为；此前真实鼠标最大化不稳定仍未解决。S4 和 T25 保持开放。
