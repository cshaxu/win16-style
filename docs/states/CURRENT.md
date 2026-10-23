# Project Status

## Current Work

M0 T3“启动器与首次自动附着”已准入，当前 S3 active。
用户授权自动按队列完成全部剩余 T；每项通过独立审计后继续下一项，不重复询问准入。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 3
- Last allocated Td S: 3
- Last delivery: M0 T3 S2 P1 (fc12e3c)

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

[M0 T2 完成审计](../history/M0-T2-completion.md)：六单位覆盖、三 S 实际交付与
失败处置、双宽 PE/资源/来源核验通过。S3 P1 527f40c 已推送并实际复核。
本任务 scratch、CTest 日志及研究字节码已回收，build 只保留 .ninja。

[M0 T1 完成审计](../history/M0-T1-completion.md)：来源、资源、独立化设计及本地
校验已关闭于 80eccb2，其计划状态由当前基线取代。

[M0 Td S3 层级纠正](../history/M0-Td-S3-task-level.md)：T 级候选各自拥有 S，
队列不预分配 T 号；此前治理记录继续保留在 history。

## M0 T3 S3 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation；S2 已推送复核关闭，进入T3 S3 |
| Admission And Approval | 全部剩余T自动顺序准入已授权；S2 fc12e3c实际20文件+629/-54、引导/测试/ABI差异已复核 |
| Objective | 覆盖CLI和安装器失败/回滚、helper有限超时、拒绝初始化不执行入口，完整双宽核心回归和T级审计 |
| Non-goals | 不扩展到传播/demo/完整产品包，不以私有桌面结果替代真实输入验收 |
| Reference Baseline | fc12e3c；77回执，31public+NONAME helper1；双宽真实CLI/窗口/八对话框/六消息框已验证 |
| Candidate Proposal | [启动器与首次自动附着](../proposals/launcher-injection.md) S3与T级退出条件 |
| Files And ABI Surface | tests/run16s-exe及必要窄修复、构建/tools、回执/当前状态/历史/设计/候选；不扩31公开接口 |
| Applicable Rules | 全部执行/架构/编码/文档规则与设计、来源/独立化方案 |
| Verification | 缺失/错宽/损坏DLL的CLI诊断与无入口；受限进程句柄拒绝；坏载荷DLL初始化拒绝；真实W helper失败/10秒超时和目标原挂起；两宽13项核心全回归与启动矩阵；PE/来源/治理/清理 |
| Expected Markers | 失败码/错误诊断明确；原目标未运行/未交付，owned helper被回收，无句柄泄漏；逐项全T目标有证据/接收者 |
| Asset Needs | build/.ninja两宽稳定树；build/t3-launch/s3-*日志/包/故障helper/marker，hidden owned worker每轮600秒500MB；结束回收整个t3-launch及Testing日志 |
| Reporting Requirements | 有限覆盖全集、实际失败与资源所有者、净行数、全部交付实际审查、未支持面与接收者、清理及远端一致 |
| Stop Conditions | 漏执行目标、错误目标被恢复、helper超时后无限等、资源或原生契约失败先修复，不越过T级退出条件 |
| Exit Criteria | T3全部目标审计/失败及完整回归通过、清理、提交推送实际复核后关闭；按授权准入下一队列项 |
| Original Owner Request | 请你自动准入 完成所有t任务预定内容 毕竟ntvdm64都已经实现过了 |
| Similar-Issue Sweep | 两架构所有安装/恢复/清理分支、A/W GUI/消息返回、载荷边界、PE/路径长度与编码、唯一trampoline和状态所有者 |
