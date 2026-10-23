# Project Status

## Current Work

M0 T3已完成实际交付/全任务复核并关闭；自动准入M0 T4“子孙传播与创建语义”，当前S1 active。
用户授权自动按队列完成全部剩余 T；每项通过独立审计后继续下一项，不重复询问准入。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 4
- Last allocated Td S: 3
- Last delivery: M0 T3 S3 P2 (本次关闭/下一准入记录)

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

## M0 T4 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New；T3已关闭，队首传播任务分配T4并从S1开始 |
| Admission And Approval | 用户授权自动完成全部剩余T；T3 97a777d已推送并实际全任务复核，无阻塞 |
| Objective | 适配固定来源原生CreateProcess A/W拦截，继承最小目录/策略，每代复用同一安装器；CLI默认启用传播 |
| Non-goals | 不加入broker/VDM重定向/服务身份，不实现demo/正式完整包；复杂语义和全回归分别在本T S2/S3 |
| Reference Baseline | 97a777d及T3关闭审计；79回执、ABI3/31public+helper1、双宽首次附着和13核心回归通过 |
| Candidate Proposal | [子孙传播与创建语义](../proposals/descendant-propagation.md) S1与T1 INDEPENDENCE |
| Files And ABI Surface | injection/create_process.cpp、entry/intercept/context、版本、根构建、双宽传播fixture及回执/状态/历史；不扩公开ABI |
| Applicable Rules | 全部执行/架构/编码/文档规则与设计、来源及payload/安装所有权设计 |
| Verification | 原生A/W参数原样转发，真实新子进程暂存及原挂起；四种父子位宽边及第三代基本生产路径；原请求失败错误；来源/治理/PE门禁 |
| Expected Markers | 每代同位宽DLL及真实窗口附着，原生业务数据/返回保持；安装只借用，创建层只抵消自己加的挂起，失败不交付半安装目标 |
| Asset Needs | build/.ninja两宽稳定树；build/t4-propagate/s1-*日志/目录包/marker，hidden owned worker600秒500MB；任务末回收及Testing日志 |
| Reporting Requirements | 原生字段与局部递归防护、源删除/保留清单、每代实际附着/错误/挂起证据、失败及限制、实际增删统计 |
| Stop Conditions | 参数改写、调用者挂起丢失、重复安装、跨宽错误或影响既有进程先修复，不将核心语义失败转发布 |
| Exit Criteria | A/W传播实现及基本四边/第三代验证通过，推送并实际复核后进入S2完整语义/回滚 |
| Original Owner Request | 请你自动准入 完成所有t任务预定内容 毕竟ntvdm64都已经实现过了 |
| Similar-Issue Sweep | 两入口原函数所有者、递归/异常退出、每种flags/environment/startup及句柄所有权、helper仅新目标、全部4边 |
