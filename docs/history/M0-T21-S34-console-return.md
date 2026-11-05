# M0 T21 S34：控制台重绑定归回启动器

用户要求仅有少量内容时送回产品。原 console 库仅一个 stderr 重绑定函数，唯一产品调用方为 run16s；将函数体归入 `src/run16s-exe/elevation.c` 的 `run16s_rebind_console_stderr`，移除独立库、头和构建链接，保留控制台句柄所有权、失败和 CRT 重绑定行为；security 库不变。符号和链接搜索确认旧库无生产或测试引用。无新增产品或公开 ABI。

`s34-clean1` 采用 GCC 16.1.0、Ninja、ccache 4.13.6，双宽干净构建、ABI、来源与原资源检查通过。`s34-package` 同批实包 15/15 聚焦通过、无跳过，覆盖双宽重绑定、模拟 UAC、CLI、嵌套 cmd 四位宽传播和固定基线。真实 UAC、全量 UI 和完整集成矩阵未运行。

`s34-publish` 发布到 assets 根和 O:/winnt，并与测试包逐项核对：

- run16s.exe: `5b2fcda8f151aff0cd4bade3b93a4a1f052cd3c3549cf5535855cef398d1f3c8`
- ntw16s32.dll: `5611573f70f959dd2ecf6a0ceaba4093a69f0f2b8097db572e6dbcef5788183e`
- ntw16s64.dll: `9d244bb2a0c7734e942b7d7072a63911c5f27c9143bb288bad4023ab5c312798`
- demo.exe: `890d473c9998403db6a2eb3cd3c4ca6d5cf8447d8d783ab8c468fc05d2b630a5`

P1 `fae3f5e9ac3c7d27882a1ce899a0e1fffb209960` 已推送，远端树实际复核四产物、启动器源码与旧库删除；两处发布文件与测试包摘要再次核对。S34 关闭，T21 保持开放。
