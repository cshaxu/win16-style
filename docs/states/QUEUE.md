# Ordered Candidates

这是 T 级任务队列：每一行是可独立准入、交付和关闭的 T 候选，各自再分 S。
全部属于 M0。以下候选待准入，未分配数值 T；顺序号不属于 MTSP 标识。
CURRENT 是唯一活动工作权威；本文件拥有顺序，各 proposal 拥有本 T 的 S 分解。
[导入总览](../proposals/native-style-import.md)保留共同来源边界和跨任务验收矩阵。
已完成的[来源冻结与独立化设计](../proposals/source-baseline.md)不在候选表中，证据见其完成记录。
已完成的[双位宽样式核心](../proposals/style-core.md)不在候选表中，证据见其完成审计。

| 顺序 | T 候选 | 前置完成条件 | 核心交付/任务级退出条件 |
| --- | --- | --- | --- |

[启动器与首次自动附着](../proposals/launcher-injection.md)已关闭；
[子孙传播与创建语义](../proposals/descendant-propagation.md)已关闭。
[纯原生demo与启动对照](../proposals/native-demo.md)已关闭。
[独立包与M0产品验收](../proposals/package-acceptance.md)已关闭，证据见
CURRENT中的完成审计。待准入候选及顺序见上表。
[菜单取消修复](../proposals/menu-cancellation.md)已关闭，实际交付及复核见CURRENT。
后续项依赖前项交付，不能跨过失败项
直接发布；不增加额外 milestone、产品组件或产品文件。

[字体与布局兼容](../proposals/font-layout.md)本轮修复交付及任务按用户明确指令收口，状态由CURRENT管理。

[英文README与真实截图展示](../proposals/readme-showcase.md)为已交付收口的治理文档工作，
不是新增产品候选；状态仍由CURRENT管理。

[VDM传播诊断](../proposals/vdm-propagation.md)及其T9本地修复/原生cmd测试已交付收口，状态由CURRENT管理。

[构建与测试效率](../proposals/build-test-efficiency.md)已完成Td S9工具修复并收口，证据见其完成记录，状态由CURRENT管理。

[UWP/XAML接入](../proposals/xaml-support.md)已按用户请求取消，无产品支持发布。

[UAC提权与样式注入](../proposals/uac-elevation.md)已按用户请求准入，编号及范围由CURRENT管理。
