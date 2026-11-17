# Project Status

## Current Work

M0 T26开放，按[三层架构与14个S规划](../proposals/style-library-reorganization.md)执行S13呈现与图形边界。S1–S12已按有限重组范围交付及实际远端复核；既有失败不计通过。S13终整、S14全范围审计仍未完成，不称全T完成。

[S5交付](../history/M0-T26-S5-runtime.md)b08d601d已实际fetch并逐blob复核37个变更；同快照八文件先发布assets根/O:/winnt，随后118项去重实际包回归通过。另2项DWM旧失败与S4一致，不计通过。见[开发证据](../etc/evidence/t26-s5-runtime.json)与[最终交付](../etc/evidence/t26-s5-delivery.json)。双宽各17个公共样式归档门禁通过，1005个源码/构建/测试/工具输入一致，ABI及原资源保留。

S5运行时module/请求/通知/late批次已公共化，产品侧净减少105行。S11已交付creation/creation_call、COM/file controller、combo与font/color观察完整事务，产品保留必要ABI/策略/呈现装配；现代与字体/GDI剩余分别由S12/S13接收；全局session/phase与管理协调已由S6提取并交付。已知失败/未覆盖见TODO及下方历史事实，S14统一复核。

S6已将session/control/start/worker/late/attach/manager/receiver/host/owner/trace迁入公共层并接入产品，删除14个产品C/H文件及六个空转发，必要线程导出合并至hook装配。927项来源回执、原资源及双宽各18个样式归档门禁通过。沙箱外同一自有目标12项原生接入全部通过，确认此前拒绝与AppContainer环境有关，未改生产策略；原沙箱失败保留在[开发证据](../etc/evidence/t26-s6-global-development.json)。

S6最终同快照双宽干净构建通过（x86约158秒，x64约176秒），1024个输入摘要一致，来源/资源/ABI及双宽公共归档门禁通过。同批八文件已先发布assets根/O:/winnt，再进行162项实际包聚焦回归：160通过、2项管理员ETW跳过；另2项DWM恢复旧失败仍归S7/S14，不计通过。见[交付证据](../etc/evidence/t26-s6-delivery.json)。S6 P1 `4d1cdcb7`已推送并实际fetch复核95个现存blob、14个删除路径及八文件摘要，见[远端证据](../etc/evidence/t26-s6-remote.json)。S6收口并自动准入S7，T26保持开放。

两轮S6构建/测试均结束，已核验无存活自有夹具及reparse后清理scratch/快照/发布备份548,376,032字节，保留.ninja/.ccache，见[清理记录](../etc/evidence/t26-s6-cleanup.json)。最终格式规范后重新构建、发布、测试，没有混用旧摘要；[全局生产链与最终映射](../history/M0-T26-S6-global.md)保留完整审计、失败对照和效率复核。禁止computer-use及私有桌面，不改或依赖兄弟仓库，不新增能力/权限/目标。

[S4事实](../history/M0-T26-S4-startup.md)及[交付](../etc/evidence/t26-s4-delivery.json)保留；T25按用户决定有限收口，未通过项不补记通过。

## M0 T26 S13 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation；S12 P1实际远端复核后按已批准14个S自动S13，M0/T26不变。 |
| Admission And Approval | 用户授权三层重组、完整能力合批、单人双角色及自动逐S推进；S12有限迁移及原始失败已归档，不冒称全面验收。 |
| Objective | 终整现有renderer注册、完整呈现操作及字体/字形/DC/bitmap/租约所有权，删除字段指针view、空转发和可复用副本，让产品仅装配年代策略。 |
| Non-goals | 不新增Win2000控件全套、Luna/Aero、主题文件加载器、XAML客户区或新框架，不扩目标/权限/安全设置，不把原生hook停用称物理卸载。 |
| Reference Baseline | S12 P1 a683cc2e：同快照八文件两处先发布，43当前case中37过/6未过、3个完整S11包控制；38个远端blob及八文件实际复核，旧失败仍S14。 |
| Candidate Proposal | docs/proposals/style-library-reorganization.md；brief docs/history/M0-T26-S13-renderers.md；初始171项docs/etc/evidence/t26-s13-renderer-audit.json。 |
| Files And ABI Surface | 既有appearance/application graphics、frame_view/style_present及实际产品/测试消费者和CMake；完整闭包须显式补录，保留五组件八文件和ABI。 |
| Applicable Rules | 纯C17/MinGW GCC/Ninja/ccache；完整公共状态和资源归实际公共所有者，年代仅提供度量/绘制/资格/资源策略，无产品或跨风格反向依赖。 |
| Verification | 顺序输入审计；原生preflight、临时字体与原异常展开、GDI/字形缓存/借还、重入与退役、双宽实际消费者、原资源/来源/ABI/依赖和实际包回归。 |
| Expected Markers | 一个共同呈现/资源生命周期，旧view/副本/空接线删除；每个保留项有实际呈现原因，不靠文件移动或字段回调伪装解耦。 |
| Asset Needs | 既有八文件和原图形/字体/来源声明；不加新DLL/服务/主题加载器或兄弟仓库依赖。 |
| Reporting Requirements | 逐项所有者/公共入口/真实调用/删除与验证，准确净代码量；原失败/skip/模型及实际宿主界限、初始快照与测试修订分列。 |
| Stop Conditions | 单人执行；禁止computer-use、私有桌面、修改或依赖兄弟仓库；只用自有有界测试，沿用有效工作，不重复UAC询问。 |
| Exit Criteria | 有限清单与实际修改逐项落实，完整操作/资源真实共用，整组构建两处先发布后测试、清理、提交推送及实际远端复核后自动S14；T26仍开放。 |
| Original Owner Request | 所有style统一lib-win32-style的access/application/appearance，极简和五大干净，未来年代复用机制，逐文件审计但按完整能力合批交付。 |
| Similar-Issue Sweep | frame_view/style_present、control dispatch/special paint/install、VB字体/字形缓存与status borrow-return、window_titles和renderer注册及原生preflight/异常资源释放。 |


