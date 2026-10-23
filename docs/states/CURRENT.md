# Project Status

## Current Work

M0 T4已关闭；自动准入M0 T5“纯原生demo与启动对照”，当前S1 active。
用户授权按队列自动完成全部剩余T；通过独立审计后推进，不重复询问准入。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 5
- Last allocated Td S: 3
- Last delivery: M0 T4 S3 P1 (f0eabb2)

## Technical Baseline

固定来源5b0b65f66c281050aa4ce1454c6cd31441a6d484，124处置/294 includes、87回执。
本地唯一renderer/五自编几何头/私有Detours；MSVC C17及必要C++17、/MT /W4 /WX。
x64 run16s及x86/x64 ntw16sDLL已实现首次自动附着和原生A/W子孙传播，默认策略3。
ABI3、31公开名称ordinal2..32及NONAME helper1；构建版本M0 T4，0.4.0.0。
四位宽边/三代24应用、64创建字段原生/注入对照、四安装故障回滚均通过；完整
回归x86 16/16、x64 24/24，target-first-exit/10秒helper和句柄回收另通过。
来源23/治理13负例、五几何头、PE及本地/SDK依赖核验通过；T4 scratch已回收。
demo与四文件完整包尚未交付。私有桌面证据不代表真实输入、任意DPI/字体或任意应用兼容性。

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

## M0 T5 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New；T4三个S已实际复核关闭，分配T5，从S1开始 |
| Admission And Approval | 用户要求收口并准入下一个T执行，且授权自动完成全部预定T |
| Objective | 将固定源示例的原生业务内容适配为x64 demo.exe，移除全部样式API及链接；本地构建 |
| Non-goals | 不复刻全部28页gallery，不加入测试开关或自绘样式，不提前发布包/声称物理验收 |
| Reference Baseline | T4 f0eabb2；87回执、完整16/24回归通过；demo来源为固定commit的main.c/gallery.c |
| Candidate Proposal | [原生demo](../proposals/native-demo.md)S1，六类标准控件和普通菜单/消息循环 |
| Files And ABI Surface | src/demo-exe main/gallery/version与说明；根x64目标、所有产物版本M0 T5和来源回执；public ABI不变 |
| Applicable Rules | Task Reading Set及全部架构/编码/文档规则、UI与来源策略 |
| Verification | 双宽构建/PE和版本；demo仅x64、静态imports及源码无样式/注入消费；focused core/abi，后续S2独立observer验收 |
| Expected Markers | 普通窗口/菜单及STATIC BUTTON EDIT LISTBOX COMBOBOX SCROLLBAR；业务通知/标准消息循环；同一exe不按参数/环境选择样式 |
| Asset Needs | build/.ninja双宽稳定树；build/t5-demo/s1-build及后续s2/s3自有fixture，hidden worker600秒500MB，T结束回收及Testing日志 |
| Reporting Requirements | 固定源逐项删改、六类内容选择、状态/资源所有者、构建/PE结果、统计与后续接收者 |
| Stop Conditions | demo依赖样式API/动态加载或测试后门、原生创建失败、构建警告先修复 |
| Exit Criteria | 原生demo与版本一致构建，来源/边界审查通过并提交推送复核，再进入S2同文件双行为 |
| Original Owner Request | 收口并准入下一个t 开始执行；自动完成所有t任务预定内容 |
| Similar-Issue Sweep | 全部demo源/构建imports，移除主动安装/字体/owner-draw/计数和测试逻辑；业务字段只由Windows和demo拥有 |
