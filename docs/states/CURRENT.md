# Project Status

## Current Work

M0 固定。T27 Win2000 常用控件及 S1–S16 已按有限范围收口。
[终审](../history/M0-T27-S16-final-audit.md)逐项核对 24 个标准组和 8 个运行库组。

[S15 同快照八件](../etc/evidence/t27-s15-demo.json)先发布两处，185/185 项聚焦用例通过；
36 项原生客户区离屏打印子检查保留跳过。实际管理员托盘、物理输入、
其他框架和复杂变体按终审证据保留范围。

M0 T28 S2 WinUI Windows 2000 桌面外框已收口。WinUI 记事本和 Paint 验证了共用外框；合成恢复发布包
验证记事本的原始标题像素、窗口、客户区和 DWM 可见边界完全恢复。计算器与
Scan 同批复验覆盖 Win2000 普通、最大化按钮，计算器默认 Win16 通过。
既有 Win32 失败由夹具窗口超过窄 RDP 工作区触发，按可容纳尺寸复测后，
双宽直接、协调器移除都通过，断言保持。适用原生用例共 33/33。
八件正式文件及远端 P1 摘要一致，P3 是夹具及收口记录。
范围与限制见 [S2 记录](../history/M0-T28-S2-winui-frame.md)和
[实包证据](../etc/evidence/t28-s2-composition-delivery.json)。

M0 T28 S3 已准入，目标为 Windows 2000 公共对话框的功能对等。从原版 SP2
安装盘只读核对了 COMDLG32 和 SHELL32 模板。现有打开／保存的双宽取消基线
16/16，带诊断的确认对照也为双宽 16/16。普通 Shell 目录选择的发布基线通过。
兼容的 COM 目录请求接到经典目录树，兼容的 COM 文件请求接到原生 Explorer
型 Win32 文件框，两者共用参数和结果转换。P1 候选同快照八件已先发布至
两处，并完成远端核对。发布版普通桌面 42/42 组、核心 24/24、Shell 目录
16/16、扩展打印设置确认和 Apply 对照 8/8。
P5 修复原生色板展开时复用旧外框尺寸的问题，双宽 8/8 恢复 298×184 DLU
客户区，保留 RGB 和自定义色槽回写。消息框的警告图标、是／否／取消和
默认按钮双宽 12/12。新八件包先发布两处，普通桌面 46/46 组及核心 30/30。
普通宿主的页面设置和 PrintDlgEx DC 各通过 4/4。S3 仍开放。原版 Explorer
完整布局、扩展打印属性页和管理员全局路径仍待验证，当前 427×272 工作区
会裁切带常用位置栏的经典 Explorer，未换肤时也能复现。P7 补强发布版文件
选择契约 16/16，双宽真实色板在本地全局样式适配器应用和撤销后均保持尺寸
与回写。绕过窗口尺寸钳制仍无法让原生 Explorer 子控件落回客户区。
P8 发布版文件缓冲区不足回写 8/8。应用自定义打印属性页在双宽原生与
`--2k` 对照均只观察到系统代理窗，尚未验证属性页实际显示。
P9 紧凑 Explorer 已通过增量验证，在 427×272 工作区内原生重排目录视图、
文件名框和按钮，保留调用方参数和结果回写。同快照八件已发布两处，发布版
紧凑几何 8/8、桌面 48/48 组和核心 30/30。带常用位置栏的完整 Explorer、
扩展打印属性页和管理员全局路径仍开放。
常用位置栏重排原型在同一工作区双宽触发原生导航 8/8；导航后选定文件
首次回写 6/8；关闭重排也复现 x86 打开失败。显式返回测试目录后，开启重排的
双宽八种组合均正确回写。Shell 切换目录会替换目录视图，重新读取句柄后新版
视图均在客户区内。窄工作区的原生图标和文字无法容纳于压缩行高；按原版模板
客户区做几何对照时，双宽八种组合的两轮缩放和导航均保持控件完整。
P14 已交付带常用位置栏的打开／保存及 COM 复用路径。以测试工作区尺寸验证
完整版双宽 Win32 8/8、COM 8/8；正式八件发布两处后，普通桌面 48/48 组、
同包原生 14/14 组、核心 42/42。实际大工作区与其他 DPI、扩展打印属性页和
管理员全局路径仍待验，S3 开放。
P15 补充 `PrintDlgEx` 默认打印机回退，原生、Win16、Win2000 双宽 A/W 共
12/12。可见 owner 和调用方 General 模板均未让系统代理显示实际属性页；
无沙箱普通会话的对照结果一致。
P16 复用 Win16 的扩展打印夹具，发布版 Win16／Win2000 合计 188/188。
包括范围、当前页、Apply、调用方设置句柄和设备上下文，以及实际切换本地打印机后
返回设备名称。系统 General 属性页和打印作业仍无验收证据。
P17 修正 `ChooseFont` 加框后的尺寸偏差，由 433×325 恢复至原版模板换算的
431×319。八件已发布两处，原生及双风格字体 24/24，同包 Win2000 复验 8/8，
普通桌面 48/48 组、核心 42/42。命名模板保留原有行为。
P18 用调用方固定内存模板补齐 `ChooseFont` 正例。原生、Win16 和 Win2000 的双宽
A/W、确认和取消共 24/24，标准模板回归另通过 24/24。模板尺寸与回调保留，调用方
均能释放内存。
P19 验证文件对话框原生回退。双宽原生、Win16、Win2000 的 Win32／COM 多选
60/60，Explorer 内存模板和首次确认拒绝后再确认／取消 48/48。会话仍断开，工作区
427×272，实际大工作区、其他 DPI、扩展打印属性页和管理员全局验收仍开放。
P20 用发布版验证 `MessageBoxIndirect` 的调用方图标、帮助回调、默认按钮及
返回值，原生和两种风格的双宽 A/W 共 12/12。发布 DLL 的本地颜色对话框
应用／撤销也通过双宽复验，客户区和 RGB 回写保持。八件摘要未变。
P21 用同一份自有属性页做双宽对照。`PropertySheetW` 正常创建、初始化、
释放页面；原生和发布版 `--2k` 的 `PrintDlgExW` 均只显示系统代理窗，
未到达应用属性页创建回调。等可用桌面复验实际 General 及调用方属性页。
P22 修复 144 DPI 下紧凑 Explorer 保存对话框超出工作区一个物理像素。
双宽同批包先发布两处，三处实际文件回写 96/96，其中紧凑几何实测 48/48。
文件缓冲区 8/8，其他对话框回归 32 组，核心 20/20。物理工作区为
640×408（144 DPI），完整版 Places 按真实工作区明确跳过。
[参考与开发证据](../etc/evidence/t28-s3-dialog-reference.json)

