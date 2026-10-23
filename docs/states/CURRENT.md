# Project Status

## Current Work

T1 收口已确认。用户授权准入 M0 T2“双位宽样式核心”，S1 已交付并复核关闭，当前 S2 active。
本任务执行已审核心导入/新素材、本地双宽构建、真实 DLL 复验与任务审计；
不准入后续启动器、传播或 demo 产品实现。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 2
- Last allocated Td S: 3
- Last delivery: M0 T2 S1 P1 (20671cd)

## Technical Baseline

T1 已关闭，固定来源为 5b0b65f66c281050aa4ce1454c6cd31441a6d484；124 处置、
294 include 和 ABI 决策继续有效。T2 S1 20671cd 已交付唯一 renderer、五自编
几何头、本地 MSVC C17 /MT /W4 /WX 双宽构建；真实 DLL ABI 3、31 public、
无 private binding exports。DllMain 被动，注入/传播/launcher/demo 尚未实现。
S1 43 回执、两架构 ABI 各 1/1；S2 新增真实窗口回归，尚待完整结果与交付。
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

## M0 T2 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation；T2 下一 S2 |
| Admission And Approval | 整个 T2 获准执行；S1 20671cd 已推送并实际提交复核，无阻塞 |
| Objective | 通过真实 DLL 验证附着、frame/menu/六类控件、字体/线程/资源销毁与重入，观察自编素材呈现 |
| Non-goals | 不实现透明注入/进程传播，不制作产品 demo，不把私有桌面 Print 结果当真实桌面物理交互通过 |
| Reference Baseline | 20671cd；双宽 ABI 3/31 public、43 回执，原像素零复制 |
| Candidate Proposal | [已准入样式核心](../proposals/style-core.md)的 S2；T1 支持面与限制有效 |
| Files And ABI Surface | tests/ntw16s-dll、根测试注册、回执/导入记录、必要核心窄修复、CURRENT/history；不扩公开 ABI |
| Applicable Rules | 全部执行/架构/编码/文档规则与设计、来源策略 |
| Verification | 两宽真实 DLL 的 frame/menu/六控件、creation scope、wrong thread、destroy/reentrancy、font lease/GDI资源、移除卸载；源已审 focused regression；打印图人工观察 |
| Expected Markers | 明确完整测试名与逐项结果；两宽图像可观察；原生文本/通知/选择保持、资源不泄漏；不支持面有处置 |
| Asset Needs | 稳定 build/.ninja/x86/x64；build/t2-core/s2-* 日志/图片 scratch，隐藏 owned worker、360 秒/500 MB；任务末只回收该 root |
| Reporting Requirements | 运行与视觉证据、实际DLL与纯数学unit区别、失败/限制，不引用源历史通过数字 |
| Stop Conditions | 资源泄漏、原生状态破坏、断言失败先定位修复，不修改正确oracle掩盖失败；越界目标转后续候选 |
| Exit Criteria | 有限支持面与资源/线程/重入路径复验通过、图像检查及全部失败处置，提交推送并复核 |
| Original Owner Request | 收口并准入下一个t 开始执行 |
| Similar-Issue Sweep | 同类控件原生状态/清理/调用者DC保持、所有glyph状态和两宽一致；修复点有永久生产路径回归 |
