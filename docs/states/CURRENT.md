# Project Status

## Current Work

M0 T5已关闭；自动准入M0 T6“独立包与M0产品验收”，当前S1 verified locally：同包9项通过，四文件已本地发布，待提交推送及复核。
用户授权按队列自动完成全部剩余T；通过独立审计后推进，不重复询问准入。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 6
- Last allocated Td S: 4
- Last delivery: M0 Td S4 P2 (df8727e)

## Technical Baseline

当前本地已验证M0 T6/0.6.0.0：纯C17/MinGW-w64 GCC16.1.0双独立工具链、Ninja、
ccache4.13.6，静态GCC/线程支持库；当前src/tests无C++，不使用MSVC或MSYS运行库。
三组件四产物、唯一renderer/五自编几何头；SDK PE staging/有界OS helper、私有
MinHook/HDE初始补丁及原生异常清理，ABI3/31公开名称ordinal2..32/NONAME helper1。
固定ntvdm64来源124处置/294 includes、118当前回执；MinHook固定14项独立清单。
新干净副本完成双宽全部产品/fixture构建、ABI/PE/声明/依赖门禁。默认样式+传播策略3。

实际同包9/9通过，四文件前后哈希不变，已复制到assets/binary且再次匹配；
[发布清单](../etc/release.json)绑定121输入摘要及逐文件SHA。该批同文件demo两路
模块/外观/业务、双宽首次附着、四位宽边A/W三代/调用者挂起、长路径和失败路径通过。
首轮GCC完整矩阵的失败已逐项修复并focused复验，不能代替最终快照完整矩阵。
完整矩阵、剪贴板77环境处置、真实输入/DPI及M0审计仍待S2/S3；本地发布不等于M0通过。
私有桌面、合成消息、截图不证明物理输入或任意应用/字体/DPI兼容性。

用户要求每产品S构建同批四文件、对该包测试、发布、提交推送及实际复核；不再
仅以源码推送收口。历史MSVC/Detours及未发布旧S保留历史事实，不作为新基线证据。
S1验证细节与所有失败处置见[工作记录](../history/M0-T6-S1-package.md)。

## Recent Closure

[M0 Td S4规则同步](../history/M0-Td-S4-delivery-policy.md)：3fdac64已推送并实际复核；
每产品S发布同批四文件、哈希绑定测试包，纯C/GCC目标生效。T6 S1恢复活动，尚未交付。

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

## M0 T6 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New；前五个T已审计关闭，分配T6，从S1开始 |
| Admission And Approval | 用户授权自动完成全部T；T5 a4e5504已实际推送复核，无前置阻塞 |
| Objective | 完成纯C17/MinGW-w64 GCC+Ninja+ccache迁移及独立四文件包；替换当前C++/MSVC依赖并保持既有契约，再按每S完整产物闭环交付 |
| Non-goals | 不把旧MSVC候选称为新工具链通过，不把私有桌面当物理验收；开发S产物标明阶段与待验收面，M0最终关闭仍有独立条件；不修改sibling/系统配置 |
| Reference Baseline | a4e5504，93回执，M0 T5，16/26全量；产品三组件四文件已实现 |
| Candidate Proposal | [独立包与M0验收](../proposals/package-acceptance.md)S1；共同八类矩阵 |
| Files And ABI Surface | 纯C注入/拦截与GCC构建/ccache/打包、工具链及来源治理同步、四文件版本/回执；新增MinHook v1.3.4/c3fcafdc固定清单及声明，限初始装载的无等待私有事务；纯C同宽SDK导入表更新/头恢复，跨宽OS rundll32与有界目标载荷交接，核对PID/创建时间并共用10秒截止；ABI3/三组件四文件保持 |
| Applicable Rules | Task Reading Set全部规则、来源资源策略、UI及三组件四产物契约 |
| Verification | 干净复制树双宽全构建、来源/版本/PE/imports/导出/哈希/依赖；剩余测试契约有限审计及必要故障补验；S2完整矩阵 |
| Expected Markers | 候选目录恰四文件，同批0.6.0.0；全路径来源有证据；实际输入只来自复制树/MinGW-w64 GCC工具链/Windows系统接口，不依赖原树/sibling |
| Asset Needs | build/t6-package下preflight、clean-source、candidate/日志/对照；hidden owned分阶段worker各600秒、总1800秒1GB；T结束回收，必要用户验收候选精确保留 |
| Reporting Requirements | 原始目标/八矩阵/来源逐项收口，干净重建与本机可达性证明边界，四文件哈希/声明/限制及真实输入待办 |
| Stop Conditions | 依赖越界/混包/缺测试或来源角色不清先修复；必要物理验收待定保留状态 |
| Exit Criteria | 独立同批四文件包及可复现步骤已验证、发布、提交推送复核，进入S2完整自动及真实交互；最终S3另做M0全任务审计 |
| Original Owner Request | 自动完成全部T并推送；新增要求纯MSYS/MinGW GCC+Ninja+ccache，不用C++/MSVC；每S编译发布测试提交推送 |
| Similar-Issue Sweep | 所有四文件/version/PE/依赖；旧测试逐项角色，SaveDC/绘制资源/菜单转发/剪贴板及长路径，避免用仅名称/PID代替真实产品证明 |
