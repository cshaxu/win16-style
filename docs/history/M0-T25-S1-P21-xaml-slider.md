# M0 T25 S1 P21：WinUI Slider 阶段包

为默认样式来源的 WinUI Slider 增加 Win16 风格模板，保留横竖 Thumb、
轨道、刻度和 Header 绑定；复用既有发现、样式来源保护和恢复机制。
S1/T25 保持开放。

GCC、Ninja、ccache 双位宽干净构建和 13 项同批聚焦实包回归通过；同一
快照自有 x64 WinUI 横竖对象验证数值变化、Thumb 偏移、右下刻度部件
可见、程序化焦点和禁用状态及停止后来源恢复。标准启动自有记事本中
动态 Slider 自动接入，最后窗口关闭后会话记录清空。

此前竖直对象测试有一次回调超时，单独重试通过；真实鼠标拖动、键盘、
刻度像素、不同 DPI、反向方向及 x86 WinUI 未验证，不视为用户验收。

四文件 SHA-256：

- run16s.exe `657cc05dc1c4b0c4e8a3b79dbbcaf51e8746dea8791f8753ea7452856e1b87b9`
- ntw16s32.dll `13799ba56ad731da4a89c1d8bcacebfaaa2ad9ef3d7d24d86150e496c1646958`
- ntw16s64.dll `77d406f1e199be349151d682c96caf5ff4c2579fcbff0a17fddd2e2b1de69457`
- demo.exe `6214f8042d0b09d995dd2b6a34dd29afb6b77e5b25cbbe3f791b8b233eb8b008`

同批四文件已发布 assets 根目录和 O:/winnt，摘要与测试包一致。
