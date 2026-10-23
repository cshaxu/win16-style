# Project Status

## Current Work

M0 T3“启动器与首次自动附着”已准入，当前 S2 active。
用户授权自动按队列完成全部剩余 T；每项通过独立审计后继续下一项，不重复询问准入。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 3
- Last allocated Td S: 3
- Last delivery: M0 T3 S1 P1 (2cc5d4d)

## Technical Baseline

固定来源 5b0b65f66c281050aa4ce1454c6cd31441a6d484；124处置/294 includes。
T3 S1 2cc5d4d 已交付 x64 CLI、独立载荷/双宽安装器；70回执及两宽挂起暂存验证。
S2 当前构建为77回执、唯一renderer、五自编几何头及私有Detours；MSVC C17/
必要C++17、Release /MT /W4 /WX。两个DLL为ABI3、31公开名ordinal2..32，
真实NONAME helper占ordinal1，版本M0 T3。无载荷加载被动，有载荷自动窗口附着。
实际CLI两宽root/动态控件/新GUI线程及八对话框、六消息框矩阵已验证；完整核心
复验/安装失败回滚在S3。来源23/治理13负例通过，构建/运行不读取sibling。

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

## M0 T3 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation；T3 S1 已交付复核，进入 S2 |
| Admission And Approval | 用户授权全部剩余 T 自动顺序准入；S1 2cc5d4d 已推送、实际 installer/CLI diff 复核 |
| Objective | 合并 DLL 引导/helper ordinal1、最小 Detours 地址事务及 loader-lock 外唯一 renderer 绑定，实际双宽自动窗口附着 |
| Non-goals | 不启用子孙进程传播，不制作产品 demo，不把私有桌面测试宣称物理交互完成 |
| Reference Baseline | 2cc5d4d；70回执，双宽安装暂存各2/2，x64 CLI诊断通过；T2核心 ABI3 |
| Candidate Proposal | [启动器与首次自动附着](../proposals/launcher-injection.md) S2 与 INDEPENDENCE 三种加载情形 |
| Files And ABI Surface | DLL entry/window hooks/private binding、.def/helper ordinal1、CMake/tools与双宽原生目标夹具、回执/状态/历史；31公开名不变 |
| Applicable Rules | 执行/架构/编码/文档规则、UI、来源/ABI/引导设计；静态CRT/TLS保留通知 |
| Verification | 两宽被动LoadLibrary仍不自动hook；x64CLI注入x86/x64、真实入口与窗口/动态控件/新GUI线程/对话框；无源依赖；完整核心回归在S3 |
| Expected Markers | 真实同位宽DLL、窗口root/children附着、原生文本/消息/参数保持；无载荷被动、helper仅安装，初始化失败不继续目标 |
| Asset Needs | build/.ninja/x86/x64稳定树；build/t3-launch/s2-* 私有桌面/log/测试包，hidden owned worker单轮600秒/500MB；任务末回收及Testing日志 |
| Reporting Requirements | 自动附着与暂存区别、真实生产路径、两宽结果/失败/有限支持面、loader-lock操作清单、实际统计 |
| Stop Conditions | loader-lock GUI/等待/LoadLibrary、半初始化目标入口运行、第二renderer或参数/状态破坏先修复 |
| Exit Criteria | 双宽自动附着矩阵与被动模式通过，提交推送并实际复核后进入S3失败/全回归审计 |
| Original Owner Request | 请你自动准入 完成所有t任务预定内容 毕竟ntvdm64都已经实现过了 |
| Similar-Issue Sweep | A/W Window/Dialog/MessageBox入口、owner线程、私有scroll/theme原函数、helper模式/无载荷/非法载荷、全部公开名/ordinal |