S10交付见[实施记录](../history/M0-T26-S10-extended-controls.md)、[归属表](../etc/evidence/t26-s10-controls-audit.json)、[开发](../etc/evidence/t26-s10-controls-development.json)、[整组交付及补充](../etc/evidence/t26-s10-delivery.json)和[实际远端](../etc/evidence/t26-s10-remote.json)。317项既有输入（309初始冻结+8终审显式补录）及86新增输出已分类；生产+3669/-2844净+825，58新增C/H。初轮416中410过/6失败原样保存，修正同线程witness/DC与图像夹具后24调用通过，其余有限对照提供当前证据，不写成重新416全过。现代USER/原像素和既有DWM限制由TODO/S12/S14承载，不称修复。

S10正式八文件已发布assets根/O:/winnt并从远端逐blob复核；同快照干净构建约248/307秒、ccache直接818/906命中各miss68。源码/资源/ABI及双宽各4进程/21样式归档通过；最后EOF格式规范化的双宽编译对象字节一致，未重复整包发布。S10全部当前scratch已归档清理，保留稳定.ninja/.ccache，无本S在用构建/测试。

[S11交付](../etc/evidence/t26-s11-delivery.json)P1 `f6c5185c`已实际fetch并逐blob复核67个变更及八文件，见[远端证据](../etc/evidence/t26-s11-remote.json)。118输入/18输出已分类，生产C/H+1053/-1014净+39；双宽整包、每宽22样式/4进程归档、604生产/工具输入、1066回执/原资源/ABI通过。91个有限case按首67中65过/2个synthetic provider纠正后复测、另20接受结果与4 scope全过记录，无skip，不冒称首轮全过/物理操作。八文件先发布两处且摘要始终不变，所有本S临时构建/测试/探针/发布备份已归档核验清理；缓存保留。S11收口并自动S12，原dialog_scope激活NULL旧失败仍S14，不将已知限制伪装修复。S12按67输入/3输出完成现代完整根的有限重组并收口，P1 `a683cc2e`已实际fetch核对38个变更blob及八文件。公共frame拥有菜单/actions/输入/bitmap，产品/profile真实消费，生产净+17行。双宽干净构建、每宽22样式/4进程归档、1163输入、原资源/ABI及两处先发布通过；43当前case中37过/6未过、0skip，另3个完整S11包控制复现126/1460。未通过不转成成功，实际宿主/原生接入继续S14。见[S12交付](../etc/evidence/t26-s12-modern-delivery.json)、[远端](../etc/evidence/t26-s12-modern-remote.json)与brief。本S交付临时目录及发布备份已归档清理336,190,533字节，两轮开发scratch也已回收，缓存保留。自动S13初始171项冻结、0已审，没有S13实现或新运行证明。


## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 26
- Last allocated Td S: 21
- Last allocated T18 S: 64
- Last allocated T19 S: 3
- Last allocated T20 S: 19
- Last allocated T21 S: 44
- Last allocated T25 S: 8
- Last allocated T26 S: 13
- Last verified remote delivery: M0 T26 S12 P1，`a683cc2e`；38个变更blob与八文件实际复核。
- Open task: M0 T26 S13；呈现与图形边界终整。

## Technical Baseline

以下S1历史证据中的lib-style/lib-win32/style是当时路径；S2当前物理去向以381项清单为准。迁移中的真实验证状态以上方活动包为准，不把历史局部通过当作新目录已交付。

