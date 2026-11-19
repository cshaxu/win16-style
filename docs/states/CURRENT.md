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
M0 T29 S2 已完成主题源和构建契约、113 单元迁移验证与 44 个导出入口处置。
见 [主题构建契约](../proposals/msstyles-build-contract.md)和
[逐项迁移与 ABI 台账](../history/M0-T29-S2-theme-and-abi.md)。

M0 T29 S3 已收口。[S3 记录](../history/M0-T29-S3-msstyles-v4.md)保存两对最小主题构建、
独立读回、版本与引用负例、标题按钮组扩展及原图像素证据。S4 接续三类代表部件的实际消费。
[S4 记录](../history/M0-T29-S4-representative-rendering.md)保存双宽只读模型和标题颜色离屏证据。
S4 P2 同批八件已发布至 `assets` 与 `O:/winnt`，实际包验证覆盖双宽 20 项
直接回归和 16 组资源消费。S4 接续其余度量、状态及原始资源迁移。

## M0 T29 S4 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation；M0 固定。 |
| Admission And Approval | 用户批准 T29 现有 S2–S12 连续执行。 |
| Objective | 标题栏、普通按钮和滚动条从同源主题资源进入真实布局、绘制与命中。 |
| Non-goals | 不改系统主题、权限或接入目标；其他部件及完整运行时替换按后续 S 执行。 |
| Reference Baseline | S2 逐项台账、S3 同源四件、已发布 P45 双宽呈现。 |
| Candidate Proposal | docs/proposals/msstyles-classic-runtime.md。 |
| Files And ABI Surface | MSStyles 读取与主题模型、窗口标题栏和按钮/滚动条共享应用入口、自有 Win32 验证；保留既有产品 ABI。 |
| Applicable Rules | C17 双位宽，主题数据只读，原资源身份和原应用语义。 |
| Verification | 96/144 DPI、普通及适用禁用或按压状态的区域、像素、命中和恢复；以候选主题属性替换证明实际消费。 |
| Expected Markers | 双宽两种主题使用同一输入契约，三类代表部件证据与旧路径对照。 |
| Asset Needs | S3 主题、既有原版栅格与样式定义、自有窗口或离屏原生测试。 |
| Reporting Requirements | 逐项记录标准与扩展数据、实际消费者、同源构建、适用状态及缺口。 |
| Stop Conditions | 保持既有支持范围、安全策略和桌面约束。 |
| Exit Criteria | 三部件实际应用通路、双宽 DPI 和适用状态的回归成立，格式验证不能替代绘制与恢复证据。 |
| Original Owner Request | 共用主题机制，以可编译的 MSStyles 和 theme 文件承载 Win16、Win2000 风格。 |
| Similar-Issue Sweep | P45 标题栏及滚动条窗口、Win16 原栅格、Win2000 标准按钮、S2 逐项接收者和已有 focused 测试。 |

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
- Last verified remote delivery: M0 T29 S4 P2；T29 同批八件。
- Last allocated T27 S: 16
- Last allocated T28 S: 3
- Last allocated T29 S: 4
- Open task: M0 T29 S4；三类代表部件闭环。

## Technical Baseline

- 产品为[契约](../etc/product-contract.json)固定五组件八文件：run16s.exe、demo.exe、ntw16s32/64.dll、ntw2ks32/64.dll、ntwshk32/64.dll；远端、O:/winnt 和 assets 核对 [T29 S4 P2](../history/M0-T29-S4-representative-rendering.md)。
- 样式统一src/lib-win32-style的access/application/appearance；通用无样式Windows能力在src/lib-win32。当前结构和接口见[ARCHITECTURE](../design/ARCHITECTURE.md)及[CODING](../design/CODING.md)。
- 三接入、共同生命周期/窗口/菜单/控件协议/对话框/现代宿主/图形根已有真实消费者；年代保留度量、绘制、字体、资源与支持策略。产品保留CLI/托盘业务、目标/会话选择、原API trampoline、兼容ABI和装配。Win2000控件、其他框架与Luna/Aero不因公共根存在即得到完整支持。
- 来源1176回执、五原资源、包内原始栅格表和ABI/PE门禁通过；发布基线两宽各25公共样式归档与4进程归档无产品/年代反向依赖。所有当前构建/运行/测试不依赖或修改兄弟仓库。
- propagation停用、窗口恢复、thread hook解除与物理DLL/late patch卸载分别报告；原有进程存活期固定/保留能力不改称安全卸载。
- 禁止computer-use、创建或切换私有桌面；只用自有有界当前桌面探针。用户已永久授权非computer-use验证及按身份处理正式文件实际占用者；不重复请求同一授权，不改系统安全设置。
- 历史逐文件来源及迁移见[原顺序台账](../history/M0-T26-S1-ordered-audit.md)、各S证据与T26规划；早期lib-style/lib-win32/style路径是历史路径，当前旧生产目录已删除。
