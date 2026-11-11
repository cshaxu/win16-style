# M0 T25 S4 P26：文件打开和保存窗口外框

旧式 `GetOpenFileNameA/W` 和 `GetSaveFileNameA/W` 可在 COMDLG32 内部创建窗口，绕过已有通用对话框入口。现复用已有线程观察与异常清理范围，对所创建的符合条件的窗口附着 Win2000 外框；原参数、应用钩子、返回值和错误状态不变，不转换布局或修改对话框内部控件。

自建私有桌面探针通过 `OFN_ENABLEHOOK` 且不带 `OFN_EXPLORER` 请求旧式路径，带 `OFN_EXPLORER` 请求 Explorer 路径。发布版 P25 在两种位宽、ANSI／Unicode、打开／保存共 8 个旧式组合均未观察到附着，8 个 Explorer 组合已附着；候选 16 个组合均观察到附着且取消返回无扩展错误。测试不证明实际选择、打开、保存、所有标志、回调重入或逐像素视觉。

源码快照清单 SHA-256 `beed4acb1c70e45bcb2258a81336e5b73e26693f23a297d219adacd78a0a81b1`；纯 C17、MinGW GCC、Ninja、ccache 双位宽干净构建，同批包 82 项重点回归通过、无跳过，非完整矩阵；另在私有桌面验证旧版 32 位记事本和计算器的标题及普通菜单打开并正常退出。发布至 assets 根目录及 `O:\winnt`，两处与测试包 SHA-256 一致：

| 文件 | SHA-256 |
| --- | --- |
| demo.exe | a40192026d027e1937d215c1c79be9eb013158cbc900bffca5f8005ae9bc4960 |
| ntw16s32.dll | fedd624138666612d78bcc870529e50c7fcac142c3c0e7f070712adefab7dcce |
| ntw16s64.dll | 216f4b4c78292cd4d3ef17dac61ef2b78612edd4e0ab51a255768c68a87f802e |
| ntw2ks32.dll | 64839c4ee2da28e427588171d65d5c720b1c09641ea58052c67e2a956824c9b8 |
| ntw2ks64.dll | 025181b4c2aed6574a75095fd758f9291ed5f51a32f1fc7a1ad063e1de90a36b |
| ntwshk32.dll | f6359d0bc5bbd4a29c18b4f9605916ca90ba6dcafada943e1052ccb6e9971cc9 |
| ntwshk64.dll | b7bd69de3b24f4323573bdc625f528adef5f7b1a02cf8de15c2b975d2b7f3348 |
| run16s.exe | 1bda59aae7fb105e5e4f7962246fbdade8799137c006579d9f3e9fc7d533ba59 |

现代 COM 文件选择框、其他公共对话框、真实桌面输入及用户视觉验收未据此证明；S4、T25 继续开放。