- **活动范围**：T25 S9 去重已并入 T26。S1已完成全部窗口、控件、对话框及接入生产入口与测试分类；S2目录归位已完成，S3–S13实施剩余机制拆分；不把已迁移的局部当作 T26 完成。
- **覆盖台账**：当前生产与测试目录的盘点、已审证据和待审边界见[台账](../history/M0-T26-S1-coverage-ledger.md)；目录统计不是逐文件审计结论。
- **产品接线精简**：VB 绘制作用域的产品转发文件已删除，Win16 附着和通知处理直接调用现有控件实现并提供目标策略；双位宽字体、文本及正式发布包自建控件回归通过，见[记录](../history/M0-T26-S1-shared-candidate-23.md)。不改变全局覆盖结论。
- **现代对话框状态**：线程局部重入深度归入公共对话框库，Win16 COM 代理直接使用；双位宽嵌套/隔离测试及正式发布包实际打开、另存、文件夹对话框自动取消通过，见[记录](../history/M0-T26-S1-shared-candidate-23.md)。确认与其他风格行为未覆盖。
- **公共依赖门禁**：源码检查扩展至 XP、Vista、Win7；打包时自动发现公共样式归档，干净打包验证双位宽各 14 个归档通过，见[记录](../history/M0-T26-S1-shared-dependency-check.md)。
- **Win16 基础目录复核**：七个文件的布局、配色、字体资源和产品 ABI 已分类，公共位图已复用，未来高对比度策略的边界见[记录](../history/M0-T26-S1-win16-common-review.md)。字体布局测试默认入口误尝试创建私有桌面，系统拒绝且未切换；未运行部分不计通过，后续须先核对测试入口。
- **S1阶段发布证据**：顺序审计第 23 项 VB Frame 分类候选曾发布至 `assets` 与 `O:\winnt`，八文件摘要一致。构建、针对性回归及未覆盖范围见[交付记录](../history/M0-T26-S1-ordered-23-class.md)；S1已收口，T26仍开放，当前发布以上方活动状态为准。
- **其他已发布共享**：同进程窗口钩子分发、现代窗口缩放边缘命中、普通/现代外框光标映射、现代宿主类名核验、Win2000 系统菜单公共缓冲及滚动条公共位置映射已发布；相关验证见各历史候选记录。后者不表示 Win2000 滚动条已实现。
- **第八批新增共享**：普通、单参数和消息框入口的调用骨架由 Win16 和 Win2000 共用；真实窗口回调异常及 Win2000 打印夹具仍未通过，见[对话框复核](../history/M0-T26-S1-dialog-current-review.md)。现代窗口表面刷新生命周期由公共组件负责，真实宿主仍未覆盖，见[表面记录](../history/M0-T26-S1-uwp-surface.md)。同线程子窗口遍历由公共组件负责，产品保留风格和目标策略，见[接入复核](../history/M0-T26-S1-product-adapter-review.md)。
- **第十批新增共享**：Win16 和 Win2000 系统菜单 `WM_PRINT` 的窗口/客户区坐标偏移已共用；双位宽自建窗口及实际系统菜单增量验证通过，见[菜单坐标记录](../history/M0-T26-S1-menu-coordinate.md)。
- **第九批新增共享**：普通 Win16 和 Win2000 启动入口已共用公共启动事务；双位宽事务及两种风格各四种父子位宽组合的自建传播测试通过，见[接入复核](../history/M0-T26-S1-product-adapter-review.md)。XAML 工作区边缘保护及普通、现代外框的工作区交集计算已共享，公共及 Win16 自建外框测试双位宽通过，见[外框记录](../history/M0-T26-S1-frame-xaml.md)。
- **公共组件边界**：当前样式源码与测试统一归 `lib-win32-style` 的 access/application/appearance；旧 `lib-style`、`lib-win32/style` 已由S2归位，不再作为当前目录契约。公共主题状态由现有共享DLL承载，Win16与Win2000已有实际消费者；系统主题仅能恢复已观察到的请求。原S1轻量导入/懒加载、v5/v6与异常验证见[主题审计](../history/M0-T26-S1-theme-lifecycle-review.md)，当前进度见上方活动包。
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
- **剩余实现**：对话框与窗口创建事务已按S11交付；现代宿主与具体字体/GDI/renderer边界由S12/S13接收，不能凭公共索引称全T已完成。Win2000 控件外观、所有目标种类和真实应用的完整覆盖仍未证明。
- **执行约束**：不使用 computer-use，不创建或切换隔离桌面；保留源码与资源声明，不依赖或修改兄弟仓库。先发布再进行产品包测试；清理本次临时构建、测试副本和发布备份，保留必要证据。P52 已提交推送并复核远端；后续交付仍须逐批执行。
- **历史记录**：此前完整状态文字保存在[基线历史日志](../history/M0-T26-S1-baseline-log.md)；其中旧的“未发布”等状态可能已被本基线及候选记录取代。
