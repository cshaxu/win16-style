# Project Status

## Current Work

用户决定以现有实际成果有限收口 T25，未完成的 S8 验证和 S10 最终审计不计通过；原 S9 去重已明确并入 T26，遗留范围见[T25 收口记录](../history/M0-T25-owner-closure.md)。准入 T26，抽离跨风格通用机制并整理样式库。

第十九批同快照候选已发布并完成针对性测试。当前范围和未通过项见下方技术基线。

滚动条键盘命令映射已从 Win16 控件迁入公共 scroll-state，原调用路径和行为保持不变。双位宽映射边界测试及 Win16 控件夹具通过；已纳入第十九批，Win2000 滚动条接入尚未实现。

现代外框表面子类的引用、消息转发和销毁脱离已进一步并入公共宿主，Win16 保留输入命中及菜单策略。双位宽公共宿主重入、表面销毁、安装回滚与 Win16 隐藏窗口回归通过；该改动已纳入第十九批，真实可见宿主仍未验证。

## M0 T26 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New；T25 按用户决定有限收口，T26 从 S1 开始。 |
| Admission And Approval | 用户要求把 Win16 中跨年代可用的外框、控件、对话框以及 Win32/UWP/WinUI 机制抽离至 lib-style，并准入下一 T。 |
| Objective | 冻结全部相关生产组件、调用与测试清单，区分风格无关机制及各年代专属行为，确定分批迁移和实际复用契约；按用户要求对已确认单元同步实施。 |
| Non-goals | 审计不宣称已支持未实现的主题或控件；不改变既有挂钩权限与用户体验。 |
| Reference Baseline | T25 S8 P13 已发布包及有限收口记录；保留既有 Win16 与 Win2000 行为。 |
| Candidate Proposal | docs/proposals/style-library-reorganization.md，结合本次明确扩展的窗口、控件、对话框和现代框架范围。 |
| Files And ABI Surface | 样式库、通用样式机制、产品适配、CMake、全部相应测试及来源声明；审计先不改公开 ABI。 |
| Applicable Rules | M0；纯 C17、MinGW GCC、Ninja、ccache；现有八文件同批交付。 |
| Verification | 静态生产依赖及调用方盘点、来源与测试映射；迁移后验证两种风格实际调用、双位宽及适用框架回归。 |
| Expected Markers | 每一项列明旧位置、分类、目标组件、调用方、迁移顺序及验证；未覆盖明确保留。 |
| Asset Needs | 保留原始图形资源及来源声明，不依赖或修改兄弟仓库。 |
| Reporting Requirements | 分别报告已共享、待迁移、风格专属和未验证能力；不把目录移动当作代码复用。 |
| Stop Conditions | 不创建或切换隔离桌面，不使用 computer-use；不改系统安全设置或兄弟仓库。 |
| Exit Criteria | 审计覆盖全部相关生产入口和测试，形成可执行的迁移次序、明确边界及 S 序列，完成规范验证与远端复核。 |
| Original Owner Request | 收口 T25；把 Win16 中可用于 Win2000、XP、Vista、Win7 的外框、控件、对话框及 Win32/UWP/WinUI 通用能力抽离到 lib-style。 |
| Similar-Issue Sweep | 检查现有 style-common、Win16/Win2000 样式模块、产品适配及共享进程入口，避免重复机制和风格反向依赖。 |

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 26
- Last allocated Td S: 19
- Last allocated T18 S: 64
- Last allocated T19 S: 3
- Last allocated T20 S: 19
- Last allocated T21 S: 44
- Last allocated T25 S: 8
- Last allocated T26 S: 1
- Last verified remote delivery: M0 T26 S1 P40，`15aad9f0`。
- Open task: M0 T26 S1；跨风格能力完整审计与迁移规划。

## Technical Baseline

