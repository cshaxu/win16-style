# M0 T25 S4 P31：页面设置窗口外框

`PageSetupDlgA/W` 可在 COMDLG32 内创建窗口而绕过已接入的对话框入口。现复用已有模态窗口观察及异常清理范围，为符合条件的页面设置窗口附着 Win2000 外框；不改变应用参数、布局、内部控件、返回值或打印机配置。普通 `PrintDlgA/W` 未添加额外接入。

本机有默认打印机。自建私有桌面对照中，P28 发布版 x86/x64、ANSI／Unicode 页面设置 4 例均未附着，普通打印窗口 4 例已附着；候选 8 例均附着、正常取消且扩展错误为零，测试未发送打印任务。无默认打印机或无设备时，新增测试以精确标记跳过，不计通过；其他错误仍按失败处理。该检查不证明确认设置、实际打印、扩展打印 API、所有驱动或真实鼠标与原版视觉。

源码快照清单 SHA-256 `d2396a2f87d430fa8ba96848f2cadaf04734e34f3011e4b2285c5af9ad9ad88b`；纯 C17、MinGW GCC、Ninja、ccache 双位宽干净构建，同批 90 项重点回归通过、无跳过，耗时约 98 秒，非完整矩阵。发布至 assets 根目录及 `O:\winnt`，两处与测试包 SHA-256 一致：

| 文件 | SHA-256 |
| --- | --- |
| demo.exe | 261cfe1e7bad4c558885df5595cd351b7888e3020a50cf0d30e15ccd471aaa97 |
| ntw16s32.dll | 25337e2f44d93583863f010865af9f523d0b82a8a034d3da1e519f2303040291 |
| ntw16s64.dll | 0b6e98d57a9f58d14b36b1c754cad37f871dc64c3d024cc4a2390755a1d79a2d |
| ntw2ks32.dll | 9f69ea0612fb6fe636e9fde060eab5d3b11e7ada8709cfe4361583092e47d963 |
| ntw2ks64.dll | 33eb841d907e0db63554cf372ec2fb8a7189459ddae540328ec10e3dfe94820d |
| ntwshk32.dll | 99c8fa6c7eb695c5c9ffed2accf2953d73f1f8b03b67cf2b5ec39e7fd6b5b5c2 |
| ntwshk64.dll | 924d2faa8e9fa7bfa598416859780f33e00795aeb58c562813d0c04d034b9a96 |
| run16s.exe | 51c99ea3fc8285f21cefd26745fa250cb101f3cbebea8d22f7ae721484aa0acf |

扩展打印和其他公共对话框、真实桌面交互及用户视觉验收尚未据此证明；S4、T25 继续开放。
