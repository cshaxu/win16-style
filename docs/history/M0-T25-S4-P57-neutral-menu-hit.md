# M0 T25 S4 P57：阻止原生菜单浮现

用户反馈 P56 中 Solitaire 菜单仍会浮出。对照 Win16 的 `frame_layout.c` 和 `frame_dispatch.c`：Windows 在收到后续鼠标消息之前就可能对 `HTMENU` 做原生热跟踪；Win16 将自绘菜单区域命中返回为 `HTBORDER`，再依据样式自己的菜单几何路由点击。Win2000 原先仍返回 `HTMENU`，P54 的事后局部重绘不能保证覆盖系统绘制。

Win2000 现采用同一中性命中与点击路由机制，删除 P54 事后重绘状态及其辅助函数，保留应用显式自定义命中结果。菜单几何及绘制由各自窗口样式负责；`ntwshk-dll` 负责通用挂接，不把 Win16 或 Win2000 的样式布局放入其中。两者是否提取独立窗口公共机制可在具体接口稳定后再评估，本次没有引入重复框架或改变挂接 ABI。

同条件真实 Solitaire 隔离桌面对照中，旧发布版在原生菜单区域仍返回 `HTMENU`；新版本返回中性命中，连续悬停两个菜单项时重叠客户区像素不变，实际菜单仍可打开。按新规范，纯 C17、MinGW GCC、Ninja、ccache 双位宽干净构建后先发布，再使用 `O:/winnt` 的实际文件复测：真实 Solitaire 悬停通过，双位宽真实菜单点击和切换通过且标题栏重绘计数未增加；发布文件同摘要副本的 38 项重点回归通过，无跳过。用户环境和其他 DPI 仍需验证；S4、T25 保持开放。

源码快照清单 SHA-256 `3155ecf9ba4e7a1c8eca8301a1b3e8fb09fbeb42b1d97cbb23fbcea26801e76d`；assets 和 `O:/winnt` 八文件与测试包摘要一致：

| 文件 | SHA-256 |
| --- | --- |
| demo.exe | 40bf22590d80075158fcd845f42c27d09a6dbb13991bbbca431ee1f8264a6eac |
| ntw16s32.dll | 1a306b906db4ab01822b3cd71039f85c305bf8dadafd8e0a031ec565dc1295a9 |
| ntw16s64.dll | b907ff451dc79f400e1c8d26f3f12b1b194994917eba1ed7c157b10fd206bab8 |
| ntw2ks32.dll | 0e7e10589380d325894a214bffd61514a5e92070b33bd6c53c73e6c340ba3126 |
| ntw2ks64.dll | 02339c90ea9474b73660af02dfc576d3eb180468cbb9260084feb29093c5e40e |
| ntwshk32.dll | bd3fda3728c994a05b0e52622270bc2a0b86a07dd7fc148c7833dda1a1344d68 |
| ntwshk64.dll | 5e7dec3fd23a9b536334497bdb6bea85b3cbe5969514fee3358593c07d59550e |
| run16s.exe | 11310a58c54e6a6cf890f388cd1511cb0b3c608f89ce40e910ed136841aa51a5 |
