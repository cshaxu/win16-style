# Ordered Candidates

这是 T 级任务队列：每一行是可独立准入、交付和关闭的 T 候选，各自再分 S。
全部属于 M0。以下候选待准入，未分配数值 T；顺序号不属于 MTSP 标识。
CURRENT 是唯一活动工作权威；本文件拥有顺序，各 proposal 拥有本 T 的 S 分解。
[导入总览](../proposals/native-style-import.md)保留共同来源边界和跨任务验收矩阵。
已完成的[来源冻结与独立化设计](../proposals/source-baseline.md)不在候选表中，证据见其完成记录。
已完成的[双位宽样式核心](../proposals/style-core.md)不在候选表中，证据见其完成审计。

| 顺序 | T 候选 | 前置完成条件 | 核心交付/任务级退出条件 |
| --- | --- | --- | --- |
| 1 | [子孙传播与创建语义](../proposals/descendant-propagation.md) | 首次注入任务关闭 | 四种父子位宽边及第三代；A/W、调用参数、挂起与回滚语义；无 broker/服务 |
| 2 | [纯原生 demo 与启动对照](../proposals/native-demo.md) | 启动与传播任务关闭 | x64 demo.exe 裸启动原生、经 run16s 启动 Win16；不链接/调用样式 API |
| 3 | [独立包与 M0 产品验收](../proposals/package-acceptance.md) | 前置任务关闭且已知失败有处置 | 四文件同批构建、PE/哈希/依赖核验、完整矩阵与真实交互、可重现说明及跨任务审计 |

[启动器与首次自动附着](../proposals/launcher-injection.md)已准入当前任务，当前活动包见 CURRENT。
下一候选为子孙传播与创建语义；用户已授权按完成前置后的顺序自动准入。
后续项依赖前项交付，不能跨过失败项
直接发布；不增加额外 milestone、产品组件或产品文件。
