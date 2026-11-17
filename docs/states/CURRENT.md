# Project Status

## Current Work

M0固定。用户已准入T27 Windows 2000常用控件，并授权自行设定目标持续执行；S1已完成有限parity/来源/接口设计冻结并实际远端复核，当前S2实现基础呈现与普通按钮，不提前称控件parity完成。按[16个S规划](../proposals/windows-2000-controls.md)以完整能力合批，先依据后实现，不逐文件完整交付。

T26三层重组已有限收口，正式八文件仍为S13同快照包，最终审计/已知失败见[记录](../etc/evidence/t26-s14-final-audit.json)。S1未修改产品/测试/构建输入，也未虚构新发布；Win2000旧分支关闭主题不计作独立renderer完成。

## M0 T27 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation；S1来源/设计冻结已实际远端复核，按已批准16S自动S2，M0/T27不变。 |
| Admission And Approval | 用户要求参考Win2000源代码、原资源和真实截图，目标达到现有Win3.x已实现控件范围的parity，授权自行设目标持续执行。 |
| Objective | 依据原版来源实现Win2000语义palette/edge与普通/默认按钮完整呈现，接入产品并保留原生输入/数据/资源所有权。 |
| Non-goals | 不凭印象画；不把关闭主题当实现；本T不做公共对话框/菜单、XAML客户区、新主题格式或扩权限/目标，不复制Win16机制。 |
| Reference Baseline | S1 P1 0742a00f，24控件组/8运行时组、32原文件和真实原图已冻结；两处仍T26八文件。参考Win2000SRC fd809a81只读。 |
| Candidate Proposal | docs/proposals/windows-2000-controls.md；brief docs/history/M0-T27-S2-buttons.md。 |
| Files And ABI Surface | appearance/win2000/common/control、ntw2ks装配、对应公共实际消费者/原生测试与CMake；固定五组件八文件及ABI，不改Win3.x字形。 |
| Applicable Rules | C17/MinGW GCC/Ninja/ccache，沿用access/application/appearance；固定来源/SHA/声明，无外部仓库构建依赖。 |
| Verification | 原版按钮状态/边缘/颜色/文字合同，原图可见像素对照，双宽原生实际安装/移除/输入/打印/重入/资源与Win16回归；先发布后实际包。 |
| Expected Markers | 实际公共根驱动Win2000按钮呈现；默认/普通/按下/禁用/焦点及适用内容有据，未验证分支明确保留，不靠关闭主题冒充完成。 |
| Asset Needs | 只读Win2000原源码及可核实截图；任何新增资源先逐项来源审查，保留已有五头/原声明；不加产品文件。 |
| Reporting Requirements | 实际盘点数、已确认/缺失依据、公共能力/产品/年代职责、S顺序和原始失败；不得把计划/原生保留称实现。 |
| Stop Conditions | 禁止computer-use、私有桌面、改兄弟/参考仓库和安全设置；保留工作区改动，不重启已有有效构建，不反复UAC。 |
| Exit Criteria | 源状态合同、真实产品消费者及适用验证落实，同快照双宽八文件两处先发布后测试、清理/提交推送实际复核后自动S3；T27保持开放。 |
| Original Owner Request | Win2000常用控件，目标Win3.x样式parity；必须看Win2000源代码/资源和真实图像，千万别猜，自行设目标开始执行。 |
| Similar-Issue Sweep | 原生基础/公共控件，VB/VCL/OCX与容器，独立/非客户滚动条，字体/颜色/位图/状态与demo，以及既有Win16保留模式。 |

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
- Last verified remote delivery: M0 T27 S1 P1，`0742a00f`；7个变更blob、无产品输入变化及原八文件实际复核。
- Last allocated T27 S: 2
- Open task: M0 T27 S2；Win2000基础呈现与普通按钮。

## Technical Baseline

- 产品为[契约](../etc/product-contract.json)固定五组件八文件：run16s.exe、demo.exe、ntw16s32/64.dll、ntw2ks32/64.dll、ntwshk32/64.dll；正式文件在assets根与O:/winnt，摘要见最终审计。
- 样式统一src/lib-win32-style的access/application/appearance；通用无样式Windows能力在src/lib-win32。当前结构和接口见[ARCHITECTURE](../design/ARCHITECTURE.md)及[CODING](../design/CODING.md)。
- 三接入、共同生命周期/窗口/菜单/控件协议/对话框/现代宿主/图形根已有真实消费者；年代保留度量、绘制、字体、资源与支持策略。产品保留CLI/托盘业务、目标/会话选择、原API trampoline、兼容ABI和装配。Win2000控件、其他框架与Luna/Aero不因公共根存在即得到完整支持。
- 来源1077回执、五原资源、包内原始栅格表和ABI/PE门禁通过；两宽各23公共样式归档与4进程归档无产品/年代反向依赖。所有当前构建/运行/测试不依赖或修改兄弟仓库。
- propagation停用、窗口恢复、thread hook解除与物理DLL/late patch卸载分别报告；原有进程存活期固定/保留能力不改称安全卸载。
- 禁止computer-use、创建或切换私有桌面；只用自有有界当前桌面探针。用户已永久授权非computer-use验证及按身份处理正式文件实际占用者；不重复请求同一授权，不改系统安全设置。
- 历史逐文件来源及迁移见[原顺序台账](../history/M0-T26-S1-ordered-audit.md)、各S证据与T26规划；早期lib-style/lib-win32/style路径是历史路径，当前旧生产目录已删除。
