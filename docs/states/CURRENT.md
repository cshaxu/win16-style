# Project Status

## Current Work

M0固定。T27 Win2000常用控件保持开放；S1来源冻结、S2基础呈现/普通按钮已按有限范围交付关闭，当前S3复选/单选/三态。按[16个S规划](../proposals/windows-2000-controls.md)合批，不逐文件完整交付；不提前称所有控件parity完成。

正式八文件已更新为[T27 S3已发布/测试批次](../etc/evidence/t27-s3-choice-controls.json)，36/36聚焦通过，当前待S3 P1推送/实际远端复核；[上次实际远端](../etc/evidence/t27-s2-remote.json)38变更与八文件字节一致，9个旧输入行尾差异披露。T26已知失败保留在[终审](../etc/evidence/t26-s14-final-audit.json)，未被本组局部通过替代。Win2000其他关闭主题分支不计独立renderer完成。

## M0 T27 S3 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation；S2有限条件完成并实际远端复核，按既定顺序S3，M0/T27不变。 |
| Admission And Approval | 用户准入Win2000常用控件并要求参照原源码/原资源/真实图片，自行设目标持续执行，现按16S规划推进。 |
| Objective | 实现有原版依据的checkbox/radio/3state及适用pushlike，复用S2控件对象和公共生命周期/原生输入。 |
| Non-goals | 不猜字形，不复制Win16状态/位图，不做XAML客户区、新对话框/菜单或扩权限/目标；不声称原生保留为年代实现。 |
| Reference Baseline | S2 P1 4912b70d；双宽14项增量及36项实际发布包通过，原Win2000SRC fd809a81只读，S1 parity清单继续有效。 |
| Candidate Proposal | docs/proposals/windows-2000-controls.md；brief docs/history/M0-T27-S3-choice-controls.md。 |
| Files And ABI Surface | appearance/win2000/common/control、ntw2ks装配与对应原生测试/CMake；既有五组件八文件及公开ABI保持。 |
| Applicable Rules | 纯C17、MinGW GCC/Ninja/ccache；三层方向、唯一共同状态/原资源声明，源参考不成为外部构建依赖。 |
| Verification | 原版状态/字形/布局、真实图像可见状态；v5/v6/aware与双宽实际安装/输入/资源/恢复/重入/启动，旧push回归；先发布后实际包。 |
| Expected Markers | 六类选择控件与适用pushlike真实呈现/原生状态相容；类型/状态子用例与进程用例区分，所有未覆盖/原生保留如实列出。 |
| Asset Needs | Win2000原字形资源/构造及可核实真实图像；缺原Marlett不得假称已取得，不新增未审查替代图形。 |
| Reporting Requirements | 来源/代码/产品调用/实际计数/原始失败、清理与交付证据；消息输入不冒称物理输入，skip不算通过。 |
| Stop Conditions | 禁computer-use、创建/切换隔离桌面、兄弟仓库改动/依赖和安全设置变更；保护已有工作，不重启有效构建，不反复UAC。 |
| Exit Criteria | 本组有限矩阵实际接入、适用验证、同快照双宽八文件两处先发布后测试、即时清理/提交推送/实际复核后自动S4，T27开放。 |
| Original Owner Request | 目标Win3.x控件parity，必须看Win2000源码/原资源和真实图片，不要自己捏造；自行设目标执行。 |
| Similar-Issue Sweep | checked/mixed/pressed/disabled/focus、UI提示、自动分组/通知、pushlike、标签/字体/图像/背景、角色变化与借用资源。 |

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
- Last verified remote delivery: M0 T27 S2 P1，`4912b70d`；38变更/八文件实际fetch字节复核，旧9行尾差异披露。
- Last allocated T27 S: 3
- Open task: M0 T27 S3；Win2000复选、单选和三态。

## Technical Baseline

- 产品为[契约](../etc/product-contract.json)固定五组件八文件：run16s.exe、demo.exe、ntw16s32/64.dll、ntw2ks32/64.dll、ntwshk32/64.dll；正式文件在assets根与O:/winnt，当前摘要/版本/有限验证见[T27 S3证据](../etc/evidence/t27-s3-choice-controls.json)。
- 样式统一src/lib-win32-style的access/application/appearance；通用无样式Windows能力在src/lib-win32。当前结构和接口见[ARCHITECTURE](../design/ARCHITECTURE.md)及[CODING](../design/CODING.md)。
- 三接入、共同生命周期/窗口/菜单/控件协议/对话框/现代宿主/图形根已有真实消费者；年代保留度量、绘制、字体、资源与支持策略。产品保留CLI/托盘业务、目标/会话选择、原API trampoline、兼容ABI和装配。Win2000控件、其他框架与Luna/Aero不因公共根存在即得到完整支持。
- 来源1088回执、五原资源、包内原始栅格表和ABI/PE门禁通过；两宽各23公共样式归档与4进程归档无产品/年代反向依赖。所有当前构建/运行/测试不依赖或修改兄弟仓库。
- propagation停用、窗口恢复、thread hook解除与物理DLL/late patch卸载分别报告；原有进程存活期固定/保留能力不改称安全卸载。
- 禁止computer-use、创建或切换私有桌面；只用自有有界当前桌面探针。用户已永久授权非computer-use验证及按身份处理正式文件实际占用者；不重复请求同一授权，不改系统安全设置。
- 历史逐文件来源及迁移见[原顺序台账](../history/M0-T26-S1-ordered-audit.md)、各S证据与T26规划；早期lib-style/lib-win32/style路径是历史路径，当前旧生产目录已删除。
