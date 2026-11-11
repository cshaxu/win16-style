# M0 T25 S4 P34：运行时新增标题栏

已有外框接入仅处理创建时带 `WS_CAPTION` 的窗口。双位宽发布包对照中，先创建无标题顶层窗口、再设置标题样式的自建窗口均未附着，亦未绘制经典标题。现在无标题顶层窗口暂时观察自身样式变化，出现标题后移除观察并调用现有附着器；原窗口过程继续接收消息，销毁时移除观察。窗口所有线程、子窗口排除及原有附着器条件保持不变。

使用纯 C17、MinGW GCC 16.1.0、Ninja 和 ccache 双位宽干净构建；源码快照清单 SHA-256 `acee8ce62a9e8e6cc3bcbaacb20c5fd258473a4aef13c094e908cc9c65aa0824`。同批包的私有桌面重点回归串行 54 项通过、无跳过，包含双位宽新增标题、已有标题的移除与恢复、窗口、菜单、对话框及四种双位宽传播组合。首次并发回归的 x86 颜色／字体对话框在第四个窗口等待时超时；单独复测及后续串行回归通过，原因未确认，不抹去该失败。

已发布到 assets 根目录与 `O:/winnt`，八文件与测试包摘要一致：

| 文件 | SHA-256 |
| --- | --- |
| demo.exe | 26cc7815d22ee549c885b18a38ad37fa6e1b25478c23fc71971ce9ea36e18423 |
| ntw16s32.dll | 8efb99081424aafef95f75b29a6cc4a547ced7d1b9c459caa96a1e61defd9ac2 |
| ntw16s64.dll | 7f9800315278d4c4551c253b6dc57a598ede466a53fec86169898c5bb9fc26ec |
| ntw2ks32.dll | 69c61afc01702a6eab887262a419b0035a1d4cc6a69a7f42b918fe5acad6af3f |
| ntw2ks64.dll | e93eb0173f2f82fa8d781cf960c6e0fcd56bfe5afbc6be000d2bb72d7b5c8805 |
| ntwshk32.dll | a4b88df4316914512b6593fb5255d80ccfbbf9ec903484592043a8bef192d207 |
| ntwshk64.dll | 0f58eb5b22151eddc1eb7e24da6fcf8dcf23460e88219c6e7f38acb5a6dfe5cd |
| run16s.exe | 5d3dbd1d7a93d5c34d4b08c1a42ba275cdd66e861a1d1c0e6192ea867aca4675 |

修改范围为外框附着器、测试程序和包测试登记；没有第二套外框绘制。未验证实际程序动态切换、全部窗口样式、物理输入或原版逐像素外观；不是完整矩阵或用户视觉验收。S4 与 T25 继续开放。
