# Project Status

## Current Work

M0固定。T27 Win2000常用控件保持开放。S1–S13 已按有限范围交付收口，当前S14VB、VCL 与复合控件对照。按[16个S规划](../proposals/windows-2000-controls.md)合批，逐项记录Win3.x覆盖对照。

正式八文件为[T27 S13日期与月历批次](../etc/evidence/t27-s13-dates.json)，两宽同快照，先发布后170/170进程用例通过，既有36项客户区打印子检查明确跳过。[实际远端](../etc/evidence/t27-s13-remote.json)已核验23个变更文件和八件发布物。T26既有失败保留在[终审](../etc/evidence/t26-s14-final-audit.json)。

## M0 T27 S14 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation 保持 M0、T27 和既定 S 序。 |
| Admission And Approval | T27 Win2000 常用控件既有授权。 |
| Objective | 逐项对照 S1 冻结的 8 个运行库组，将可证明等价的常用角色复用到 Win2000。 |
| Non-goals | 无证据的 OCX、任意类前缀接入、重做应用绘制、全局范围或权限扩展。 |
| Reference Baseline | S13 同快照八件及 170/170 有限实包用例，实际远端已经核验。 |
| Candidate Proposal | docs/proposals/windows-2000-controls.md；brief docs/history/M0-T27-S14-runtime-parity.md。 |
| Files And ABI Surface | 公共身份及 Win2000 控件装配，必要的原生 fixture；五组件八件 ABI 保持。 |
| Applicable Rules | 两宽 C17 GCC、Ninja、ccache、同快照、三层、原资源。 |
| Verification | 精确身份、原生实际角色、误接入反例、原生应用行为、绘制、移除与普通启动。 |
| Expected Markers | 真实运行库、原生 C 模型、原生保留、未验证明确分列。 |
| Asset Needs | S1 组清单、T17 和 T18 原样本身份记录、S2–S13 原 Win2000 绘制来源。 |
| Reporting Requirements | 8 组逐项核对、真实范围和缺口、适用包发布、取回远端、临时文件清理。 |
| Stop Conditions | 禁 computer-use、私有桌面，保护现有改动，兄弟目录只读。 |
| Exit Criteria | 8 组均有对照结论，适用八件先发布后测，提交推送取回，清理后进入 S15。 |
| Original Owner Request | 按 Win3.x 现有覆盖逐项对照，以原版 Win2000 源码及真实截图为准。 |
| Similar-Issue Sweep | VB5/6 模块及类身份、VCL 注册角色与公开协议、原生子窗、owner-draw、OCX 资源和内容。 |

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
- Last verified remote delivery: M0 T27 S13 P1，971638b9；23个变更文件及八件取回核对。
- Last allocated T27 S: 14
- Open task: M0 T27 S14；Win2000VB、VCL 与复合控件对照。

## Technical Baseline

- 产品为[契约](../etc/product-contract.json)固定五组件八文件：run16s.exe、demo.exe、ntw16s32/64.dll、ntw2ks32/64.dll、ntwshk32/64.dll；正式文件在assets根与O:/winnt，当前摘要/版本/有限验证见[T27 S13证据](../etc/evidence/t27-s13-dates.json)。
- 样式统一src/lib-win32-style的access/application/appearance；通用无样式Windows能力在src/lib-win32。当前结构和接口见[ARCHITECTURE](../design/ARCHITECTURE.md)及[CODING](../design/CODING.md)。
- 三接入、共同生命周期/窗口/菜单/控件协议/对话框/现代宿主/图形根已有真实消费者；年代保留度量、绘制、字体、资源与支持策略。产品保留CLI/托盘业务、目标/会话选择、原API trampoline、兼容ABI和装配。Win2000控件、其他框架与Luna/Aero不因公共根存在即得到完整支持。
- 来源1134回执、五原资源、包内原始栅格表和ABI/PE门禁通过；两宽各23公共样式归档与4进程归档无产品/年代反向依赖。所有当前构建/运行/测试不依赖或修改兄弟仓库。
- propagation停用、窗口恢复、thread hook解除与物理DLL/late patch卸载分别报告；原有进程存活期固定/保留能力不改称安全卸载。
- 禁止computer-use、创建或切换私有桌面；只用自有有界当前桌面探针。用户已永久授权非computer-use验证及按身份处理正式文件实际占用者；不重复请求同一授权，不改系统安全设置。
- 历史逐文件来源及迁移见[原顺序台账](../history/M0-T26-S1-ordered-audit.md)、各S证据与T26规划；早期lib-style/lib-win32/style路径是历史路径，当前旧生产目录已删除。
