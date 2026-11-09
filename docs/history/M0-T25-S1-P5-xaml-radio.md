# M0 T25 S1 P5：WinUI RadioButton 阶段包

在已有 Button、CheckBox 路径上增加默认来源的 RadioButton 模板，保留控件
本身的选中、分组和命令语义；已有 Local 样式不会被覆盖。此为阶段交付，
S1 和 T25 保持开放。

GCC、Ninja、ccache 双宽干净构建及来源、资源、ABI 和包检查通过；13项
聚焦实包回归通过、无跳过。同批包自有 x64 记事本中动态 RadioButton 接入、
原 Button/CheckBox 双窗口恢复对照通过；同快照夹具验证单项选中标记、同组
互斥、Local 样式保护与来源恢复。真实键鼠、像素、不同 DPI、其他版本和
x86 WinUI 运行未覆盖，不能视为全部 WinUI/UWP 控件支持或用户验收通过。

四文件 SHA-256：

- run16s.exe `c75516d9a92c2063ce0582e0c3d48fab1d29c7f40595958b5d1b5da2e94b12f6`
- ntw16s32.dll `3f76b86990d4747fc357440a186e8f4cdf99ad98d105986e74c54aad161aaf62`
- ntw16s64.dll `0f63c2d0500d1c1b23e7a3b1068d722ccf8539f5d9d50ffa22f87389ae228681`
- demo.exe `c416222224bd0ee21eb833a07b504e8301d797ef9b445ad250577df73a89e04c`

同批文件已发布 assets 根目录及 O:/winnt 并核对摘要。源码统计：5个文件，
增加40行、删除2行、净增38行；不含测试和构建文件。
