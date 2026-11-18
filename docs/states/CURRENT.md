# Project Status

## Current Work

M0 固定。T27 Win2000 常用控件及 S1–S16 已按有限范围收口。
[终审](../history/M0-T27-S16-final-audit.md)逐项核对 24 个标准组和 8 个运行库组。

[S15 同快照八件](../etc/evidence/t27-s15-demo.json)先发布两处，185/185 项聚焦用例通过；
36 项原生客户区离屏打印子检查保留跳过。实际管理员托盘、物理输入、
其他框架和复杂变体按终审证据保留范围。

T28 S1 的 UWP 经典外框已进入手测。
[S1 记录](../history/M0-T28-S1-uwp-frame.md)包含真实计算器和 Scan 的外框、切换、
缩放和系统菜单回归。
此前同一候选在固定路径的真实 UWP 案例返回 126，两处一度恢复 T27。
当时按用户要求仅在 O:/winnt 保留经摘要核对的 T28 候选供手测。
计算器从该路径启动已获用户验收。针对标题尺寸与按钮偏差，两个年代各自统一
了 Win32/UWP 定义，修正候选已更新到 O:/winnt 并通过两种风格的定向原生回归。
实际托盘已通过计算器和 Scan 的双向切换、停用与测试退出。
按用户要求，当前活动包转为 T28 S2 的 WinUI 桌面窗口外框。S1 仍有正式
交付待办，由 S2 同批复验；已验证的候选和未发布源码均保留。
手测证据见[实包记录](../etc/evidence/t28-s1-uwp-delivery.json)。

S2 恢复修正版双宽干净构建已发布到 O:/winnt 与 assets，两处各八件的文件
摘要一致。聚焦原生矩阵 31/31；扩展客户区的双宽对照由 DWM `1→0→0`
修正为 `1→0→1`，窗口和客户区几何恢复。记事本与 Win32 演示的 144 DPI
三个标题按钮在窗口 DC 中逐像素相同；记事本菜单、标签、缩放与标题命中见
[首版实包证据](../etc/evidence/t28-s2-winui-frame.json)。

RDP 恢复后，修正版已在真实计算器的 Win16/Win2000 和 Scan 的 Win2000
路径创建宿主、操作标题按钮并移除外框，几何恢复通过。独立操作探针验证了
两款应用的 Win2000 系统菜单及垂直缩放。窗口级合成捕获已验证两款 UWP
在 144 DPI 下普通态、最大化态的三个标题按钮与 Win32 相应字形像素一致。
真实记事本单进程停用后，DWM 和客户区已恢复。RDP 重连后的窗口合成捕获
显示原生深色标题栏右侧仍有浅色重复按钮。同快照合成恢复包已发布到两处，
16 项摘要一致；真实记事本停用后的三个矩形和标题区 300×50 像素均与应用
前一致。Paint 使用实际 WinUI 窗口交叉验证，计算器与 Scan 的两种状态按钮
与 Win32 参考一致。Win32 生命周期几何断言在 T27、上一 T28 包及本包
均失败；双宽的 96/144 DPI 最大化与还原直测通过。见
[合成恢复实包证据](../etc/evidence/t28-s2-composition-delivery.json)。S2 待远端复核。

## M0 T28 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation 在 M0 T28 下新增 S2。 |
| Admission And Approval | 用户要求准入新 S，给 WinUI 程序提供 Win2000 窗口样式。 |
| Objective | 使用 T23 已验证的现代桌面窗口识别及宿主机制，将 WinUI 顶层外框接入与 Win32、UWP 相同的 Win2000 绘制。 |
| Non-goals | XAML 客户区控件、系统级主题、增加进程接入范围、修改注册表或目标文件。 |
| Reference Baseline | T23 Win16 的 Notepad 和 Explorer 适配、T25 Win2000 Win32 外框与 T28 S1 UWP 手测版。 |
| Candidate Proposal | docs/proposals/windows-2000-uwp-frame.md，WinUI 桌面外框一节。 |
| Files And ABI Surface | application/adapters 的现代窗口契约、Win2000 外框接入和对应原生夹具。沿用现有导出和五组件八文件。 |
| Applicable Rules | C17、MinGW GCC、Ninja、ccache；增量验证后按同快照双宽交付。 |
| Verification | 自有 Notepad 为首验，能创建独立目标时交叉验证 Explorer。检查标题、边框、单套按钮、应用标签/菜单、最大化、缩放重绘与恢复。回归 Win32、UWP 及默认 Win16。 |
| Expected Markers | 确认最终 WinUI 窗口 HWND；真实画面与操作各有证据，现有标签与菜单仍可用，移除只还原自身外框。 |
| Asset Needs | 复用共同 Win2000 标题栏度量、字体和位图，沿用原资源与来源声明。 |
| Reporting Requirements | 逐个实际应用报告启动、呈现、交互及恢复；可用候选、受限场景和正式验证分别记录。 |
| Stop Conditions | 禁 computer-use、私有桌面及系统安全设置变更。测试只操作自有目标，已有 Explorer/Shell 会话不作为测试目标。 |
| Exit Criteria | WinUI 首验及适用交叉验证通过，回归 S1 的 UWP 和已有 Win32，完成两处同包交付与实际远端复核。 |
| Original Owner Request | 准入一个新 S，让 WinUI 程序也有 Win2000 窗口样式。 |
| Similar-Issue Sweep | 检查 T23 的自绘按钮重复、导航重绘回退、不可改变的客户区、共享宿主识别和移除恢复。 |

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
- Last verified remote delivery: M0 T27 S16 P1，7448d0be；终审与 S15 八件远端核对。
- Last allocated T27 S: 16
- Last allocated T28 S: 2
- Open task: M0 T28 S2；WinUI Windows 2000 桌面外框。

## Technical Baseline

- 产品为[契约](../etc/product-contract.json)固定五组件八文件：run16s.exe、demo.exe、ntw16s32/64.dll、ntw2ks32/64.dll、ntwshk32/64.dll；远端已审基线为 [T27](../etc/evidence/t27-s15-demo.json)，O:/winnt 和 assets 已更新 [T28 S2 合成恢复包](../etc/evidence/t28-s2-composition-delivery.json)。
- 样式统一src/lib-win32-style的access/application/appearance；通用无样式Windows能力在src/lib-win32。当前结构和接口见[ARCHITECTURE](../design/ARCHITECTURE.md)及[CODING](../design/CODING.md)。
- 三接入、共同生命周期/窗口/菜单/控件协议/对话框/现代宿主/图形根已有真实消费者；年代保留度量、绘制、字体、资源与支持策略。产品保留CLI/托盘业务、目标/会话选择、原API trampoline、兼容ABI和装配。Win2000控件、其他框架与Luna/Aero不因公共根存在即得到完整支持。
- 来源1135回执、五原资源、包内原始栅格表和ABI/PE门禁通过；两宽各23公共样式归档与4进程归档无产品/年代反向依赖。所有当前构建/运行/测试不依赖或修改兄弟仓库。
- propagation停用、窗口恢复、thread hook解除与物理DLL/late patch卸载分别报告；原有进程存活期固定/保留能力不改称安全卸载。
- 禁止computer-use、创建或切换私有桌面；只用自有有界当前桌面探针。用户已永久授权非computer-use验证及按身份处理正式文件实际占用者；不重复请求同一授权，不改系统安全设置。
- 历史逐文件来源及迁移见[原顺序台账](../history/M0-T26-S1-ordered-audit.md)、各S证据与T26规划；早期lib-style/lib-win32/style路径是历史路径，当前旧生产目录已删除。
