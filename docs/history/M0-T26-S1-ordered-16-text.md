# T26 S1 顺序第 16 项：窗口文本快照

选择控件与组合框重复的窗口文本长度查询、堆分配和复制迁入 `lib-style/common/window_text.h`。调用方仍负责释放，空字符串仍分配独立缓冲；字体、绘制与控件状态策略继续由 Win16 处理。组合框源文件仅接入新函数，仍待按顺序审计。

双位宽 MinGW GCC、Ninja、ccache 干净构建及公共依赖检查通过，先发布到 assets 和 O:\winnt，再运行双位宽 Unicode、空字符串、独立快照测试及正式包组合框、分组按钮和复选框夹具，均通过。使用直接工作入口，没有创建或切换隔离桌面。真实第三方程序、其他风格和独立静态图标产品路径仍未验证。

| 文件 | SHA-256 |
| --- | --- |
| demo.exe | 5b05005012bd6a7ac1c6211670031424ecb29deaae201376c2d8266ae26d2463 |
| ntw16s32.dll | 760945f24a244ee782a7524a704104a8db079f7cd740cd85e9b71e8dd31f7542 |
| ntw16s64.dll | 1b17f71ef19fca9fa344398de59519689b21c520829e254dab3dbcaea855dca7 |
| ntw2ks32.dll | 5aa6defdc3920af62b8b65d20099b7f3bfde23487adc5675c1855392479455e8 |
| ntw2ks64.dll | 3e65946ce922ef2a3b66c1de3fe27e094a442a5c979712cdf3dfedd36d140e78 |
| ntwshk32.dll | c169a9f72173df4f3065c34053a88dc43886a72ba40dbae4463165d8692c4904 |
| ntwshk64.dll | 822baafb3dbcc96d0d4cd96af35262651ba3709177212aee96be2e563ccd9dcd |
| run16s.exe | cae263562da6cfac52bb9f17bbcc9f71f2e0e282f1db0a18ecf81fc3bafccda3 |

既有导航焦点、Win2000 DWM 恢复和现代窗口覆盖限制继续开放。
