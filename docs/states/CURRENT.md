# Project Status

## Current Work

M0 T1“来源冻结与独立化设计”已由用户准入，S1/S2 已交付复核关闭，当前 S3 active。
本任务包含来源/依赖/资源处置、独立化设计与工具链验证、本地来源校验工具；
不准入样式核心及后续产品实现。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 1
- Last allocated Td S: 3
- Last delivery: M0 T1 S2 P1 (9cf7183)

## Technical Baseline

远端初始基线 4666cd745da53bf16067365032f4a22a09a12e58 仅有 MIT LICENSE。
P1 bd013545d5fd81423c20154c8ba39811c579981e 已推送并核对远端 main。
只新增治理、检查和来源元数据。没有产品源码、构建或运行验收。
参考 softpc 889b309aabb24d3b4748dcc209ec520a3c35c375 的治理拓扑；
导入候选固定 ntvdm64 5b0b65f66c281050aa4ce1454c6cd31441a6d484。
其工作区有无关未提交修改，本项目未复制其工作区内容。

## Recent Closure

[M0 Td S3 层级纠正](../history/M0-Td-S3-task-level.md)：P1 3897192 已推送并复核。
六个独立 T 候选各有 proposal、前置关系、本 T 的 S 分解和独立关闭标准。
治理检查、六候选/18 局部 S 对照通过；没有启动产品实现。

[M0 Td S2 队列细化](../history/M0-Td-S2-queue.md)：P1 b3a23a7 已推送并复核。
该交付曾将队列误划为一个实现候选的六个 S；用户本次明确应为 T 级队列，
其层级划分由本次纠正替代。历史提交保留事实，没有启动产品导入。

[M0 Td S1 交付与复核](../history/M0-Td-S1-bootstrap.md)：28 文件初始交付，
治理正向与 13 负例通过，105/105 来源记录精确匹配固定提交，diff 检查通过。
无 build scratch、产品进程或二进制；未分配 T，产品所有接受项仍未运行。
原始活动包保留在 P1 提交的本文件中，后续规则与状态以当前文档为准。

## M0 T1 S3 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation；T1 的下一 S3 |
| Admission And Approval | 整个 T1 已获用户执行授权；S2 9cf7183 已推送并实际复核 |
| Objective | 交付无兄弟仓库依赖的本地来源/ABI/导入回执校验器，负例验证与设计交接，完成 T1 级审计 |
| Non-goals | 不导入产品源码、不实现 renderer/注入、不准入后续 T、不把工具链探针当产品通过 |
| Reference Baseline | 9cf7183；124 个来源条目/294 include、34 ABI 决策、双宽工具链通过记录 |
| Candidate Proposal | [已准入 T1](../proposals/source-baseline.md)，S3 与独立 T 完成条件 |
| Files And ABI Surface | tools/verify_source_manifest.py、CI/验证入口、import 支持文档、CURRENT/proposal/history；无产品 ABI 实现 |
| Applicable Rules | 执行/编码/架构/文档规则、来源策略与独立化设计 |
| Verification | 本地元数据一致性、导入回执/路径/内容校验及负例；可选固定源码 tree/blob 研究复核；治理全部自检；diff --check；T1 有限覆盖审计 |
| Expected Markers | 本地无源仓库即可通过；篡改/不安全路径/资源原字节导入拒绝；T1 六类交付均有证明与接收者 |
| Asset Needs | 只保留 S1/S2 已登记 scratch，源复核批量读取内存；完成后只删除两精确 build 子目录，确认无运行 job/reparse/无关文件 |
| Reporting Requirements | 校验命令、结果及局限，源与导入状态区别，代码统计/状态所有者、任务完成审计及推送 |
| Stop Conditions | 发现来源/清单不符、工具错误或无证据的完成声明时先修复；需要后续产品实现则只记录接收者 |
| Exit Criteria | 本地/可选源复核及负例通过、设计交接和 T1 审计满足原目标、scratch 回收、提交推送和实际复核 |
| Original Owner Request | 开始执行T1 |
| Similar-Issue Sweep | 覆盖源/目的路径、重复条目、blob/size/SHA、五限制资源、声明、源依赖边、ABI及回执引用；所有失败有永久拒绝用例 |
