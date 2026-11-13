# T26 S1 顺序第 12 项：实色边框

原 Win16 实色矩形边框的内缩绘制与画刷生命周期迁入 `lib-style/common/paint_frame.h`；Win16 基本、组合框、列表、滚动条和特殊控件调用公共入口。按钮立体边、配色与箭头图案仍在 Win16 侧；被改动的其他源文件仅更新调用，仍按顺序等待审计。

双位宽 MinGW GCC、Ninja、ccache 干净构建及公共库依赖检查通过；先发布到 assets 和 O:\winnt，再运行公共边框像素测试，以及正式启动器加载的双位宽网格、组合框和滚动条夹具，均通过。测试覆盖边框厚度、内部及外部像素、重入移除和资源清理；使用直接工作入口，没有创建或切换桌面。其他风格和真实第三方程序尚未验证。

| 文件 | SHA-256 |
| --- | --- |
| demo.exe | 6904f85d1b30633470e0837b420ea234822df573ef0a3d51202df32d6b8c40ad |
| ntw16s32.dll | 38a32f4335b5eeb342efafa6ad3f49f1548b7972474ca92ee6d25ce313e32236 |
| ntw16s64.dll | ede2ca35e245497e78da9a8aa1601dd3def5e29a305955acb089decd194d93f1 |
| ntw2ks32.dll | 50a3869ef8c1f5c1c9b5f98647e23f63bb840544e1ed76f7b195e078a37fe477 |
| ntw2ks64.dll | f207533527def291e660d00d517a99a209e97e8793bad9f64b24917051307654 |
| ntwshk32.dll | b58c21c79f3bcdebaa58c43556dd0ca5988bc7fdd40227d5c8225ae1293b6759 |
| ntwshk64.dll | b3ca175a4452eede8b0821d2fedd7755afb7f149757216f61b8bb70d589670db |
| run16s.exe | 14b00a8e4f264b105f6c8909274a58c393bfdd089bd38e0f86281cb329961ca5 |

第 12 项剩余箭头栅格化仍须核对；既有导航焦点、Win2000 DWM 恢复和现代窗口覆盖限制继续开放。
