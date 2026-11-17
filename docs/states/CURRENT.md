# Project Status

## Current Work

M0固定。T27 Win2000常用控件保持开放；S1-S7来源/按钮/选择/静态分组/编辑/列表组合/滚动条已按有限范围交付关闭，当前S8标签页和表头。按[16个S规划](../proposals/windows-2000-controls.md)合批，不逐文件完整交付；不提前称所有控件parity完成。

正式八文件为[T27 S7滚动条批次](../etc/evidence/t27-s7-scroll.json)，两宽同快照，先发布后84/84进程用例通过，既有36项客户区打印子检查明确跳过。[实际远端](../etc/evidence/t27-s7-remote.json)已核验35个变更文件和八件发布物。T26已知失败保留在[终审](../etc/evidence/t26-s14-final-audit.json)，未被本组局部通过替代。Win2000其他关闭主题分支不计独立renderer完成。

## M0 T27 S8 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation；按既定S序，M0/T27不变。 |
| Admission And Approval | T27 Win2000 常用控件既有授权。 |
| Objective | Win2000 Tab 与 Header，在原生项目布局和输入上应用年代呈现。 |
| Non-goals | 其他控件组、系统配置、系统窗口管理和跨进程入口。 |
| Reference Baseline | S7 同快照八件及84/84进程验证，已取回核对。 |
| Candidate Proposal | docs/proposals/windows-2000-controls.md；brief docs/history/M0-T27-S8-tab-header.md。 |
| Files And ABI Surface | Win2000 appearance、共享控件快照与所属者协议、交付探针；五组件八件及现有ABI。 |
| Applicable Rules | 两宽C17 GCC/Ninja/ccache、三层方向、保留原资源和独立工作区。 |
| Verification | 原版边缘、背景与选中/按下状态；标签方向、图文与禁用焦点、表头对齐/顺序/列宽、原生回调及移除；两宽实际启动。 |
| Expected Markers | 可见像素、原生状态、内容通知、绘图借用和清理有独立断言。 |
| Asset Needs | 固定 comctl32 tab/header 源码、实际可得原图。 |
| Reporting Requirements | 来源、支持范围、失败及未覆盖、实际包与远端、临时文件清理。 |
| Stop Conditions | 禁computer-use和私有桌面，保护现有改动，兄弟目录只读。 |
| Exit Criteria | 有限矩阵完成、同快照八件先发布后测、推送取回复核及清理后进入S9。 |
| Original Owner Request | Win2000常用控件按已有Win3.x实际范围对齐，采用固定源码和真实图像依据。 |
| Similar-Issue Sweep | 单行/多行、方向、图标、owner draw、回调文字、排序/拖列/过滤与热点、DPI。 |

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
- Last verified remote delivery: M0 T27 S7 P1，`d3a5dd4c`；35变更/八件取回核对。
- Last allocated T27 S: 8
- Open task: M0 T27 S8；Win2000标签页与表头。

## Technical Baseline

- 产品为[契约](../etc/product-contract.json)固定五组件八文件：run16s.exe、demo.exe、ntw16s32/64.dll、ntw2ks32/64.dll、ntwshk32/64.dll；正式文件在assets根与O:/winnt，当前摘要/版本/有限验证见[T27 S7证据](../etc/evidence/t27-s7-scroll.json)。
- 样式统一src/lib-win32-style的access/application/appearance；通用无样式Windows能力在src/lib-win32。当前结构和接口见[ARCHITECTURE](../design/ARCHITECTURE.md)及[CODING](../design/CODING.md)。
- 三接入、共同生命周期/窗口/菜单/控件协议/对话框/现代宿主/图形根已有真实消费者；年代保留度量、绘制、字体、资源与支持策略。产品保留CLI/托盘业务、目标/会话选择、原API trampoline、兼容ABI和装配。Win2000控件、其他框架与Luna/Aero不因公共根存在即得到完整支持。
- 来源1107回执、五原资源、包内原始栅格表和ABI/PE门禁通过；两宽各23公共样式归档与4进程归档无产品/年代反向依赖。所有当前构建/运行/测试不依赖或修改兄弟仓库。
- propagation停用、窗口恢复、thread hook解除与物理DLL/late patch卸载分别报告；原有进程存活期固定/保留能力不改称安全卸载。
- 禁止computer-use、创建或切换私有桌面；只用自有有界当前桌面探针。用户已永久授权非computer-use验证及按身份处理正式文件实际占用者；不重复请求同一授权，不改系统安全设置。
- 历史逐文件来源及迁移见[原顺序台账](../history/M0-T26-S1-ordered-audit.md)、各S证据与T26规划；早期lib-style/lib-win32/style路径是历史路径，当前旧生产目录已删除。
