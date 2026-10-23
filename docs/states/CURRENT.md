# Project Status

## Current Work

M0 T3已完成实际交付/全任务复核并关闭；自动准入M0 T4“子孙传播与创建语义”，当前S3 active。
用户授权自动按队列完成全部剩余 T；每项通过独立审计后继续下一项，不重复询问准入。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 4
- Last allocated Td S: 3
- Last delivery: M0 T4 S2 P1 (17720ae)

## Technical Baseline

固定来源 5b0b65f66c281050aa4ce1454c6cd31441a6d484；124处置/294 includes。
T3 S1 2cc5d4d 已交付 x64 CLI、独立载荷/双宽安装器；70回执及两宽挂起暂存验证。
S3 当前构建为79回执、唯一renderer、五自编几何头及私有Detours；MSVC C17/
必要C++17、Release /MT /W4 /WX。两个DLL为ABI3、31公开名ordinal2..32，
真实NONAME helper占ordinal1，版本M0 T3。无载荷加载被动，有载荷自动窗口附着。
实际CLI两宽root/动态控件/新GUI线程及八对话框、六消息框矩阵已验证；最终
完整回归x86 16/16、x64 21/21，失败回滚/helper10000ms/stdio及退出码均通过。来源23/治理13负例通过，构建/运行不读取sibling。

尚未实现子孙传播和产品demo，未发布完整四文件包。私有桌面自动验证不等于
物理输入、任意DPI/字体环境或任意应用兼容性通过，后续产品候选继续接收。

## Recent Closure

[M0 T3完成审计](../history/M0-T3-completion.md)：S1 2cc5d4d、S2 fc12e3c、
S3 P1 97a777d已推送并复核；最终16/21完整回归，全部失败有处置，scratch已回收。

[M0 T2 完成审计](../history/M0-T2-completion.md)：六单位覆盖、三 S 实际交付与
失败处置、双宽 PE/资源/来源核验通过。S3 P1 527f40c 已推送并实际复核。
本任务 scratch、CTest 日志及研究字节码已回收，build 只保留 .ninja。

[M0 T1 完成审计](../history/M0-T1-completion.md)：来源、资源、独立化设计及本地
校验已关闭于 80eccb2，其计划状态由当前基线取代。

[M0 Td S3 层级纠正](../history/M0-Td-S3-task-level.md)：T 级候选各自拥有 S，
队列不预分配 T 号；此前治理记录继续保留在 history。

## M0 T4 S3 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation；S2 17720ae已推送并完成实际差异复核 |
| Admission And Approval | 用户授权按队列自动执行；S1/S2关闭，无剩余阻塞 |
| Objective | 核心/首次启动/传播完整回归，有限T4全集审计、依赖和scratch收口 |
| Non-goals | 不制作demo或发布最终四文件，不声称物理输入/所有API变体验收 |
| Reference Baseline | 17720ae；87回执，版本M0 T4，ABI3；64创建向量及四故障回滚通过 |
| Candidate Proposal | [传播候选](../proposals/descendant-propagation.md)S3 |
| Files And ABI Surface | 当前状态/历史与必要生产修复；ABI不变 |
| Applicable Rules | Task Reading Set、执行/架构/编码/文档规则、来源和独立化设计 |
| Verification | x86/x64全部core/launch/propagate；PE/导出/imports/版本/哈希；来源23及治理13负例、glyph/check；实际Git差异审计 |
| Expected Markers | 两宽全部测试有最终成功状态；逐单位真实证据和限制；依赖仅本地/SDK；scratch回收 |
| Asset Needs | build/t4-propagate/s3-full，owned hidden worker600秒500MB；T4结束回收整个t4-propagate和两宽Testing，保留.ninja |
| Reporting Requirements | 六单位覆盖/各S实际提交、失败处置、两宽结果、产物哈希/统计和未覆盖面 |
| Stop Conditions | 任一回归失败/资源残留/依赖越界先定位修复，不跳过失败发布 |
| Exit Criteria | T4逐项审计通过、已推送实际复核，随后关闭并准入demo候选 |
| Original Owner Request | 收口并准入下一个t 开始执行；自动准入完成所有t任务预定内容 |
| Similar-Issue Sweep | A/W事务所有分支和字段、四位宽边及三代、安装/挂起/资源所有权、首次启动与被动DLL回归 |