## M0 T28 S3 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation；T28 新增 S3。 |
| Admission And Approval | 用户要求添加 S、自行设定目标并覆盖公共对话框。 |
| Objective | Win2000 公共对话框对齐 Win16 已交付入口及调用能力，呈现符合 Windows 2000 原版样本。 |
| Non-goals | XAML 客户区、系统主题、修改注册表、未知应用私有模板及扩大进程接入范围。 |
| Reference Baseline | T20 收口审计、T27 控件终审及 T28 S2 已发布八件。 |
| Candidate Proposal | docs/proposals/windows-2000-common-dialogs.md。 |
| Files And ABI Surface | application/dialog 共用契约、Win2000 对话框呈现、ntw2ks 装配及相应测试。维持五组件八件。 |
| Applicable Rules | C17、MinGW GCC、Ninja、ccache；先增量验证，再进行同快照双宽交付。 |
| Verification | 自有普通桌面双宽样本，逐类验证正常操作、取消、结果及资源所有权，按证据保留原生回退，并复测 Win16。 |
| Expected Markers | 记录实际入口、拥有者、所用布局、确认／取消、返回值和回退路径，提供原版视觉对照。 |
| Asset Needs | 从固定本地 Win2000 源码和实机样本取证；保留原图形资源、来源与声明。 |
| Reporting Requirements | 逐入口说明呈现、契约和验证条件；区分通过、回退、跳过和未覆盖。 |
| Stop Conditions | 禁止 computer-use 和私有桌面，仅启动自有有界样本，保留既有权限和系统配置。 |
| Exit Criteria | 提案各组的适用功能及 Win16 回归已验证，两处同包交付后验证，并完成提交、推送和实际远端复核。 |
| Original Owner Request | 添加 S 任务，自行设定目标，覆盖公共对话框。 |
| Similar-Issue Sweep | 核对 T20 文件多选、目录树、COM 回写、模板／回调回退及打印驱动路径。 |

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 28
- Last allocated Td S: 21
- Last allocated T18 S: 64
- Last allocated T19 S: 3
- Last allocated T20 S: 19
- Last allocated T21 S: 44
- Last allocated T25 S: 8
- Last allocated T26 S: 14
- Last verified remote delivery: M0 T28 S3 P22；构建快照和八件摘要见公共对话框证据。
- Last allocated T27 S: 16
- Last allocated T28 S: 3
- Open task: M0 T28 S3；Windows 2000 公共对话框。

## Technical Baseline

- 产品为[契约](../etc/product-contract.json)固定五组件八文件：run16s.exe、demo.exe、ntw16s32/64.dll、ntw2ks32/64.dll、ntwshk32/64.dll；远端、O:/winnt 和 assets 已核对 [T28 S3 公共对话框候选](../etc/evidence/t28-s3-dialog-reference.json)。
- 样式统一src/lib-win32-style的access/application/appearance；通用无样式Windows能力在src/lib-win32。当前结构和接口见[ARCHITECTURE](../design/ARCHITECTURE.md)及[CODING](../design/CODING.md)。
- 三接入、共同生命周期/窗口/菜单/控件协议/对话框/现代宿主/图形根已有真实消费者；年代保留度量、绘制、字体、资源与支持策略。产品保留CLI/托盘业务、目标/会话选择、原API trampoline、兼容ABI和装配。Win2000控件、其他框架与Luna/Aero不因公共根存在即得到完整支持。
- 来源1135回执、五原资源、包内原始栅格表和ABI/PE门禁通过；两宽各23公共样式归档与4进程归档无产品/年代反向依赖。所有当前构建/运行/测试不依赖或修改兄弟仓库。
- propagation停用、窗口恢复、thread hook解除与物理DLL/late patch卸载分别报告；原有进程存活期固定/保留能力不改称安全卸载。
- 禁止computer-use、创建或切换私有桌面；只用自有有界当前桌面探针。用户已永久授权非computer-use验证及按身份处理正式文件实际占用者；不重复请求同一授权，不改系统安全设置。
- 历史逐文件来源及迁移见[原顺序台账](../history/M0-T26-S1-ordered-audit.md)、各S证据与T26规划；早期lib-style/lib-win32/style路径是历史路径，当前旧生产目录已删除。
