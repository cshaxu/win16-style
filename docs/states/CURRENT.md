# Project Status

## Current Work

M0固定。T27 Win2000常用控件保持开放；S1-S6来源/按钮/选择/静态分组/编辑/列表组合已按有限范围交付关闭，当前S7滚动条与尺寸角。按[16个S规划](../proposals/windows-2000-controls.md)合批，不逐文件完整交付；不提前称所有控件parity完成。

正式八文件为[T27 S7滚动条批次](../etc/evidence/t27-s7-scroll.json)，两宽同快照，先发布后84/84进程用例通过，既有36项客户区打印子检查明确跳过。实际远端复核待完成。T26已知失败保留在[终审](../etc/evidence/t26-s14-final-audit.json)，未被本组局部通过替代。Win2000其他关闭主题分支不计独立renderer完成。

## M0 T27 S7 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation；按既定S序，M0/T27不变。 |
| Admission And Approval | T27 Win2000 常用控件既有授权。 |
| Objective | Win2000 独立滚动条、窗口与控件非客户区滚动条、尺寸角。 |
| Non-goals | 全局和进程传播、其他控件组及系统配置。 |
| Reference Baseline | S6 同快照八件及76/76进程验证，已取回核对。 |
| Candidate Proposal | docs/proposals/windows-2000-controls.md；brief docs/history/M0-T27-S7-scroll.md。 |
| Files And ABI Surface | Win2000 appearance、共同滚动控制器和交付探针；五组件八件及现有ABI。 |
| Applicable Rules | 两宽C17 GCC/Ninja/ccache、三层方向、保留原资源和独立工作区。 |
| Verification | 原版箭头、轨道、滑块及尺寸角，横纵、禁用、短轴、页比例、拖动/点击/滚轮、借用DC与移除；两宽普通启动。 |
| Expected Markers | 公开状态/几何、画面、原生通知和清理分别有断言。 |
| Asset Needs | 固定Win2000SRC sbctl/draw和真实截图，记录适用证据。 |
| Reporting Requirements | 来源、支持范围、失败及未覆盖、实际包与远端、临时文件清理。 |
| Stop Conditions | 禁computer-use和私有桌面，保护现有改动，兄弟目录只读。 |
| Exit Criteria | 有限矩阵完成、同快照八件先发布后测、推送取回复核及清理后进入S8。 |
| Original Owner Request | Win2000常用控件按已有Win3.x实际范围对齐，采用固定源码和真实图像依据。 |
| Similar-Issue Sweep | 独立/内嵌、横纵、短轨/禁用/无滑块、滚轮/禁用捕获/程序更新、尺寸区和背景。 |

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
- Last verified remote delivery: M0 T27 S6 P1，`a1852441`；29变更/八件取回核对。
- Last allocated T27 S: 7
- Open task: M0 T27 S7；Win2000滚动条与尺寸角。

## Technical Baseline

- 产品为[契约](../etc/product-contract.json)固定五组件八文件：run16s.exe、demo.exe、ntw16s32/64.dll、ntw2ks32/64.dll、ntwshk32/64.dll；正式文件在assets根与O:/winnt，当前摘要/版本/有限验证见[T27 S7证据](../etc/evidence/t27-s7-scroll.json)。
- 样式统一src/lib-win32-style的access/application/appearance；通用无样式Windows能力在src/lib-win32。当前结构和接口见[ARCHITECTURE](../design/ARCHITECTURE.md)及[CODING](../design/CODING.md)。
- 三接入、共同生命周期/窗口/菜单/控件协议/对话框/现代宿主/图形根已有真实消费者；年代保留度量、绘制、字体、资源与支持策略。产品保留CLI/托盘业务、目标/会话选择、原API trampoline、兼容ABI和装配。Win2000控件、其他框架与Luna/Aero不因公共根存在即得到完整支持。
- 来源1107回执、五原资源、包内原始栅格表和ABI/PE门禁通过；两宽各23公共样式归档与4进程归档无产品/年代反向依赖。所有当前构建/运行/测试不依赖或修改兄弟仓库。
- propagation停用、窗口恢复、thread hook解除与物理DLL/late patch卸载分别报告；原有进程存活期固定/保留能力不改称安全卸载。
- 禁止computer-use、创建或切换私有桌面；只用自有有界当前桌面探针。用户已永久授权非computer-use验证及按身份处理正式文件实际占用者；不重复请求同一授权，不改系统安全设置。
- 历史逐文件来源及迁移见[原顺序台账](../history/M0-T26-S1-ordered-audit.md)、各S证据与T26规划；早期lib-style/lib-win32/style路径是历史路径，当前旧生产目录已删除。
