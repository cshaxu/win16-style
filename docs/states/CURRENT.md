# Project Status

## Current Work

M0固定。T27 Win2000常用控件保持开放；S1-S5来源/按钮/选择/静态分组/编辑已按有限范围交付关闭，当前S6 ListBox/ComboBox。按[16个S规划](../proposals/windows-2000-controls.md)合批，不逐文件完整交付；不提前称所有控件parity完成。

正式八文件已更新为[T27 S5编辑控件批次](../etc/evidence/t27-s5-edit-controls.json)，两宽同快照干净构建及两处八文件发布完成，发布后62/62进程测试通过，36个原生客户区打印子检查明确skip；[实际远端](../etc/evidence/t27-s5-remote.json)25变更/八文件一致，1191输入仅9个旧行尾差异。T26已知失败保留在[终审](../etc/evidence/t26-s14-final-audit.json)，未被本组局部通过替代。Win2000其他关闭主题分支不计独立renderer完成。

## M0 T27 S6 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation；S5有限条件完成并实际远端复核，自动按既定顺序S6，M0/T27不变。 |
| Admission And Approval | 用户准入Win2000常用控件并要求原源码/原资源/真实图片，授权按16S持续执行。 |
| Objective | 原生ListBox与三种ComboBox的Win2000有据呈现，复用共同快照/消息/输入/子窗口及寿命机制。 |
| Non-goals | 不替换选择/数据/排序/回调/输入，不扩框架身份/全局/权限；原生owner-draw和未知角色保持，滚动独立由S7负责。 |
| Reference Baseline | S5 P1 8325ccbb同源双宽八文件先发布，62/62进程验证、36明确客户打印skip；1191输入实际远端复核。 |
| Candidate Proposal | docs/proposals/windows-2000-controls.md；brief docs/history/M0-T27-S6-list-combo.md。 |
| Files And ABI Surface | appearance/win2000/control与共同list/combo协议/快照的实际复用、产品装配和对应测试；五组件八文件/公开ABI保持。 |
| Applicable Rules | 纯C17、MinGW GCC/Ninja/ccache，三层方向、唯一公共状态，原资源/声明保留；兄弟仓库只读且无依赖。 |
| Verification | 原版背景/选中/禁用/边缘/箭头、三种combo与子窗口/原生数据和自绘保持；双宽v5/v6/aware、标准启动及生命周期/DC/资源和旧控件回归。 |
| Expected Markers | 实际DLL选择/呈现/输入/移除分列；普通字符串和原生自绘边界明确，不把native回退算独立renderer。 |
| Asset Needs | 固定Win2000SRC fd809a81对应list/combo/DFC源码、原资源及真实图像，按需记录来源/SHA，不猜图形。 |
| Reporting Requirements | 有限类/状态清单、来源与共同根、实际发布包、失败/skip和未覆盖、八文件摘要、实际远端及清理。 |
| Stop Conditions | 禁computer-use及创建/切换私有桌面，不改兄弟仓库/安全设置，不重启有效worker或反复UAC；保护已有改动。 |
| Exit Criteria | 有限矩阵有据实现并实际接入，必要双宽同快照干净八文件先发布后测，提交推送/实际复核/即时清理后自动S7，T27开放。 |
| Original Owner Request | Win2000常用控件对齐已有Win3.x有限parity；必须原源码/原资源/真实图片，不瞎猜，持续自行执行。 |
| Similar-Issue Sweep | 单/多选/扩展/列/制表/禁用、simple/dropdown/dropdownlist、编辑/列表子窗、ownerdraw、父回调/换父/恢复与借用DC资源。 |

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
- Last verified remote delivery: M0 T27 S5 P1，`8325ccbb`；25变更/八文件实际fetch字节复核，1191输入旧9行尾差异披露。
- Last allocated T27 S: 6
- Open task: M0 T27 S6；Win2000 ListBox/ComboBox。

## Technical Baseline

- 产品为[契约](../etc/product-contract.json)固定五组件八文件：run16s.exe、demo.exe、ntw16s32/64.dll、ntw2ks32/64.dll、ntwshk32/64.dll；正式文件在assets根与O:/winnt，当前摘要/版本/有限验证见[T27 S5证据](../etc/evidence/t27-s5-edit-controls.json)。
- 样式统一src/lib-win32-style的access/application/appearance；通用无样式Windows能力在src/lib-win32。当前结构和接口见[ARCHITECTURE](../design/ARCHITECTURE.md)及[CODING](../design/CODING.md)。
- 三接入、共同生命周期/窗口/菜单/控件协议/对话框/现代宿主/图形根已有真实消费者；年代保留度量、绘制、字体、资源与支持策略。产品保留CLI/托盘业务、目标/会话选择、原API trampoline、兼容ABI和装配。Win2000控件、其他框架与Luna/Aero不因公共根存在即得到完整支持。
- 来源1098回执、五原资源、包内原始栅格表和ABI/PE门禁通过；两宽各23公共样式归档与4进程归档无产品/年代反向依赖。所有当前构建/运行/测试不依赖或修改兄弟仓库。
- propagation停用、窗口恢复、thread hook解除与物理DLL/late patch卸载分别报告；原有进程存活期固定/保留能力不改称安全卸载。
- 禁止computer-use、创建或切换私有桌面；只用自有有界当前桌面探针。用户已永久授权非computer-use验证及按身份处理正式文件实际占用者；不重复请求同一授权，不改系统安全设置。
- 历史逐文件来源及迁移见[原顺序台账](../history/M0-T26-S1-ordered-audit.md)、各S证据与T26规划；早期lib-style/lib-win32/style路径是历史路径，当前旧生产目录已删除。
