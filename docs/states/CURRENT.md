# Project Status

## Current Work

M0 Td S5发布规范已推送复核并关闭，恢复M0 T6 S4唯一活动包。
用户要求不用computer-use、自己用原生探针验收；本S同时修复滚动条端钮长方形缺陷。
S4的新四文件验证后发布assets根目录及O:/winnt/system32；S5最终审计尚未准入。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 6
- Last allocated Td S: 5
- Last delivery: M0 Td S5 P2 (P1 618c51b远端复核及关闭，恢复T6 S4)

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

## M0 T6 S4 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation；Td5后恢复；用户改为自己用探针做，新增滚动条方形端钮缺陷 |
| Admission And Approval | 用户要求先处理启动器新S、随后继续后续S；每S编译发布测试提交推送；基线618c51b（规则及路径）/4b83994（产品） |
| Objective | 从本S新干净快照构建双宽四文件，完整矩阵及实际同包复验；完成原生焦点/控件消息/捕获/DPI及隔离剪贴板探针，修复滚动条端钮比例 |
| Non-goals | 不修改原图形资源、不切GUI子系统、不扩展组件/ABI、不接触sibling或共享剪贴板；不把自动合成消息冒充真实输入 |
| Reference Baseline | S3 P1 4b83994；Console目标默认等待，GUI默认异步，--wait均等待；六份原资源及双DLL18表，纯C17/GCC，ABI3 |
| Candidate Proposal | [独立包与M0验收](../proposals/package-acceptance.md)S4；原资源视觉一致已由用户确认 |
| Files And ABI Surface | 验收记录、发布清单及本S同批四产物；修复controls/scroll_layout及demo滚动条几何，扩展外部demo/几何探针；公开ABI不变 |
| Applicable Rules | 完整Task Reading Set、每S四文件闭环、真实交互与自动证据分开、来源身份及任务级退出条件 |
| Verification | 新干净双宽全构建；完整注册矩阵及失败/跳过逐项核对；同批实际包10项含等待6场景；发布assets及O:/winnt/system32 SHA与测试一致；本机原生探针状态/DPI及环境限制 |
| Expected Markers | 全矩阵无未处置失败、四文件同批且无sibling依赖；裸demo原生/注入demo原样式业务正常；焦点/捕获/控件消息/DPI有探针证据，剪贴板权限限制有明确处置 |
| Asset Needs | build/t6-package/s4-acceptance（旧布局基线，已通过构建）、s4-probe-dev（探针开发）、s4-probe-final及s4-probe-final-test（最终新批次）新构建/矩阵/同包证据；保留S2/S3已登记scratch至T结束；hidden owned每构建架构600秒、矩阵每架构600秒、同包1800秒、总scratch1GB |
| Reporting Requirements | 区分通过/失败/跳过及用户已确认外观；明确待物理操作和选定DPI；不将S3等待自动测试扩张为交互键盘通过 |
| Stop Conditions | 探针存在未处置失败、完整矩阵失败或混包时不得关闭S4/T6/M0；先完成不依赖用户的工作 |
| Exit Criteria | 本S编译/同批打包/测试/发布/提交推送/实际复核及用户要求的原生探针验收/限制记录完成，才关闭S4并准入S5审计 |
| Original Owner Request | 每次S编译发布测试提交推送；原资源相同；Console自然等待优先；完成启动器新S后继续后续S |
| Similar-Issue Sweep | 检查完整矩阵注册和实际执行、private clipboard拒绝访问、物理反馈缺项、旧外观待确认措辞及发布清单/测试包身份 |
