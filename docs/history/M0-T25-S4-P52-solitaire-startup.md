# M0 T25 S4 P52：修复启动时过早改变窗口尺寸

用户报告 `run16s --2k D:\programs\misc\sol.exe` 没有显示窗口。隔离桌面复现到 Solitaire 初始化期间收到样式附着触发的同步 `WM_SIZE`，随后发生访问异常。普通顶层窗口附着后的外框重算改为投递到窗口线程，并在窗口首次显示时及时执行；不根据应用名称特判。本批也保留应用自定义的 `HTCLIENT` 命中结果。

MinGW GCC、Ninja、ccache 的纯 C17 双位宽干净构建通过，源码快照清单 SHA-256 为 `90b91741c4e4f8590461aeceef7c2451034e13f2bb09bd5c8bf35b7bb4534022`。同批发布包 74 项重点回归通过，无跳过；其中双位宽探针确认创建窗口返回前没有额外尺寸消息，之后仍会重算。隔离桌面使用发布版启动的受控测试进程再启动用户指定 Solitaire，确认窗口创建、标题、菜单、模拟激活、最大化还原以及一次移动和缩放。该验证没有覆盖物理输入和全部应用；S4、T25 保持开放。

八文件已发布到 assets 根目录与 `O:/winnt`，摘要一致：

| 文件 | SHA-256 |
| --- | --- |
| demo.exe | 4da3e04b239f6b41463a36eb668b3bddca50bf5feed048ea27cfaae8999a4fb2 |
| ntw16s32.dll | f1b1ac9d3a12f45fd30b07f36089459ece6b75af743f5b1a90b46070227f2a64 |
| ntw16s64.dll | f30233b8b2d1fbf2a2faf96309a6ed6d40a51c91b1280e7c1f5525ff57629b05 |
| ntw2ks32.dll | f6ec30818e5373f9cc908bc4d19b82adffd8ab0a8b5b26dc87056ea666368bf6 |
| ntw2ks64.dll | 6c841c19f3d1f66b08e2e08f7dc71fdd153fdcd5af9b36a943054ab1918c3173 |
| ntwshk32.dll | 430d82fd9a9ba46999feecfef492578fa20dfa8311b427504a39435dcf746f7b |
| ntwshk64.dll | ff44c4a9ce7b881c44c9ac9819ccf40835d69758c34c05a54763a6770104ac80 |
| run16s.exe | c88ac4a116a0fa9cf756b7b8f3078a38b30ae836f162b2c1435e7e705cb2d659 |
