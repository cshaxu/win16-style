# T26 S1 顺序第 8 项：打印剪裁快照

打印消息跨调用保存 DC 剪裁区的捕获、应用和释放由 `lib-style/common/clip.h` 提供；Win16 颜色和日期控件的既有路径直接使用。调用方仍负责选择何时捕获及绘制，快照持有区域直到释放，捕获失败、没有应用剪裁、已有区域分别保留原行为。其他风格尚未接入此能力。

双位宽 MinGW GCC、Ninja、ccache 干净构建及公共库依赖检查通过；先发布至 assets 和 O:\winnt，再分别用正式启动器执行双位宽颜色下拉框和日期控件夹具，剪裁、DC 状态、重复打印资源、异常展开和清理通过。双位宽公共剪裁测试覆盖有区域、无区域及无效 DC；测试使用直接工作入口，没有创建或切换桌面。真实第三方程序和其他风格尚未验证。

| 文件 | SHA-256 |
| --- | --- |
| demo.exe | 4add541d52d83e85f9369c678238a175803dca8b53210bd4d0617bb1fe01ea05 |
| ntw16s32.dll | 9a13f528720a5fc9a2e72c1d6b4fafa22b597128d2d84e07c280cea4125da96f |
| ntw16s64.dll | 1837bb42250cccd4ccb98bc9b463ac24c5a9004d8023ba40480e28e67ea5c602 |
| ntw2ks32.dll | 3463d39aa088b6d8e785bcf4e3fc298a47212e2c8d86a82920ac65deacdbd3d7 |
| ntw2ks64.dll | 9ae9afbe3b862a006b0257be9d781f55f4ec83a92168b53dcfcd426dbba444cf |
| ntwshk32.dll | 2ec7411f67ad461f1561062c4823e34bb29c949ecd91cbc5e20de8ab3eede7b2 |
| ntwshk64.dll | 224d2f1d4aaa5ed9c0e47f45287f18892f8baad68d2b949e2cc454126d537e21 |
| run16s.exe | 6462750b81d6a741ba941d462ce6627236b0cee5310251b204a2876efa324ee9 |

本项不代表 `attachment_guard.c` 全部审计通过；既有导航焦点、Win2000 DWM 恢复与现代窗口覆盖限制继续开放。
