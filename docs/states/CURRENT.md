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
S6 接续已交付的 Win32 标准控件。复选框、单选框、静态文字、蚀刻边缘和分组框已通过双宽原生对照，
标准编辑框、列表框、组合框、标签页和表头已通过双宽主题资源原生对照，
列表视图、树视图、进度条、滑杆和微调按钮的已支持路径已完成阶段性双宽对照，
工具栏、状态栏及 Rebar 已完成阶段性双宽对照，
Win2K 日期、月历、RichEdit 和热键边线已完成双宽原生及换色对照，
见[开发验证](../history/M0-T29-S6-win32-controls.md)。
S6 P1 八件开发运行包和四份主题已发布两处，聚焦包 102 项通过。
10 项私有桌面外层跳过后以直接 worker 复测通过。实包摘要与逐项证据
见[发布记录](../etc/release.json)。已复核远端 `99c04b6f`，S6 收口。
S7 已完成首轮双宽开发候选的 VB/VCL 和复合控件调色板对照，
见[开发验证](../history/M0-T29-S7-composite-controls.md)。
S7 P1 已完成同批发布与实包验证，聚焦包 39/39 和两处发布路径 8/8 通过。
已复核远端 `c0efd378` 的 12 个发布文件，S7 收口。S8 接续公共对话框。
S8 已把 Win2K 已有对话框度量及打印 General 三份模板接入主题，
双宽开发回归详见[验证记录](../history/M0-T29-S8-common-dialogs.md)。
S8 P1 同批发布后聚焦回归 73/73，直接复验 36 项通过、2 项 x86 打印上下文跳过。
已复核远端 `79dc7c2e` 的 12 个发布文件，S8 收口。S9 接续现代外框。
用户确认 T29 最终发行包为七件。启动器和两份 `ntwshk` 加两组主题，
见[主题构建契约](../proposals/msstyles-build-contract.md)。S11 负责独立七件目录验收与切换。
S9 已收口。直接 UWP 与 Win32、WinUI 共用主题模型及呈现。P1 聚焦实包
29/29、两处双宽补充 24/24。真实 Calculator、Scan 和 WinUI 记事本复验通过。
已核对实际远端 `407e14d0` 的源码及 12 个文件摘要。
见[验证记录](../history/M0-T29-S9-modern-frames.md)。S10 接续中性运行时。
S10 已实现外部主题只读装入，双宽实际标题、按钮、UWP 及打印模板对照通过。
两种呈现已链接进双位宽 ntwshk，无旧 DLL 目录的直接调用及自有 UWP 对照通过。
普通启动及子进程传播双向跨位宽已在无旧 DLL 目录通过；缺失或无效主题在启动前拒绝。
全局模块选择已改用中性运行时；仅自有线程的双宽生产接收器对话框 14/14 通过。
双宽托盘单元测试和自有 UWP 测试验证了目标主题失效时保留当前状态。
受限 Win32 接收器 36/36 次切换通过，包含普通启动两种样式及新会话重启。
Win16 晚期传播四种位宽组合及普通启动 COM 回归 12/12 通过。
S10 P1 已发布两处，聚焦实包 18/18，Scan、Calculator 与 WinUI Notepad 实样通过。
S10 P1 的 12 个远端文件和 1,281 个构建输入已复核，S10 收口。四个旧 DLL 和演示仍属 S10 过渡包，S11 接续正式切换。
见[开发记录](../history/M0-T29-S10-neutral-runtime.md)。
S11 已切换七件契约、构建器、验包和可回滚部署。独立七件的增量聚焦回归
13/13，旧文件、多余目录、跨目录主题引用负例及独立两处部署回滚通过。
S11 P1 双宽干净构建通过，七件已发布到两处并逐件核对摘要。四个年代 DLL
和演示已从两个发布路径退役。发布后聚焦回归 17/17，并完成当前 Calculator、
Scan 和真实 WinUI 记事本的样本复验。GitHub `main` 的 `a810d5b2` 已逐件
复核七份产物、五个退役路径和 1,281 个构建输入，S11 收口。见
[S11 验证记录](../history/M0-T29-S11-seven-file-package.md)。

S12 已逐项核对 113 个适用单元、44 个旧入口和两处正式包。两宽主题读回
及扩展／模型探针 12/12 通过。实际 GitHub 提交 b4e66f86 的终审、七件文件和
五个退役路径已复核。S12 和 T29 按既定范围收口，验收与待证范围见
[S12 终审](../history/M0-T29-S12-final-audit.md)。

