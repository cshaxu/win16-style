# Project Status

## Current Work

M0 T26开放，按[三层架构与14个S规划](../proposals/style-library-reorganization.md)执行S10扩展控件与运行时适配。S1审计、S2目录、S3 GUI根生命周期、S4启动、S5运行时接入、S6全局协调、S7窗口与标题栏、S8菜单、S9基础控件已收口；不把局部完成称为全T完成。

[S5交付](../history/M0-T26-S5-runtime.md)b08d601d已实际fetch并逐blob复核37个变更；同快照八文件先发布assets根/O:/winnt，随后118项去重实际包回归通过。另2项DWM旧失败与S4一致，不计通过。见[开发证据](../etc/evidence/t26-s5-runtime.json)与[最终交付](../etc/evidence/t26-s5-delivery.json)。双宽各17个公共样式归档门禁通过，1005个源码/构建/测试/工具输入一致，ABI及原资源保留。

S5运行时module/请求/通知/late批次已公共化，产品侧净减少105行。API创建链仍由产品attachment/window_titles拥有，明确按S11与dialog装配迁application/lifecycle，不称其为纯业务或遗漏；全局session/phase与管理协调已由S6提取并交付。已知失败/未覆盖见TODO及下方历史事实，S14统一复核。

S6已将session/control/start/worker/late/attach/manager/receiver/host/owner/trace迁入公共层并接入产品，删除14个产品C/H文件及六个空转发，必要线程导出合并至hook装配。927项来源回执、原资源及双宽各18个样式归档门禁通过。沙箱外同一自有目标12项原生接入全部通过，确认此前拒绝与AppContainer环境有关，未改生产策略；原沙箱失败保留在[开发证据](../etc/evidence/t26-s6-global-development.json)。

S6最终同快照双宽干净构建通过（x86约158秒，x64约176秒），1024个输入摘要一致，来源/资源/ABI及双宽公共归档门禁通过。同批八文件已先发布assets根/O:/winnt，再进行162项实际包聚焦回归：160通过、2项管理员ETW跳过；另2项DWM恢复旧失败仍归S7/S14，不计通过。见[交付证据](../etc/evidence/t26-s6-delivery.json)。S6 P1 `4d1cdcb7`已推送并实际fetch复核95个现存blob、14个删除路径及八文件摘要，见[远端证据](../etc/evidence/t26-s6-remote.json)。S6收口并自动准入S7，T26保持开放。

两轮S6构建/测试均结束，已核验无存活自有夹具及reparse后清理scratch/快照/发布备份548,376,032字节，保留.ninja/.ccache，见[清理记录](../etc/evidence/t26-s6-cleanup.json)。最终格式规范后重新构建、发布、测试，没有混用旧摘要；[全局生产链与最终映射](../history/M0-T26-S6-global.md)保留完整审计、失败对照和效率复核。禁止computer-use及私有桌面，不改或依赖兄弟仓库，不新增能力/权限/目标。

[S4事实](../history/M0-T26-S4-startup.md)及[交付](../etc/evidence/t26-s4-delivery.json)保留；T25按用户决定有限收口，未通过项不补记通过。

