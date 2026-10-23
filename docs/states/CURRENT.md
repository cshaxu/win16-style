# Project Status

## Current Work

T1 收口已确认。用户授权准入 M0 T2“双位宽样式核心”，S1 已交付并复核关闭，当前 S3 active。
本任务执行已审核心导入/新素材、本地双宽构建、真实 DLL 复验与任务审计；
不准入后续启动器、传播或 demo 产品实现。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 2
- Last allocated Td S: 3
- Last delivery: M0 T2 S2 P1 (6243b15)

## Technical Baseline

T1 已关闭，固定来源为 5b0b65f66c281050aa4ce1454c6cd31441a6d484；124 处置、
294 include 和 ABI 决策继续有效。T2 S1 20671cd 已交付唯一 renderer、五自编
几何头、本地 MSVC C17 /MT /W4 /WX 双宽构建；真实 DLL ABI 3、31 public、
无 private binding exports。DllMain 被动，注入/传播/launcher/demo 尚未实现。
S2 6243b15：53 回执，双宽各 13/13 完整核心回归，资源/字体/重入/卸载通过；
两宽打印图一致。原生对照和纯数学检查按证据类型单列，未宣称物理输入验收。
来源/治理负例分别为 23/13；默认校验、构建与运行不依赖 sibling。

## Recent Closure

[M0 T1 完成审计](../history/M0-T1-completion.md)：三 S 与六单位目标均满足。
T1 的下一候选已准入为 T2；该历史不覆盖上方当前技术基线。

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

## M0 T2 S3 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation；T2 下一 S3 |
| Admission And Approval | 整个 T2 已获准；S2 6243b15 已推送并实际 diff 复核，无阻塞 |
| Objective | 冻结有限覆盖全集，核验完整双宽核心、来源/ABI/独立依赖、统计与卫生，完成 T2 审计 |
| Non-goals | 不准入启动器/传播/demo，不发布完整包，不把私有桌面验证升级为物理输入通过 |
| Reference Baseline | 6243b15；53 回执、17 C、5 自编头、ABI 3/31 public；s2-complete 两宽各 13/13 |
| Candidate Proposal | [样式核心](../proposals/style-core.md) S3 与 T 级退出条件 |
| Files And ABI Surface | tools PE 检查、CURRENT/QUEUE/proposal/README/history；不扩 ABI，不修改已通过 renderer |
| Applicable Rules | 全部执行/架构/编码/文档规则及设计、来源/ABI 决策 |
| Verification | 复核 S1/S2 实际提交；13 tests × 2 完整结果不重复无理由运行；53 回执及负例；实际 PE 名称/ordinal/imports/版本；Ninja dependency 闭包；素材一致；任务统计/清理/远端一致 |
| Expected Markers | 有限六单位审计逐项通过或有明确接收者；零 sibling 产品依赖、原像素零复制、31 稳定导出，无未处置任务内失败 |
| Asset Needs | 保留 build/.ninja/x86/x64 配置/对象；收口前回收 build/t2-core 全部已完成 owned scratch，及两个稳定树内 CTest 自动生成的 Testing 日志目录；500 MB，无新长任务 |
| Reporting Requirements | 每项证据和边界、失败处置、增删净行数、旧路径/状态所有者、scratch 实测及推送复核 |
| Stop Conditions | 发现遗漏或失败先修复并按影响复验；原生交互/启动集成转对应后续候选 |
| Exit Criteria | 全任务审计、来源/治理门禁及清理完成，交付推送实际复核后关闭 T2 |
| Original Owner Request | 收口并准入下一个t 开始执行 |
| Similar-Issue Sweep | 五素材消费者、全部 31 名字和 ordinal、17 生产 C 与依赖、13 注册测试、加载/清理与未支持面；不将源码历史数字冒充结果 |
