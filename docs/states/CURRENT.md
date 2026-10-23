# Project Status

## Current Work

M0 Td S3 verified：已改为六个独立 T 候选，各自划分 S，待交付后复核。
产品实现未准入，未分配数值 T。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 0
- Last allocated Td S: 3
- Last delivery: M0 Td S2 P2 (closure record; P1 b3a23a7)

## Technical Baseline

远端初始基线 4666cd745da53bf16067365032f4a22a09a12e58 仅有 MIT LICENSE。
P1 bd013545d5fd81423c20154c8ba39811c579981e 已推送并核对远端 main。
只新增治理、检查和来源元数据。没有产品源码、构建或运行验收。
参考 softpc 889b309aabb24d3b4748dcc209ec520a3c35c375 的治理拓扑；
导入候选固定 ntvdm64 5b0b65f66c281050aa4ce1454c6cd31441a6d484。
其工作区有无关未提交修改，本项目未复制其工作区内容。

## Recent Closure

[M0 Td S2 队列细化](../history/M0-Td-S2-queue.md)：P1 b3a23a7 已推送并复核。
该交付曾将队列误划为一个实现候选的六个 S；用户本次明确应为 T 级队列，
其层级划分由本次纠正替代。历史提交保留事实，没有启动产品导入。

[M0 Td S1 交付与复核](../history/M0-Td-S1-bootstrap.md)：28 文件初始交付，
治理正向与 13 负例通过，105/105 来源记录精确匹配固定提交，diff 检查通过。
无 build scratch、产品进程或二进制；未分配 T，产品所有接受项仍未运行。
原始活动包保留在 P1 提交的本文件中，后续规则与状态以当前文档为准。

## M0 Td S3 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Governance；用户纠正后的下一治理 S |
| Admission And Approval | 用户明确“不是t任务队列吗?”，授权纠正任务层级 |
| Objective | 六项成为独立 T 候选，逐项独立 proposal、S 分解和 T 完成条件 |
| Non-goals | 不准入实现、不预占 T、不改变产品目标、来源和测试范围 |
| Reference Baseline | d45ea01；无产品源码和产物 |
| Candidate Proposal | [导入总览](../proposals/native-style-import.md) |
| Files And ABI Surface | QUEUE、proposals、CURRENT、治理历史；无代码/ABI 变化 |
| Applicable Rules | EXECUTION、DOCUMENT、ROADMAP、来源策略及现有产品架构 |
| Verification | 治理正向/13 负例、六独立 proposal 和局部 S 分解检查、旧跨 T 的 S 引用搜索、diff --check |
| Expected Markers | 六 T 候选，分别从本 T 的 S1 开始；Last allocated T 仍为 0 |
| Asset Needs | 无 |
| Reporting Requirements | 明确承认层级错误，展示修正后的 T 队列与各自 S；报告交付 |
| Stop Conditions | 需要产品实现或产品目标变更时停止该越界步骤 |
| Exit Criteria | 六独立候选有依赖/交付/任务级退出标准，验证、推送及实际复核完成 |
| Original Owner Request | 不是t任务队列吗? |
| Similar-Issue Sweep | 搜索当前权威中单 T 六 S 假设及跨任务 S 编号，全部改为候选名称；不可变历史保留并注明已替代 |
