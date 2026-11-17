# Project Status

## Current Work

M0固定。T27 Win2000常用控件保持开放；S1-S4来源/按钮/选择/静态分组已按有限范围交付关闭，当前S5编辑/RichEdit/HotKey。按[16个S规划](../proposals/windows-2000-controls.md)合批，不逐文件完整交付；不提前称所有控件parity完成。

正式八文件已更新为[T27 S4已发布/测试批次](../etc/evidence/t27-s4-static-group.json)，48/48聚焦通过；[实际远端](../etc/evidence/t27-s4-remote.json)30变更与八文件字节一致、1187输入仅9个旧行尾差异披露。T26已知失败保留在[终审](../etc/evidence/t26-s14-final-audit.json)，未被本组局部通过替代。Win2000其他关闭主题分支不计独立renderer完成。

## M0 T27 S5 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation；S4有限条件完成并实际远端复核，自动按既定顺序S5，M0/T27不变。 |
| Admission And Approval | 用户准入Win2000常用控件并要求原源码/原资源/真实图片，授权自行设目标按16S持续执行。 |
| Objective | 标准EDIT与已有RichEdit/HotKey范围的Win2000原版边缘/适用颜色，复用原生编辑和共同生命周期/消息机制。 |
| Non-goals | 不替换编辑器/富文本布局/IME，不复制输入/滚动状态或Win16绘制，不扩全局/宿主/权限/未知框架，未完成滚动由S7接收。 |
| Reference Baseline | S4 P1 802fe1fe同源双宽八文件先发布48/48实际包，30变更/八文件和1187输入实际远端核对；Win2000SRC fd809a81只读。 |
| Candidate Proposal | docs/proposals/windows-2000-controls.md；brief docs/history/M0-T27-S5-edit-controls.md。 |
| Files And ABI Surface | appearance/win2000/control与公共edit/NC协议的实际复用、ntw2ks装配和对应测试/CMake；既有五组件八文件/公开ABI保持。 |
| Applicable Rules | 纯C17、MinGW GCC/Ninja/ccache；三层方向、唯一公共状态、原图形/声明保留，兄弟仓库只读且无构建依赖。 |
| Verification | 原版边缘/颜色与文字/选择/undo/只读/禁用/密码保持，字体/刷/格式/打印裁剪及重入/资源/恢复；双宽v5/v6/aware及标准启动和旧控件回归。 |
| Expected Markers | 本组有限编辑呈现与真实原生编辑消费者；安装/呈现/输入/恢复分列，不把原生保留和消息用例冒称全年代或物理支持。 |
| Asset Needs | 按需固定源文件/原资源和可核实真实图片；没有原字形则如实说明，不生成近似替代。 |
| Reporting Requirements | 来源/设计/调用方、有限状态和实际运行、失败/skip/资源/清理、同快照八文件摘要与实际远端；不夸大全部控件parity。 |
| Stop Conditions | 禁computer-use和创建/切换私有桌面、改兄弟仓库/安全设置或扩大权限；保护已有改动，不重启有效worker或反复UAC。 |
| Exit Criteria | 有限矩阵实际接入/验证，双宽必要干净构建，八文件两处先发布后测试，提交推送/实际复核与即时清理后自动S6，T27开放。 |
| Original Owner Request | Win2000常用控件对齐已有Win3.x有限parity；必须原源码/原资源/真实图片，不瞎猜，持续自行执行。 |
| Similar-Issue Sweep | 单/多行/只读/禁用/密码、客户端/非客户端边缘、应用颜色/字体/富文本与HotKey值、打印坐标及重入/退出/恢复。 |

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
- Last verified remote delivery: M0 T27 S4 P1，`802fe1fe`；30变更/八文件实际fetch字节复核，旧9行尾差异披露。
- Last allocated T27 S: 5
- Open task: M0 T27 S5；Win2000 EDIT与已有RichEdit/HotKey范围。

## Technical Baseline

- 产品为[契约](../etc/product-contract.json)固定五组件八文件：run16s.exe、demo.exe、ntw16s32/64.dll、ntw2ks32/64.dll、ntwshk32/64.dll；正式文件在assets根与O:/winnt，当前摘要/版本/有限验证见[T27 S4证据](../etc/evidence/t27-s4-static-group.json)。
- 样式统一src/lib-win32-style的access/application/appearance；通用无样式Windows能力在src/lib-win32。当前结构和接口见[ARCHITECTURE](../design/ARCHITECTURE.md)及[CODING](../design/CODING.md)。
- 三接入、共同生命周期/窗口/菜单/控件协议/对话框/现代宿主/图形根已有真实消费者；年代保留度量、绘制、字体、资源与支持策略。产品保留CLI/托盘业务、目标/会话选择、原API trampoline、兼容ABI和装配。Win2000控件、其他框架与Luna/Aero不因公共根存在即得到完整支持。
- 来源1094回执、五原资源、包内原始栅格表和ABI/PE门禁通过；两宽各23公共样式归档与4进程归档无产品/年代反向依赖。所有当前构建/运行/测试不依赖或修改兄弟仓库。
- propagation停用、窗口恢复、thread hook解除与物理DLL/late patch卸载分别报告；原有进程存活期固定/保留能力不改称安全卸载。
- 禁止computer-use、创建或切换私有桌面；只用自有有界当前桌面探针。用户已永久授权非computer-use验证及按身份处理正式文件实际占用者；不重复请求同一授权，不改系统安全设置。
- 历史逐文件来源及迁移见[原顺序台账](../history/M0-T26-S1-ordered-audit.md)、各S证据与T26规划；早期lib-style/lib-win32/style路径是历史路径，当前旧生产目录已删除。
