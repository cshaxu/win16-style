# M0 T26 S4 启动接入

## 冻结边界与次序

基线254340e7（S3代码8e21860a），工作区干净；上一轮属于实际进展。普通启动链已核对：run16s.main创建挂起目标，经process_lifecycle调用ntwshk_install；context/installer/helper提供指针无关载荷、位宽路径及交接，process-hook实际安装；两风格DllMain读载荷、恢复暂存并加入startup事务，再注册传播和各自API集合。S3所有自有构建会话已结束并清理，无需重启旧构建。

现有context.c、installer.c、helper.c和system_process.c的通用机制迁入access/startup；参数化共享依赖模块前缀，不能把ntwshk写入公共库。32字节v2载荷、flags和session_cookie、目录编码、位宽/映像校验、helper身份及原错误保留。解析/路径/过滤器旧导出用产品.def别名保留；旧install签名需要依赖前缀适配，旧helper需要ExitProcess语义，合并在一份产品ABI适配文件，不保留原算法副本。删除无消费者/无导出的旧payload GUID。

两风格bootstrap的读/验证/恢复/事务编排公共化；模块选择、是否允许global session、API注册集合和自提权的run16s命令业务仍是产品配置。传播的active策略可由产品提供，共同A/W资格、安装和AsUser注册复用一个公共装配，并继续使用process-hook已有实现与相同生命周期。普通启动不全局扫描，不新建另一套MinHook或跨位宽机制；global/late协议按S5/S6继续，不在此发明新能力。

run16s的process_lifecycle现有创建后激活/等待逻辑属于无风格Win32基础，原样移入lib-win32/process-launch及对应tests；兄弟仓库仅只读、不修改、不成为依赖。CLI解析、诊断、UAC、Explorer/UWP别名选择保持产品侧。控件/对话框等实际API实现按S7–S12继续，S4只处理注册/启动协调。

实施按整批能力执行：公共载荷/安装和原ABI接入；bootstrap/传播装配及生产消费者；同步全部CMake直接编译消费者、来源与测试；双位宽全目标增量及针对性原生测试；源码冻结后同批干净构建、先发布再测试、提交推送及实际远端复核。禁止每文件完整交付。

## 验证契约

纯C独立测试覆盖载荷合法/损坏/路径和机器边界、bootstrap被动加载/错误/事务回滚、传播注册/策略和失败、进程激活资源所有权。既有staging/failure和四父子位宽组合（Win3.x及Win2000）复验安装、初始化、传播、挂起和绘制；通用库不反向依赖产品/风格私有头。原始资源和八产物保持；不使用computer-use或隔离桌面。旧DWM恢复失败及真实现代/全局未覆盖不计通过。

有限生产基线清单暂存build/t26-s4/baseline.json，所有迁移去向与旧路径删除另在本S交付审计对账。审计、开发测试和日志只暂存build/t26-s4，提炼后回收，保留.ninja/.ccache。

## 开发验证与效率检查

双位宽all增量通过，新增bootstrap/propagate与既有payload、process lifecycle、print-host、自提权、startup transaction/88-slot原生回归共16项通过；暂存安装双位宽通过，Win3.x四位宽边/A-W/三代/挂起8组合、Win2000四位宽边及父子外框/绘制/控件移除通过。首次Win2000跨宽调用只给了单宽目录，返回目标2；改用同批双宽四DLL自有包后通过，未改产品。

x64失败测试曾在最终总句柄计数处失败（53→56），而拒绝安装、helper超时/目标先退出均通过。固定HEAD旧源码最小对照最初通过；统一编译参数/目录后旧源码也失败、当前源码通过，证实该总量观察不稳定，尚未确定新增系统句柄来源。诊断性stdout预热未稳定改善，已删除，不放宽断言、不改产品；保留前后数量诊断，最终发布包再观察，若失败如实报告，不无限扩展排障。本轮新增最小对照仅暂存在build/t26-s4/baseline-failure，用完清理。

