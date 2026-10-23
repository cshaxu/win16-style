# Project Status

## Current Work

M0 T3“启动器与首次自动附着”已准入，当前 S1 active。
用户授权自动按队列完成全部剩余 T；每项通过独立审计后继续下一项，不重复询问准入。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 3
- Last allocated Td S: 3
- Last delivery: M0 T2 S3 P2 (本次关闭记录)

## Technical Baseline

固定来源 5b0b65f66c281050aa4ce1454c6cd31441a6d484；124 处置、294 include。
本地 53 回执、唯一 renderer 的 17 C、五个项目自编几何头；MSVC C17 Release
/MT /W4 /WX 双宽图 build/.ninja/x86、x64。实际 ntw16s32.dll/ntw16s64.dll
ABI 3、31 public、ordinal 2..32，1 预留，私有 binding 不导出，版本 M0 T2。
S2 6243b15 的完整核心回归两宽各 13/13；来源 23/治理 13 负例及素材一致检查通过。

当前 DLL 为显式同线程 API 核心；DllMain 被动，没有载荷/helper/自动注入/传播。
启动器和产品 demo 尚未实现，也没有完整四文件发布包。打印图与私有桌面回归
不等于物理输入、任意 DPI/字体环境或任意应用兼容性通过。后续任务接收范围见
完成审计和候选。默认校验/构建/运行不读取 sibling。

## Recent Closure

[M0 T2 完成审计](../history/M0-T2-completion.md)：六单位覆盖、三 S 实际交付与
失败处置、双宽 PE/资源/来源核验通过。S3 P1 527f40c 已推送并实际复核。
本任务 scratch、CTest 日志及研究字节码已回收，build 只保留 .ninja。

[M0 T1 完成审计](../history/M0-T1-completion.md)：来源、资源、独立化设计及本地
校验已关闭于 80eccb2，其计划状态由当前基线取代。

[M0 Td S3 层级纠正](../history/M0-Td-S3-task-level.md)：T 级候选各自拥有 S，
队列不预分配 T 号；此前治理记录继续保留在 history。

## M0 T3 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New；T2 关闭后分配 T3，从 S1 开始 |
| Admission And Approval | 用户明确授权自动准入并完成全部预定 T；当前只激活 T3 S1 |
| Objective | 建立 x64 CLI、目标位宽选择、唯一挂起目标安装事务，导入已审 Detours 私有实现 |
| Non-goals | 本 S 不宣称自动窗口附着通过，不提前发布完整包；传播由下一 T 接收 |
| Reference Baseline | e29b3e4；T2 双宽 ABI3 核心、53 回执；固定来源 5b0b65f66c281050aa4ce1454c6cd31441a6d484 |
| Candidate Proposal | [启动器与首次自动附着](../proposals/launcher-injection.md)；T1 INDEPENDENCE 已选事务/载荷/10秒预算 |
| Files And ABI Surface | run16s-exe、新增 DLL 私有 injection/Detours、构建/测试/tools、回执/状态/历史；helper ordinal1 在 S2 落地 |
| Applicable Rules | 执行/架构/编码/文档规则与设计、来源策略、固定来源/ABI/独立化约束 |
| Verification | 明确 CLI 引用/Unicode/wait/错误契约；真实挂起目标两宽 PE/安装暂存、载荷边界、失败清理，源码/治理/exit/diff门禁 |
| Expected Markers | run16s x64 与双宽 installer 编译；借用目标只安装不终止，创建者只恢复本层挂起或回滚本层目标；无 broker/第五产物 |
| Asset Needs | build/.ninja/x86/x64 稳定图；build/t3-launch scratch 日志/测试包/结果，owned hidden worker 单轮600秒/500MB；任务末回收及 Testing 日志 |
| Reporting Requirements | 导入差异/声明、CLI契约、真实暂存与目标初始化区别、失败/限制及实际提交统计 |
| Stop Conditions | 目标提前执行、位宽误判、loader-lock禁用操作、无限等待或源依赖先修复；不以模块加载冒充附着 |
| Exit Criteria | 本地双宽 installer 和 CLI 有限契约验证通过，推送并实际复核后进入 S2 引导/自动附着 |
| Original Owner Request | 请你自动准入 完成所有t任务预定内容 毕竟ntvdm64都已经实现过了 |
| Similar-Issue Sweep | 全部A/W创建入口、两宽选择、路径编码/长度、helper等待及终止清理、载荷边界、句柄所有者 |