S13 P1 已在两处发布。本次主题发现、按名启动、托盘目录和自有窗口
切换校验通过。独立的 22 项聚焦用例通过，两项既有 Win32 外框测试在
发布前后均复现最大化、还原失败，详见 [S13 记录](../history/M0-T29-S13-theme-folder-cli.md)。
七件发布文件和两处安装已在远端 P2 逐件复核。两项既有断言转入 TODO，按用户指令继续 S14。

S14 同批七件已发布两处，19 项聚焦用例全部通过。双宽原生探针覆盖内置与混搭
主题的文件、目录、打印及自有线程晚接入。远端 P1 摘要已核对，S14 已收口，
T29 保持开放。详见 [S14 记录](../history/M0-T29-S14-dialog-policy.md)。

S15 冻结正式运行件的年代依赖，并将托盘名称和菜单命令分配接到主题目录。
同批七件发布和远端摘要已复核，15 项实包聚焦测试全通过。S15 已收口，
T29 全主题驱动目标保持开放。后续顺序见 [T29 规划](../proposals/msstyles-classic-runtime.md)，
迁移接收者及验证见 [S15 台账](../history/M0-T29-S15-theme-runtime-boundary.md)。

S16 P1 同批七件已发布两处，聚焦验证 23/23。实际远端
`cd7b6178` 的七件、九个退役路径和 1,282 项构建输入已复核。
S16 收口。三类接入统一目录标识，仍用现有兼容绘制适配器，
见 [S16 验证记录](../history/M0-T29-S16-theme-identity.md)。

M0 T29 S17 已收口。双位宽 Win32 外框共享主题定义的标题、边框、按钮与命中。
同批七件发布两处，发布后聚焦 12/12 及补充实窗动作 24/24。两项历史综合夹具失败与
S16 同条件逐字节一致。实际远端 P1 `7f72b60a` 的七件、九个退役路径和 1,286 项构建
输入已核验。见 [S17 记录](../history/M0-T29-S17-shared-frame.md)及[发布清单](../etc/release.json)。

M0 T29 S18 已收口。P1 `acf79b8fb32a` 的双宽现代外框读取同一已选主题模型，
复用标题、边框、按钮图形和命中。两处实际发布与远端七件已核对。双宽混搭主题和
自有窗口恢复通过。真实 Scan 和计算器附着前超时，见 [S18 记录](../history/M0-T29-S18-modern-frame-model.md)。

M0 T29 S19 已收口。两宽共用主题声明的菜单文字、行列、字体和图形。P1 ebe2a34300ff 已发布并从实际远端复核，聚焦实包 22/22。
混搭菜单资源、离屏像素和原生菜单状态通过。S18 原版也复现现代外框最大化断言。
见 [S19 记录](../history/M0-T29-S19-menu-font.md)。

M0 T29 S20 已收口。P1 c1edb6c5856f 的七件包已在两处发布并从实际远端复核。
聚焦实包 50/50，另有 12 次安装读回或真实启动像素探针通过。旧兼容启动型
STATIC 断言在 S19 同样复现，以中性运行时实际像素独立验证。见
[S20 记录](../history/M0-T29-S20-basic-controls.md)。

M0 T29 S21 已收口。P1 `627034800fb4` 的双宽正式七件已发布两处并复核实际远端。
56 项聚焦实包、16 次安装读回与像素复验、24 次原生窗口适配对照均通过。
四种 VCL 模型均验证换主题改变实际像素、换父撤销后恢复原样。
见 [S21 记录](../history/M0-T29-S21-composite-controls.md)。

M0 T29 S22 已收口。P1 076779b0a999 的双宽正式七件先发布两处，聚焦
实包 37/37，无跳过。已从实际远端复核七件、九个退役路径和 1,291 项
构建输入。补测与未覆盖项见 [S22 台账](../history/M0-T29-S22-dialog-routes.md)。

