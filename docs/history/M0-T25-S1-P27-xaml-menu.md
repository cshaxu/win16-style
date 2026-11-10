# M0 T25 S1 P27：WinUI 菜单阶段包

为默认样式来源的 MenuFlyoutPresenter、MenuFlyoutItem、
ToggleMenuFlyoutItem、MenuFlyoutSubItem 和 MenuFlyoutSeparator 接入
Win16 风格模板，沿用既有发现、样式来源保护和撤销机制。
S1/T25 保持开放。

GCC、Ninja、ccache 双位宽干净构建与同批 13 项聚焦实际包回归通过。
同快照自有 x64 对象对照五项通过；标准 run16s 启动的自有记事本实际
弹出菜单中，容器与四类项目均观察到自动应用，关闭最后窗口后会话
记录清空。首轮对象回归失败由测试运行器使用空环境变量而使前一测试
开关残留造成；移除环境变量后逐项通过。真实键鼠、命令触发、屏幕
像素及 x86 WinUI 未验证，不代表全部菜单类型已支持。

四文件 SHA-256：

- run16s.exe `1445474d22acc570f8e823070d833b62fd00b3a09a2f64b04702b98f22fdf2c7`
- ntw16s32.dll `7f4c2c50ede3df06ec8cdea7767272ce794885959a2fa905442f05993d1f89b5`
- ntw16s64.dll `e0a099a3302f1e481d220dfe9407a63c9d403c0088b650813a77601f085ccfad`
- demo.exe `3c49e7ca2cc4c346cb8d9aac3605af7374a9a74d4dbce3881518e18b71e8df5a`

同批四文件已发布 assets 根目录和 O:/winnt，摘要与测试包一致。
