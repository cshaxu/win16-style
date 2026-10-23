# Project Status

## Current Work

M0 T4已关闭；自动准入M0 T5“纯原生demo与启动对照”，当前S2 active。
用户授权按队列自动完成全部剩余T；通过独立审计后推进，不重复询问准入。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 5
- Last allocated Td S: 3
- Last delivery: M0 T5 S1 P1 (9d94cdf)

## Technical Baseline

固定来源5b0b65f66c281050aa4ce1454c6cd31441a6d484，124处置/294 includes、91回执。
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

## M0 T5 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation；S1 9d94cdf已推送并复核 |
| Admission And Approval | 用户授权自动完成预定T，S1已关闭，无新增范围 |
| Objective | 独立observer验证同一demo裸/真实CLI注入的模块、六类控件/标题/数据/通知及外观截图 |
| Non-goals | 不在demo加入观测API或开关，不把私有桌面消息驱动当物理输入验收 |
| Reference Baseline | 9d94cdf，91回执，M0 T5四产物；demo无样式依赖，双宽ABI通过 |
| Candidate Proposal | [原生demo](../proposals/native-demo.md)S2 |
| Files And ABI Surface | tests/demo-exe外部observer、根注册/回执和历史/状态；必要窄修复；无public ABI变更 |
| Applicable Rules | 执行/架构/编码/文档规则、UI与来源策略 |
| Verification | 同一exe两路；实际模块路径/字体/WM_PRINT外观差异；六类数据、按钮通知、菜单重置、滚动和系统最小/最大/恢复 |
| Expected Markers | native无样式模块、injected只有同包64DLL；相同标题和初始业务数据；实际通知改变状态；截图明确native/injected |
| Asset Needs | build/t5-demo/s2-*自有私有桌面/job/截图，hidden worker180秒500MB；T末统一回收 |
| Reporting Requirements | 同一文件路径/哈希、真实外观证据与消息驱动边界、失败处置和实际差异统计 |
| Stop Conditions | 半成功零退出、控件数据/消息被外观改变、外部观察依赖demo后门先修复 |
| Exit Criteria | 两路对照及边界通过，提交推送实际复核，进入S3回归与T级审计 |
| Original Owner Request | 收口并准入下一个t 开始执行；自动完成所有t预定内容 |
| Similar-Issue Sweep | 六类控件及原生通知、窗口/菜单所有权、实际模块/字体/画面；外部observer只接触自有树 |
