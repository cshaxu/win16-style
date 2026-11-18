# Project Status

## Current Work

M0固定。T27 Win2000常用控件保持开放。S1–S14 已按有限范围交付收口，当前S15Demo 控件目录与风格切换。按[16个S规划](../proposals/windows-2000-controls.md)合批，逐项记录Win3.x覆盖对照。

正式八文件为[T27 S14VB、VCL 与复合控件批次](../etc/evidence/t27-s14-runtime.json)，两宽同快照，先发布后182/182进程用例通过，既有36项客户区打印子检查明确跳过。[实际远端](../etc/evidence/t27-s14-remote.json)已核验20个变更文件和八件发布物。T26既有失败保留在[终审](../etc/evidence/t26-s14-final-audit.json)。

## M0 T27 S15 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation 保持 M0、T27 和既定 S 序。 |
| Admission And Approval | T27 Win2000 常用控件既有授权。 |
| Objective | 让用户从现有 demo 控件目录直接验证各实际支持组及主要状态，回归普通启动和六种风格切换。 |
| Non-goals | 新控件年代、修改托盘或全局安全策略、模拟第三方绑定或自绘内容。 |
| Reference Baseline | S14 同快照八件及 182/182 有限实包用例，远端已核验。 |
| Candidate Proposal | docs/proposals/windows-2000-controls.md；brief docs/history/M0-T27-S15-demo-transition.md。 |
| Files And ABI Surface | 原生 demo 控件例子、既有外部观测与两宽过渡测试。五组件八件 ABI 保持。 |
| Applicable Rules | 两宽 C17 GCC、Ninja、ccache、同快照、先发布后测。 |
| Verification | demo 普通 Win2000、Win3.x 和原生路径，六种切换及其窗口、控件像素和状态。 |
| Expected Markers | 原生示例可操作，附着和绘制有独立证据，失败或未覆盖范围可复查。 |
| Asset Needs | 已有 demo 例子和控件 API，S2–S14 已核准来源及实图。 |
| Reporting Requirements | 按页列出手测入口、自动验证结果、远端实物和清理。 |
| Stop Conditions | 禁 computer-use 或私有桌面，不修改全局安全设置、不要求反复物理操作。 |
| Exit Criteria | 布局与功能、合批八件先发布后测、提交推送取回、清理后进入最终 S16 审计。 |
| Original Owner Request | Win2000 常用控件参考原版，达到可核实的 Win3.x 常用范围对照。 |
| Similar-Issue Sweep | 两位宽启动、已有或后加窗口、三风格六方向、窗口菜单或控件数据残留。 |

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
- Last verified remote delivery: M0 T27 S14 P1，5c9b33ed；20个变更文件及八件取回核对。
- Last allocated T27 S: 15
- Open task: M0 T27 S15；Win2000Demo 控件目录与风格切换。

## Technical Baseline

- 产品为[契约](../etc/product-contract.json)固定五组件八文件：run16s.exe、demo.exe、ntw16s32/64.dll、ntw2ks32/64.dll、ntwshk32/64.dll；正式文件在assets根与O:/winnt，当前摘要/版本/有限验证见[T27 S14证据](../etc/evidence/t27-s14-runtime.json)。
- 样式统一src/lib-win32-style的access/application/appearance；通用无样式Windows能力在src/lib-win32。当前结构和接口见[ARCHITECTURE](../design/ARCHITECTURE.md)及[CODING](../design/CODING.md)。
- 三接入、共同生命周期/窗口/菜单/控件协议/对话框/现代宿主/图形根已有真实消费者；年代保留度量、绘制、字体、资源与支持策略。产品保留CLI/托盘业务、目标/会话选择、原API trampoline、兼容ABI和装配。Win2000控件、其他框架与Luna/Aero不因公共根存在即得到完整支持。
- 来源1135回执、五原资源、包内原始栅格表和ABI/PE门禁通过；两宽各23公共样式归档与4进程归档无产品/年代反向依赖。所有当前构建/运行/测试不依赖或修改兄弟仓库。
- propagation停用、窗口恢复、thread hook解除与物理DLL/late patch卸载分别报告；原有进程存活期固定/保留能力不改称安全卸载。
- 禁止computer-use、创建或切换私有桌面；只用自有有界当前桌面探针。用户已永久授权非computer-use验证及按身份处理正式文件实际占用者；不重复请求同一授权，不改系统安全设置。
- 历史逐文件来源及迁移见[原顺序台账](../history/M0-T26-S1-ordered-audit.md)、各S证据与T26规划；早期lib-style/lib-win32/style路径是历史路径，当前旧生产目录已删除。
