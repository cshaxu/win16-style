# Ordered Candidates

以下六项属于同一项“独立原生样式导入”候选，按顺序执行，计划在准入后映射为
一个实现任务的六个 S。当前均为待准入，未分配数值 T；顺序号不属于 MTSP 标识。
CURRENT 是唯一活动工作权威。本队列拥有顺序，链接的 proposal 拥有详细工作包。

| 顺序 | 候选工作包 | 前置完成条件 | 核心交付/验收 |
| --- | --- | --- | --- |
| 1 | [来源冻结与独立化设计](../proposals/native-style-import.md#source-baseline) | 现有治理与导入准备已交付 | 逐文件去留/资源处置、依赖与 ABI 清单、初始化/跨位宽方案、工具链探测记录 |
| 2 | [双位宽样式核心](../proposals/native-style-import.md#style-core) | 来源与独立化方案可执行 | 本地 ntw16s32/64.dll；真实 DLL 附着、绘制、移除与线程/资源生命周期复验 |
| 3 | [启动器与首次自动附着](../proposals/native-style-import.md#launcher-injection) | 核心双位宽复验通过 | x64 run16s.exe 启动 x86/x64 目标；目标无需样式调用即可附着；安装失败不放行 |
| 4 | [子孙传播与创建语义](../proposals/native-style-import.md#descendant-propagation) | 首次注入与自动附着成立 | 四种父子位宽边及第三代；A/W、调用参数、挂起与回滚语义；无 broker/服务 |
| 5 | [纯原生 demo 与启动对照](../proposals/native-style-import.md#native-demo) | 启动和传播契约通过 | x64 demo.exe 裸启动原生、经 run16s 启动 Win16；不链接/调用样式 API |
| 6 | [独立包验收与任务关闭审计](../proposals/native-style-import.md#package-acceptance) | 前五项交付且已知失败有处置 | 四文件同批构建、PE/哈希/依赖核验、完整矩阵与真实交互、可重现说明及全任务审计 |

下一项：来源冻结与独立化设计。当前只已具备候选来源的 105 条 Git 指纹，
逐文件导入处置与工具链能力尚未验证。后续项依赖前项交付，不能跨过失败项
直接发布；不增加额外 milestone、产品组件或产品文件。
