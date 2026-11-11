# M0 T25 S4 P10：标题按钮尺寸与命中

普通和工具窗口标题按钮使用 Windows 2000 默认资源中分别为 18 和 15 像素的尺寸，并按 DPI 缩放；绘制、`WM_GETTITLEBARINFOEX` 和鼠标命中使用相同矩形。原版度量依据、早期鼠标测试的不稳定结果和未覆盖面见 [S4 审计](M0-T25-S4-window-audit.md)。标题栏和客户区高度仍由当前 Windows 控制，RTL、狭窄窗口及其他 DPI 未完成原版验证。

源码快照 SHA-256 `f0ef396aeb6ce6d35a903ac1f326e0ceb527bcb699631ff1222b0f0708e42fdf` 使用纯 C17、MinGW GCC 16.1.0、Ninja 和 ccache 4.13.6 双位宽干净构建；正式包 36 项重点测试通过、无跳过。随后以同一批正式文件在自建可见私有桌面分别进行一次 x86/x64 真实鼠标测试：按下、拖离取消、最大化和还原均通过，鼠标与桌面已恢复。该有限测试不覆盖所有按钮、实际应用、RTL、高 DPI 或此前增量鼠标测试间歇失败的根因。用户视觉验收尚未完成。

同批产品发布到 assets 根目录和 `O:\winnt`，与测试包 SHA-256 一致：

| 文件 | SHA-256 |
| --- | --- |
| demo.exe | a678f31b1231945c60c3514ac2bbd1cabb4881ad8d472602af9922114da2265c |
| ntw16s32.dll | 82ed21418eb9239115855a13ba66832e229095e683279cd4c6ac5fd2367e791d |
| ntw16s64.dll | e26d4c4234f99b0c7b85f536138a032a5e0e62ac2ed9a2581f573ac1c5d54c87 |
| ntw2ks32.dll | f039f7d07f0caf4ae6271d0301f50c24b2fd8589f56b255c4cb51f0e33dd11c3 |
| ntw2ks64.dll | 7ac1f150fc6d48851d68dc2d4343b1204b32908b25d7ace1e8dd9cc566647fa5 |
| ntwshk32.dll | 23b9a9f8d716fd61c2ca89e44d6663b67036ea076c8b37806e8c4091a15889b9 |
| ntwshk64.dll | 44a867ac102a1419466fb710aa468eb094c42dab34b790438aa0bb8372dc7420 |
| run16s.exe | acbb7f8118c792f3a910720e50899ebb066740960cad3e6b28304e95abe5d0a3 |

S4 和 T25 保持开放。
