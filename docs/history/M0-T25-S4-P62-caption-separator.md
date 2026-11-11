# M0 T25 S4 P62：标题渐变下的分隔线

只读对照 Windows 2000 `ntuser/kernel/inctlpan.c` 的默认 `iCaptionHeight=18` 及 `kernel/caption.c`：标题绘制高度使用 `CYSIZE`，之后另画一条 `CYBORDER` 分隔线。用户提供的原版 Solitaire 图中，标题渐变为 y=9–26，下一行 y=27 为 `#D4D0C8`。当前实现保留 19 个逻辑像素的总体标题布局，但此前将整个区域都绘制为渐变；现将最后一个 DPI 换算后的边框单位绘为经典底色，并对剩余区域绘制渐变与文字。窗口客户区及按钮几何不变。使用 `rg -n 'caption_height|19.*dpi|19,' tests/ntw2ks-dll/frame_target.c src/lib-win32/win2k-window/frame.c` 检查高度调用，分隔线绘制由窗口 `paint` 唯一承担，其他命中属于几何或测试；生产净增 7 行，测试净增 1 行，来源摘要两行更新。

纯 C17、MinGW GCC 16.1.0、Ninja、ccache 4.13.6 双位宽干净构建后，先发布八文件到项目 assets 根目录和 `O:/winnt`，再运行与发布文件同摘要包的 30 项聚焦回归，全部通过、无跳过；包括 x86/x64 标题分隔线底色、窗口、按钮、工具窗口及命中。发布版实际 Solitaire 的菜单、激活、最大化/还原、单次移动/缩放探针通过。原版截图 DPI 未知；其他 DPI、跨屏及用户视觉验收未覆盖，S4 和 T25 保持开放。

源码快照清单 SHA-256 `fe43e1e325656ac67a8ce53e166d6a434a20d250b3ba4d43307ac989802d7df8`。发布与测试包八文件摘要：

| 文件 | SHA-256 |
| --- | --- |
| demo.exe | 72e1ecaa19d0fcf7cba72a071ca16b164a4c4be62fae1892333050a65e9cfd4f |
| ntw16s32.dll | 37b271ec24321dbb3937a089657787b90f7aed45c691a497f4dd7461f8632155 |
| ntw16s64.dll | 56c94ba6fafda0ee6d0c7314f93b6947a87a23617ca9f0a874191580eca7efc2 |
| ntw2ks32.dll | 73885db6805953068328783aacbd1ce6ce46b4c93d5fae3b811de10ca3aaf574 |
| ntw2ks64.dll | ce9a894dfaec7708aa21787a9b9d3d489f2e30c02df5604376256eeff26f70fb |
| ntwshk32.dll | 601f311a1746da1db5e0f6917710b94410ffedcf093a4c0e2e6458b8c0544424 |
| ntwshk64.dll | d68612e75c0c288f61822481b3301485599759886bbe4343616a35764230b50e |
| run16s.exe | 15f6874534642c5655698ab9f39c6a99a278611b95e44cfb7a917ec52c350032 |