## M0 T26 S10 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation；S9正式交付并实际远端复核后自动S10，沿用M0/T26。 |
| Admission And Approval | 用户批准三层架构与14个S按能力合批自动推进，单人实施及第二角色复核。 |
| Objective | 扩展控件/VB/VCL的原生状态、通知、快照和资源所有权进入公共application；实际Win3x呈现策略独立，产品调用公共机制并删除旧接线。 |
| Non-goals | 不新增Win2000控件或扩大运行库支持集合；创建/对话框S11、现代宿主S12、最终图形/renderer契约S13。不能以这些边界留下本应共享的扩展生命周期。 |
| Reference Baseline | S9 P1 f9de3404：八文件先发布两处再146项聚焦通过，2项DWM旧失败不变；实际fetch复核107现存blob、16删除路径及八文件。 |
| Candidate Proposal | docs/proposals/style-library-reorganization.md；brief docs/history/M0-T26-S10-extended-controls.md；S9逐项交接docs/etc/evidence/t26-s9-controls-audit.json。 |
| Files And ABI Surface | appearance/win3x/control扩展状态/消息/绘制、application/control和win32运行时适配、产品attachment/controls及其全部实际测试调用者；保持五组件八文件及已有ABI。 |
| Applicable Rules | 纯C17、MinGW GCC/Ninja/ccache；公共对象持状态/资源，完整操作回调，无反向依赖/字段view/空包装；内层清理、外层报告。 |
| Verification | 先冻结有限输入与具体接收者；原生通知、项目/文本快照、字体/DC、所有权、换父/角色、重入移除/销毁/失败资源及双宽实际产品路径；整批八文件先发布再测和远端复核。 |
| Expected Markers | 同一扩展协议唯一生产实现；运行时身份与呈现资格分离，产品只组合实际策略；旧字段机制和重复路径删除，各保留项有具体原因/接收位置。 |
| Asset Needs | 当前五组件八文件及原图形/字体资源和声明；无兄弟仓库、额外服务或产品文件依赖。 |
| Reporting Requirements | 文件顺序覆盖、所有者/公共入口/实际消费与对应验证，生产增删净行数；错误/skip和未覆盖分开，不冒称所有框架/真实键鼠验收。 |
| Stop Conditions | 禁止computer-use和私有桌面，不改兄弟仓库/系统安全或其他任务；自建fixture有界清理，UAC同次等待一分钟后才询问。 |
| Exit Criteria | 有限清单逐项落实、扩展机制实际公共化与调用迁移、旧实现删除；原行为/资源/ABI/边界通过及规范发布提交推送和实际远端复核后自动S11。 |
| Original Owner Request | 所有style归lib-win32-style的access/application/appearance；极简统一、五大干净，逐文件审计，按完整能力合批交付。 |
| Similar-Issue Sweep | status/header/tab/progress/track/updown/toolbar/tree与owner/custom draw的身份、parent/witness、缓存代次、TLS作用域和子类通知；VB/VCL借用文本/图片/字体/DC及重入失败退场。 |

S9基础控件已收口，完整事实见[brief](../history/M0-T26-S9-controls.md)、[逐项输入](../etc/evidence/t26-s9-controls-audit.json)、[整批交付](../etc/evidence/t26-s9-delivery.json)及[实际远端](../etc/evidence/t26-s9-remote.json)。字体/protocol/tabstop、根安装/退场、basic/list/edit/combo消息与快照和scroll输入/资源/管理已由公共对象拥有，实际产品接入。生产含新文件净增478行（+1606/-1128）；扩展状态/运行时与创建/现代/最终renderer仍分别由后续S处理，不能称全T完成。

S9同快照双宽干净构建约160/187秒，1080源码/测试/工具输入一致，原资源及双宽各21样式归档/4进程归档边界通过。八文件已先发布assets根及O:/winnt，再146项去重实际包聚焦全部通过；2项DWM恢复旧失败不变、不计通过。首次测试驱动遗漏capture环境导致入口断言失败，保留后补齐并从失败项继续，无产品变化。正式发布的资源、版本和八文件SHA均已从远端读取复核。

S9全部终态scratch/源码副本/发布备份已提炼并清理约293MB，保留.ninja/.ccache及两处正式包；历史旧desktop入口违规与纠正保留于开发证据，不计合规通过。S10当前冻结清单/审计，未启动新构建；整体覆盖和既有失败由S14终审，T26保持开放。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 26
- Last allocated Td S: 21
- Last allocated T18 S: 64
- Last allocated T19 S: 3
- Last allocated T20 S: 19
- Last allocated T21 S: 44
- Last allocated T25 S: 8
- Last allocated T26 S: 10
- Last verified remote delivery: M0 T26 S9 P1，`f9de3404`；107现存blob、16删除路径及八文件复核。
- Open task: M0 T26 S10；扩展控件与运行时适配。

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
- **执行约束**：不使用 computer-use，不创建或切换隔离桌面；保留源码与资源声明，不依赖或修改兄弟仓库。先发布再进行产品包测试；清理本次临时构建、测试副本和发布备份，保留必要证据。P52 已提交推送并复核远端；后续交付仍须逐批执行。
- **历史记录**：此前完整状态文字保存在[基线历史日志](../history/M0-T26-S1-baseline-log.md)；其中旧的“未发布”等状态可能已被本基线及候选记录取代。
