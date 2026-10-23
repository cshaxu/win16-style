# Execution Rules

## MTSP

| 标识 | 含义及分配 |
| --- | --- |
| M0 | 唯一 milestone，永不递增 |
| T<n> | 全仓严格递增实现任务，同时是程序 build version；首次实现从 T1 开始 |
| Td | 文档/治理任务，不消耗数值 T；治理工具也归其授权范围 |
| S<n> | T 内顺序子任务；Td 的 S 在 M0 内递增；同一时间只有一个活动 S |
| P<n> | 同一 S 内递增的完整、已验证并推送的交付单元；不能作为未完工作占位 |

提交标题为 `M0 T1 S1 P1: summary` 或 `M0 Td S1 P1: summary`。
禁止重用标识；T 只在用户请求授权的实现任务准入时分配，QUEUE 不预分配。
CURRENT 记录最后分配的 T、Td S 及当前工作；同一 S 的交付/关闭提交依次用 P。
代码任务不得改 rules；规则修改必须由明确授权的 Td 承担。

## 准入与活动包

用户请求是准入依据；执行者可以在该请求明确范围内细分 S 并推进验证和交付。
不得擅自扩展产品范围或把候选当作已获准实现。用户要求本次“先建立治理体系，
然后准备导入”，因此只准入治理和准备，不自动执行产品源码导入。
关闭依据为请求明确的完成条件及实际验证，不能制造重复确认环节；要求用户
物理验收或重大范围变更的事项保留待用户处理。

CURRENT 至多一个 `## M0 Td S<n> Packet` 或 `## M0 T<n> S<n> Packet`，
使用固定的 `Field | Required record` 两列表，每个字段非空：

`Identifier Mode`, `Admission And Approval`, `Objective`, `Non-goals`,
`Reference Baseline`, `Candidate Proposal`, `Files And ABI Surface`,
`Applicable Rules`, `Verification`, `Expected Markers`, `Asset Needs`,
`Reporting Requirements`, `Stop Conditions`, `Exit Criteria`, `Original Owner Request`,
`Similar-Issue Sweep`。

Identifier Mode 为 New、Continuation、Corrective、Owner-Reopen、Governance。
新 T 从 S1 开始，续作使用下一 S。Corrective/Owner-Reopen 不复用旧 S；须说明
失败契约并保留原任务边界。空闲 CURRENT 明确无活动包，不保留过期 Packet。
生命周期为 accepted -> planned -> active -> implemented -> verified -> closed；
阻塞、转移或暂停须记录真实原因、接收者和恢复条件。

## 交付、复核与证据

实现前声明有限覆盖清单、预期标记、focused/full regression、相似问题搜索、
失败处置及退出条件。产物行为需按相关架构覆盖，当前治理任务不虚构产品测试。
运行测试区分 background、private desktop 与真实 desktop；自动截图不代表
真实鼠标、键盘、焦点、DPI 和原生通知验收。

每个 P 应包括该交付的实现/文档、必要验证及未解决项。执行者自审后提交推送；
随后切换为协调复核角色，读取实际 Git diff/commit、原始请求和证据，不以自己
的总结代替检查。角色可由同一会话顺序承担，不要求另开 agent。
S 关闭前完成对应条件；T 关闭须另做全任务覆盖审计，不能凭一个 S 或 smoke
宣称整个 T 完成。推送成功前状态只能 verified/delivered locally，不能 closed。
交付后可用下一治理 P 记录复核与关闭。

代码变更记录统计范围、git diff --numstat、增加/删除/净行数、保留状态所有者
与旧路径处置。缺陷修复记录搜索命令、每个生产命中处置及修后证明；越界发现
转 TODO/QUEUE，不默默忽略。全量审计先冻结覆盖全集、覆盖单位、允许处置、
逐项证据/接收者和完成谓词。

源码导入固定 source commit + blob 清单，改动有明确差异，保留原有声明。
完整包须重建所有受影响产物，记录 SHA-256/PE 架构及回归清单，禁止混包。

## 执行卫生

长任务启动前明确 log、退出码、超时/无进展预算及拥有者，使用隐藏后台进程，
定期报告；超时属于未定结论。只停止自己拥有的任务。scratch 按源码布局规则
管理，禁止将历史证据藏入缓存。提交前检查 scratch 和无关变更。

交付及关闭前运行 `pwsh -NoProfile -File tools/Verify-Governance.ps1 -SelfTest`
与 `git diff --check`，再运行活动包所需检查。结构检查不证明文档陈述真实，
也不替代人工实际 diff 复核。
