# Project Status

## Current Work

M0 T1“来源冻结与独立化设计”已由用户准入，当前 S1 active。
本任务包含来源/依赖/资源处置、独立化设计与工具链验证、本地来源校验工具；
不准入样式核心及后续产品实现。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 1
- Last allocated Td S: 3
- Last delivery: M0 Td S3 P2 (closure record; P1 3897192)

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

## M0 T1 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New；首次实现 T1，S1 |
| Admission And Approval | 用户明确“开始执行T1”，准入队首来源冻结与独立化设计的整个任务 |
| Objective | 固定提交逐文件去留、声明、资源处置及依赖闭包，交付可核验本地清单 |
| Non-goals | 不导入 renderer/Hook 产品字节、不发布二进制、不修改兄弟仓库、不准入后续 T |
| Reference Baseline | 本仓库 98a6459；ntvdm64 5b0b65f66c281050aa4ce1454c6cd31441a6d484；候选 inventory 105 条 |
| Candidate Proposal | [来源冻结与独立化设计](../proposals/source-baseline.md)，已准入；原范围及三 S 分解有效 |
| Files And ABI Surface | docs/etc/import 的清单/研究证据、CURRENT、QUEUE、proposal、history；无产品 ABI 修改 |
| Applicable Rules | 全部执行/文档/架构/编码规则、产品 design 和来源策略 |
| Verification | 固定 Git tree/blob 与清单逐项核对、include 闭包与资源使用者核对、原声明检查；治理正向/13 负例及 diff --check |
| Expected Markers | 所有冻结条目有处置与接收者；未处置资源零复制；依赖边逐项分类 |
| Asset Needs | 仅元数据与研究；build/t1-source-scan 为本任务元数据扫描 scratch（30 分钟/50 MB），不写来源源码/原图字节；T1 执行者回收 |
| Reporting Requirements | 具体文件数、分类/依赖/资源结论、局限、后续设计接收者及提交验证 |
| Stop Conditions | 来源缺失/不符、需要改产品目标或第五产品、未决资源处置阻止相关复制；记录问题并继续独立研究 |
| Exit Criteria | 原候选及依赖发现有逐文件处置、资源处理方案和声明接收者，验证推送并实际复核后关闭 S1 |
| Original Owner Request | 开始执行T1 |
| Similar-Issue Sweep | 全固定来源扫描 include、pragma/link、原图形 header 引用、服务/RPC/个人路径，所有命中有排除/改造/保留接收者 |
