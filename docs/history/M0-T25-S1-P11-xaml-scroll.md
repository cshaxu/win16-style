# M0 T25 S1 P11：WinUI ScrollBar 阶段包

为默认来源的 WinUI ScrollBar 增加横竖方向的 Win16 风格轨道、箭头和滑块
模板；应用现有本地样式的控件不被覆盖。S1 和 T25 保持开放。

GCC、Ninja、ccache 双位宽干净构建及来源、资源、ABI、包检查通过；同批
四文件13项聚焦实包回归通过。自有 x64 对象验证横竖范围、滑块几何组合
变化、来源保护及恢复；标准启动的自有记事本验证动态滚动条自动接入和
会话清理。仅改变视口的200ms观察中系统默认模板及候选均未调整滑块，原因
未明。真实箭头、拖动、滚轮、像素、其他运行时及 x86 WinUI 未验证，不能
视为交互或用户验收通过。

四文件 SHA-256：

- run16s.exe `f726ab0b7b7caf4ccafdd1ebb9e9f318e4148b7683df44676983459376670619`
- ntw16s32.dll `5c5f8ae28a1563b6fe00b41ff98826b94d353a0d707df0ec24df694f1e77205e`
- ntw16s64.dll `c43863387796b750615db1952b9c62fb25e25ec8549b2eef9adf24817e5ab3dc`
- demo.exe `49804422988ebff1a6b9041bd06a3d22baa288fbaaef715eb6ae197a7f35d3df`

同批四文件已发布 assets 根目录及 O:/winnt，两个位置逐项核对摘要一致。
