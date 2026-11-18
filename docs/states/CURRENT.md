# Project Status

## Current Work

M0固定。T27 Win2000常用控件保持开放。S1–S15 已按有限范围交付收口，当前S16全任务终审。按[16个S规划](../proposals/windows-2000-controls.md)合批，逐项记录Win3.x覆盖对照。

正式八文件为[T27 S15 Demo 控件目录与风格切换批次](../etc/evidence/t27-s15-demo.json)，两宽同快照，先发布后185/185进程用例通过，既有36项客户区打印子检查明确跳过。[实际远端](../etc/evidence/t27-s15-remote.json)已核验19个变更文件和八件发布物。T26既有失败保留在[终审](../etc/evidence/t26-s14-final-audit.json)。

## M0 T27 S16 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation 保持 M0、T27 和既定 S 序。 |
| Admission And Approval | T27 Win2000 常用控件既有授权，执行最终审计。 |
| Objective | 将 S1 冻结的 24 个标准控件组与 8 个运行库/复合组逐项对上已验证范围、来源、产品路径和未覆盖边界。 |
| Non-goals | 新控件、新图形资源、新主题、新 T 或扩大已交付的 Win32 适用范围。 |
| Reference Baseline | S15 已发布同快照八件、185 项自有进程用例与实际取回。 |
| Candidate Proposal | docs/proposals/windows-2000-controls.md；brief docs/history/M0-T27-S16-final-audit.md。 |
| Files And ABI Surface | 审计台账、任务状态和未覆盖记录。沿用现有产品构建输入、ABI 与八件。 |
| Applicable Rules | 复核实际远端来源与发布物；无产品输入变化沿用已发布验证，不重建。 |
| Verification | 逐条冻结组对账，历史 P1 来源和交付校验，确认两处八件、实际远端与源输入身份。 |
| Expected Markers | 24 + 8 项各有独立处置，原图可见状态与源码规则区分，交付范围和跳过项可复查。 |
| Asset Needs | 不新增资产；复核五份既有原资源和各阶段只读参考。 |
| Reporting Requirements | 各页验证和产品入口，有限 Win3.x 对照，欠项、包摘要、用户入口及清理。 |
| Stop Conditions | 禁 computer-use 或私有桌面，不改变全局安全设置。 |
| Exit Criteria | 完成审计、发布身份保持并实际复核远端，收口 S16 与 T27，不准入新 T。 |
| Original Owner Request | 原版 Win2000 源码、资源与真实截图优先，和现有 Win3.x 常用控件范围逐项对照。 |
| Similar-Issue Sweep | 分别审查未覆盖主题、复杂及远期窗口框架、原生保留和真实托盘输入。 |

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 27
- Last allocated Td S: 21
- Last allocated T18 S: 64
- Last allocated T19 S: 3
- Last allocated T20 S: 19
- Last allocated T21 S: 44
- Last allocated T25 S: 8
- Last allocated T26 S: 14
- Last verified remote delivery: M0 T27 S15 P1，73f60c5a；19个变更文件及八件取回核对。
- Last allocated T27 S: 16
- Open task: M0 T27 S16；Win2000全任务终审。

## Technical Baseline

- 产品为[契约](../etc/product-contract.json)固定五组件八文件：run16s.exe、demo.exe、ntw16s32/64.dll、ntw2ks32/64.dll、ntwshk32/64.dll；正式文件在assets根与O:/winnt，当前摘要/版本/有限验证见[T27 S15证据](../etc/evidence/t27-s15-demo.json)。
- 样式统一src/lib-win32-style的access/application/appearance；通用无样式Windows能力在src/lib-win32。当前结构和接口见[ARCHITECTURE](../design/ARCHITECTURE.md)及[CODING](../design/CODING.md)。
- 三接入、共同生命周期/窗口/菜单/控件协议/对话框/现代宿主/图形根已有真实消费者；年代保留度量、绘制、字体、资源与支持策略。产品保留CLI/托盘业务、目标/会话选择、原API trampoline、兼容ABI和装配。Win2000控件、其他框架与Luna/Aero不因公共根存在即得到完整支持。
- 来源1135回执、五原资源、包内原始栅格表和ABI/PE门禁通过；两宽各23公共样式归档与4进程归档无产品/年代反向依赖。所有当前构建/运行/测试不依赖或修改兄弟仓库。
- propagation停用、窗口恢复、thread hook解除与物理DLL/late patch卸载分别报告；原有进程存活期固定/保留能力不改称安全卸载。
- 禁止computer-use、创建或切换私有桌面；只用自有有界当前桌面探针。用户已永久授权非computer-use验证及按身份处理正式文件实际占用者；不重复请求同一授权，不改系统安全设置。
- 历史逐文件来源及迁移见[原顺序台账](../history/M0-T26-S1-ordered-audit.md)、各S证据与T26规划；早期lib-style/lib-win32/style路径是历史路径，当前旧生产目录已删除。
