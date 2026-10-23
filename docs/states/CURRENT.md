# Project Status

## Current Work

M0 T1“来源冻结与独立化设计”三个 S 已交付，任务级审计完成并关闭。无活动任务包。
已具备本地来源/依赖/资源处置、独立化设计、双宽工具链及来源校验工具；
不准入样式核心及后续产品实现。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 1
- Last allocated Td S: 3
- Last delivery: M0 T1 S3 P2 (closure record; P1 d10aeb9)

## Technical Baseline

T1 交付 e26b444 / 9cf7183 / d10aeb9；固定 ntvdm64 来源仍为
5b0b65f66c281050aa4ce1454c6cd31441a6d484。124 个逐文件处置（105 seeds +
19 排除边界）、294 include、34 ABI 声明/33 symbol（31 public + 2 private），
新 ABI 3 和 helper ordinal 1 为已选设计。五原图头不复制，后续核心任务自编替代。
MSVC/SDK C17/C++17 /MT 双宽工具链探针各 1/1 通过；默认本地清单校验及
23 负例、固定源研究对照、治理 13 负例通过。两 scratch 已回收，build 为空。
当前没有产品源码/产物，不声称 renderer、Detours 注入或真实 UI 已验证。

## Recent Closure

[M0 T1 完成审计](../history/M0-T1-completion.md)：三 S 与六单位目标均满足。
产品任务仍待准入；下一候选为双位宽样式核心，不自动执行后续 T。

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
