# M0 T25 S1 P19：WinUI ToggleSwitch 阶段包

为默认样式来源的 WinUI ToggleSwitch 增加 Win16 风格开关模板，保留
SwitchThumb、Header 与开关文案绑定；沿用已有发现、来源保护和恢复机制。
S1/T25 保持开放。

GCC、Ninja、ccache 双位宽干净构建、来源与资源检查及 13 项同批聚焦实包
回归通过。自有 x64 WinUI 对象通过实际适配器验证程序化开关、文案绑定及
显示、程序化焦点状态与停止后默认来源恢复；同批正式启动器的自有记事本
动态创建开关后观察到自动接入及最后窗口关闭后的会话清理。

真实鼠标点击、拖动、键盘焦点、实际屏幕像素和 x86 WinUI 未验证，不视为
用户验收。首次尝试 put_Style(NULL) 未恢复默认来源，现使用已有的属性清除
路径；首次探针受沙箱 CIM 访问限制未执行，允许环境重跑通过。

四文件 SHA-256：

- run16s.exe `92607651639504150ba874c8e89ea8814da49f3b509ee15cdebce4df38920ff7`
- ntw16s32.dll `76c85c518ba4185b722376b34f44d01f004d5898b0b7a92b4c99722f0189ca69`
- ntw16s64.dll `0ebbaff5863ef6947decf26842df78ed5d36dde12984f6aa7908de984d5ee92a`
- demo.exe `454e0d92049c78f99c5c1ae9579571ee1a1b5e8503e53c41df42c322c7c8b31a`

同批文件已发布 assets 根目录及 O:/winnt，两处摘要与测试包一致。
