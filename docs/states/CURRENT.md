# Project Status

## Current Work

用户授权发布规范变更，当前唯一活动包为M0 Td S5；T6 S4转交本治理包完成后恢复。
用户要求不用computer-use、由探针验收，不再反复要求用户整套手工操作；
本治理包更新验收方式和双发布位置。已启动S4基线构建由原owner保管，不作为新版最终包。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 6
- Last allocated Td S: 5
- Last delivery: M0 T6 S3 P2 (P1 4b83994远端身份复核及关闭，准入S4)

## Technical Baseline

本轮M0 T6/0.6.0.0：纯C17、MinGW-w64 GCC16.1.0双工具链、Ninja、ccache4.13.6；
静态GCC/线程支持，不使用C++/MSVC/MSYS运行库。三组件四产物，ABI3，31公开名称
ordinal2..32及NONAME helper1；SDK staging/有界OS helper、私有MinHook/HDE后端。

用户否决旧五资源自绘替代；现已从固定ntvdm64提交逐字节恢复五头及原resources.md，
Git暂存blob与来源一致，121回执。旧生成器删除；独立摘要门禁拒绝替换，两个DLL
各含18个原图形表及RCDATA5完整来源说明。124处置/294 includes及MinHook14项保持。

s2-original-art新干净双宽构建通过；完整矩阵x86为23通过/1剪贴板跳过、x64为34通过/
1剪贴板跳过，0失败。原生CreateWindowStation拒绝访问5，77仍不计通过。
同包13/13、系统双宽cmd中转8/8通过；全程不重链接，四文件哈希保持相同，已复制
assets并复核。发布清单绑定122输入、快照/工具链/PE/ABI及全部四文件SHA。
已查看实际demo两路截图；新标题及控件图形来自原表，不用旧自绘包证明外观通过。

S3新快照s3-wait-final双宽全构建通过，相关回归x86 6/6及x64 9/9无跳过，
同包14/14含系统cmd等待6场景通过；四文件发布到assets且SHA与测试包一致。
清单更新为124源码输入。启动器保持Console，控制台目标默认等待并返回实际退出码，
GUI目标默认异步，--wait均等待。S3完整矩阵未重跑，S4接收最终全量与交互探针（用户后续指定）。

用户确认“这次跟ntvdm64的一致”，原资源新版视觉验收通过；真实输入/焦点/捕获/
选定DPI/剪贴板的未完成项由S4接收。私有桌面、合成消息与
PrintWindow不等于物理验收。启动器仍Console，未实施GUI子系统切换。S2剩余项转交S4，
S4现为唯一活动包，M0未完成；记录见[S2验收](../history/M0-T6-S2-acceptance.md)。

用户要求每产品S构建同批四文件、对该包测试、发布、提交推送及实际复核；不再
仅以源码推送收口。历史MSVC/Detours及未发布旧S保留历史事实，不作为新基线证据。
S1验证细节与所有失败处置见[工作记录](../history/M0-T6-S1-package.md)。

## Recent Closure

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

## M0 Td S5 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Governance；用户明确要求将新发布步骤写入执行规范，并改用探针自行验收 |
| Admission And Approval | 用户：每S若有binary change发布到本项目assets根目录及O:/winnt/system32；不用computer-use，自己用探针做 |
| Objective | 更新条件式双位置发布规范、发布工具、当前资产路径及探针验收边界；完成后恢复T6 S4 |
| Non-goals | 不改生产源码/ABI/资源，不把探针冒充人工操作，不修改系统其他文件或兄弟仓库 |
| Reference Baseline | 603e661；S3测试包4b83994；T6 S4基线构建worker32332首次未启动，后以同步owned worker重启 |
| Candidate Proposal | [独立包与M0验收](../proposals/package-acceptance.md)；用户最新验收和发布指令优先 |
| Files And ABI Surface | rules/EXECUTION、当前文档/发布路径、gitignore、治理与发布工具；现有四二进制仅移动路径，字节不变，无编译输入变化 |
| Applicable Rules | Task Reading Set、Governance编号、保留历史事实和原资源、用户指令优先 |
| Verification | governance SelfTest、diff检查、移动四文件哈希与S3测试包一致、路径引用及门禁负例验证 |
| Expected Markers | assets根目录恰四产品和README；不再使用binary子目录；二进制变化时测试后双发布并哈希核验；验收由原生探针执行 |
| Asset Needs | build/t6-package/td5-policy用于发布工具隔离验证及旧路径迁移记录；S4基线构建仍由T6 owner维护；总scratch1GB |
| Reporting Requirements | 区分规则交付与产品编译；保留S3真实历史路径；后续S4执行系统目录部署，探针局限明确记录 |
| Stop Conditions | 路径超界、未经测试的字节、改动四产品以外系统文件、原资源变化时停止并纠正 |
| Exit Criteria | 规范/路径引用/迁移验证、提交推送及实际复核，恢复唯一活动T6 S4 |
| Original Owner Request | binary change时发布本项目assets（不要binary目录）及O:/winnt/system32作为规范；不用computer-use，自己用探针做 |
| Similar-Issue Sweep | 搜索当前权威/工具中assets、物理验收强制要求、旧发布脚本；历史交付路径只保留事实 |
