# M0 T25 S4 P55：按原版参考调整按钮位置与颜色

用户提供的 Windows 2000 Solitaire 截图中，菜单及按钮面采样为 `#D4D0C8`，普通标题按钮约 16×14 截图像素，按钮组右侧保留边框空隙，关闭按钮与前一按钮间保留两像素间隔；截图的原始 DPI 未确认。标题渐变右端为浅蓝，图标与渐变同底。以此参考调整按钮大小及纵向居中位置、菜单和窗口边框底色、标题渐变；渐变延伸到按钮间隙。不将参考截图或原系统图形复制进产品。

纯 C17、MinGW GCC、Ninja、ccache 双位宽干净构建通过，源码快照清单 SHA-256 `98047816dc5be7c6771a4716bad63521279e8a3ce3dc5aa0614ee0c1d660e5b0`；同批包 68 项重点回归通过，无跳过。双位宽探针验证按钮宽高、相对标题的纵向位置、右缘、命中、窗口消息级最小化/最大化/还原及关闭；实际 Solitaire 候选画面与参考做过目视对照。真实鼠标探针中两种位宽的按下、拖离取消通过，但紧接着再次点击最大化未观察到成功，未计通过，原因仍需调查；不以消息级通过替代真实输入。其他 DPI、用户环境视觉验收仍未完成。S4、T25 保持开放。

八文件已发布到 assets 根目录和 `O:/winnt`，摘要一致：

| 文件 | SHA-256 |
| --- | --- |
| demo.exe | 307505585cbe9b837d762efe649c0043af4095f6e1a516728fefcefa754c5bf4 |
| ntw16s32.dll | 65aa90d6756a11dc866684f2b487c8915ace6b82fc7560c211eab2cbda5c7d79 |
| ntw16s64.dll | ffee8b956fca3fa8d21a46ff894097e250234291b5b75b7bb174cdc5f5fe399c |
| ntw2ks32.dll | f443a172bbdbc4e80728d26c95a2af6051c4e67ea93f34390c4d56ec1c300753 |
| ntw2ks64.dll | fc34761db59efcbe6c91ec6832081ac78e49addee591f4fdacf8c900c4036d33 |
| ntwshk32.dll | 21d81bf1354d9e7892a966d5f41b9507dc961edb73469ae1fe47b431f6762df4 |
| ntwshk64.dll | 4554837dbcce160dff4a0560826a2ce3ed6c2304c54140f523d0b3510586a6b8 |
| run16s.exe | cda61ca00ea980a3423019995a672112eaa093280eed7a3f66bb3a3aacdbae72 |
