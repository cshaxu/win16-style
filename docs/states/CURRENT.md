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
S4 P3 接入标题度量与横纵滚动轨道、滑块和尺寸框数据。同批八件已发布两处，
直接测试 30/30、替换资源对照 16/16 通过，干净树四主题文件通过独立读回。
S4 P4 接入 Win16 标题图像、两风格四种按钮状态和滚动端钮度量。
同快照双宽开发运行包与四个主题预览已发布，直接实包 32/32 及资源替换对照通过。
S4 已在 `53ab9307` 复核远端源码和全部发布摘要后收口。S5 接续窗口、菜单和字体。
S5 已验证双宽标题及菜单字体、Win32 外框和菜单栏配色、实际系统菜单、
Win16 弹出菜单列位移及勾选和箭头原图。Win16 位图字体继续按主题描述采样宿主，
独立历史字形待可核验来源和许可，见
[验证记录](../history/M0-T29-S5-window-menus-fonts.md)。
S5 P1 双宽干净开发包及四个主题已发布两处，发布后聚焦验证 58/58。
GitHub main 已复核 `fe51bb15` 的源码和十二个发布文件，S5 收口。
S6 接续已交付的 Win32 标准控件。
用户确认 T29 最终发行包为七件。启动器和两份 `ntwshk` 加两组主题，
见[主题构建契约](../proposals/msstyles-build-contract.md)。S11 负责独立七件目录验收与切换。

## M0 T29 S6 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation；M0 固定。 |
| Admission And Approval | 用户批准 T29 现有 S2–S12 连续执行，先完成 Win32 标准控件。 |
| Objective | S2 台账 16–21、25–36、48 的已交付 Win32 标准控件状态、图形资源与原生协议。 |
| Non-goals | 专属 VB/VCL、公共对话框、现代宿主和运行时切包按 S7–S11 执行。 |
| Reference Baseline | S5 P1 双宽同批开发包，按钮与滚动条主题数据，以及两种风格各自已交付的控件范围。 |
| Candidate Proposal | docs/proposals/msstyles-classic-runtime.md。 |
| Files And ABI Surface | MSStyles 标准控件记录、共用控件状态和两套年代 renderer；保留已发布 ABI。 |
| Applicable Rules | C17 双位宽，保留原资源及来源。沿现有原生状态、应用回调、所有权以及失败传播。 |
| Verification | 按基础输入控件、列表与容器、范围与日期三批迁移。读取标准类和状态，覆盖双位宽、受支持 Common Controls 版本、96/144 DPI 以及实际绘制和通知。 |
| Expected Markers | 更换主题记录后实际状态像素改变；默认像素、控件数据、值、回调和清理与基线一致。 |
| Asset Needs | 五份已固定图形头、项目既有 Win32 控件夹具、真实类及已验收系统状态。 |
| Reporting Requirements | 逐台账记清两种风格、原生保留和测试边界。聚焦测试与最终同批发布分开。 |
| Stop Conditions | 保持现有来源、权限及桌面范围，区域或角色不明时守住既有原生边界。 |
| Exit Criteria | 已交付控件组成完整双宽主题输入、实际绘制和协议验证；发布同批构建并复核远端。 |
| Original Owner Request | 共用可编译主题承载现有 Win16、Win2000 风格，按原能力逐项迁移。 |
| Similar-Issue Sweep | 已验证 Win32 标准控件与画笔来源；梳理原生保留路径，集中共用主题状态，保留 S4 按钮／滚动路径。 |

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
- Last verified remote delivery: M0 T29 S5 P1；八件开发运行包及四个主题预览。
- Last allocated T27 S: 16
- Last allocated T28 S: 3
- Last allocated T29 S: 6
- Open task: M0 T29 S6；Win32 标准控件。

## Technical Baseline

- 产品为[契约](../etc/product-contract.json)固定五组件八文件：run16s.exe、demo.exe、ntw16s32/64.dll、ntw2ks32/64.dll、ntwshk32/64.dll；远端、O:/winnt 和 assets 核对 [T29 S5 P1](../history/M0-T29-S5-window-menus-fonts.md)。
- 样式统一src/lib-win32-style的access/application/appearance；通用无样式Windows能力在src/lib-win32。当前结构和接口见[ARCHITECTURE](../design/ARCHITECTURE.md)及[CODING](../design/CODING.md)。
- 三接入、共同生命周期/窗口/菜单/控件协议/对话框/现代宿主/图形根已有真实消费者；年代保留度量、绘制、字体、资源与支持策略。产品保留CLI/托盘业务、目标/会话选择、原API trampoline、兼容ABI和装配。Win2000控件、其他框架与Luna/Aero不因公共根存在即得到完整支持。
- 来源1178回执、五原资源、包内原始栅格表和ABI/PE门禁通过；发布基线两宽各25公共样式归档与4进程归档无产品/年代反向依赖。所有当前构建/运行/测试不依赖或修改兄弟仓库。
- propagation停用、窗口恢复、thread hook解除与物理DLL/late patch卸载分别报告；原有进程存活期固定/保留能力不改称安全卸载。
- 禁止computer-use、创建或切换私有桌面；只用自有有界当前桌面探针。用户已永久授权非computer-use验证及按身份处理正式文件实际占用者；不重复请求同一授权，不改系统安全设置。
- 历史逐文件来源及迁移见[原顺序台账](../history/M0-T26-S1-ordered-audit.md)、各S证据与T26规划；早期lib-style/lib-win32/style路径是历史路径，当前旧生产目录已删除。