效率检查：实现已按完整启动能力批次合并，首次all增量覆盖全部编译调用方，不按文件发布；同类错误两次无确定结论后采用固定旧源码/相同工具链对照，避免为系统总句柄计数添加容错或改变挂钩流程。下一步完成依赖/来源/ABI复核后冻结源码，只做一次必要的干净交付。传播测试scratch已清理4,680,600字节，Win2000双宽临时包963,526字节亦已清理；开发JSON/日志保留到提炼交付证据后删除。

## 最终生产归属与验证

| 原生产职责 | 唯一当前实现 | 产品保留与原因 | 验证 |
| --- | --- | --- | --- |
| 载荷解析、读取与包内路径 | access/startup/context | ntwshk.def保留旧导出名/ordinal，injection.h只保留类型/常量/ABI声明 | context、损坏载荷、真实暂存/初始化 |
| 同宽安装与跨宽helper选择/校验 | access/startup/install、helper | ntwshk/startup.c两入口提供产品依赖前缀及ExitProcess语义，不保留算法副本 | 双宽staging/failure、四位宽边 |
| 系统打印host资格 | access/startup/target | 旧过滤导出别名及兼容声明保留，实际算法唯一 | ANSI/Unicode/引用/伪造路径 |
| 初始化事务装配 | access/startup/bootstrap及startup | entry、dllmain提供模块名、session资格和API准备操作 | 独立失败/回滚及两风格真实启动 |
| 后代传播装配 | access/startup/propagate | create_process提供全局active策略及私有自提权CLI；Win2000提供配置 | 独立操作契约、后端原生无样式传播、A/W三代、late复用回归 |
| 创建后激活/等待 | lib-win32/process-launch/lifecycle | main保留参数/风格选择与最终错误报告 | prepare/resume失败、已退出目标、普通启动组合 |

原context/installer/helper/system_process生产算法文件及run16s私有process_lifecycle旧路径已删除，CMake直接编译消费者与对应测试均迁移；旧ntw16s-installer归档删除。公共style-startup不反向依赖产品头，不传入整个风格私有结构；调用者选择既有shared/static hook provider，未复制后端。公共库不拥有目标恢复/终止，创建者按原生命周期负责；传播停用/discard不称物理卸载。

同一源码快照双位宽干净构建通过：x86 145.42秒、801 cache hits/0 misses；x64 172.19秒、901 hits/0 misses，双宽各16个样式公共归档依赖检查通过。1000个源码/构建/测试/工具输入逐摘要与工作区一致，六个DLL导出、ordinal及导入集合与S3一致。五头及资源声明原字节保持。

同批八文件已先发布assets根与O:/winnt，随后73项实际包聚焦回归全部通过，包括两种风格四位宽边、A/W、原挂起语义、无样式传播、两风格实际窗口/控件及既有生命周期组合。失败测试最终x86句柄88/88、x64 56/56；开发阶段观察不稳定仍保留原始证据，不虚称根因已查明。另2项Win2000 DWM恢复旧失败不计通过，与S3同条件一致，仍归S7/S14。没有执行全部历史矩阵、真实UWP/WinUI或全局宿主全覆盖。

源码/头按不检测rename的统计为28个路径、增加386/删除304/净+82行（包含迁移路径两端）；测试14个路径、增加506/删除418/净+88。新增代码用于完整公共操作、必要ABI与独立契约测试，不预建未来风格。最终命令、原输出、八文件摘要、工具链和已知失败见[交付证据](../etc/evidence/t26-s4-delivery.json)。当前已本地验证，提交推送及实际远端读取完成后才关闭S4。

完整测试/构建进程均已结束，必要证据提炼后清理开发scratch、完整快照和发布备份276,326,075字节；此前传播、Win2000包和最小对照另清理7,888,448字节。均先核对绝对归属与reparse point，保留.ninja/.ccache。
