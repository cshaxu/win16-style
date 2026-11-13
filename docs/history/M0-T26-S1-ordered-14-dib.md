# T26 S1 顺序第 14 项：索引 DIB 绘制

清单图案的索引像素缩放、临时缓冲和 `SetDIBitsToDevice` 提交移入 `lib-style/common/pixel_dib.h`，保留原有调用方尺寸限制、DIB 色表、失败返回和 DC 所有权；Win16 保留原始图案与清单资格策略。公共 `pixel_grid.h` 的 `FillRect` 路径与 DIB 路径的 DC 契约不同，仍分别提供。

双位宽 MinGW GCC、Ninja、ccache 干净构建及公共库依赖检查通过。先发布到 assets 和 O:\winnt，再运行双位宽公共 DIB、清单模型以及 VB5/VB6 自建清单夹具，覆盖颜色、剪裁、DC 画刷、像素、错误来源、重入和清理；专用 VB5/VB6 144 DPI 夹具在两个位宽均通过。使用直接工作入口，没有创建或切换隔离桌面。真实 VB 程序、全部 DPI 和其他风格尚未验证。

| 文件 | SHA-256 |
| --- | --- |
| demo.exe | daadcc0105a110a9202ebd0d027050622463e4d32862ccb38166d10d1e89b044 |
| ntw16s32.dll | b2dd6509f12918bf6ea47ef5d808843c8b976a0cdfec92233dc82f41084e5417 |
| ntw16s64.dll | 65df347487d105062eb412273f0f20b9714352fc424f7cbb6274cd02490b4c3f |
| ntw2ks32.dll | b6bee313925b275778bb04faeaeb6deed4f570a7b867bdbfe745bd5ec91cfaf7 |
| ntw2ks64.dll | d8aaf12550901c59e4992374ed9fba719afe02dd90b4c20d1364fd8922b6d235 |
| ntwshk32.dll | f2bda3c076f09239897c1a5c22bbf3f19027c9345583077951cb47cb93897bda |
| ntwshk64.dll | e47236291d0621eedecf875348ba0c6ea629220009f00f11f294abaa4bc804ea |
| run16s.exe | 338803c7b660ad43d510e59b6f9ab684c240ddda2be6b446b76762d89aeaf112 |

既有导航焦点、Win2000 DWM 恢复和现代窗口覆盖限制继续开放。
