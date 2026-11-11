# M0 T25 S4 P24：消息框窗口外框

系统消息框可能在 USER 内部创建对话框而绕过已接入的对话框 API。现对 `MessageBoxA/W`、`MessageBoxExA/W`、`MessageBoxIndirectA/W` 使用已有的线程对话框观察与异常清理范围，让符合条件的消息框窗口接入 Win2000 外框；保留原 API 返回值和错误状态。消息框内部控件及系统外部宿主不属于本次修改。

自建私有桌面探针对发布版 P20 的 x86/x64 六种入口共 12 例均观察到未附着；候选版 12 例均观察到外框属性、19 像素标题和原 `IDOK` 返回值。首次探针因其关闭消息不正确等待了外层 35 秒，修正为关闭自建窗口后完成；测试线程现有有界失败退出。上述检查不证明实际绘制像素、所有消息框参数或真实桌面输入。

源码快照清单 SHA-256 `56e90b10905c208fa269dda67b22cd721704c867e6eb334c177ccacdf1118cfc`；纯 C17、MinGW GCC、Ninja、ccache 双位宽干净构建，同批 78 项重点回归通过，无跳过，非完整矩阵。发布至 assets 根目录及 `O:\winnt`，两处与测试包 SHA-256 一致：

| 文件 | SHA-256 |
| --- | --- |
| demo.exe | 39fd12ed41876f489f99e5d1e98d9a4219149b63415fb753b4e8a1fe78b14b56 |
| ntw16s32.dll | 73f6f83ba7e1e487b42a3bca947dd5dc8f4fc7373c1e027ae983e86647c1c6b2 |
| ntw16s64.dll | 30d4a49632a632cc4c5401bdd4f59398ee30cbc5d91b8fca5e3c31a825ac0fe4 |
| ntw2ks32.dll | 9a8a707b12013897934f0c8c57e5b3af20cb824ba28189f867c46eff7b0d88e1 |
| ntw2ks64.dll | 99b7e41622dd1e5fac891d9b667bffbe8dbd5ab38d01af44727a0ad41944faee |
| ntwshk32.dll | 7a3a0b799471671cae14dae71d147186b81569281412de18e8a2f82aa467f78c |
| ntwshk64.dll | 1b0d9e14e44f36960334cf69ab9849bf50b762c5e5edde18838f5181d45c7122 |
| run16s.exe | fb891f519d244458fd2211c703e6b7d3ddcbc0815a0a6b62468da8f201edf62b |

完整消息框样式、其他系统窗口、真实鼠标与用户视觉验收尚未完成；S4、T25 继续开放。
