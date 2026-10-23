# Project Status

## Current Work

M0 T6 S4已完成滚动条修复、原生探针、双发布及远端复核，closed；自动准入S5。
本S执行T6与跨任务M0完成审计，并按每S规范重建/同包测试/双发布/推送复核。
用户指定探针自行验收；隔离clipboard权限限制明示，不计通过，M0尚未完成。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 6
- Last allocated Td S: 5
- Last delivery: M0 T6 S4 P2 (P1 418be5e双发布及远端复核，关闭S4准入S5)

## Technical Baseline

本轮M0 T6/0.6.0.0：纯C17、MinGW-w64 GCC16.1.0双工具链、Ninja、ccache4.13.6；
静态GCC/线程支持，不使用C++/MSVC/MSYS运行库。三组件四产物，ABI3，31公开名称
ordinal2..32及NONAME helper1；SDK staging/有界OS helper、私有MinHook/HDE后端。

用户否决旧五资源自绘替代；现已从固定ntvdm64提交逐字节恢复五头及原resources.md，
Git暂存blob与来源一致，121回执。旧生成器删除；独立摘要门禁拒绝替换，两个DLL
各含18个原图形表及RCDATA5完整来源说明。124处置/294 includes及MinHook14项保持。

当前s4-probe-final新干净双宽全构建通过，完整矩阵x86 23通过/1跳过，x64 35通过/
1跳过，0失败；注册/执行24/36集合相同。同包10/10，四文件前后SHA一致，已发布
assets根目录及O:/winnt/system32，备份旧四文件且核验一致，release.json绑定124输入。
S4修复demo滚动条28->17逻辑像素及适配器按横截面保持端钮/滑块正方形；横纵
17/28/35/42几何双宽通过，旧实现负例在28失败。真实same-demo原生探针验证Tab焦点、
捕获、业务/菜单/About及窗口操作；GetDpiForWindow=144/150%，滚动条厚26物理像素。

用户已确认原资源与ntvdm64外观一致，并指定不用computer-use、自己用探针做；
不再要求整套人工操作。探针不冒称物理键鼠。隔离剪贴板77不计通过，Windows拒绝
CreateWindowStation错误5（四种access mask均失败），共享剪贴板未触碰，记录宿主限制。
启动器仍Console；S3默认等待语义本包六场景复验通过。S2原交接项由S4按用户最新
验收方式接收；S4唯一活动，最后审计为S5。[S4证据](../history/M0-T6-S4-acceptance.md)。

用户要求每产品S构建同批四文件、对该包测试、发布、提交推送及实际复核；不再
仅以源码推送收口。历史MSVC/Detours及未发布旧S保留历史事实，不作为新基线证据。
S1验证细节与所有失败处置见[工作记录](../history/M0-T6-S1-package.md)。

## Recent Closure

[M0 T6 S4修复与探针](../history/M0-T6-S4-acceptance.md)：418be5e远端124输入及四文件复核，
58通过/2权限跳过、同包10/10，assets和O:/winnt/system32一致。S4关闭，准入S5。

[M0 T6 S3等待契约](../history/M0-T6-S3-wait.md)：4b83994已推送且实际复核，
新包14/14及相关回归6/9通过，124输入与四远端文件身份一致。S3关闭，准入S4。

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

## M0 T6 S5 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation；S4已关闭，最后T6及跨任务M0覆盖审计 |
| Admission And Approval | 用户授权完成后续S；每S编译/测试，binary change双发布assets根目录及O:/winnt/system32；自己用探针，不用computer-use |
| Objective | 冻结原目标到T1..T6/S交付、生产所有者、证据及限制的映射；复核独立三组件四文件、原资源、纯C/GCC和最终部署，决定有界M0完成 |
| Non-goals | 不扩展产品/ABI/资源，不修改兄弟仓库，不把权限跳过或探针当作物理输入；无源码变更时不重复无理由的全矩阵 |
| Reference Baseline | S4 P1 418be5e；124输入、原资源六blob及18表、ABI3；完整58通过/2权限跳过、同包10/10、双发布并远端复核 |
| Candidate Proposal | [独立包与M0验收](../proposals/package-acceptance.md)S5；前五任务闭环记录及S1..S4实际交付 |
| Files And ABI Surface | 审计/状态/发布清单及本S新四文件；预期124个产品/测试输入与S4相同，无公开ABI或源代码变更 |
| Applicable Rules | 完整Task Reading Set、T级全覆盖审计、每S新双宽编译/同包验证、binary change双发布及实际复核 |
| Verification | 新干净双宽全构建/PE/ABI/资源；同包10项；逐项原始Git提交/覆盖映射与源码所有者；124输入与S4一致则保留S4完整矩阵证据而不虚称本S重跑 |
| Expected Markers | 六任务与八验收面均有来源、实现及当前证据，未完项只有明示宿主隔离clipboard限制；四文件双发布、远端SHA/源码可复取，无sibling依赖 |
| Asset Needs | build/t6-package/s5-audit（最终快照/审计）、s5-audit-test（实际包）、s5-deploy（两处备份）；S2/S3/S4 scratch保留至证据提炼后安全回收，预算1GB，构建每宽600秒、同包1800秒 |
| Reporting Requirements | 当前与历史工具链/资源明确区分；S4全矩阵不冒称S5重跑；两处部署及远端相同；权限跳过和真实物理输入未覆盖明示 |
| Stop Conditions | 覆盖缺项、未处置产品失败、124输入漂移、ABI/资源变化、部署混包或远端身份不符时先纠正，不关闭T6/M0 |
| Exit Criteria | 本S新包编译/测试/双发布/提交推送/实际复核，T6及M0全部原目标审计无未处置缺项，提炼并回收owned scratch后关闭 |
| Original Owner Request | 三组件四产物独立分出、原资源相同、M0/MTSP、纯C/GCC、Console等待、每S闭环、探针自行验收、binary change双位置发布 |
| Similar-Issue Sweep | 原项目隐含依赖/第五产物、旧自绘/旧MSVC证据冒用、S2交接是否由S4接收、跳过是否误报通过、发布目录旧引用及备份可恢复性 |
