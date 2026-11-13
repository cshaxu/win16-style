# T26 S1 顺序第 16 项：图标尺寸

静态图标和按钮图标重复的自然尺寸读取与 `GetIconInfo` 返回位图副本释放迁入 `lib-style/common/icon_size.h`；图标由调用方借用，公共函数不销毁原图标。Win16 保留位置、裁剪、状态和绘制策略。其他调用文件仅调整接线，仍待按顺序审计。

双位宽 MinGW GCC、Ninja、ccache 干净构建及公共库依赖检查通过；先发布到 assets 和 O:\winnt，再运行双位宽彩色、单色及无效图标单测和正式程序 VB 图形按钮图标夹具，图标所有权、状态、像素及清理通过。使用直接工作入口，没有创建或切换隔离桌面。静态图标产品路径、真实 VB 程序和其他风格尚未单独验证。

| 文件 | SHA-256 |
| --- | --- |
| demo.exe | 04b899f1af0195b8d0adac13251ecc5da04a1cee0a2417ed45f258d5609c14f9 |
| ntw16s32.dll | d6a12207f3d27799896f8cba6f3634c59fe028926ffb697d7a7483fa42db624a |
| ntw16s64.dll | 43397eb3245238efcaba44a75771133d8db3e7725431f67a1169a538b71a2ca6 |
| ntw2ks32.dll | d338c3200da28182220b08464f221734360c8ec55f79731ebdef93b5a451c0a3 |
| ntw2ks64.dll | 8a9285f8ce1239cfedbd6b17bfa1e84bf5bbfead7f6cf70faa1a62a5fe7bab5b |
| ntwshk32.dll | 69a0c5caa65019a71bced11fd1a44e8742782c39d90a4a7786af981919d26a9e |
| ntwshk64.dll | 5236893d9948637a18a827fbb31a03f07f1a3491c4ecde19ed9be693b4aa49df |
| run16s.exe | cd1e7c97e111a3ece74957c99584ef27c98473c5df184aaedc945147b65860d8 |

第 16 项剩余文本读取边界仍须核对；既有导航焦点、Win2000 DWM 恢复和现代窗口覆盖限制继续开放。
