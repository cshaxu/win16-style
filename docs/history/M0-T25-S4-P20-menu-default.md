# M0 T25 S4 P20：系统菜单默认项字重

Windows 2000 原版 `private/ntos/w32/ntuser/kernel/mndraw.c`（SHA-256 `f200afbfb8ad408938b3bc7c1b8b4a5ec83a878286cae355d230fff8069ed56f`）对默认菜单项使用默认项字体或相应加重处理。当前实现按项目默认状态选择粗体，普通项目保留普通字重；字体不可用时沿用现有回退。

私有桌面探针对同一菜单切换默认项并比较该行实际绘制像素：已发布 P19 字形无变化，候选 x86/x64 有变化。探针验证字形对状态的响应，不证明原版字形逐像素一致。可见私有桌面切换失败，真实鼠标和人工视觉尚未验证。

源码快照清单 SHA-256 `d72f802168e1c9341732533985fe531c8a7eb22d38a5289ade209d34f33ecbba`；纯 C17、MinGW GCC、Ninja、ccache 双位宽干净构建，同批包 76 项重点回归通过，无跳过，非完整矩阵。发布到 assets 根目录及 `O:\winnt`，与测试包 SHA-256 一致：

| 文件 | SHA-256 |
| --- | --- |
| demo.exe | 93cd78618f8e8dd13776e16923bbdf475cd5ef8ce708cec8624f4bb170c29c24 |
| ntw16s32.dll | 1feecb425c566c2af430eaa996298b0c8437eac0443972dcd2860ec3312f7d64 |
| ntw16s64.dll | b78da2a874cac377a0715a306e6cb19d38f833ad286e3a158eff9fe796fe9c38 |
| ntw2ks32.dll | 201941aec13a567fad2b2091f36d24a055416512f30417a2d6b78bb392709064 |
| ntw2ks64.dll | fe0362d03ced1114e5a4aede6004e43be9d82229967c084cf45606b9f429d3c4 |
| ntwshk32.dll | 2bd35929429fa3e7d0f454adc1ea9d9acad203b8bcf6270b0e98401530f09a3e |
| ntwshk64.dll | 7daadd4590864124940907847a4e64c13a9c1df897bd6f830de60a023a3e3691 |
| run16s.exe | 1a1efb5f1e8a6be6228bd232e49d2a0ce8023bbc80b0fd805c9ac59678782ea7 |

菜单边缘、阴影、完整 DPI 和实际交互仍待验证；S4、T25 继续开放。
