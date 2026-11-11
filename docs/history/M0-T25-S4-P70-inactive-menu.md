# M0 T25 S4 P70：失活窗口菜单文字

用户提供的 Windows 2000 图中，后方失活 Solitaire 的菜单文字为灰色。只读原版 `ntuser/kernel/mndraw.c`：菜单绘制根据窗口 frame 活动状态设置 `MFINACTIVE`，随后按非活动菜单绘制文字。搜索 `rg -n 'WM_NCACTIVATE|MFS_DISABLED|paint_menu' src/lib-win32/win2k-window/frame.c`：窗口活动状态由既有 `frame_property` 持有，唯一菜单栏 painter 此前仅按菜单项禁用状态选色，未使用窗口活动状态。当前实现用同一状态控制菜单文字：活动时正常颜色，失活时灰色，菜单项本身不被修改；`WM_NCACTIVATE` 已触发重绘，不增加新状态或重复路径。

旧发布版的 x64 自有窗口负例观测 `active_black=28 inactive_black=28 inactive_gray=0`。修正后 x86/x64 各报告 `active_black=28 inactive_black=0 inactive_gray=28`。按要求双位宽干净构建后先发布，再对同摘要包运行 42 项聚焦回归，全部通过、无跳过；覆盖菜单激活颜色、菜单键盘与输入、边框、标题命令及对话框。发布版实际 Solitaire 的菜单打开、窗口激活颜色、单次移动/缩放和最大化/还原消息级测试通过。没有对用户提供的原版画面做完整像素复刻，也未运行全部包矩阵或用户手动验收；S4、T25 保持开放。源码修改为生产 2 增 1 删，测试新增失活菜单探针，使用现有测试和源码来源摘要。

使用纯 C17、MinGW GCC 16.1.0、Ninja、ccache 4.13.6 双位宽干净构建，源码快照清单 SHA-256 `eebf6a64c0cfddd77aad41e8529cefafa5ba47f6df031cac510eb76a37f95a9a`。项目 assets 根目录及 `O:/winnt` 与测试包八文件一致：

| 文件 | SHA-256 |
| --- | --- |
| demo.exe | ca550604223085bec0fa44f69e8afb666788e79cd9bae101d6d8303ec7bd2ef1 |
| ntw16s32.dll | 139fb8b70bf2b38589c3a1017989e5e86871ecd039d722bb0f08c99a49019b76 |
| ntw16s64.dll | 6b341cf42fbfe7ea413f6b75d79202e0215640872e6f815eba7af9425d33bdbc |
| ntw2ks32.dll | 10b76201f26205157348a571b2cd9713c01f687eb51734a8d18be88ef9e972cd |
| ntw2ks64.dll | f4b31c41984d2942fa73062828a057eae71333c0e6a7421e892fad2d9e870520 |
| ntwshk32.dll | d3364f5ffbbd91f527f121aed8b49092ad7e55bc421a86bf96b11486c9056eb2 |
| ntwshk64.dll | f224fd1dfa1e4c3eb1cc7475a03445cb1aa287c52da4324adc5a30125c21f2b1 |
| run16s.exe | aa2aa67a9b18ca734a9b2ad57327efcb9aa0fb38ca7147ada5e8a3c8a763a576 |
