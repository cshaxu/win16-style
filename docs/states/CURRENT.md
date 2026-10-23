# Project Status

## Current Work

M0 T6 S4已完成滚动条方形端钮修复及原生探针；新双宽构建、完整58通过/2权限跳过、
同包10/10及双位置发布通过，本地verified，待提交推送和实际复核。用户指定不用
computer-use、自己用探针验收。S5最终任务审计尚未准入，M0未完成。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 6
- Last allocated Td S: 5
- Last delivery: M0 T6 S4 P1 (本地verified及双发布，待推送后实际复核)

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
| Asset Needs | build/t6-package/s4-acceptance（旧布局基线，已通过构建）、s4-probe-dev、s4-probe-dev-fixed、s4-probe-about、s4-probe-dev-about-thread（失败及修正日志）、s4-probe-dev-tree（开发构建）、s4-probe-final及s4-probe-final-test（最终新批次）新构建/矩阵/同包证据；保留S2/S3已登记scratch至T结束；hidden owned每构建架构600秒、矩阵每架构600秒、同包1800秒、总scratch1GB；s4-deploy保存两处原文件备份及部署结果 |
| Reporting Requirements | 区分通过/失败/跳过及用户已确认外观；明确原生探针覆盖和实际DPI、隔离station权限限制；不将S3等待自动测试扩张为交互键盘通过 |
| Stop Conditions | 探针存在未处置失败、完整矩阵失败或混包时不得关闭S4/T6/M0；先完成不依赖用户的工作 |
| Exit Criteria | 本S编译/同批打包/测试/发布/提交推送/实际复核及用户要求的原生探针验收/限制记录完成，才关闭S4并准入S5审计 |
| Original Owner Request | 每次S编译发布测试提交推送；原资源相同；Console自然等待优先；完成启动器新S后继续后续S |
| Similar-Issue Sweep | 检查完整矩阵注册和实际执行、private clipboard拒绝访问、探针覆盖缺项、旧外观待确认措辞及发布清单/测试包身份 |
