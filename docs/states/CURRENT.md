# Project Status

## Current Work

M0固定。T27 Win2000常用控件保持开放；S1来源冻结、S2基础呈现/普通按钮、S3复选/单选/三态已按有限范围交付关闭，当前S4静态/分组/容器。按[16个S规划](../proposals/windows-2000-controls.md)合批，不逐文件完整交付；不提前称所有控件parity完成。

正式八文件已更新为[T27 S3已发布/测试批次](../etc/evidence/t27-s3-choice-controls.json)，36/36聚焦通过，[实际远端](../etc/evidence/t27-s3-remote.json)32变更与八文件字节一致，9个旧输入行尾差异披露。T26已知失败保留在[终审](../etc/evidence/t26-s14-final-audit.json)，未被本组局部通过替代。Win2000其他关闭主题分支不计独立renderer完成。

## M0 T27 S4 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation；S3有限条件完成并实际远端复核，自动按既定顺序S4，M0/T27不变。 |
| Admission And Approval | 用户准入Win2000常用控件，要求原源码/原资源/真实图片，授权自行设目标持续按16S执行。 |
| Objective | 原生STATIC和GROUPBOX有据呈现，保持图像/父背景/字体/子内容；接通既有对话框完成观察器的已创建子控件。 |
| Non-goals | 不复制Win16 renderer/机制，不扩大owner-draw/未知框架/XAML客户区/全局目标/权限，不宣称全部控件parity。 |
| Reference Baseline | S3 P1 b4ca7bf2同源双宽八文件先发布36/36实际包，32变更/八文件实际远端字节验证；Win2000SRC fd809a81只读。 |
| Candidate Proposal | docs/proposals/windows-2000-controls.md；brief docs/history/M0-T27-S4-static-group.md。 |
| Files And ABI Surface | appearance/win2000/common/control、ntw2ks对话框装配、对应测试/CMake；既有五组件八文件和公开ABI保持。 |
| Applicable Rules | 纯C17、MinGW GCC/Ninja/ccache；三层方向、唯一公共状态，原图形及声明保留，兄弟仓库只读且无构建依赖。 |
| Verification | 原版static/group布局色层、应用文字/字体/图像/背景/子内容，重入/资源/恢复；双宽v5/v6/aware和真实DialogManager普通启动，旧按钮/Win16回归。 |
| Expected Markers | 本组有据呈现、原生输入和资源借还，已创建对话框子控件实际附着；独立原语/产品组合与原生保留分列。 |
| Asset Needs | 固定原源码及已核实真实Mouse/Effects等截图；缺原字体/资源如实报告，不生成近似替代图形。 |
| Reporting Requirements | 来源/文件/生产调用/有限覆盖、原始失败/skip、实际八文件摘要、清理及实际远端；消息不冒称物理操作。 |
| Stop Conditions | 禁computer-use及创建/切换私有桌面、改兄弟仓库/安全设置、扩大权限或目标；保护已有工作，不重启有效构建。 |
| Exit Criteria | 有限矩阵实际接入、双宽必要干净构建、八文件两处先发布后实际测试、提交推送/实际复核和即时清理后自动S5，T27保持开放。 |
| Original Owner Request | Win2000常用控件对齐已有Win3.x有限parity；必须看原源码/原资源与真实图片，不瞎猜，自行持续执行。 |
| Similar-Issue Sweep | 静态子型/文字布局/禁用/UI提示、分组标题/透明内部、图像与应用刷、已创建子控件、重入换父/销毁和恢复。 |

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
- Last verified remote delivery: M0 T27 S3 P1，`b4ca7bf2`；32变更/八文件实际fetch字节复核，旧9行尾差异披露。
- Last allocated T27 S: 4
- Open task: M0 T27 S4；Win2000静态、分组和高频容器。

## Technical Baseline

- 产品为[契约](../etc/product-contract.json)固定五组件八文件：run16s.exe、demo.exe、ntw16s32/64.dll、ntw2ks32/64.dll、ntwshk32/64.dll；正式文件在assets根与O:/winnt，当前摘要/版本/有限验证见[T27 S3证据](../etc/evidence/t27-s3-choice-controls.json)。
- 样式统一src/lib-win32-style的access/application/appearance；通用无样式Windows能力在src/lib-win32。当前结构和接口见[ARCHITECTURE](../design/ARCHITECTURE.md)及[CODING](../design/CODING.md)。
- 三接入、共同生命周期/窗口/菜单/控件协议/对话框/现代宿主/图形根已有真实消费者；年代保留度量、绘制、字体、资源与支持策略。产品保留CLI/托盘业务、目标/会话选择、原API trampoline、兼容ABI和装配。Win2000控件、其他框架与Luna/Aero不因公共根存在即得到完整支持。
- 来源1088回执、五原资源、包内原始栅格表和ABI/PE门禁通过；两宽各23公共样式归档与4进程归档无产品/年代反向依赖。所有当前构建/运行/测试不依赖或修改兄弟仓库。
- propagation停用、窗口恢复、thread hook解除与物理DLL/late patch卸载分别报告；原有进程存活期固定/保留能力不改称安全卸载。
- 禁止computer-use、创建或切换私有桌面；只用自有有界当前桌面探针。用户已永久授权非computer-use验证及按身份处理正式文件实际占用者；不重复请求同一授权，不改系统安全设置。
- 历史逐文件来源及迁移见[原顺序台账](../history/M0-T26-S1-ordered-audit.md)、各S证据与T26规划；早期lib-style/lib-win32/style路径是历史路径，当前旧生产目录已删除。