## M0 T29 S23 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | 沿用户批准的 T29 终态顺序继续。S22 实包和远端验收已完成。 |
| Objective | 从正式启动器和双位宽中性运行时的真实链接闭包移除年代专属制式、配方、选择分支及旧生产装配。 |
| Non-goals | 多增发行文件、改变已有接入权限、扩张主题覆盖或准入下一 T。 |
| Reference Baseline | S22 P1 076779b0a9992b5a30fcf45e4b64b9568cbd85c6。 |
| Candidate Proposal | [T29 实施顺序](../proposals/msstyles-classic-runtime.md)及[S23 链接台账](../history/M0-T29-S23-runtime-link-closure.md)。 |
| Files And ABI Surface | run16s、ntwshk、两套旧装配、主题加载器、Win32 和现代框架及相应归档。保留逐项证实所需的薄 ABI。 |
| Applicable Rules | 纯 C17，双宽 MinGW/Ninja/ccache。合批干净构建且同批七件先发布后测试。 |
| Verification | 双宽真实链接输入、归档成员、PE 资源、导入导出与调用闭包。以原主题和混搭主题验证已交付的宿主、控件、对话框能力及还原。 |
| Expected Markers | 只换同目录主题文件即可改变风格，正式运行件隐藏及公开分派均不绑定 Win16 或 Win2000。 |
| Asset Needs | 现有两组主题与原图、模板、来源证据。只在自有夹具临时生成混搭数据。 |
| Reporting Requirements | 逐项区分产品装配、通用机制、主题数据、已退役旧路径、经验证的兼容 ABI 与剩余未验证项。 |
| Stop Conditions | 禁止 computer-use 和私有桌面，只运行有界自有夹具。 |
| Exit Criteria | 双宽链接闭包和实际运行验证满足终态，正式七件两处发布后测试、提交推送、实际远端复核完成。 |
| Original Owner Request | EXE 和 DLL 完全遵循主题文件，当前 Win16 和 Win2000 效果及能力保留。 |
| Similar-Issue Sweep | 启动、全局托盘、传播、窗口宿主和对话框，核对名称、年代枚举、嵌入表、链接与退役路径。 |

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
- Last verified remote delivery: M0 T29 S23 P30 `bf3ffdd9933c`，两处七件正式文件和远端摘要已复核。
- Last allocated T27 S: 16
- Last allocated T28 S: 3
- Last allocated T29 S: 23
- Open task: M0 T29 S23，正式运行件旧产物和年代链接退出。

## Technical Baseline

- [正式契约](../etc/product-contract.json)为 run16s.exe、ntwshk32/64.dll 和两对主题。两处与实际远端均已核验 [T29 S11 P1](../history/M0-T29-S11-seven-file-package.md)。
- CLASSIC_STYLE 1.18 已发布，颜色、字体与查找路线可独立声明。Win32 基础控件和复合控件读取主题的边缘、几何和颜色，端钮资源随主题交付。菜单呈现沿用双宽 S19 模型。
- 托盘菜单名称来自已验证的主题描述，显示目录以区分同名主题。
- CLASSIC_STYLE 1.8 进一步声明标题、按钮、边框绘制及几何。外观与文件、目录、打印策略沿用记录 9 的契约。
- 样式统一src/lib-win32-style的access/application/appearance；通用无样式Windows能力在src/lib-win32。当前结构和接口见[ARCHITECTURE](../design/ARCHITECTURE.md)及[CODING](../design/CODING.md)。
- 三接入、共同生命周期/窗口/菜单/控件协议/对话框/现代宿主/图形根已有真实消费者；年代保留度量、绘制、字体、资源与支持策略。产品保留CLI/托盘业务、目标/会话选择、原API trampoline、兼容ABI和装配。Win2000控件、其他框架与Luna/Aero不因公共根存在即得到完整支持。
- 来源1192回执、五原资源、包内原始栅格表和ABI/PE门禁通过；发布基线两宽各25公共样式归档与4进程归档无产品/年代反向依赖。所有当前构建/运行/测试不依赖或修改兄弟仓库。
- propagation停用、窗口恢复、thread hook解除与物理DLL/late patch卸载分别报告；原有进程存活期固定/保留能力不改称安全卸载。
- 禁止computer-use、创建或切换私有桌面；只用自有有界当前桌面探针。用户已永久授权非computer-use验证及按身份处理正式文件实际占用者；不重复请求同一授权，不改系统安全设置。
- 历史逐文件来源及迁移见[原顺序台账](../history/M0-T26-S1-ordered-audit.md)、各S证据与T26规划；早期lib-style/lib-win32/style路径是历史路径，当前旧生产目录已删除。
