# Project Status

## Current Work

M0 T6当前准入S3“Console启动器默认等待语义”，用户明确要求先完成本S再继续后续。
S2自动交付及原资源新版视觉验收已通过；真实输入/DPI/剪贴板未全部确认，按用户
插入任务的顺序转交S4继续验收，不能写成S2整体closed。S5负责最终全任务审计。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 6
- Last allocated Td S: 4
- Last delivery: M0 T6 S3 P1 (本地verified，待推送及实际复核)

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
assets/binary并复核。发布清单绑定122输入、快照/工具链/PE/ABI及全部四文件SHA。
已查看实际demo两路截图；新标题及控件图形来自原表，不用旧自绘包证明外观通过。

S3新快照s3-wait-final双宽全构建通过，相关回归x86 6/6及x64 9/9无跳过，
同包14/14含系统cmd等待6场景通过；四文件发布到assets/binary且SHA与测试包一致。
清单更新为124源码输入。启动器保持Console，控制台目标默认等待并返回实际退出码，
GUI目标默认异步，--wait均等待。S3完整矩阵未重跑，S4接收最终全量与物理验收。

用户确认“这次跟ntvdm64的一致”，原资源新版视觉验收通过；真实输入/焦点/捕获/
选定DPI/剪贴板的未完成项由S4接收。私有桌面、合成消息与
PrintWindow不等于物理验收。启动器仍Console，未实施GUI子系统切换。S2剩余项转交S4，
S3现为唯一活动包，M0未完成；记录见[S2验收](../history/M0-T6-S2-acceptance.md)。

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

## M0 T6 S3 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation；新S3由用户纠正为Console自然等待；S2未完物理项转S4，最终审计顺延S5 |
| Admission And Approval | 用户要求新S先处理启动器，随后明确否决GUI让交互cmd不等待的行为；保留Console且按目标子系统默认等待；基线be4c486 |
| Objective | 保留x64 Console启动器；控制台目标默认等待并返回目标退出码，GUI目标默认异步；--wait始终等待；run16s cmd.exe在原console内自然交互，无需start包装 |
| Non-goals | 不切GUI子系统、不连接/分配额外console、不接管父shell；不改原图形资源或传播机制；不改sibling/全局设置 |
| Reference Baseline | be4c486；原资源六文件及双DLL18表相同，纯C17/GCC，ABI3；用户确认新版外观一致 |
| Candidate Proposal | [独立包与M0验收](../proposals/package-acceptance.md)S3；用户后续否决GUI方案，以本包为准 |
| Files And ABI Surface | 启动器等待选择、既有PE验证输出子系统的私有接口、CLI与nativecmd等待测试、回执/文档及四文件；公开ABI和图形资源保持 |
| Applicable Rules | 完整Task Reading Set、每S四文件闭环、来源身份及原生进程/console契约 |
| Verification | 实际Console PE；console目标默认及--wait退出码；GUI目标默认异步和--wait；既有console HWND/成员及真实cmd等待；stdio/长路径/失败/双宽传播回归 |
| Expected Markers | 普通run16s cmd.exe使父cmd等到目标退出；双宽console默认返回目标码；GUI默认返回0且目标仍存活；同console身份，原资源与四文件同批哈希通过 |
| Asset Needs | build/t6-package/s3-wait（开发验证）、s3-wait-package（中间批次/相关回归）、s3-wait-final（最终快照）、s3-wait-test（中间失败证据）、s3-wait-final-test（最终同包）；s3-dev及s3-dev-tree仅保留已终止的GUI试验，未发布；原S2 scratch留至T结束；hidden owned每构建架构600秒、同包1800秒、总scratch1GB |
| Reporting Requirements | 区分默认同步console、异步GUI和显式wait；记录退出码/console身份/发布SHA，保留S4未完验收；GUI试验不得称为产品交付 |
| Stop Conditions | 父shell提前返回、异步GUI被强制等待、stdio丢失、注入/资源回归先修复；不得发布混包 |
| Exit Criteria | 本S新双宽四文件编译/同包测试/发布/提交推送/实际复核，随后自动准入S4继续剩余验收 |
| Original Owner Request | 新S先处理启动器；GUI在交互cmd不自然等待不可接受；Console自然等待及原console复用优先；原资源必须相同 |
| Similar-Issue Sweep | 原PE子系统判断、默认/显式wait退出码、无wait GUI实际存活、原始命令行/stdio及跨宽传播；文档中GUI试验假设全部撤销 |
