# Project Status

## Current Work

M0 T5已关闭；自动准入M0 T6“独立包与M0产品验收”，S1已发布、推送并实际复核关闭；当前S2 active：最终快照完整自动矩阵及同包13项已验证，真实交互待用户实测。
用户授权按队列自动完成全部剩余T；通过独立审计后推进，不重复询问准入。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 6
- Last allocated Td S: 4
- Last delivery: M0 T6 S2 P2 (P1 b044345推送后复核记录)

## Technical Baseline

当前已发布并推送M0 T6/0.6.0.0：纯C17/MinGW-w64 GCC16.1.0双独立工具链、Ninja、
ccache4.13.6，静态GCC/线程支持库；当前src/tests无C++，不使用MSVC或MSYS运行库。
三组件四产物、唯一renderer/五自编几何头；SDK PE staging/有界OS helper、私有
MinHook/HDE初始补丁及原生异常清理，ABI3/31公开名称ordinal2..32/NONAME helper1。
固定ntvdm64来源124处置/294 includes、118当前回执；MinHook固定14项独立清单。
新干净副本完成双宽全部产品/fixture构建、ABI/PE/声明/依赖门禁。默认样式+传播策略3。

S2新干净快照最终构建完整矩阵：x86注册24，23通过/1剪贴板跳过；x64注册35，
34通过/1剪贴板跳过；0失败。原生CreateWindowStation拒绝访问5，77不计通过。
完整矩阵发生重建后重新取齐四文件，实际同包13/13通过，前后哈希不变；已发布
assets/binary，身份与[发布清单](../etc/release.json)匹配。121输入与快照及工作区一致。
S2 P1 b044345已推送并实际复核远端四文件/121输入及原始用例；自动部分已验证。
真实鼠标/键盘/焦点/捕获及选定DPI、剪贴板仍待实际操作。
私有桌面、合成消息、截图不证明物理输入或任意应用/字体/DPI兼容性。
S2不关闭，S3全任务审计尚未准入，M0未完成。记录见[S2验收](../history/M0-T6-S2-acceptance.md)。

用户要求每产品S构建同批四文件、对该包测试、发布、提交推送及实际复核；不再
仅以源码推送收口。历史MSVC/Detours及未发布旧S保留历史事实，不作为新基线证据。
S1验证细节与所有失败处置见[工作记录](../history/M0-T6-S1-package.md)。

## Recent Closure

[M0 T6 S1独立包](../history/M0-T6-S1-package.md)：1bc0440已推送并读取实际提交复核；
远端四文件SHA及121输入身份通过，同包9/9通过。S1关闭，自动准入S2。

[M0 Td S4规则同步](../history/M0-Td-S4-delivery-policy.md)：3fdac64已推送并实际复核；
每产品S发布同批四文件、哈希绑定测试包，纯C/GCC目标生效。T6 S1按此门槛完成交付，历史规则变更记录保留。

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

## M0 T6 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation；T6 S1已关闭，顺序准入S2 |
| Admission And Approval | 用户授权自动完成全部T；S1 1bc0440源码及四文件推送、实际复核通过 |
| Objective | 在本S新干净快照重建同批四文件，完成双宽完整自动矩阵、同包扩展集成及必要真实交互验收 |
| Non-goals | 不把历史矩阵、私有桌面或合成输入冒充本批完整或物理验收；不修改sibling/系统配置 |
| Reference Baseline | 1bc0440；纯C17/GCC/Ninja/ccache，ABI3，四文件0.6.0.0，同包9/9 |
| Candidate Proposal | [独立包与M0验收](../proposals/package-acceptance.md)S2及共同八类矩阵 |
| Files And ABI Surface | 三组件四文件及现有ABI保持；测试、必要缺陷修复、版本/来源回执、发布清单及交互证据 |
| Applicable Rules | Task Reading Set全部规则；每S四产物闭环、来源资源策略及UI契约 |
| Verification | 新干净副本双宽构建，顺序完整core/launch/propagate/demo矩阵；实际候选四文件扩展同包测试；来源/PE/哈希/声明/依赖；真实输入及选定DPI |
| Expected Markers | 包恰四文件，所有注册用例逐项有执行或明确处置；发布文件与被测包哈希相同；物理验收另记实际操作与环境 |
| Asset Needs | build/t6-package/s2-final及s2-package-test；新快照内部完整矩阵scratch；hidden owned worker，每架构600秒、全矩阵1200秒，同包1800秒，总scratch1GB；T结束回收 |
| Reporting Requirements | 成功/失败/跳过/未覆盖分别报告；剪贴板77不计通过；原生与注入同文件对照；工具链、源码输入、四文件SHA及发布位置 |
| Stop Conditions | 混包、隐含依赖、必要用例缺失先修复；必要物理验收待用户时保留状态，不关闭S或M0 |
| Exit Criteria | 完整自动矩阵与必要物理验收有证据，同批四文件发布、提交推送并实际复核；之后准入S3全任务审计 |
| Original Owner Request | 自动完成全部T并推送；纯C/MinGW GCC+Ninja+ccache；每S编译、发布、测试、提交推送 |
| Similar-Issue Sweep | 双宽完整注册集与实际执行对照；创建字段/回滚/对话框/消息框/资源生命周期/长路径；剪贴板环境权限及物理交互边界 |
