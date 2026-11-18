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
16/16、扩展打印设置确认和 Apply 对照 8/8。原生系统对照同样无法取得打印机 DC。
P3 发布版展开色板双宽 8/8，验证 RGB 回写、取消以及两个自定义色槽的列序。
可读色块像素及凹边与本机原生对照一致，展开客户区仍相差 2×6 像素。
S3 仍开放，原版 Explorer、扩展打印属性页、色板客户区尺寸、
打印机 DC 和全局路径仍待验证。
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
- Last verified remote delivery: M0 T28 S3 P1，3bf842f2；八件均与远端所指提交及两处发布一致。
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
