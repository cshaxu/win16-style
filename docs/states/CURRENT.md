# Project Status

## Current Work

T1 收口已确认。用户授权准入 M0 T2“双位宽样式核心”，当前 S1 active。
本任务执行已审核心导入/新素材、本地双宽构建、真实 DLL 复验与任务审计；
不准入后续启动器、传播或 demo 产品实现。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 2
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


## M0 T2 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New；全仓下一实现 T2，从 S1 开始 |
| Admission And Approval | 用户“收口并准入下一个t 开始执行”；T1 已在 80eccb2 关闭，工作区干净 |
| Objective | 导入已审唯一 renderer，实施五头自编几何替代及 ABI 3 私有绑定，建立本地双位宽 DLL 构建 |
| Non-goals | 不导入 Detours/Hook/launcher/demo，不复制五头原像素，不发布整个产品包，不宣称透明注入 |
| Reference Baseline | 80eccb2；source-manifest 124 项/ABI 决策，固定来源 5b0b65f66c281050aa4ce1454c6cd31441a6d484 |
| Candidate Proposal | [双位宽样式核心](../proposals/style-core.md)，本次准入整个 T2 的三 S |
| Files And ABI Surface | src/ntw16s-dll、根 CMake/预设、tools 本地后台构建、tests/ntw16s-dll、导入回执、CURRENT/设计/历史；ABI 3，helper 留待启动任务 |
| Applicable Rules | 全部规则，GOAL/ARCHITECTURE/CODING/UI/ROADMAP，来源策略与 T1 独立化/回执约束 |
| Verification | 导入内容/回执/原图零复制，x86/x64 Release /MT /W4 /WX DLL 构建与版本/导出验证，治理及来源自检，diff --check |
| Expected Markers | 两宽 DLL、31 public 核心 symbol、ABI 3、无私有 bind 导出/服务依赖，源码可独立构建 |
| Asset Needs | build/.ninja/x86 与 x64 为可复用图；build/t2-core 为唯一 scratch/log/result/capture，后台拥有 PID，单次构建/测试 180 秒/500 MB，任务末晋升证据并回收 |
| Reporting Requirements | 来源/素材处置、双宽结果、回执及差异、失败处置、产品与测试边界，不引用旧项目通过数字 |
| Stop Conditions | 来源不符、受限字节被复制、需要第二 renderer/兄弟依赖或目标扩大时，先修复当前边界 |
| Exit Criteria | 已审核心和自编素材落地、本地双宽 DLL 构建与ABI检查通过，推送并实际复核后关闭 S1 |
| Original Owner Request | 收口并准入下一个t 开始执行 |
| Similar-Issue Sweep | 源/目标路径和声明、五资源消费者、ABI 版本/绑定可见性、所有本地 include，逐项回执；不吞原生行为失败 |
