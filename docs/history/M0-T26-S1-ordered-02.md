# T26 S1 顺序审计第 2 项交付

双位宽 MinGW、Ninja、ccache 干净构建通过，先发布 assets 和 O:\winnt，再验证正式包。

`win16-common/font.c` 的字形选择、缩放、DPI、资源和线程内租约属于 Win16 方案；两处重复 ANSI FNT v2 字节序列化已抽入 `common/bitmap_font.h`，Win16 两条生成路径直接调用。公共部分不引用 Win16 或产品私有实现，其他风格尚未接入。

双位宽公共格式字段及偏移、字体隔离和租约释放测试通过。正式 DLL 的双位宽完整字体布局测试通过：两个 DPI 上下文、12 种字体变化、84 个控件场景及原生对话框、菜单、重入和资源清理。未覆盖真实 UWP/WinUI 或其他风格的字体实现。

导航焦点、Win2000 DWM 恢复等既有失败及真实全局等未覆盖项仍开放。

| 文件 | SHA-256 |
| --- | --- |
| run16s.exe | a80d3bd3f792b282d3e19a91378a7a67e0aeadfd6aa73a514804c8edf48d295d |
| ntw16s32.dll | e1884b309f320bc22b8d3ac31b445c50ebb92e8bacffd5edd07a9ffb6eb27388 |
| ntw16s64.dll | c375198018bc7877af6f3a12864973194e61ea568d71b35eddad9fb4e4dcf92f |
| ntw2ks32.dll | a2689ae64ecd534a1fc106654c2715abfbb07118daf5897a013bda8cebb570bd |
| ntw2ks64.dll | ce9c413bdaca2c50a3a33e1bd964cd56977a0af1f524cd6e3515b663ac406fc7 |
| ntwshk32.dll | b08bee242412be6353dbbba017870ef14a270eb46a15636958aaaaf21de22cf8 |
| ntwshk64.dll | a99aec2871961651f7370f27b4979d01a530f7b8a3ffbc72dbae09e8ca3103ed |
| demo.exe | e2e227bc828aca17b76cd8c607ce1bb7bae91861d34e8b7a5faab9df5ecc1213 |

源码快照清单 SHA-256：`4940118c064171fdd47bcbfcb94b27dd2320013c603aa9cb6fb25967bb1eebf8`。测试后清理本批构建、候选及发布备份。

提交差异统计（不含本记录）：8 个文本文件，新增 110 行、删除 36 行、净增 74 行，另有 8 个二进制文件。公共层拥有格式序列化，Win16 保留字体资源与租约；两份私有标头编码已合并。
