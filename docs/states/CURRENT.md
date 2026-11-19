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

M0 T29 S1 完成 MSStyles 可迁移性调研。冻结 113 个按年代计的适用单元，
其中 66 项可用标准属性、13 项需项目扩展、30 项必须由运行时执行、4 项待证。
本机三类部件样本 20 项完成只读探针。见 [S1 审计台账](../history/M0-T29-S1-msstyles-audit.md)
和 [调研提案](../proposals/msstyles-classic-runtime.md)。

用户确认 Win16 与 Win2000 的主题整理继续在 T29 内实施。
[S2–S12 实施规划](../proposals/msstyles-classic-runtime.md)已记录。
M0 T29 S2 已准入，先确定主题与 ABI 契约。两套主题源必须能生成合规的 MSStyles、theme 描述及所需引用资源。参见 [主题构建契约](../proposals/msstyles-build-contract.md)。

## M0 T29 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation；M0 固定，T29 按已确认队列实施。 |
| Admission And Approval | 用户要求自行设定目标并开始执行，要求可编辑源正确编译为合规 MSStyles 及相关主题文件。 |
| Objective | 冻结两年代主题源、合规输出、项目扩展及旧 DLL ABI 的迁移契约。 |
| Non-goals | S2 为契约与取证，不提前替换正式包、删旧 DLL、设置系统主题或扩大适用目标。 |
| Reference Baseline | T29 S1 P3 规划、S1 的 113 单元账本和 T28 P45 发布八件。 |
| Candidate Proposal | docs/proposals/msstyles-classic-runtime.md。 |
| Files And ABI Surface | 主题源生成真实 MSStyles、theme 和引用资源；审计 Win16 39 个及 Win2000 5 个导出。 |
| Applicable Rules | M0、纯 C17 产品运行时、原资源与来源保留、双宽共用主题数据。 |
| Verification | 本机只读资源与描述对照，核对生产消费者与公开格式，按 113 单元列迁移和验证归属。 |
| Expected Markers | 两套可编辑源和生成件定义、独立解析及路径校验判据、扩展最小字段与 ABI 处置。 |
| Asset Needs | 已冻结旧资源和本机 Aero 样本；S3 自有最小主题用于互操作正负例。 |
| Reporting Requirements | 区分已观察的标准样本、设计决定以及仍待 S3/S4 实测的内容。 |
| Stop Conditions | 无系统主题设置、权限调整或二进制发布变更。 |
| Exit Criteria | 完成首批格式、默认值、所有权、13 扩展、113 单元和 44 个导出的可复核契约。 |
| Original Owner Request | 自行设定目标开始执行；源代码能正确编译成合规的 MSStyles 和 theme 相关文件。 |
| Similar-Issue Sweep | 复用 S1 标准槽映射、当前八件来源门禁及 T28 双宽回归。 |

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
- Last verified remote delivery: M0 T29 S2 P1；二进制仍为 P45 八件。
- Last allocated T27 S: 16
- Last allocated T28 S: 3
- Last allocated T29 S: 2
- Open task: M0 T29 S2；主题文件与 ABI 契约。

## Technical Baseline

- 产品为[契约](../etc/product-contract.json)固定五组件八文件：run16s.exe、demo.exe、ntw16s32/64.dll、ntw2ks32/64.dll、ntwshk32/64.dll；远端、O:/winnt 和 assets 已核对 [T28 S3 P45 公共对话框候选](../etc/evidence/t28-s3-dialog-reference.json)。
- 样式统一src/lib-win32-style的access/application/appearance；通用无样式Windows能力在src/lib-win32。当前结构和接口见[ARCHITECTURE](../design/ARCHITECTURE.md)及[CODING](../design/CODING.md)。
- 三接入、共同生命周期/窗口/菜单/控件协议/对话框/现代宿主/图形根已有真实消费者；年代保留度量、绘制、字体、资源与支持策略。产品保留CLI/托盘业务、目标/会话选择、原API trampoline、兼容ABI和装配。Win2000控件、其他框架与Luna/Aero不因公共根存在即得到完整支持。
- 来源1157回执、五原资源、包内原始栅格表和ABI/PE门禁通过；两宽各23公共样式归档与4进程归档无产品/年代反向依赖。所有当前构建/运行/测试不依赖或修改兄弟仓库。
- propagation停用、窗口恢复、thread hook解除与物理DLL/late patch卸载分别报告；原有进程存活期固定/保留能力不改称安全卸载。
- 禁止computer-use、创建或切换私有桌面；只用自有有界当前桌面探针。用户已永久授权非computer-use验证及按身份处理正式文件实际占用者；不重复请求同一授权，不改系统安全设置。
- 历史逐文件来源及迁移见[原顺序台账](../history/M0-T26-S1-ordered-audit.md)、各S证据与T26规划；早期lib-style/lib-win32/style路径是历史路径，当前旧生产目录已删除。
