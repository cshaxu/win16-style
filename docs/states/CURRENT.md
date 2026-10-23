# Project Status

## Current Work

M0 T1“来源冻结与独立化设计”已由用户准入，S1 已交付复核关闭，当前 S2 active。
本任务包含来源/依赖/资源处置、独立化设计与工具链验证、本地来源校验工具；
不准入样式核心及后续产品实现。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 1
- Last allocated Td S: 3
- Last delivery: M0 T1 S1 P1 (e26b444)

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

## M0 T1 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation；T1 的下一 S2 |
| Admission And Approval | 用户开始执行整个 T1；S1 e26b444 已推送并实际复核，继续其设计与工具链子任务 |
| Objective | 确定 API/载荷/引导/跨位宽方案，验证本机 x86/x64 MSVC/SDK/CMake/Ninja 编译链接和执行 |
| Non-goals | 不实现产品注入、不复制源 renderer/Detours 字节、不准入后续 T |
| Reference Baseline | e26b444 的 124 条清单与静态边界；固定来源 commit 不变 |
| Candidate Proposal | [已准入来源任务](../proposals/source-baseline.md)与[来源处置](../etc/import/BASELINE.md) |
| Files And ABI Surface | design/ARCHITECTURE 与 CODING 的设计细化、import/设计证据、tests/toolchain、tools/Probe-Toolchain.ps1、CURRENT/history；无实际产品 ABI |
| Applicable Rules | 执行、架构、编码、文档规则及全部相关设计、来源策略 |
| Verification | 真实 x86/x64 C17/C++ 编译、/MT DLL+EXE 链接/运行、TLS 和 SDK API；治理正向/13 负例、diff --check；初始化与 helper 源码审计 |
| Expected Markers | 两架构 CTest 全通过，明确 ABI/载荷/ordinal、初始化和 helper 失败预算，无第五产品 |
| Asset Needs | build/t1-toolchain 是唯一构建探针 scratch；后台拥有 PID、log/err/result 路径，120 秒硬超时/100 MB 预算，执行者负责停止回收；前一扫描 scratch 待任务末回收 |
| Reporting Requirements | 实际工具版本、构建架构/flags/结果与有限证明；记录源 helper 的无界清理发现及接收改造 |
| Stop Conditions | 缺编译器/SDK则记录可复现缺项；需要第五产品或违反 loader-lock 约束则不推进相关方案，其他独立工作继续 |
| Exit Criteria | 确定可执行设计与后续有限证明责任，两架构真实编译执行通过，交付推送后复核 |
| Original Owner Request | 开始执行T1 |
| Similar-Issue Sweep | 源 helper 的 INFINITE 等待、MAX_PATH/ACP 与 TLS/DisableThreadLibraryCalls 冲突逐一分类；方案不能默认继承 |
