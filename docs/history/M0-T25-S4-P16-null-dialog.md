# M0 T25 S4 P16：空过程对话框外框

非模态对话框创建成功返回后，调用已有外框附着器；适用于没有对话框过程或未产生 `WM_INITDIALOG` 的普通顶层窗口。附着器保留现有窗口筛选，应用原始返回值和错误状态不变。

只读检查旧版计算器发现其 `CreateDialogParamA` 调用传入空过程。双位宽自建探针覆盖 ANSI／Unicode 空过程的内存模板和已有模态／非模态路径；使用同批候选包在私有桌面实际启动旧版 32 位计算器，观察标题高 19、菜单命中及打开，进程正常退出。此结果不证明该应用全部交互或其他对话框入口均已覆盖。

源码快照清单 SHA-256 `a664e5750a42821df8c627dc2d3ee82a94face3abd84ac253835393c1e263b56`；纯 C17、MinGW GCC、Ninja 和 ccache 双位宽干净构建。60 项重点包测试以 2 并发通过，没有跳过，并非完整矩阵。之前 4 并发运行时 `global-attach-x64` 超时，单项复测及本次重跑通过，首次失败原因未确定，不能据此认定已修复。旧版录音机此前的非正常退出仍未解决。

八文件发布到 assets 根目录和 `O:\winnt`，与测试包 SHA-256 一致：

| 文件 | SHA-256 |
| --- | --- |
| demo.exe | 2cbf8089863b2a082935c5697477c5ff8bda8eda9452854c3a41110a7ffd75e6 |
| ntw16s32.dll | a076f2f445bc26e41ceb6f0112374f46badd501c4aa855ea6babcb2394631092 |
| ntw16s64.dll | 6b13807d5fbfa53ed18e48876e6e14da9169de8fa97a6085ec4f9f1ee87c80f2 |
| ntw2ks32.dll | 17eaf4810e0831a21a7ddcebad481e19a7b09381848d55fb2d6c881450bcbe4d |
| ntw2ks64.dll | ecd113e83eacfa33f744f019408c78e8f70ebfce7be71359823d6d2e27486624 |
| ntwshk32.dll | 25daafd031fba130403d7b2bb1791459782abeffee9ff6cb60c2c35cd269fc4b |
| ntwshk64.dll | 9f3159411ef443bc684069a38965d622a99504750243182b64246a34e0453871 |
| run16s.exe | 5554a61133fca9333a8a537f3c5a7060377b43815dd5835e4945fbc37f2621a3 |

S4、T25 继续开放；原版视觉、完整集成、全部 DPI 和真实鼠标尚待验证。
