# M0 T25 S4 P58：共享菜单命中协议

按用户要求，将自绘菜单的中性命中与点击映射规则提取到 `src/lib-win32/style-common/menu_hit.h`。Win16 窗口的命中与输入分发、Win2000 窗口的命中与点击分发均直接使用该协议。菜单几何、绘制和运行时状态仍由各样式持有；公共头不依赖具体样式或挂接 DLL，也不新增运行产物或导出 ABI。对应的纯 C 协议测试位于 `tests/lib-win32/style-common/menu_hit.c`。

纯 C17、MinGW GCC、Ninja、ccache 双位宽干净构建通过，源码快照清单 SHA-256 `acf4055e14e5e66856e07f8a6cd0fd098eb2793084fc5c3a1a3a5ff54cea658e`。按用户要求先发布，再验证：发布包同摘要副本的 36 项重点回归通过、无跳过；双位宽公共协议测试、使用 `O:/winnt/run16s.exe` 的 Win16 菜单键盘操作与 Win2000 真实鼠标菜单切换通过，后者没有额外标题栏重绘。构建前还运行了增量开发检查；它们不作为发布后验证的替代。用户环境及全部 DPI 未覆盖，S4、T25 保持开放。

八文件已发布到 assets 根目录与 `O:/winnt`，摘要一致：

| 文件 | SHA-256 |
| --- | --- |
| demo.exe | 3f1437a322f0ae96fd42136d61fa280eb8f12deda77c826af9d5868907c41dc6 |
| ntw16s32.dll | 1753cc7c071007aa9a43d29d916bf05c7f908dbd213c7dd08aa5327181c1bfad |
| ntw16s64.dll | 9b603261a8c9ab4d794443700249b4129dfc231cdc6f01e0a799fb0e4bf466c8 |
| ntw2ks32.dll | daf2564d83c4757247485dc7663b7c370b91482648e7b4515c9090bc0a58e58b |
| ntw2ks64.dll | 49890e2c59428ad004c56673c6afc2841da1b2406e6ff5dab2ac5dbe36840769 |
| ntwshk32.dll | 0f42dbe0df9c04ea32b1b89324f8efac38e02b876bf3b69d9a263580a0dd827a |
| ntwshk64.dll | ec28955025a1f58b7f11a804910ac914f08a372655ffe7777f8dfe5b01fadcdc |
| run16s.exe | 4a882dabd5b3dcdd20ad21a6ef622da6c43d32478ea1d6d70bade65fdb93c22e |
