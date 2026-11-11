# M0 T25 S4 P28：颜色选择窗口外框

`ChooseColorA/W` 可在 COMDLG32 内部创建窗口而绕过通用对话框入口。现复用文件窗口的线程观察及异常清理范围，为符合条件的颜色窗口附着 Win2000 外框；不更改应用参数、布局、内部控件或原返回值。字体窗口未添加额外接入。

自建私有桌面对照中，P26 发布版 x86/x64、ANSI／Unicode 的颜色窗口 4 例均未附着，字体窗口 4 例已附着；候选的 8 例均附着并正常取消、扩展错误为零。候选颜色窗口另外验证 4 例确认返回预设颜色，结果与输入一致。以上检查不证明完整控件样式、所有参数、所有字体选择行为、真实鼠标或原版像素。

源码快照清单 SHA-256 `1ff544b9562d4b94a937d894bc93ae7179f7f9f98aded0bd468d90dc57d957cd`；纯 C17、MinGW GCC、Ninja、ccache 双位宽干净构建；同批 88 项重点回归通过、无跳过，耗时约 96 秒，非完整矩阵。发布至 assets 根目录及 `O:\winnt`，两处与测试包 SHA-256 一致：

| 文件 | SHA-256 |
| --- | --- |
| demo.exe | 43b057f2c5bd989f57bf69c2bec9a00ddf216c6f6b8b934c8e601fac303679bb |
| ntw16s32.dll | 657276bd0aa8ae11d46c1e4517da427e5a5fec810633e0886d6081752e2d2b4e |
| ntw16s64.dll | 6a35ac661412080e5a2f21d3c11f61b4f4077e528ef70e0e8e04244571081b6b |
| ntw2ks32.dll | bd30d3ee7e7f25a9e83adc3b085a007a080bfc4d9c799b7e89db5ee9251b8375 |
| ntw2ks64.dll | da54cc93d8d9836bd661acde938626777f013d06ed70ae6bef45b93afa8d8be6 |
| ntwshk32.dll | c3fb264e709c27d88ee02066371ce934042b1217a8a6c73127756ed611bbedf2 |
| ntwshk64.dll | cfcd2728173bdcb0df50632500e9a6480045691b2173c6df1109216acbe12ed5 |
| run16s.exe | bd00e831d837b74a7460461c67e29df26d60e57d932d51118a6f37c25dd0a284 |

其他公共对话框、现代 COM 窗口、真实桌面输入与用户视觉验收尚未据此证明；S4、T25 继续开放。
