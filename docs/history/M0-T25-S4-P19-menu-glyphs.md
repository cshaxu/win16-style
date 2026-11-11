# M0 T25 S4 P19：系统菜单标准图标

P18 的菜单绘制覆盖了还原、最小化、最大化及关闭的系统图标。原版 `private/ntos/w32/ntuser/kernel/mndraw.c`（SHA-256 `f200afbfb8ad408938b3bc7c1b8b4a5ec83a878286cae355d230fff8069ed56f`）可见这些项目使用系统位图。现按其类别自行绘制图形，并根据选中和禁用状态着色；未复制原版位图，不声称逐像素一致。

自建菜单探针在已发布 P18 上检查到 0 个图标，在候选双位宽各观察到 4 个；这检查图标区域像素存在，不证明图案精确性。私有桌面切换可见测试返回失败，真实鼠标及人工视觉仍未验证。

源码快照清单 SHA-256 `46eb7db9d27a20e037fc03aa971ee747237dbb4acca136931798a11d0d862cff`；纯 C17、MinGW GCC、Ninja、ccache 双位宽干净构建，同批包 74 项重点回归通过，无跳过，非完整矩阵。发布到 assets 根目录和 `O:\winnt`，两处与测试包 SHA-256 一致：

| 文件 | SHA-256 |
| --- | --- |
| demo.exe | 95568e212fef836d3c4803ef1b478550ee5ce166bd22eb30344a39658f0568b2 |
| ntw16s32.dll | 4381bc106290931ac01524a382311fc7701e70bdd711560651d273cee96646a1 |
| ntw16s64.dll | 04789b6a88caa2b2231fab15d086452a6dee8422f89f51378a7a3d850c56803b |
| ntw2ks32.dll | ef7e9bee485e2441e1b2f533c017b0ba205fee68bdb442c3953965233b22be84 |
| ntw2ks64.dll | 04b379672426e8cc75cf66cc4d7680e7f4bad84d40aa360a9db789973e699810 |
| ntwshk32.dll | dee35e0a67226e3ad764c8c3c3c255b0922699896fb84970a54363ecb4f325a7 |
| ntwshk64.dll | c236615b05b69e5b726b0ea151551b69ee9593a055849e79b952a0fb9fc8ad89 |
| run16s.exe | a6db9af2af0e82ccab57bc2c162a3d2538636546f4e6a2f1912b7fa7756402f0 |

菜单边缘、阴影、完整 DPI 和实际交互仍待验证；S4、T25 继续开放。
