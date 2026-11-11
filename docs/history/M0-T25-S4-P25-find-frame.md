# M0 T25 S4 P25：查找与替换窗口外框

`FindTextA/W` 和 `ReplaceTextA/W` 可在 COMDLG32 内部创建非模态对话框，绕过通用对话框创建入口。现复用已有线程对话框观察与异常清理范围，创建成功后补充附着窗口；保留原返回值、错误状态、应用参数及通知。此次仅处理外框，不修改查找界面内部控件。新增系统 `comdlg32.dll` 导入，并在包校验中明确列出。

发布版 P24 的 x86/x64、ANSI／Unicode、查找／替换共 8 例均未观察到窗口附着；候选 8 例均观察到附着，关闭后窗口消失并各收到一次 `FR_DIALOGTERM`。该测试不代表实际应用查找匹配、所有参数、桌面鼠标或像素视觉已通过。

首次干净构建编译成功，但包校验因导入清单未包含新系统 DLL 失败；修正清单并核对两种位宽的实际导入后，以新快照重新干净构建通过。源码快照清单 SHA-256 `12ca132d36e4ef8c349eb5794dcbd700a24656d332a0c38d85d6bbd2f47879f9`；纯 C17、MinGW GCC、Ninja、ccache 双位宽构建，同批包 80 项重点回归通过、无跳过，非完整矩阵。发布至 assets 根目录及 `O:\winnt`，与测试包 SHA-256 一致：

| 文件 | SHA-256 |
| --- | --- |
| demo.exe | 54dadd796435d75f3c47e59234e2203adac29fecd3a229804a5621bb142c4692 |
| ntw16s32.dll | 57cb2d5d27a626e5e2179cb2a26d97ba1f9d07f9760aee51e1d0ddea960b4df1 |
| ntw16s64.dll | 6b5e25a2ed7467e42aa298bf92ac59f2bc310bfdc6fc68d7971409c0f227234d |
| ntw2ks32.dll | 9c9b90def7a0a25e8f5416a8d594f1a62fe07202ceb5ab851c4508ab2e1dfff2 |
| ntw2ks64.dll | 21d91485d54ddb5b02d2a919f57ecf70c75e93ebda698600c9f36efb8d646e04 |
| ntwshk32.dll | 6eaa23630b773a6f599571e9413752a3161b4b03c86303eddebb8a80f66575c4 |
| ntwshk64.dll | fac3b834bb772749b2e86cff6707d3931f46e10540ee7c42a257f5cf37cab3c0 |
| run16s.exe | 175e19dd3f97af68ae65803715eb2bd93de036adfe0dd58d89a74eab22d7258c |

文件选择和其他公共对话框外框、真实桌面交互及视觉验收未据此证明；S4、T25 继续开放。
