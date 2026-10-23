# Project Status

## Current Work

M0 T5已关闭；自动准入M0 T6“独立包与M0产品验收”，S1保留未交付现场，短暂转入M0 Td S4同步用户新增规则。
用户授权按队列自动完成全部剩余T；通过独立审计后推进，不重复询问准入。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 6
- Last allocated Td S: 4
- Last delivery: M0 T5 S3 P1 (a4e5504)

## Technical Baseline

固定来源5b0b65f66c281050aa4ce1454c6cd31441a6d484，124处置/294 includes、93回执。
本地唯一renderer/五自编几何头/私有Detours；MSVC C17及必要C++17、/MT /W4 /WX。
x64 run16s及x86/x64 ntw16sDLL已实现首次自动附着和原生A/W子孙传播，默认策略3。
ABI3、31公开名称ordinal2..32及NONAME helper1；构建版本M0 T5，0.5.0.0。
四位宽边/三代24应用、64创建字段原生/注入对照、四安装故障回滚均通过；完整
回归x86 16/16、x64 26/26，target-first-exit/10秒helper和句柄回收另通过。
来源23/治理13负例、五几何头、PE及本地/SDK依赖核验通过；T4/T5 scratch已回收。
x64纯原生demo已在9d94cdf交付，imports仅USER32/KERNEL32；同文件两路渲染/业务已通过。
四文件完整发布和真实交互尚待T6验证。私有桌面证据不代表真实输入、任意DPI/字体或任意应用兼容性。

## Owner Toolchain And Delivery Update

用户本轮要求目标为纯C17/MinGW-w64 GCC、Ninja、ccache，产品不依赖C++或MSVC。
当前源仍有Detours C++及SEH，旧MSVC候选只作过渡事实，T6 S1尚未交付/关闭。
本机两套独立x86/x64 GCC纯C探针均编译运行成功；不能用x64 GCC -m32代替x86工具链。
每个后续S须构建实际四文件、对同批包测试、发布产物、提交推送及实际复核；
不再仅以源码推送收口。相关规则由治理交付同步，用户新要求立即生效，历史事实不改写。

## Recent Closure

[M0 T5完成审计](../history/M0-T5-completion.md)：9d94cdf、bd0dd65、a4e5504已推送
复核，双宽16/26全量及同文件对照通过，93回执，scratch已回收。

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

## M0 Td S4 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Governance；用户要求每S编译发布测试提交推送，并明确纯C/GCC目标 |
| Admission And Approval | 用户本轮直接授权规则纠正；暂存T6 S1现场，治理交付后恢复，不并行活动S |
| Objective | 将每产品S同批四文件闭环和纯C/GCC工具链写入当前规则 |
| Non-goals | 不交付或宣称旧MSVC包符合新目标；不改写过去S的发布事实；不改产品构建输入 |
| Reference Baseline | 60bb624；T6 S1未提交修改保留其所有权 |
| Candidate Proposal | 直接用户规则纠正；不另建产品T候选 |
| Files And ABI Surface | EXECUTION/CODING规则、源码布局、CONTRIBUTING、包说明、CURRENT及本次history；无ABI或产品输入变更 |
| Applicable Rules | Task Reading Set、DOCUMENT、EXECUTION；规则修改由本Td承担 |
| Verification | 治理正向及13负例、git diff --check、选定路径diff及提交/远端复核 |
| Expected Markers | 每产品S编译打包测试发布提交推送复核；哈希绑定被测包；明确阶段验收；旧基线待迁移 |
| Asset Needs | build/t6-package/governance-resume-current.md保存T6恢复包，属于T6已登记scratch，恢复后删除 |
| Reporting Requirements | 治理仅文档不冒充产品发布；历史缺包如实保留；产品及工具未提交现场不得夹带 |
| Stop Conditions | 若选定提交夹带产品输入或旧包则先修正，不关闭T6 S1 |
| Exit Criteria | 本Td推送后实际复核，记录治理关闭并恢复T6 S1 |
| Original Owner Request | 每一次S任务应该编译发布测试提交推送；纯msys gcc+ninja+ccache，不使用C++和MSVC |
| Similar-Issue Sweep | 现行执行规则、贡献入口、源码布局、包说明及状态基线；历史记录保留 |
