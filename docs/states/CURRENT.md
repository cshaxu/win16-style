# Project Status

## Current Work

M0固定。用户已准入T27 Windows 2000常用控件，并授权自行设定目标持续执行；当前S1建立Win3.x实际parity清单、Win2000源码/资源/真实图像依据与实施设计，不称控件呈现已完成。按[16个S规划](../proposals/windows-2000-controls.md)以完整能力合批，先依据后实现，不逐文件完整交付。

T26三层重组已有限收口，正式八文件仍为S13同快照包，最终审计/已知失败见[记录](../etc/evidence/t26-s14-final-audit.json)。T27当前未修改产品/测试/构建输入，也未虚构新发布；Win2000旧分支关闭主题不计作独立renderer完成。

## M0 T27 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New；用户准入原建议第二项Win2000常用控件，T27 S1，M0固定。 |
| Admission And Approval | 用户要求参考Win2000源代码、原资源和真实截图，目标达到现有Win3.x已实现控件范围的parity，授权自行设目标持续执行。 |
| Objective | 冻结完整parity/状态清单、原版来源/资源/图像与公共机制/年代呈现设计，形成有证据的后续实施和验收矩阵。 |
| Non-goals | 不凭印象画；不把关闭主题当实现；本T不做公共对话框/菜单、XAML客户区、新主题格式或扩权限/目标，不复制Win16机制。 |
| Reference Baseline | efcf9109，T26最终三层/八文件；参考Win2000SRC fd809a81098565b33f52d0f65925159de8f4c337，只读且无本地改动。 |
| Candidate Proposal | docs/proposals/windows-2000-controls.md；brief docs/history/M0-T27-S1-controls-reference.md。 |
| Files And ABI Surface | 当前appearance/win3x/control及实际公共/产品消费者与demo，参考ntuser/comctl32；S1仅研究文档，不改五组件八文件/ABI。 |
| Applicable Rules | C17/MinGW GCC/Ninja/ccache，沿用access/application/appearance；固定来源/SHA/声明，无外部仓库构建依赖。 |
| Verification | 逐控件原入口/支持范围、Win2000文件/函数/资源与图像对照；完整状态/真实与模型/缺图/未覆盖分列，治理与现有包摘要核对。 |
| Expected Markers | 完整有限清单、原版依据、接入设计和验证判据；没有依据的状态不捏造，没有原版等价的现代控件单列。 |
| Asset Needs | 只读Win2000原源码及可核实截图；任何新增资源先逐项来源审查，保留已有五头/原声明；不加产品文件。 |
| Reporting Requirements | 实际盘点数、已确认/缺失依据、公共能力/产品/年代职责、S顺序和原始失败；不得把计划/原生保留称实现。 |
| Stop Conditions | 禁止computer-use、私有桌面、改兄弟/参考仓库和安全设置；保留工作区改动，不重启已有有效构建，不反复UAC。 |
| Exit Criteria | S1清单与设计有据且可审计，现有八文件不变证据、治理/diff检查及提交推送实际复核后自动S2；本T按16S继续。 |
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
- Last verified remote delivery: M0 T26 S14 P1，`9d5ea6ea`；9个变更blob、1170冻结输入及八文件实际复核。
- Last allocated T27 S: 1
- Open task: M0 T27 S1；Win2000常用控件parity、原版依据与设计。

## Technical Baseline

- 产品为[契约](../etc/product-contract.json)固定五组件八文件：run16s.exe、demo.exe、ntw16s32/64.dll、ntw2ks32/64.dll、ntwshk32/64.dll；正式文件在assets根与O:/winnt，摘要见最终审计。
- 样式统一src/lib-win32-style的access/application/appearance；通用无样式Windows能力在src/lib-win32。当前结构和接口见[ARCHITECTURE](../design/ARCHITECTURE.md)及[CODING](../design/CODING.md)。
- 三接入、共同生命周期/窗口/菜单/控件协议/对话框/现代宿主/图形根已有真实消费者；年代保留度量、绘制、字体、资源与支持策略。产品保留CLI/托盘业务、目标/会话选择、原API trampoline、兼容ABI和装配。Win2000控件、其他框架与Luna/Aero不因公共根存在即得到完整支持。
- 来源1077回执、五原资源、包内原始栅格表和ABI/PE门禁通过；两宽各23公共样式归档与4进程归档无产品/年代反向依赖。所有当前构建/运行/测试不依赖或修改兄弟仓库。
- propagation停用、窗口恢复、thread hook解除与物理DLL/late patch卸载分别报告；原有进程存活期固定/保留能力不改称安全卸载。
- 禁止computer-use、创建或切换私有桌面；只用自有有界当前桌面探针。用户已永久授权非computer-use验证及按身份处理正式文件实际占用者；不重复请求同一授权，不改系统安全设置。
- 历史逐文件来源及迁移见[原顺序台账](../history/M0-T26-S1-ordered-audit.md)、各S证据与T26规划；早期lib-style/lib-win32/style路径是历史路径，当前旧生产目录已删除。
