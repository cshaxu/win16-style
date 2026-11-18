# Project Status

## Current Work

M0固定。T27 Win2000常用控件保持开放。S1–S11 已按有限范围交付收口，当前S12工具栏、可重排栏与状态栏。按[16个S规划](../proposals/windows-2000-controls.md)合批，逐项记录Win3.x覆盖对照。

正式八文件为[T27 S11滑块与微调按钮批次](../etc/evidence/t27-s11-track-spin.json)，两宽同快照，先发布后146/146进程用例通过，既有36项客户区打印子检查明确跳过。[实际远端](../etc/evidence/t27-s11-remote.json)已核验24个变更文件和八件发布物。T26既有失败保留在[终审](../etc/evidence/t26-s14-final-audit.json)。

## M0 T27 S12 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation 保持 M0、T27 和既定 S 序。 |
| Admission And Approval | T27 Win2000常用控件既有授权。 |
| Objective | Toolbar、Rebar 和 StatusBar 的 Win2000 原版边线、布局和标准色。 |
| Non-goals | 其他控件组、重实现原生布局和外部应用绘制、扩展接入。 |
| Reference Baseline | S11同快照八件和146/146有限用例，实际远端已核验。 |
| Candidate Proposal | docs/proposals/windows-2000-controls.md；brief docs/history/M0-T27-S12-bars.md。 |
| Files And ABI Surface | Win2000控件呈现、共享通知/几何/快照；五组件八件ABI保持。 |
| Applicable Rules | 两宽C17 GCC/Ninja/ccache、同快照、三层、原资源。 |
| Verification | 普通工具栏分隔与应用图文、Rebar 抓手和子窗、状态栏分区和缩放角、双宽启动及移除。 |
| Expected Markers | 外观、应用数据/资源、通知、借用DC和撤销分别观测。 |
| Asset Needs | 固定Windows 2000 toolbar/rebar/status源码及 WordPad、Explorer 实图。 |
| Reporting Requirements | 原版取样、实际通过与未覆盖、发布远端摘要、临时文件清理。 |
| Stop Conditions | 禁computer-use或私有桌面，保护现有改动，兄弟目录只读。 |
| Exit Criteria | 有限矩阵、八件先发布再测、提交推送取回核验、清理后进入S13。 |
| Original Owner Request | 按Win3.x现有覆盖，基于原版Win2000源码及真实截图。 |
| Similar-Issue Sweep | v5/v6、普通/平面/禁用/分隔、rebar图片/子窗、simple/owner-draw、DPI和撤销。 |

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
- Last verified remote delivery: M0 T27 S11 P1，5f461141；24个变更文件及八件取回核对。
- Last allocated T27 S: 12
- Open task: M0 T27 S12；Win2000工具栏、可重排栏与状态栏。

## Technical Baseline

- 产品为[契约](../etc/product-contract.json)固定五组件八文件：run16s.exe、demo.exe、ntw16s32/64.dll、ntw2ks32/64.dll、ntwshk32/64.dll；正式文件在assets根与O:/winnt，当前摘要/版本/有限验证见[T27 S11证据](../etc/evidence/t27-s11-track-spin.json)。
- 样式统一src/lib-win32-style的access/application/appearance；通用无样式Windows能力在src/lib-win32。当前结构和接口见[ARCHITECTURE](../design/ARCHITECTURE.md)及[CODING](../design/CODING.md)。
- 三接入、共同生命周期/窗口/菜单/控件协议/对话框/现代宿主/图形根已有真实消费者；年代保留度量、绘制、字体、资源与支持策略。产品保留CLI/托盘业务、目标/会话选择、原API trampoline、兼容ABI和装配。Win2000控件、其他框架与Luna/Aero不因公共根存在即得到完整支持。
- 来源1124回执、五原资源、包内原始栅格表和ABI/PE门禁通过；两宽各23公共样式归档与4进程归档无产品/年代反向依赖。所有当前构建/运行/测试不依赖或修改兄弟仓库。
- propagation停用、窗口恢复、thread hook解除与物理DLL/late patch卸载分别报告；原有进程存活期固定/保留能力不改称安全卸载。
- 禁止computer-use、创建或切换私有桌面；只用自有有界当前桌面探针。用户已永久授权非computer-use验证及按身份处理正式文件实际占用者；不重复请求同一授权，不改系统安全设置。
- 历史逐文件来源及迁移见[原顺序台账](../history/M0-T26-S1-ordered-audit.md)、各S证据与T26规划；早期lib-style/lib-win32/style路径是历史路径，当前旧生产目录已删除。
