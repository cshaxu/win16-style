# Project Status

## Current Work

M0固定。T27 Win2000常用控件保持开放。S1–S8 已按有限范围交付收口，当前S9列表视图和树。按[16个S规划](../proposals/windows-2000-controls.md)合批，逐项记录Win3.x覆盖对照。

正式八文件为[T27 S8标签页和表头批次](../etc/evidence/t27-s8-tab-header.json)，两宽同快照，先发布后96/96进程用例通过，既有36项客户区打印子检查明确跳过。[实际远端](../etc/evidence/t27-s8-remote.json)已核验22个变更文件和八件发布物。T26既有失败保留在[终审](../etc/evidence/t26-s14-final-audit.json)。

## M0 T27 S9 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation 保持 M0、T27 和既定 S 序。 |
| Admission And Approval | T27 Win2000常用控件既有授权。 |
| Objective | 依据原版源码和真实图，为普通Win32 ListView与TreeView实现Windows 2000呈现。 |
| Non-goals | 未准入控件组、系统配置、现代UI内部及新的接入方式。 |
| Reference Baseline | S8同快照八件和96/96有限用例，实际远端已核验。 |
| Candidate Proposal | docs/proposals/windows-2000-controls.md；brief docs/history/M0-T27-S9-listview-treeview.md。 |
| Files And ABI Surface | Win2000 appearance、现有公用控件绘制/通知/滚动，保持五组件八件ABI。 |
| Applicable Rules | 两宽C17 GCC/Ninja/ccache、三层、同快照、保留原资源。 |
| Verification | 报告/图标/列表、树分支与展开、原生数据/输入通知、自绘、恢复、两宽实际运行。 |
| Expected Markers | 原版色值与结构、通知、资源与清理分别断言；有限矩阵写明回退。 |
| Asset Needs | Windows 2000 comctl32 listview/lvrept/treeview/tvpaint；可核实的真实图。 |
| Reporting Requirements | 源码及图片摘要、实际通过和未验范围、发布 SHA、远端和清理。 |
| Stop Conditions | 禁computer-use或私有桌面，保护现有改动，兄弟目录只读。 |
| Exit Criteria | 合批验证，先发布后测同包、推送并取回核验、清理后进入S10。 |
| Original Owner Request | Win2000常用控件按既有Win3.x覆盖基准，对照真实Windows 2000。 |
| Similar-Issue Sweep | v5/v6、视图切换、选择焦点、owner data/draw、应用指定色、图标/状态图、通知与移除。 |

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
- Last verified remote delivery: M0 T27 S8 P1，`90f2b90b`；22个变更文件及八件取回核对。
- Last allocated T27 S: 9
- Open task: M0 T27 S9；Win2000列表视图与树。

## Technical Baseline

- 产品为[契约](../etc/product-contract.json)固定五组件八文件：run16s.exe、demo.exe、ntw16s32/64.dll、ntw2ks32/64.dll、ntwshk32/64.dll；正式文件在assets根与O:/winnt，当前摘要/版本/有限验证见[T27 S8证据](../etc/evidence/t27-s8-tab-header.json)。
- 样式统一src/lib-win32-style的access/application/appearance；通用无样式Windows能力在src/lib-win32。当前结构和接口见[ARCHITECTURE](../design/ARCHITECTURE.md)及[CODING](../design/CODING.md)。
- 三接入、共同生命周期/窗口/菜单/控件协议/对话框/现代宿主/图形根已有真实消费者；年代保留度量、绘制、字体、资源与支持策略。产品保留CLI/托盘业务、目标/会话选择、原API trampoline、兼容ABI和装配。Win2000控件、其他框架与Luna/Aero不因公共根存在即得到完整支持。
- 来源1111回执、五原资源、包内原始栅格表和ABI/PE门禁通过；两宽各23公共样式归档与4进程归档无产品/年代反向依赖。所有当前构建/运行/测试不依赖或修改兄弟仓库。
- propagation停用、窗口恢复、thread hook解除与物理DLL/late patch卸载分别报告；原有进程存活期固定/保留能力不改称安全卸载。
- 禁止computer-use、创建或切换私有桌面；只用自有有界当前桌面探针。用户已永久授权非computer-use验证及按身份处理正式文件实际占用者；不重复请求同一授权，不改系统安全设置。
- 历史逐文件来源及迁移见[原顺序台账](../history/M0-T26-S1-ordered-audit.md)、各S证据与T26规划；早期lib-style/lib-win32/style路径是历史路径，当前旧生产目录已删除。
