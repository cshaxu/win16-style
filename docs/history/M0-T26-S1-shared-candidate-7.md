# T26 S1 第七批共享候选

快照清单 SHA-256：`b513adf620d3305af568f76ca802459ce2707411e2cd7df0414dba48db6e17f4`。MinGW GCC、Ninja、ccache 的双位宽干净构建、ABI、源码与二进制公共依赖边界、原图形资源及八文件包检查通过。先发布后测试；以下摘要在 assets 和 O:\winnt 均逐项一致。

| 文件 | SHA-256 |
| --- | --- |
| demo.exe | 3bf575cc6b1fb832981b274b2600b7fce20f4f614888dc08d34f71acdc22e020 |
| ntw16s32.dll | c2e2daf6b7995a8eaeed2d6ffa40fb6dad8bc86fbc0f9a31d8ea69ba6050392a |
| ntw16s64.dll | 7041931c7f19b4c4615a0394fc55c1acd1c46044e4641d2211640df7c18bbd7e |
| ntw2ks32.dll | c757f922a43a5f827415cb12fa59ca3f0c2f06d09549151516d4fa726e330674 |
| ntw2ks64.dll | 652328f0355fdad8b5fa5e38294aa80d3f2ba3c28531e160172dc39c742d0587 |
| ntwshk32.dll | b2e02590f8894c9be3e7c53bbac1b5edd6e937c922c3fd3db5afc213993fc7cd |
| ntwshk64.dll | 3c5b8568628b9479263a87e002df49e5385618bacbfa0b5c891b4c88b1086a96 |
| run16s.exe | 6a19cbb7b80b910c01b60b009470cde92fee749499782a32af27e87038443485 |

发布后，使用 O:\winnt\run16s.exe 和干净快照中测试程序，自建 x86/x64 窗口的 Win2000 系统菜单显示、选中项切换、悬停通过；双位宽公共滚动状态与 Win16 几何/跟踪测试通过。对照发布 DLL 摘要一致的构建 DLL，双位宽观察器嵌套、窗口身份、共享外框恢复工具、现代外框布局、共享主题钩子链、Win16/Win2000 自建窗口切换通过；后者不是正式全局会话测试。

未通过或未覆盖：Win2000 真实外框的 DWM 恢复测试双位宽均退出 3（同机 Git 基线亦失败）；UWP 同进程分发通过，但跨进程钩子诊断仍为 126，真实 UWP/WinUI 应用未验证。全局托盘、全部应用、完整发布包矩阵及用户视觉验收未完成。不创建或切换私有桌面，不使用 computer-use。此候选不构成 T26 收口。

补充发布包回归：将 `O:\winnt` 的三个同位宽 DLL 逐字节复制到自有测试目录，x86/x64 均通过无 GUI 模块预加载、COMCTL32 v5/v6 原生异常清理、Win16 与公共主题状态交接、Win2000 控件所有权/跨线程负例/附着移除；通过正式 `O:\winnt\run16s --wait` 启动的两种位宽目标也通过 Win16 到 Win2000 的控件状态交接、移除及主题请求跟踪。测试副本已清理。这些结果不扩大上述全局、UWP 或 DWM 恢复覆盖范围。