- **活动范围**：T25 S9 去重已并入 T26。当前 S1 仍需完成全部窗口、控件、对话框、Win32/UWP/WinUI 生产入口与测试的分类、迁移次序和覆盖审计；不把已迁移的局部当作 T26 完成。
- **覆盖台账**：当前生产与测试目录的盘点、已审证据和待审边界见[台账](../history/M0-T26-S1-coverage-ledger.md)；目录统计不是逐文件审计结论。
- **当前发布**：第十九批双位宽同快照候选已发布至 `assets` 与 `O:\winnt`，八文件摘要一致。构建、针对性回归、明确失败与未覆盖范围见[候选记录](../history/M0-T26-S1-shared-candidate-19.md)；候选未正式验收，S1/T26 未收口。
- **其他已发布共享**：同进程窗口钩子分发、现代窗口缩放边缘命中、普通/现代外框光标映射、现代宿主类名核验、Win2000 系统菜单公共缓冲及滚动条公共位置映射已发布；相关验证见各历史候选记录。后者不表示 Win2000 滚动条已实现。
- **第八批新增共享**：普通、单参数和消息框入口的调用骨架由 Win16 和 Win2000 共用；真实窗口回调异常及 Win2000 打印夹具仍未通过，见[对话框复核](../history/M0-T26-S1-dialog-current-review.md)。现代窗口表面刷新生命周期由公共组件负责，真实宿主仍未覆盖，见[表面记录](../history/M0-T26-S1-uwp-surface.md)。同线程子窗口遍历由公共组件负责，产品保留风格和目标策略，见[接入复核](../history/M0-T26-S1-product-adapter-review.md)。
- **第十批新增共享**：Win16 和 Win2000 系统菜单 `WM_PRINT` 的窗口/客户区坐标偏移已共用；双位宽自建窗口及实际系统菜单增量验证通过，见[菜单坐标记录](../history/M0-T26-S1-menu-coordinate.md)。
- **第九批新增共享**：普通 Win16 和 Win2000 启动入口已共用公共启动事务；双位宽事务及两种风格各四种父子位宽组合的自建传播测试通过，见[接入复核](../history/M0-T26-S1-product-adapter-review.md)。XAML 工作区边缘保护及普通、现代外框的工作区交集计算已共享，公共及 Win16 自建外框测试双位宽通过，见[外框记录](../history/M0-T26-S1-frame-xaml.md)。
- **公共组件边界**：Win16/Win2000 样式目录已迁入 `lib-style`；窗口、控件、对话框及现代宿主的部分通用机制由 `lib-style/common`、`lib-style/uwp` 和 `lib-win32/style` 提供。公共主题状态由现有共享 DLL 统一承载，Win16 与 Win2000 控件有实际调用；系统原有主题仅能恢复已观察到的请求。轻量导入、控制台懒加载、v5/v6、重入与原生异常的双位宽自建回归见候选记录和[主题审计](../history/M0-T26-S1-theme-lifecycle-review.md)。
- **尚未通过**：Win2000 移除外框后 DWM 状态未恢复；同机 Git HEAD 基线亦复现，见[调查](../history/M0-T26-S1-win2k-dwm-investigation.md)。同进程 UWP 分发修复已发布并通过聚焦测试；自建跨进程真实钩子返回 126，UWP 全局自建目标策略准入返回 5，实际 UWP/WinUI 宿主和真实全局会话未验证，分别见[分发记录](../history/M0-T26-S1-uwp-dispatch.md)和[全局记录](../history/M0-T26-S1-uwp-global.md)。这些不是通过项。
- **第十一批新增共享**：创建作用域待处理窗口记录已移入 `lib-win32/style` 并被 Win16 实际接入；双位宽公共和产品创建回归见[记录](../history/M0-T26-S1-creation-pending.md)。Win2000 尚未接入此创建机制。
- **第十一批新增共享**：原生颜色下拉与日期控件身份核验已从 Win16 绘制中拆入公共控件库；双位宽原生正负例及 Win16 产品回归见[记录](../history/M0-T26-S1-native-combo-identity.md)。
- **第十一批新增共享**：状态栏身份核验已迁入公共控件库，框架别名由调用方提供；双位宽 v5/v6 和 Win16 产品回归见[记录](../history/M0-T26-S1-status-identity.md)。
- **第十一批新增共享**：颜色对话框模板与色块识别已并入公共对话框库，Win16 保留具体绘制策略，空的私有对话框库已删除；双位宽产品回归见[记录](../history/M0-T26-S1-color-dialog-common.md)。
- **第十二批新增共享**：同模块、同线程子窗口与类过程身份核验已由公共窗口组件提供并接入 Win16 VCL 路径；见[记录](../history/M0-T26-S1-owned-child-identity.md)及第十二批发布后测试。
- **控件生命周期复核**：逐项记录已共享和仍依赖 Win16 状态/字体的入口，见[记录](../history/M0-T26-S1-control-lifetime-review.md)。
- **第十二批新增共享**：控件子类按所有权移除已并入公共控件作用域并被 Win16 轨迹条、工具栏和报告控件使用；见[记录](../history/M0-T26-S1-owned-subclass.md)及第十二批发布后测试。
- **第十二批新增共享**：旧式字体对话框组合框资格检查已迁入公共控件，Win16 提供类别策略；公共测试及正式发布版字体对话框 A/W 确认和取消双位宽通过，见[候选记录](../history/M0-T26-S1-shared-candidate-12.md)。
- **现代外框待拆分**：脱离、重入引用、所有者/标题/表面安装事务及所有者、标题通用消息分派已由公共宿主接管；第十四批同快照自建测试通过，表面输入与风格策略仍需审计，真实宿主与回滚条件见[现代外框审计](../history/M0-T26-S1-modern-frame-audit.md)。
- **菜单弹出路径复核**：两种风格已共用输入、快照与呈现基础；选中状态和嵌套生命周期仍有不同契约，具体可拆边界见[局部审计](../history/M0-T26-S1-menu-scope-review.md)。
- **普通外框生命周期复核**：位置恢复与 DWM 操作已有共享基础，安装事务、状态和消息分派仍须按两种风格各自契约拆分；见[局部审计](../history/M0-T26-S1-frame-lifecycle-review.md)。
- **第十五批新增共享与修复**：全局窗口样式适配去掉三个转发函数，Win2000 非客户区漏掉的公共菜单重叠刷新已修复；非客户区滚动条轴选择和 DC 管理移入公共组件并由 Win16 使用。双位宽测试和真实全局及 Win2000 控件边界见[候选记录](../history/M0-T26-S1-shared-candidate-15.md)。
- **第十六批新增共享**：状态栏标准消息的字体和快照失效判定、原生分栏与文本快照采集已移入公共状态栏组件并由 Win16 使用；双位宽回归见[候选记录](../history/M0-T26-S1-shared-candidate-16.md)。
- **第十七批新增共享**：轨迹条和工具栏的自绘通知来源校验、XAML 图标消息的样式位暂存与恢复已归入公共组件，Win16 实际接入；双位宽回归和真实宿主限制见[候选记录](../history/M0-T26-S1-shared-candidate-17.md)。
- **第十三批新增共享**：原生及 VB5/VB6 控件类身份与别名已从 Win16 控件层移入公共样式层，Win16 的绘制资格保留；双位宽回归及限制见[候选记录](../history/M0-T26-S1-shared-candidate-13.md)。
- **剩余实现**：对话框与窗口创建作用域仍有两组产品入口，生命周期和错误处理不同，须按[复核记录](../history/M0-T26-S1-dialog-current-review.md)继续拆分并验证；现有索引不等于所有函数已审完。Win2000 控件外观、所有目标种类和真实应用的完整覆盖仍未证明。
- **执行约束**：不使用 computer-use，不创建或切换隔离桌面；保留源码与资源声明，不依赖或修改兄弟仓库。先发布再进行产品包测试；清理本次临时构建、测试副本和发布备份，保留必要证据。提交推送及远端复核尚未完成。
- **历史记录**：此前完整状态文字保存在[基线历史日志](../history/M0-T26-S1-baseline-log.md)；其中旧的“未发布”等状态可能已被本基线及候选记录取代。
