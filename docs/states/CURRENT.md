# Project Status

## Current Work

M0 T4已关闭；自动准入M0 T5“纯原生demo与启动对照”，当前S3 active。
用户授权按队列自动完成全部剩余T；通过独立审计后推进，不重复询问准入。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 5
- Last allocated Td S: 3
- Last delivery: M0 T5 S2 P1 (bd0dd65)

## Technical Baseline

固定来源5b0b65f66c281050aa4ce1454c6cd31441a6d484，124处置/294 includes、93回执。
本地唯一renderer/五自编几何头/私有Detours；MSVC C17及必要C++17、/MT /W4 /WX。
x64 run16s及x86/x64 ntw16sDLL已实现首次自动附着和原生A/W子孙传播，默认策略3。
ABI3、31公开名称ordinal2..32及NONAME helper1；构建版本M0 T5，0.5.0.0。
四位宽边/三代24应用、64创建字段原生/注入对照、四安装故障回滚均通过；完整
回归x86 16/16、x64 24/24，target-first-exit/10秒helper和句柄回收另通过。
来源23/治理13负例、五几何头、PE及本地/SDK依赖核验通过；T4 scratch已回收。
x64纯原生demo已在9d94cdf交付，imports仅USER32/KERNEL32；双行为与四文件包尚待验证。私有桌面证据不代表真实输入、任意DPI/字体或任意应用兼容性。

## Recent Closure

[M0 T4完成审计](../history/M0-T4-completion.md)：S1 bd619ae、S2 17720ae、
S3 f0eabb2均已推送复核，16/24全量及target-first-exit补测通过，scratch已回收。

[M0 T3完成审计](../history/M0-T3-completion.md)：S1 2cc5d4d、S2 fc12e3c、
S3 P1 97a777d已推送并复核；最终16/21完整回归，全部失败有处置，scratch已回收。

[M0 T2 完成审计](../history/M0-T2-completion.md)：六单位覆盖、三 S 实际交付与
失败处置、双宽 PE/资源/来源核验通过。S3 P1 527f40c 已推送并实际复核。
本任务 scratch、CTest 日志及研究字节码已回收，build 只保留 .ninja。

[M0 T1 完成审计](../history/M0-T1-completion.md)：来源、资源、独立化设计及本地
校验已关闭于 80eccb2，其计划状态由当前基线取代。

[M0 Td S3 层级纠正](../history/M0-Td-S3-task-level.md)：T 级候选各自拥有 S，
队列不预分配 T 号；此前治理记录继续保留在 history。

## M0 T5 S3 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation；S2 bd0dd65已推送复核关闭 |
| Admission And Approval | 用户自动完成全部T授权，S1/S2无任务内遗留 |
| Objective | demo源码/import边界、已有全回归及T5逐项覆盖审计、scratch收口 |
| Non-goals | 不发布最终包、不声称物理输入或任意DPI/字体通过 |
| Reference Baseline | bd0dd65，M0 T5；同文件裸/注入六类及业务对照通过，92回执 |
| Candidate Proposal | [原生demo](../proposals/native-demo.md)S3 |
| Files And ABI Surface | tests/demo-exe边界检查、根注册、来源回执、状态/历史及当前说明；ABI不变 |
| Applicable Rules | 全部执行/架构/编码/文档规则、来源与UI设计 |
| Verification | core/launch/propagate/demo完整注册集；demo PE/导入/源码、四文件版本/哈希；来源23/治理13/几何/diff门禁 |
| Expected Markers | 两宽实际全部执行成功；demo同文件业务/渲染独立观察仍通过；有限全T单位有证据和后续接收者 |
| Asset Needs | build/t5-demo/s3-full，hidden owned worker600秒500MB；关闭T前回收t5-demo及两宽Testing，保留.ninja |
| Reporting Requirements | 冻结五单位、实际各S提交及完整回归、失败处置、统计/清理与后续物理验收接收者 |
| Stop Conditions | 任何边界/业务/回归失败先修复，禁止跳过失败发布或混包 |
| Exit Criteria | 全T审计通过并推送实际复核，关闭T5，自动准入独立包候选 |
| Original Owner Request | 收口并准入下一个t 开始执行；自动完成所有t预定内容 |
| Similar-Issue Sweep | demo全部生产/链接/运行模块边界，六类业务，原生/注入、四架构边及失败所有权回归 |
