# Project Status

## Current Work

M0 T5已关闭；自动准入M0 T6“独立包与M0产品验收”，当前S1 active。
用户授权按队列自动完成全部剩余T；通过独立审计后推进，不重复询问准入。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 6
- Last allocated Td S: 3
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

## M0 T6 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New；前五个T已审计关闭，分配T6，从S1开始 |
| Admission And Approval | 用户授权自动完成全部T；T5 a4e5504已实际推送复核，无前置阻塞 |
| Objective | 形成干净源码副本的独立双宽构建/四文件候选包、实际PE/版本/哈希/声明及剩余来源处置 |
| Non-goals | 不把私有桌面或截图当物理验收；物理待定前不正式发布或关闭M0；不修改sibling/系统配置 |
| Reference Baseline | a4e5504，93回执，M0 T5，16/26全量；产品三组件四文件已实现 |
| Candidate Proposal | [独立包与M0验收](../proposals/package-acceptance.md)S1；共同八类矩阵 |
| Files And ABI Surface | 本地打包/PE/重建工具、版本M0 T6、源测试剩余契约适配/映射、说明/来源/状态；不新增public ABI或产品文件 |
| Applicable Rules | Task Reading Set全部规则、来源资源策略、UI及三组件四产物契约 |
| Verification | 干净复制树双宽全构建、来源/版本/PE/imports/导出/哈希/依赖；剩余测试契约有限审计及必要故障补验；S2完整矩阵 |
| Expected Markers | 候选目录恰四文件，同批0.6.0.0；全路径来源有证据；实际输入只来自复制树/MSVC/SDK，不依赖原树/sibling |
| Asset Needs | build/t6-package下preflight、clean-source、candidate/日志/对照；hidden owned分阶段worker各600秒、总1800秒1GB；T结束回收，必要用户验收候选精确保留 |
| Reporting Requirements | 原始目标/八矩阵/来源逐项收口，干净重建与本机可达性证明边界，四文件哈希/声明/限制及真实输入待办 |
| Stop Conditions | 依赖越界/混包/缺测试或来源角色不清先修复；必要物理验收待定保留状态 |
| Exit Criteria | 独立候选包及可复现步骤已验证提交推送复核，进入S2完整自动及真实交互；最终S3另做M0全任务审计 |
| Original Owner Request | 收口并准入下一个t 开始执行；自动完成所有t预定内容并推送到win16-style |
| Similar-Issue Sweep | 所有四文件/version/PE/依赖；旧测试逐项角色，SaveDC/绘制资源/菜单转发/剪贴板及长路径，避免用仅名称/PID代替真实产品证明 |
