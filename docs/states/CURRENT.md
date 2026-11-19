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

M0 T28 S3 Windows 2000 公共对话框已按用户批准的范围收口。P45 双宽正式八件
发布两处，P46–P48 补齐打印、页面设置、Shell 目录与托盘验收。用户批准跳过有效
自定义打印属性页实际显示和真实域打印机回写，保留两项后续债务。
详见 [S3 收口审计](../history/M0-T28-S3-common-dialogs.md)和
[实包与原版参考](../etc/evidence/t28-s3-dialog-reference.json)。

T28 已获用户批准收口，范围和双宽交付证据见 [T28 总验收](../history/M0-T28-closure-audit.md)。

M0 T29 已准入，当前 S1 研究用 MSStyles 定义 Win16 和 Win2000，并为移除按年代分发的 DLL 确定边界。
初步格式与产品职责实证见 [研究提案](../proposals/msstyles-classic-runtime.md)。

## M0 T29 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New；M0 保持固定。 |
| Admission And Approval | 用户批准收口 T28 并准入新 T，先研究用 MSStyles 定义 Win16 与 Win2000 并移除按年代发布的 DLL。 |
| Objective | 查清 NTW16S 与 NTW2KS 的生产职责、消费者和替换边界，验证真实 MSStyles 容器与属性的可表达性，提出共享运行时方案。 |
| Non-goals | 本 S 不删除 DLL、不换发布契约、不实现 Luna/Aero、不修改系统主题或权限。 |
| Reference Baseline | T28 P51 总验收、P45 双宽八件，以及当前三层样式架构。 |
| Candidate Proposal | docs/proposals/msstyles-classic-runtime.md。 |
| Files And ABI Surface | 先只读审计 CLI、现代宿主、共享运行时、两套风格 DLL、组件导出及打包门禁。 |
| Applicable Rules | 固定 M0；来源只读取证；有界本地样本；研究阶段不变更产品二进制。 |
| Verification | 比较可获取样本的 PE 资源、格式版本和部件状态；确认两年代标题栏、按钮和滚动条的属性映射与缺口；列出生产调用端和保留行为。 |
| Expected Markers | 可复现的样本摘要和解析边界、逐职责保留／迁移清单、一对共享运行时和合并方案的量化判据。 |
| Asset Needs | 已安装 Aero/Aero Lite、有来源的 XP Luna 文件及现有 Win16/2000 固定原资源；缺失样本注明未验证。 |
| Reporting Requirements | 分清本地实证、源代码或文档推断、待验证假设和包契约风险。 |
| Stop Conditions | 不执行读入的资源文件；不改系统、注册表、发布目录或现有目标范围。 |
| Exit Criteria | 得出可复核的格式和布局结论，明确仍需共享代码的功能，给出保留 ABI 与原表现的迁移、验证和回滚计划。 |
| Original Owner Request | 批准收口本 T；准入新 T，先调研如何去除 ntw16s、ntw2ks，用 msstyles 定义 Win16 和 Win2000 风格。 |
| Similar-Issue Sweep | 核对 T26 三层重组约束、T28 外框和公共对话框回归、本机 UxTheme 公开 API 与 XP／后期资源结构。 |

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 29
- Last allocated Td S: 21
- Last allocated T18 S: 64
- Last allocated T19 S: 3
- Last allocated T20 S: 19
- Last allocated T21 S: 44
- Last allocated T25 S: 8
- Last allocated T26 S: 14
- Last verified remote delivery: M0 T28 S3 P51；二进制仍为 P45 八件。
- Last allocated T27 S: 16
- Last allocated T28 S: 3
- Last allocated T29 S: 1
- Open task: M0 T29 S1；MSStyles 格式与共享运行时调研。

## Technical Baseline

- 产品为[契约](../etc/product-contract.json)固定五组件八文件：run16s.exe、demo.exe、ntw16s32/64.dll、ntw2ks32/64.dll、ntwshk32/64.dll；远端、O:/winnt 和 assets 已核对 [T28 S3 P45 公共对话框候选](../etc/evidence/t28-s3-dialog-reference.json)。
- 样式统一src/lib-win32-style的access/application/appearance；通用无样式Windows能力在src/lib-win32。当前结构和接口见[ARCHITECTURE](../design/ARCHITECTURE.md)及[CODING](../design/CODING.md)。
- 三接入、共同生命周期/窗口/菜单/控件协议/对话框/现代宿主/图形根已有真实消费者；年代保留度量、绘制、字体、资源与支持策略。产品保留CLI/托盘业务、目标/会话选择、原API trampoline、兼容ABI和装配。Win2000控件、其他框架与Luna/Aero不因公共根存在即得到完整支持。
- 来源1157回执、五原资源、包内原始栅格表和ABI/PE门禁通过；两宽各23公共样式归档与4进程归档无产品/年代反向依赖。所有当前构建/运行/测试不依赖或修改兄弟仓库。
- propagation停用、窗口恢复、thread hook解除与物理DLL/late patch卸载分别报告；原有进程存活期固定/保留能力不改称安全卸载。
- 禁止computer-use、创建或切换私有桌面；只用自有有界当前桌面探针。用户已永久授权非computer-use验证及按身份处理正式文件实际占用者；不重复请求同一授权，不改系统安全设置。
- 历史逐文件来源及迁移见[原顺序台账](../history/M0-T26-S1-ordered-audit.md)、各S证据与T26规划；早期lib-style/lib-win32/style路径是历史路径，当前旧生产目录已删除。
