# M0 T25 S5 P5：按下期间禁用关闭命令

只读 Windows 2000 `ntuser/kernel/dwp.c`：标题按钮跟踪结束后，发送 `WM_SYSCOMMAND` 前检查系统菜单中对应命令是否灰显，帮助命令除外。搜索 `rg -n 'WM_SYSCOMMAND|button_input|GetMenuState' src/lib-win32/win2k-window`，标题按钮命令由 `frame.c` 的 `button_input` 唯一发送；此前该路径未在松开时重新检查。现对带系统菜单的窗口检查命令状态，禁用则不发送，帮助和没有系统菜单的情况沿用原路径。生产代码 8 增 3 删；测试新增 27 行并保留原有窗口状态所有者，未新增第二条按钮路径。

有效对照为按下关闭按钮并取得捕获后，由应用禁用 `SC_CLOSE`，再松开按钮：P70 发布前备份报告 `blocked=1`（误发一次）；修正版 x86/x64 均报告 `blocked=0 enabled=1`，即禁用时不发送、恢复启用后正常发送。最初以系统菜单禁用最小化/最大化的负例不成立：Windows 查询/重绘标题栏时根据窗口样式重置其菜单状态，原版 `ntuser/kernel/mnsys.c` 也执行对应初始化；该失败不作为产品缺陷证据。首次候选包因此测试失败，随后修正测试、重新干净构建、重新发布并测试。

纯 C17、MinGW GCC 16.1.0、Ninja、ccache 4.13.6 双位宽干净构建，源码快照清单 SHA-256 `b7bb299bd9fade753fec84d9a3eea5dfa81168cc40fbffdaf94464528173291d`。先发布至 assets 根目录及 `O:/winnt`，再在当前桌面串行运行发布版双位宽共 14 项直接测试：关闭命令、正常按钮、帮助、拖离取消、标题绘制、系统菜单键盘消息和失活菜单均通过；这些是程序消息而非真实键鼠。用户禁止隔离桌面，现有完整包测试的必选项会创建私有桌面，因此本次未运行该套包测试，不计通过；真实 Alt+Space/Escape、其他 DPI 和用户视觉验收仍待完成。S5、T25 保持开放。

| 文件 | SHA-256 |
| --- | --- |
| demo.exe | 843d82346cbc660081cf454da6cb622e5c8ca64c2bac97ce74efdde95d59f84d |
| ntw16s32.dll | 13b6f7bf6c5e7545e64ea248b1a2593fcef2b69706004c21155946e51ade3f87 |
| ntw16s64.dll | bbb4069ad62b033b0d12e711dbe2ac5cd2cb6c6fd5bf561448f1b2e7d66c2c3e |
| ntw2ks32.dll | bf3496400b47d6a3cf09f155d332e641e8aca174909fe7148e5fa3a3ff0a9c52 |
| ntw2ks64.dll | 488293ad77fad37907132ebfeb67eb1578104ae6b8e14271e35bafd8ba397603 |
| ntwshk32.dll | 4e3d5210dd40bb08c08569ac6de4763d3e7f2ad920f1107617ca619824ba7c1d |
| ntwshk64.dll | 895aaa119c24539edaf8fd0367b525e2b7614b321b74408562cedb3c94ec4671 |
| run16s.exe | 12f2dc7d45e90ac7c049613e6aa650ffdb66b4cf7457a9287619369783776f23 |
