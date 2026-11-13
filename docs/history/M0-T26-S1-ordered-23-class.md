# T26 S1 顺序第 23 项：公共按钮类型

VB Frame 的分组按钮类型语义已迁入 `lib-style/common/control_class.h`；Win16 的两个生产调用方使用相同入口。Win16 的实际绘制资格限制仍留原处。

双位宽 MinGW GCC、Ninja、ccache 干净构建和公共依赖检查通过；先发布 assets 与 O:\winnt，八文件摘要一致，再对正式包运行双位宽自建 VB6 Frame 和普通按钮夹具，通过。未使用隔离桌面。真实 VB5/VB6 程序、其他风格未验证。已观察到的 VB 清单重入失败仍未解决，见顺序台账。

| 文件 | SHA-256 |
| --- | --- |
| demo.exe | 8e8cd0d929d8467d80fa7f9c74cfd5c3cbc8c18d21ec953fc90ca4b858a6e7c7 |
| ntw16s32.dll | 368309fe22dd87f98db78389605325c221477ff5ae9d8487abcaf0680cd8c5d5 |
| ntw16s64.dll | c9634f823a52f68877591187ea142e748c966c81bf8c64ed2ba4ea82bb7bb288 |
| ntw2ks32.dll | ba21a205c5107ce372dd1197ea84861e696fba13473afceb2473e2582d44ce9f |
| ntw2ks64.dll | ec5289a7e070a779c8e338ad0e17d03285e31afce9b538307bd2b27afc33b409 |
| ntwshk32.dll | 582a322f3f29880eeef4cd09212d1848b4b5f22544d78d534fb0c526b579e02e |
| ntwshk64.dll | 5d4d52b96e4c2405aca3f46c91509b1ce80e18db143966c2a9d083319e7c5051 |
| run16s.exe | 6b0950dd23ce4eb877a4ab799f1783ff9a6da7c4f416948ca8283ce4e1843e41 |
