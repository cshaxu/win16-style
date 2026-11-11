# M0 T25 S4 P54：菜单悬停与右侧残影

用户提供画面显示原生菜单高亮覆盖到 Solitaire 客户区、标题右侧有原生按钮残影。隔离桌面在实际 Solitaire 上复现：鼠标移入原生菜单位置后系统绘制侵入客户区。现保留应用鼠标消息，仅在悬停项变化时刷新与原生菜单重叠的客户区，并重绘经典菜单；菜单底色向客户区右边界延伸，遮住原按钮残影。无应用名称特判。

双位宽真实鼠标菜单点击、悬停切换、再次点击关闭均通过，测试中标题栏重绘计数不增加；候选的实际 Solitaire 悬停画面未再出现覆盖与右侧残影。纯 C17、MinGW GCC、Ninja、ccache 双位宽干净构建通过；源码快照清单 SHA-256 `ed28713fc998ca228d3d16f15924ed47f77ee2a2e8bfa19ee81e648ca588a1f6`。同批包 68 项重点回归通过、无跳过。早期含额外非客户区刷新的候选曾出现两个测试超时，收窄路径后重建并通过；不将失败计作通过。用户实际环境及全部 DPI 仍需验证；S4、T25 保持开放。

八文件已发布到 assets 根目录与 `O:/winnt`，摘要一致：

| 文件 | SHA-256 |
| --- | --- |
| demo.exe | ab83efec46e6d27c8cc4f23c45394eb5f24478c5e06bdeaabefb8a9220b2971f |
| ntw16s32.dll | 8d334b5a3589cb24866dccfc710a652b12aeb7105efaa662b175d34e7d2c4c9e |
| ntw16s64.dll | 9e0007a851b4067ae0cd9a3072aa4252528570210baa4e019dcbe892a65e7eac |
| ntw2ks32.dll | 40ed364d8a36e6e5e0254a471cc12fcf42c587131dc2374e7200c4fcfa31f1a2 |
| ntw2ks64.dll | 62a6df44722588ec3d3c714ada4db727ac055950d8a38dda7f2e127b5321e239 |
| ntwshk32.dll | 48c03d079664e817a15917af99cc27a68c809c7dc0f8e45fad187ca82b924aba |
| ntwshk64.dll | 5aedc030ef6d4efc6658a4c21cbd97b9848613bd733eec0f04e71360e8c0e3e3 |
| run16s.exe | 32aa8e8893bb5c45ab89137a4b8659a6386fe2ca303f552ee00f3af74c73b81a |
