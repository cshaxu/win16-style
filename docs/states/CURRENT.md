# Project Status

## Current Work

M0 T3已完成实际交付/全任务复核并关闭；自动准入M0 T4“子孙传播与创建语义”，当前S2 active。
用户授权自动按队列完成全部剩余 T；每项通过独立审计后继续下一项，不重复询问准入。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 4
- Last allocated Td S: 3
- Last delivery: M0 T4 S1 P1 (bd619ae)

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

## M0 T4 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation；T4 S1已推送复核，进入S2 |
| Admission And Approval | 用户授权全部剩余T；S1 bd619ae真实A/W事务及八条三代路径已审查，无阻塞 |
| Objective | 完整创建字段/挂起/环境/继承及STARTUPINFO/EX语义；安装失败只回收新目标，父/既有兄弟继续正常 |
| Non-goals | 不制作demo/发布包，不声称未覆盖扩展API或物理输入验收通过 |
| Reference Baseline | bd619ae；82回执，四边A/W三代基本验证12.67秒通过；版本M0 T4、ABI3 |
| Candidate Proposal | [传播候选](../proposals/descendant-propagation.md)S2，继承T1包/事务设计 |
| Files And ABI Surface | tests/integration合同/回滚夹具、根注册、必要窄修复、回执/状态/历史；无新public ABI |
| Applicable Rules | 全部执行/架构/编码/文档规则、产品设计及来源/独立化约束 |
| Verification | 四位宽边原生/注入对照；A/W显式/空application、Unicode/引用/空参数、ANSI/Unicode环境及cwd、继承/白名单句柄、STARTUPINFO/EX/安全属性/优先级、原挂起/错误；故障目标无入口且已退出，父/兄弟窗口保持 |
| Expected Markers | 子程序读取实际OS状态与业务字段而非仅PID；原生对照支持相同oracle；回滚job事件/存活兄弟/父继续；无误恢复原请求挂起 |
| Asset Needs | build/.ninja两宽稳定树；build/t4-propagate/s2-*私有桌面/包/日志/标记，hidden owned worker600秒500MB；任务末回收及Testing日志 |
| Reporting Requirements | 有限8种字段向量和4边/对照、真实故障和拥有者、未覆盖变体、失败处置及实际统计 |
| Stop Conditions | 原生字段不一致、半安装目标执行、句柄/挂起丢失、父兄弟被影响先定位修复，不调低正确oracle |
| Exit Criteria | 语义/回滚有限全集通过，提交推送并实际复核后进入S3完整回归和T级审计 |
| Original Owner Request | 请你自动准入 完成所有t任务预定内容 毕竟ntvdm64都已经实现过了 |
| Similar-Issue Sweep | 全部A/W字段、EX/继承和原挂起组合、每个返回/异常分支，借用/创建者边界、无环境策略后门 |
