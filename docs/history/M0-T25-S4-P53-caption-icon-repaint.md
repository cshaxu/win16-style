# M0 T25 S4 P53：标题图标与启动重绘

用户指出 Solitaire 标题图标落在渐变之外，且启动时菜单下方可见残留的原生菜单文字。普通系统菜单图标现与标题渐变使用同一背景，并保留图标命中和应用图标；工具窗口不增加图标。创建后重算非客户区时也请求刷新客户区及子窗口，以处理旧画面残留。未对 Solitaire 按名称特判。

在本机隔离桌面以直接启动链运行实际 Solitaire，采集的启动画面确认图标背景与标题渐变相连；双位宽普通和默认图标、工具窗口、启动时序检查通过。用户提供的启动残影在本机直接启动、菜单重画与一次尺寸调整后均未复现，客户区刷新是针对该现象的修复候选，效果仍须用户同环境验证，不能标为已证实修复。历史真实鼠标最大化探针仍曾失败，另一次在延迟输入后通过，尚不足以确认原因或宣称稳定。

纯 C17、MinGW GCC、Ninja、ccache 双位宽干净构建通过，源码快照清单 SHA-256 `4273ca7aaea64604626223ef6715275a51ab7e1631dfc103bf02bfa14a507cf6`。同批包 68 项重点回归通过、无跳过。首次回归中 x86 图标像素和命中通过但私有桌面释放报告超时，单独复测及重跑通过。未覆盖物理键鼠全矩阵、全部 DPI 和用户环境；S4、T25 保持开放。

八文件已发布到 assets 根目录与 `O:/winnt`，摘要一致：

| 文件 | SHA-256 |
| --- | --- |
| demo.exe | a5ea035ec292387425e7ae8855fc3ae911eaba6fad356c170a94bcf105b60fb0 |
| ntw16s32.dll | 85bfa5f337984a1b850f2faa60c4dc99d0ad0f4c0e2f8f90487eeb7ee78e9f0b |
| ntw16s64.dll | 5fadbfb7cb5bae81d2b889c2ce962d4981746a8d657fb85bbdbfaa88c4fef6af |
| ntw2ks32.dll | f69762395d493b1af5d053c69a3f53c9a6d67c52876dc6b936dfcf37fcad4c70 |
| ntw2ks64.dll | 1747955beb8605dafba8b287d25f757833d0de9ca809febd1cc140ff369a7a72 |
| ntwshk32.dll | 40fcfc3689c792a4f0d01b65ba117b94379b41336f2f59ffd12bbfa408819a76 |
| ntwshk64.dll | 2269ea2f130db78b24fa0783f392af0eb7044fd59db7bf471388e82ad35735b6 |
| run16s.exe | d6184faeb8d6437d1264ace54cdad46dcc8cea248341bdcf75505b13c04b416e |
