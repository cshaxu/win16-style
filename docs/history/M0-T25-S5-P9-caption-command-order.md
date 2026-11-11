# M0 T25 S5 P9：标题按钮命令顺序

原版只读参考 `ntuser/kernel/dwp.c` 的 `xxxDWP_NCMouse` 在标题按钮跟踪结束后同步发送系统命令；搜索 `rg -n 'WM_SYSCOMMAND|button_input' src/lib-win32/win2k-window`，`frame.c` 的 `button_input` 是标题按钮唯一命令发送路径。此前使用投递消息，释放处理返回时应用尚未收到命令。现改为同步发送，维持既有禁用检查、状态所有权和按钮路径；生产代码 1 增 1 删，测试 15 增，包清单 2 增。

修复前发布版双位宽探针均报告 `captured=1 before=0 after=1`，修复后发布版均为 `captured=1 before=1 after=1`。先用纯 C17、MinGW GCC 16.1.0、Ninja、ccache 4.13.6 双位宽干净构建，同批八文件发布至 assets 根目录与 `O:/winnt`，再对正式发布版分别运行双位宽按钮顺序、最大化/还原/最小化/关闭、拖离取消、帮助和禁用命令共十项，全部通过；Win16 双位宽 ABI 和四种位宽启动载荷组合通过。它们使用自有窗口和程序消息，不证明真实键鼠。完整包测试含用户禁止的隔离桌面，未运行；真实 Alt+Space/Escape、其他 DPI 和用户视觉验收仍未完成。S5、T25 保持开放。

源码快照清单 SHA-256 `264fb38c298dd28a4e174386101c404fec6795a38e69d71f3abf5e4e8974fbc6`，候选 PE 架构、版本及导入核验通过；原资源与来源检查通过。两处发布文件与候选摘要一致：

| 文件 | SHA-256 |
| --- | --- |
| demo.exe | 12d31d5611e682a0971c435661f443cfecc178c7a2b0efb42bbfd5c8f2e8553c |
| ntw16s32.dll | 0eb4c29e42c27a12a6fe3f7c5e51186d618af82971d74fcf4997a5b36a06cdb9 |
| ntw16s64.dll | 2272afb4a52e5216aee55167a50f8cf38c1ebf9040cfebf2d9d520b8949a1864 |
| ntw2ks32.dll | ecf4fa8f26735b7f76ecaea29f2cc396a11ec47cc3e4243957cb3dcc5275d3a7 |
| ntw2ks64.dll | 4050b235a504be3c07c0d32068c71fdd06406c1df7d852a007c1a40f4005d7b0 |
| ntwshk32.dll | 0b91a3f1c3dfd7248e5f3616c0178e8332b0ebf833600cd83341a446bdb43f54 |
| ntwshk64.dll | 9d213bd226fc08bbf6e28efdca10623a3e8ff1c0db991a43d02b58e1b4961c2e |
| run16s.exe | 9a020a6550cf450000819e4a439f14a9cb2af22014423ae450ee5193a7230b0e |
