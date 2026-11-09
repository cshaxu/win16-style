# Project Status

## Current Work

M0 T23 S7 已准入：盘点并验证 Paint、截图工具、设置和设备管理器，按收益修复外框问题；完成发布后等待用户验收。

本机只读清单：Paint 11.2609.401.0 为打包 FullTrustApplication，入口 PaintApp/mspaint.exe；截图工具 11.2608.41.0 为打包 FullTrustApplication，入口 SnippingTool/SnippingTool.exe；设置包入口 SystemSettings.exe；设备管理器为 System32/mmc.exe 与 devmgmt.msc。未发现相应已有进程；仅清单不证明实际样式或运行方式。登记 scratch build/t23-package/s7-inventory1，存放自有实例探针及输出，完成即清理。

S7 发布版探针：首次私有桌面中 Paint、截图工具和设置没有捕获到目标窗口；设备管理器 launcher exit=9、未捕获窗口，均未计通过。Paint 和截图工具目标进程脱离测试作业；按 PID、父进程和映像核验后结束两个自有残留，未操作身份不明的设置进程。普通桌面自有 Paint 和截图工具分别识别 MSPaintApp 与 XamlWindow，均为 NOREDIRECTIONBITMAP、144 DPI、DLL 加载且附着；Paint 实屏截图显示原生标题，截图工具被遮挡，二者均不计视觉通过。测试进程核验退出后 s7-inventory1 临时产物清理。

S7 开发候选：将这两种已观察的合成窗口类接入与 Chrome 相同的非客户区绘制表面及移除路径；尚未验证或发布。登记 scratch build/t23-package/s7-composed1，用于自有实例增量候选验证，结束后清理。

S7 增量候选实屏：自有截图工具在调整窗口后标题可见，移除后绘制表面销毁；自有 Paint 初始标题可见，但调整窗口后表面隐藏、恢复原生标题，移除后表面销毁。两者均未完成交互和完整回归。临时截图已提炼并删除；保留探针和候选供 Paint 几何诊断，结束后清理。

S7 Paint 隐藏原因：最大化时目标客户区超出可见工作区，现有表面布局拒绝越界几何。新增自有窗口最大化扩展客户区回归，修复前 x64 失败；候选对最大化客户区与可见边界求交后 x86/x64 各1项通过。诊断 DLL 为临时文件，不计最终包验证。

S7 当前增量候选：自有 Paint 和截图工具的非客户区表面均存在，真实屏幕截图分别显示 Win16 标题；各一次最大化和还原后表面保持可见，移除后附着清除、表面销毁、合成标志保留。截图工具最后一次前台切换失败，最终截图不能计作实屏通过；此前一次前台截图显示标题。此证据不覆盖菜单逐项或实际截图/绘画功能。设备管理器的 MMC 清单为 highestAvailable，正式启动 UAC 返回取消1223，未获得窗口，不计通过；设置当前已有身份不明的用户窗口，为避免影响该会话，未对其附着或关闭。自有实例与临时产物已核验并清理。

登记 scratch build/t23-package/s7-clean1，进行已验证候选的同快照双宽干净构建、同包测试及发布；完成后按清理规则回收。

s7-clean1 双宽干净构建、ABI、资源来源及公共库边界检查通过；登记 s7-package1 串行同包聚焦回归，结束后清理临时产物。

s7-package1 串行同包29项聚焦回归通过、零跳过；登记 scratch s7-actual1 用于同包自有 Paint 和截图工具短对照，结束后清理。

s7-actual1 同批包自有实例对照：Paint 与截图工具均观察到可见表面；查询及移除返回成功、附着状态清除、表面销毁。Paint 驱动退出0；截图工具观察器退出0，但外层 PowerShell 结束码为1且无额外错误输出，原因未知，不将外层完整测试计通过；两程序已确认退出。测试临时产物按规则清理。

s7-extra1 干净构建树的最大化几何回归 x86/x64 各1项通过；临时测试输出已清理。

登记 s7-publish1：发布 s7-clean1 四文件，使用 s7-package1 实包测试结果，发布后核对摘要并清理临时构建。

s7-publish1 因 O:/winnt/ntw16s64.dll 占用中断；依用户永久授权核验 ApplicationFrameHost PID17436、映像和启动时间 2026-11-09 08:04:54.455667 -08:00 后正常关闭。该共享宿主中的窗口可能关闭；登记 s7-publish2 使用同批候选重做全部替换并核对。

s7-publish2 四文件已发布到 assets 与 O:/winnt 且摘要一致；发布后不再有设置或 ApplicationFrameHost 进程，登记 s7-settings1 验证新建设置应用，完成清理。设备管理器此前 UAC 取消仍未计通过。

s7-settings1 使用发布版 `--app windows.immersivecontrolpanel_cw5n1h2txyewy!microsoft.windows.immersivecontrolpanel` 新建设置实例；新建 ApplicationFrameHost 加载 O:/winnt/ntw16s64.dll，拥有可见表面，实际屏幕截图可见 Win16 Settings 标题。前台切换失败且截图部分被其他窗口遮挡，不证明完整视觉或交互；设置窗口及进程已关闭，空闲系统宿主不强制结束。测试输出清理。

S7 P1 同批四文件已发布 assets 和 O:/winnt 并核对摘要，[证据和限制](../history/M0-T23-S7-P1-system-apps.md)；设备管理器提权取消未验证，Paint 和截图工具的系统别名返回1920，设置仅有限实屏观察。等待用户验收，S7/T23 保持开放。

s7-alias1 已发布包的 `run16s mspaint` 和 `run16s SnippingTool` 均因 Windows App Execution Alias 返回错误1920；两者完整安装路径此前可启动。别名为 APPEXECLINK 重解析点，其内部数据布局不是公开兼容契约；本 S 不按私有结构猜测转接，记录为待办。测试进程退出，临时输出已清理。

M0 T23 保持开放。S1 记事本已验收通过并收口；S3 Explorer 已获用户手动验收并收口；S4 Terminal/cmd/conhost 基本通过用户验收，有限收口；S5 Codex Desktop 按用户要求跳过，未实现或验收；S6 Chrome 已获用户验收，收口；T23 汇总回归和收口审计待执行。
S2 Task Manager 按用户要求有限收口，不再继续要求用户批准该项 UAC；顶部重复图标、白条和拖动标题丢帧转入 TODO，不能记作已修复。双宽干净包、29 项实际包聚焦及14项核心/注入回归通过，四文件已发布 assets 和 O:/winnt，P1 `6afe9420` 已推送并逐项读取远端复核；[有限交付记录](../history/M0-T23-S2-task-manager.md)。准入 S3 Explorer，先识别真实窗口承载及共享 Shell 边界。
P6 `e37da0db` 四文件已发布至 assets 根目录及 O:/winnt，双宽构建、核心10项及实包29项通过；最终真实菜单截图复核未完成，不计通过。
详细记录见 [S1交付](../history/M0-T23-S1-notepad-delivery.md)和[菜单修复](../history/M0-T23-S1-system-menu-hover.md)。
S3 P4 映射身份修复已通过双宽干净构建、29项实包回归并发布至 assets 与 O:/winnt。
S3 物理菜单复测：用户批准后，自建私有桌面以真实鼠标打开菜单，悬停 Move 和 Size 均保持可见并正确高亮；截图显示 Win16 菜单，原桌面和指针已恢复。该测试不证明瞬态闪烁、命令执行或拖动效果；后续用户已手动验收通过，详见 S3 记录。
S3 待修回归：用户报告切换文件夹后标题回退；自建私有桌面实际屏幕复现从 Win16 变为系统外框。独立对照中附着仍在、DWM 非客户区绘制报告关闭，但尚未确定最终覆盖来源。XAML 标题更新延迟重绘候选未消除残影；单独拦截标题更新后仍可导航到此电脑且系统外框仍出现。该候选已撤回，后续图标消息修复已交付并获用户验收。
用户已永久批准本任务私有桌面短暂切换测试，无需每次询问；仍需确认输入桌面可用并在完成后恢复桌面和指针。最新候选实屏尝试因输入桌面不可用而未执行。
S3 图标消息候选已双宽干净构建、29项聚焦实包及双宽各3项外框回归通过，并发布 assets 和 O:/winnt。增量候选实屏导航未见回退；最终干净包因输入桌面不可用未做同字节实屏验证，自动闪烁测量未完成；用户已手动验收通过。占用 DLL 的桌面 Explorer 已按用户永久授权结束并重启，原文件夹窗口已关闭。
登记scratch：build/t23-package/s3-icon-clean1，后台进程消失且无结果，保留作调查；build/t23-package/s3-icon-clean2，为重新启动的同快照双宽干净构建及候选包。
登记scratch：build/t23-package/s3-icon-package1（同批四文件聚焦回归），build/t23-package/s3-icon-publish1（占用导致中断的发布），build/t23-package/s3-icon-publish2（完成的发布与摘要）。
S3 P1 同批四文件已通过干净构建、29项实际包回归及私有桌面标准启动对照，并发布至 assets 与 O:/winnt；S3 已获用户手动验收，T23 保持开放。[交付与限制](../history/M0-T23-S3-explorer.md)。
按用户确认的顺序准入 S4 Terminal、cmd 与 conhost；保留已有用户会话，不以进程名批量操作。保留登记的 S1/S2 scratch，核验仍映射的旧 DLL 后安全清理。

登记scratch：build/t23-package/s2-launch1，Task Manager 自有挂起创建、隔离桌面启动与权限诊断。
登记scratch：build/t23-package/s2-uac-build1，双位宽增量构建自提权探针。
登记scratch：build/t23-package/s2-uac-fix1，双位宽增量构建自重启提权转接候选；候选尚未验证或发布。
登记scratch：build/t23-package/s2-uac-regression1，候选双位宽首次注入及 staging 聚焦回归。
登记scratch：build/t23-package/s2-uac-regression2，候选普通消息及对话框聚焦回归。
登记scratch：build/t23-package/s2-uac-handle1，双位宽探针增量构建与 Shell 句柄自动回归。
登记scratch：build/t23-package/s2-uac-unit1，候选转接单元测试双位宽 Ninja/CTest 回归。
登记scratch：build/t23-package/s2-uac-fix2，路径缓冲区调整后的双位宽候选增量构建与转接单元测试。
登记scratch：build/t23-package/s2-uac-fix3，策略范围及原子读取修改后的双位宽增量构建和单元回归。
fix3 双宽 Ninja 构建及 self-elevate-unit 各 1/1 通过、无 skip；这是模拟 Shell，不是 Task Manager 绘制或真实提权的证明。
登记scratch：build/t23-package/s2-uac-candidate2，调整后候选 DLL 的自有提权对照，不是最终发布包。
新版 x64 自有对照父 PID4308 attached=1；120 秒外层等待返回 WAIT_TIMEOUT=258、launcher_exit=259，无子报告；候选目录相关自有进程检查无残留。该次不计正例，不能推断 UAC 原因。
fix2 双宽 Ninja 构建成功，launch/self-elevate-unit 各 1/1 通过，无 skip；仅验证模拟 Shell，调整后真实提权路径尚未重测。
追加 Shell 结构体字段断言后，双宽 Ninja/CTest 各 1/1 再通过；x86 Ninja 提示 premature end of file 并自动恢复，当前成因未知，需在交付构建前核验缓存状态。
x86 的 .ninja_deps 对该目标显示 deps not found；确认无活动构建后将损坏缓存逐字节备份至 s2-uac-unit1/x86-ninja-deps-damaged，重建生成缓存与该目标，后续 dry-run 显示 no work to do。其他目标依赖记录需随其下次构建重建；原损坏原因未知。
登记scratch：build/t23-package/s2-shell-matrix1，Shell 正常句柄及错误回退自动回归。
登记scratch：build/t23-package/s2-custom-frame1，自有客户区顶到外框顶部的几何附着/恢复模型回归；不是 Task Manager 实测。
登记scratch：build/t23-package/s2-clean1，当前候选源码快照的双位宽干净构建与 ABI 检查；不代表最终验收或发布。
clean1 在编译前因两个已修改文件的本地来源摘要未更新而停止；已保留原始来源和许可声明并更新本地摘要，source-manifest 自检通过。登记 build/t23-package/s2-clean2 重做当前快照的干净构建。
该模型 x86/x64 各 1/1 通过，附着和移除后客户区屏幕矩形保持，移除后外框矩形与原始一致；不证明真实 Task Manager 的绘制或交互。
当前候选治理自检及13项拒绝负例通过，git diff --check 通过；未提交的 QUEUE 与 xaml-control-style 候选属于其他工作，继续保留。
该隔离包回归 x86/x64 的普通句柄、非自身无效目标、带句柄标志的无效目标共 6/6 通过；只比较这些原生/候选输出，不证明真实 Task Manager 或 runas 句柄成功路径。
路径缓冲区调整后的 fix2 DLL 再次运行同一隔离包矩阵，6/6 通过；仍不证明真实提权或 Task Manager 绘制。
该回归完成：x86/x64 各 1/1 通过、无 skip；仅为模拟 Shell 的转接单元测试。
新增无 UAC 转接单元测试：双宽 GCC 直接运行通过，验证同映像参数改写、取消错误和 hInstApp，以及句柄标志、其他映像、open、global 会话的原路径；真实 Shell/UAC 不由模拟证明。
候选将三个大路径缓冲区从目标线程栈移至临时堆分配，避免占用约192KiB栈；分配失败沿用原 Shell 路径。调整后双宽模拟转接单元测试通过；此前候选实测使用旧缓冲区版本，调整后尚未重新实测或发布。
进一步收紧：全局会话不安装该 Shell 钩子；双宽模拟测试验证全局调用原路径及注册时不安装。该修改后的 DLL 未做实际提权测试或发布。
策略审计发现仅传播载荷经 --elevated 会被启动器重置为样式+传播；候选现仅在 flags 精确为样式+传播时安装/执行转接，双宽模拟测试验证仅传播和 global 不安装。最新修改尚未真实提权验证或发布。
候选运行时策略判断改为与现有 session 路径一致的原子读取，避免与全局策略位更新发生非原子读取；双宽模拟单元测试通过。尚未实测并发竞态或发布。
句柄自动回归首次 x86 因启动器目录缺少 32 位 DLL 而失败；改为测试自有隔离双宽包后，x86/x64 两项均通过。覆盖普通 open 成功句柄，不代表 runas 句柄或 Task Manager 正例。
该回归 x86 未配置对应测试、不得计通过；x64 单独执行 dialogs/messages 2/2 通过，无 skip。
该聚焦回归完成：x86 staging 1/1、x64 staging 与 automatic 2/2，均无 skip；x86 未配置 automatic，不能计作该项通过。未覆盖真实 Task Manager 或新的 Shell 转接边界。
登记scratch：build/t23-package/s2-uac-candidate1，候选 DLL 与已发布启动器组成的开发验证目录，仅用于自有提权探针；不是最终同批发布包。
候选自有 x64 对照：父 PID27196 加载候选 DLL 且 attached=1；按实测 0x400 runas 自重启后子 PID27420 elevated=1、加载同目录候选 DLL 且 attached=1。仅证明该受限调用的自有窗口接入，不证明实际绘制、Task Manager 或其他 Shell 标志。
候选 x86 对照：父 PID24228 加载候选 DLL 且 attached=1；Shell 返回 1223（取消），没有子进程报告。x86 提权正例未完成，不能计通过。
后续有界 x86 正例：父 PID20088 加载候选 ntw16s32.dll 且 attached=1，提权子 PID25516 加载同目录候选 ntw16s32.dll 且 elevated=1、attached=1；驱动记录 wait=0、launcher_exit=0。仅为受限调用自有窗口接入，非真实 Task Manager 或绘制证明。
重试父 PID12932 attached=1，但 ShellExecuteExW 等待系统响应超过测试预算；核验映像、出生时间、父子关系后结束该自有探针，启动器随之退出，无子报告。现有 90 秒等待仅发生在 Shell 调用返回之后，不能约束 UAC 等待；测试驱动需增加外层有界管理，不能将该次计为通过。
新增 Job 外层驱动（120 秒）双宽编译通过；自有假目标对照中有效报告返回0、旧报告拒绝8、attached=0 返回9。真实 UAC 超时/清理及脱离 Job 的提权后继仍未验证。
外层驱动使用自有挂起假目标和 500ms 预算得到 WAIT_TIMEOUT=258、退出1460，目标 PID27560 已结束；此结果只证明 Job 内目标清理，不证明 UAC 后脱离 Job 的进程清理。
候选只尝试 runas、自映像、fMask 精确为 SEE_MASK_FLAG_NO_UI 且非 global 会话的转接；其他调用沿原路径。转接改变 UAC 中请求执行的映像为 run16s.exe，且 Shell 返回只表示启动器获准，不保证其后目标创建成功；这两项用户体验/错误语义差异尚未解决，不能据自有窗口正例宣布产品修复。
非自身不存在目标的 x64 无 UAC 负例：直接运行与候选 run16s 启动均返回 result=0、error=15；仅覆盖该路径，不代表所有 Shell 参数或错误均已验证。
x86 同一负例直接运行与候选启动也均为 result=0、error=15。
带 SEE_MASK_NOCLOSEPROCESS 的非自身不存在目标，双宽直接/候选均为 result=0、error=15、handle=0；它不验证成功创建时的真实句柄所有权。
双宽自有正常 open + SEE_MASK_NOCLOSEPROCESS 对照，直接和候选均取得可等待句柄、子退出37并由调用者关闭；未验证 runas 成功的句柄语义。
真实 Task Manager 候选对照：新建首进程 PID7572（父候选启动器 PID22624）后，两者均退出；没有捕获退出码或最终目标窗口，标准输出与错误输出为空。原有 PID18744 未操作；本次结果不计通过，原因待查。
随后真实对照由候选启动器 PID15984 创建首进程 PID10120，出现新 Taskmgr PID18692（父 PID19032），但查询时已退出；未保留出生时模块、窗口或退出码，不能认定绘制成功或崩溃原因。原有 PID18744 未操作。
候选原生跟踪另建首进程 PID25092 并加载候选 DLL；90 秒预算结束前未记录转交/退出或最终窗口，自有调试进程和该首进程随后均已结束。不据此认定 UAC 结果或产品故障原因。
只读 objdump 显示本机 System32/Taskmgr.exe 的静态导入包含 SHELL32.dll；这排除“完全没有静态导入 Shell”的猜测，但尚未证明候选在该进程实际成功安装钩子。
原生 EnumWindows 只读检查现有 PID18744：存在可见 TaskManagerWindow（HWND 0x30522，标题 Task Manager）；此前 Get-Process 返回 MainWindowHandle=0 不能证明没有可见窗口。未对该实例操作；是否导致新实例退出仍未证实。
只读结构探针记录该窗口 TID7344、style=0x14cf0000、exstyle=0x100、GetDpiForWindow=144，客户区起点与外框顶边相同；子层包含可见 DesktopWindowContentBridge、DirectUIHWND 和隐藏 CoreWindow。探针未设置 DPI awareness，记录的几何不可直接当作物理像素；未发送消息或附着样式。
DPI 感知上下文复测成功：同一 HWND 在144 DPI 下 outer=(-9,110,765,848)、clientorigin=(2,110)、client=(0,0,752,727)，可见 DesktopWindowContentBridge 充满客户区。仅为原生前状态，不验证样式或恢复。
S1 原生外框的实际证明仅针对 Notepad 根 HWND，复用 Ntw16sWindowAttach；源码搜索没有独立 TaskManagerWindow 专项适配。不能因同样加载 XAML 或自有普通窗口附着而推断 Task Manager 已支持，仍需真实窗口验证。
管理员跟踪已启动：高权限启动器 PID23828 创建 Taskmgr PID7392，加载候选 DLL 后该进程退出码0。日志没有记录窗口观察，原有 PID18744 同时存在；不能据退出0推断样式、窗口或复用原因。跟踪 PID25168 仍在其 90 秒预算内。
管理员跟踪 PID25168 已结束，自有 Taskmgr PID7392 不再存在；记录器的汇总 exit 字段为 null，因此只使用调试事件中明确的目标退出码0。原有 PID18744 仍在，未操作。
任务 scratch 跟踪器改为所有自有被跟踪进程退出后立即结束；自有 cmd 对照捕获两个进程均退出0，总耗时302ms，不再无条件等待90秒。管理员窗口效果尚未验证。
新增仅记录被跟踪 PID 的原生窗口 CREATE/SHOW 观察；自有短时窗口 PID25856 捕获到可见 Win16OwnedTraceControl 的 CREATE 和 SHOW，进程退出0。此为探针正例，不是 Task Manager 窗口结果。
管理员真实窗口跟踪的系统 UAC 返回取消，未创建 trace-window.exe 或日志；该次不计通过，未重复发起。
该增量构建完成：Ninja x86/x64 均编译 uac-self-elevate，原有 launch/elevation-unit 各 1/1 通过；此单元测试不覆盖目标自行提权，不能替代负例或产品修复验证。
补充实际转交证据：原生调试器观察首个 Taskmgr PID29760 已加载 O:/winnt/ntw16s64.dll，随后调用 ShellExecuteExW（verb=runas，目标为 System32/taskmgr.exe），并以 0x80004004 退出；其后继 PID28396 当时由 PID29760 创建。现有 CreateProcess 传播未覆盖此提权路径。提权后外框接入及恢复尚未验证，不据 DLL 加载声称样式生效。当前另有 PID18744，来源未核实，不操作。
工具链 shellapi.h 对照：本次调用 fMask=1024 (0x400) 是 SEE_MASK_FLAG_NO_UI，不包含 SEE_MASK_NOCLOSEPROCESS (0x40) 或 SEE_MASK_NOASYNC (0x100)。这仅是此调用的观察，不代表其他调用可忽略句柄所有权。
测试审计：tests/run16s-exe/elevation_unit.c 覆盖启动器自身 ERROR_ELEVATION_REQUIRED 的 UAC 重入参数，uac_driver.c 覆盖提权启动包及载荷；两者均未覆盖已注入的目标进程自行 ShellExecuteExW(runas) 重启，不能作为 Task Manager 传播通过的证据。PID18744 当前仍存在，路径和窗口查询受权限限制，来源仍未核实。
自有 self-elevate 探针经发布版 run16s 启动并调用 ShellExecuteExW(runas)，实际提权子进程 PID18720 报告 elevated=1、dll 路径为空；负例证实此自提权路径未传播 DLL。该报告不验证窗口附着或绘制，产品修复及回归仍待完成。
增强对照同时记录父子：父 PID7032 加载 O:/winnt/ntw16s64.dll，提权子 PID404 报告 elevated=1 且 DLL 路径为空。此结果证明该夹具的模块传播断点；不证明父钩子初始化或子窗口绘制。
x86 同一自有对照：父 PID8328 加载 O:/winnt/ntw16s32.dll，提权子 PID9920 报告 elevated=1 且 DLL 路径为空；同样仅证明模块状态，未证明初始化或绘制。
增强窗口探针 x64：父 PID12444 加载 O:/winnt/ntw16s64.dll，提权子 PID2652 报告 elevated=1、dll 为空、attached=0；为当前发布版的窗口附着负例，不代表实际绘制验证。
父窗口附着增强对照：PID4828 的 DLL 路径为 O:/winnt/ntw16s64.dll，attached=1；同次启动器 PID27616 已退出但未产生子进程报告，未捕获退出码或 UAC 结果，子进程部分不计通过或失败。
原生挂起创建成功 error0，进程立即回收；隔离桌面 run16s --wait taskmgr 返回0x80004004，未发现自有窗口，不能计通过；直接启动同一隔离桌面对照同样返回0x80004004且无自有窗口；目前不能归因于注入。现有PID26784仍存活，隔离桌面或Job及单实例交互的原因待查。用户退出既有实例后普通桌面标准启动产生Taskmgr PID26460（父4448已退出），TaskManagerWindow在144DPI显示；管理员只读模块检查只发现系统XAML，未发现ntw16s DLL，需定位中转及权限边界；本次实例普通权限无法结束，已核验身份后管理员关闭。复用原生调试器观察标准启动：run16s PID26676创建Taskmgr PID14784，后者加载O:/winnt/ntw16s64.dll并以0x80004004退出；最终Taskmgr PID21116的父进程为14784，管理员只读检查无ntw16s模块。仅记录观察，提升或创建API尚未确定。

S4 初始实测：标准 cmd 私有桌面探针检测到已有 Windows Terminal 进程，按保护条件跳过，未操作该会话。标准 conhost 自建窗口可见且目标模块枚举到发布 DLL；线程探针发送成功但附着及结果属性均为0，不能判为附着或样式通过。两次自建任务按自身清理机制结束。先前多窗口伪控制台 owner 关联只是候选，边界见 proposal。

S4 自有 conhost 进一步对照：同一自建作业内 conhost 的线程钩子捕获窗口消息；外部观察的窗口 PID/TID 属于 cmd，在回调内部则为实际 conhost 消息线程。现有附着因 WS_EX_LAYERED 返回50；原型仅对自建窗口临时移除分层后附着和移除均成功，离屏截图显示标题与控制台文字。原始透明度180时仅恢复样式位会变255；显式恢复属性后180、标志和样式位一致，附着期间自建控制台 echo 输入输出通过。未验证实屏、鼠标、一般分层窗口、异常清理或产品启动路径；原型未发布。

S4 后续候选：仅对具有可查询恒定分层属性的 ConsoleWindowClass 保留 WS_EX_LAYERED 并尝试既有外框附着；自建 conhost 的探针在宿主真实 UI 线程手动附着、移除并恢复分层属性，实屏截图出现 Win16 标题。该结果不是标准启动自动附着，候选未发布。当前会话新建私有桌面返回错误 5，alpha=180 对照未执行；x64 core/window-thread 通过，core/frames 与 core/pending-frame-owner 因 PRIVATE_DESKTOP_CREATE_FAIL error=5 未执行，不算通过。继续查标准启动接入及桌面权限，不影响用户已有终端。

S4 补充：在允许创建私有桌面的执行环境中，自建 conhost 候选保持 WS_EX_LAYERED，透明度设为180后附着和移除均成功，移除时仍为180；测试后探针恢复原属性。产品 Ntw16sWindowThreadHook 对自有宿主的线程定向接入与移除均返回成功，但仍由探针手动触发。作业内快照见 conhost 为启动器子进程、cmd 为 conhost 子进程；这是本次观察而非普遍保证。同一候选 x64 core/frames、core/window-thread、core/pending-frame-owner 各通过；不代表标准启动已实现。

S4 外框候选已完成 x86/x64 增量编译；补充普通分层窗口显式设置 alpha=180 后仍应拒绝安装的负例，双宽 core/frames 各 1/1 通过。此负例不替代真实 conhost 的自动接入与恢复测试。

S4 标准 conhost 候选：启动器针对本次创建的系统 conhost，查找其子进程报告的唯一控制台窗口，并使用该宿主线程定向产品回调。自建作业与私有桌面中，标准候选命令启动后独立线程探针查询 attached=1，随后 remove=1；窗口和宿主均为本次测试创建。首次候选因客户端句柄缺少 SYNCHRONIZE 返回1400，已修正后复测；尚未验证并发身份、输入输出、Terminal/cmd、恢复和完整交付。代码未提交未发布。

S4 候选进一步验证：标准启动自有 conhost 自动附着后，向其 cmd 输入 echo，成功观察回显与输出，移除后原分层位及透明度255一致；启动器重查子进程出生时间和父子关系后的候选重测也通过。x64 启动器目标选择单元测试及 Explorer 选择回归各通过。当前存在既有 WindowsTerminal PID24120（由权限外只读检查确认），独立 Terminal 探针按保护条件跳过，未操作该进程；不能计为 Terminal 或 cmd 通过。

S4 负例：启动器针对本次创建且已退出的宿主返回 ERROR_PROCESS_ABORTED；x64 launch/console-host 通过。窗口枚举失败时现在保留非零错误。Terminal 上游资料指出其窗口可能复用已存在的进程，且曾移除 isolated mode（https://github.com/microsoft/terminal/issues/19468）；目前本机已有用户 Terminal，仍需证明仅针对新窗口且不改变现有会话的测试与产品路径。

S4 自建 conhost 参数对照：`run16s conhost.exe cmd.exe /d /k` 自动附着查询成功，控制台 echo 输入及输出标记成功，移除后分层属性保持；该次仍是开发候选，不是四文件发布包。

S4 快速退出对照：自有 `conhost cmd /d /c exit 37` 在私有桌面中结束，候选不再因宿主已退出输出附着失败；探针未发现窗口，不据此推断退出码语义或其他参数覆盖。

S4 已将自建私有桌面的 conhost 自动接入、echo 输入输出和移除整理为纯 C 原生集成测试 launch/console-integration；x64 构建及测试 1/1 通过，耗时约1秒。测试仅查询实际附着与控制台内容，不代表物理绘制、Terminal 或正式包通过；辅助观察 DLL 只用于测试，不加入产品包。

S4 分层边界自动回归补充：自有 ConsoleWindowClass 测试窗口在没有可查询分层属性时拒绝附着，设置恒定 alpha=180 后允许附着和移除，移除后样式位、alpha 与标志不变；x86/x64 core/frames 均通过。普通分层窗口仍被拒绝。该模型不替代其他桌面合成模式验证。

S4 Terminal 新证据：自建私有桌面内启动 Terminal 后，从其测试 cmd 的 PseudoConsoleWindow 查到唯一 CASCADIA_HOSTING_WINDOW_CLASS owner；独立 run16s cmd 中 cmd 加载 DLL，Terminal 宿主未加载 DLL，故尚未自动接入。无持久系统修改的临时原型在真实自有 Terminal GUI 线程调用已有外框附着，保留 WS_EX_NOREDIRECTIONBITMAP，另建自有分层表面；物理屏幕截图出现蓝色标题区域且标签行位于其下，移除外框及销毁表面均成功，扩展样式前后均为 0x00200100。该表面目前只是纯色，无标题文字、按钮、命中测试、重绘、缩放、透明度或可靠清理验证，不是产品实现；测试窗口与进程已关闭、桌面已恢复。上游 Terminal 源码说明该标志用于 vintage 透明度（https://github.com/microsoft/terminal/blob/main/src/cascadia/WindowsTerminal/IslandWindow.cpp），因此不能无验证地长期清除此标志。

S4 Terminal 候选：Win16 UWP 绘制层新增已分配非客户区模式，供 Terminal 在保留 WS_EX_NOREDIRECTIONBITMAP 时使用；真实自有 Terminal 的定向附着与移除均成功，实屏可见 Win16 标题和下方标签行，移除后扩展样式仍为 0x00200100。启动器从自身 PseudoConsoleWindow 的 root owner 定位单个 Terminal，标准候选 `run16s cmd` 对照中宿主加载候选 DLL 且 attached=1；该次为自有窗口、结束后进程退出。双宽 UWP frame 回归、x64 conhost/Explorer 相关三项回归通过。尚未完成标准命令实屏、标题按钮/菜单、移动缩放、透明、共用进程隔离、run16s Terminal 直接启动及完整发布；不计 S4 验收通过。

S4 直接 Terminal 候选：创建时附着路径也启用非客户区绘制层；自有标准启动对照中 DLL 已加载、附着状态为1，私有桌面144 DPI实屏可见 Win16 标题与标签行，窗口最大化和还原状态分别为3和1。截图同时显示标签行右侧仍有原生最小化、最大化和关闭图标，与 Win16 外框按钮重复；尚未修复，不能计验收通过。该截图只证明当次绘制，不证明移动、鼠标或移除后恢复。

S4 重复按钮调查：自有 Terminal UI Automation 可见 Minimize、Maximize、Close，子窗口仅见 DRAG_BAR_WINDOW_CLASS 与 XAML bridge 等；临时隐藏并恢复 drag bar 后三按钮仍在。另在宿主 GUI 线程临时清除 WS_MINIMIZEBOX/WS_MAXIMIZEBOX 后，Win16 按钮消失但 Terminal 三按钮仍在；原样式已恢复。故不能通过隐藏该拖拽窗口或清除这两个样式位安全解决，重复按钮仍未修复。

S4 直接 Terminal 候选增加启动前窗口快照，只选择本次新增且映像匹配的 Terminal 窗口以处理宿主复用；自有无既存实例的直接启动复测 attached=1、实屏显示标题、最大化/还原状态正常，x64 三项启动回归通过。已存在宿主时的新窗口分支仍未实测，且启动已有窗口而无新窗口可能等待上限，需验证处理；重复 XAML 原生按钮仍未解决。

S4 共享宿主自有对照：先独立创建 Terminal 第一窗口，再用候选 run16s 打开第二个；两窗口同属自有 PID，第二窗口 attached=1，第一窗口 attached=0，均按唯一身份关闭，宿主退出。仅验证此次新窗口选择与附着状态，不证明所有窗口竞争场景。Terminal 上游源码明确 caption 按钮由其 XAML 标题栏提供、拖拽 HWND 只转发交互（https://github.com/microsoft/terminal/blob/main/src/cascadia/WindowsTerminal/NonClientIslandWindow.cpp）；目前不修改客户区 XAML，重复按钮仍为已知限制。

S4 自有 Terminal 真实鼠标对照：私有桌面单击 Win16 左上角按钮后，GUI 线程进入菜单模式且 owner 为绘制表面；发送 Esc 后退出菜单模式，鼠标位置和桌面已恢复。随后原生最大化及还原状态正确。该次未检查菜单逐项命令或拖拽；重复 XAML 按钮仍存在。

登记 scratch：build/t23-package/s4-terminal-clean1，用于当前候选同快照双位宽干净构建及四文件包；本候选包含 Terminal 重复 XAML 按钮已知限制，构建完成不代表 S4 收口。

s4-terminal-clean1 后台进程未创建源码快照即退出，日志和结果均为空，保留该目录；登记 build/t23-package/s4-terminal-clean2 用于有权后台重试。

干净构建 s4-terminal-clean2 在 x86 测试夹具链接时暴露缺少 win16-window-uwp 依赖，结果 failed；已修正 test-painter-common 的公开链接依赖。x64 全目标增量构建通过，x86 全目标增量构建仍在进行；此前失败包不得用于发布。

双宽全目标增量构建已完成；登记 build/t23-package/s4-terminal-clean3 为依赖修复后的同快照干净构建及候选包。

干净构建 s4-terminal-clean3 双宽及依赖边界通过，候选四文件已生成；登记 build/t23-package/s4-terminal-package1 为同包聚焦回归，不以构建通过替代包测试。

同批干净包补充：自有 conhost 集成程序针对 s4-terminal-clean3 候选启动器与同目录 DLL 返回0（附着、echo输入输出及移除）；独立 Terminal 中候选 run16s cmd 的宿主 DLL 已加载且 attached=1，测试宿主正常退出。标准包回归 s4-terminal-package1 仍在运行。

s4-terminal-package1 的 cmd-nested 64→32 项失败：测试在等待标记时报告失败，但最终 transcript 包含 inner 标记，子进程及 DLL 实际出现；原因未判定，不能计通过。登记 build/t23-package/s4-terminal-package2 同包聚焦重测并保留首次证据。

s4-terminal-package2 同包29项聚焦回归通过、零跳过，cmd-nested 四组合均通过；首次 package1 的间歇失败原因仍未判定，不删除证据。为发布已按永久占用授权核验并结束映射 O:/winnt/ntw16s64.dll 的 Explorer PID22072（2026-11-08 16:02:47.8184880 -08:00），随后重启 Explorer；原 Explorer 文件夹窗口可能已关闭。登记 build/t23-package/s4-terminal-publish1 存放发布前备份和摘要。

S4 P1 候选四文件已按同包测试摘要发布 assets 与 O:/winnt；[交付与已知限制](../history/M0-T23-S4-P1-console-terminal.md)。重复 Terminal XAML 按钮、首次 cmd-nested 间歇失败原因及用户验收仍未解决，S4/T23 和目标保持开放；P1 已提交 00d3e6c6 并推送，远端 main 已读取核对。

S4 P3 开发中：已复现 P1 在传统 conhost 内运行 run16s cmd 时宿主 DLL 未加载、窗口未附着。自有探针的当前进程 ProcessConsoleHostProcess 查询给出所属 conhost PID（低两位为标志），候选仅验证系统 conhost 映像和当前 HWND 后向该宿主定向附着；查询不可用时保持原状。该 NtQueryInformationProcess 信息类别为 Windows 内部接口，未来兼容性不可保证，动态查询并明确降级。自有私有桌面负例 attached=0，修复候选正例 attached=1，echo 输入输出及移除成功；新增 launch/console-inherited，与新建控制台和边界测试共 3/3 通过。候选尚未干净打包、发布或提交，不改变 P1 已发布版本。

S4 P3 增量回归：x64 启动器边界、直接 conhost、继承 conhost 及 Explorer 选择共4/4通过；登记 build/t23-package/s4-inherited-clean1 为双宽同快照干净构建。

S4 P3 s4-inherited-clean1 双宽干净构建及包检查通过；登记 build/t23-package/s4-inherited-package1 为实际包回归。

S4 P3 同批包29项聚焦回归通过、无跳过；传统控制台直接与继承模式的同包输入输出/移除测试通过。登记 build/t23-package/s4-inherited-publish1 为发布备份及摘要，交付后按测试清理规则回收不再需要的临时构建。

S4 P3 同批四文件已发布 assets 及 O:/winnt，摘要与通过的包一致；[交付及限制](../history/M0-T23-S4-P3-inherited-console.md)。用户基本验收通过；Terminal 重复 XAML 按钮与 P1 首次 cmd-nested 间歇失败原因仍未解决，列入 TODO；S4 有限收口，T23 保持开放。

发布版 P3 自有 Terminal 实屏短测：Win16 标题及标签行可见；真实鼠标打开系统菜单，Esc 后关闭；最大化与还原状态分别为 3、1，扩展样式为 0x00200100，测试进程退出并恢复桌面和指针。原生 XAML 三按钮仍重复；该次不证明菜单每项命令、拖拽或所有透明模式。测试用临时探针按清理规则移除。

发布版 P3 自有传统 conhost 中运行 run16s cmd 的实屏短测：附着后可见 Win16 标题与滚动条；移除后可见原生标题和滚动条，分层属性保持，桌面恢复且测试实例退出。该次不证明每种 DPI 或透明模式；临时探针与截图按清理规则移除。

按测试后清理规则，已清理 s3-private-attach 内99个已结束的 core/launch 测试输出目录；保留尚需的 Chrome 后续研究原型及其目录，其后续用途结束即清理。

S5 只读身份调查：本机 OpenAI.Codex_26.1103.3517.0_x64__2p2nqsd0c76g0 清单中的 App 入口为 app/ChatGPT.exe（FullTrustApplication），文件产品版本 157.0.8103.39；当前已有该映像的主进程与 GPU、renderer 等子进程。resources/codex.exe 是不同入口，不能仅按文件名选择。受限环境 Get-AppxPackage 返回访问拒绝，非受限只读清单和进程查询可用。先前只读窗口观察到主进程的 Chrome_WidgetWin_1 无 WS_CAPTION，client 顶部偏移为0，加载 chrome.dll；自有模型中现有附着状态为真但无蓝色标题，显式添加 WS_CAPTION 后才出现标题。该模型不能证明真实 Codex 的可用性或安全恢复；先核实独立实例，不操作已有会话。

S5 进一步只读检查已安装应用的 app.asar：早期入口读取 CODEX_ELECTRON_USER_DATA_PATH 并据此设置 Electron userData，随后调用单实例锁；另有 macOS 演示启动代码同时隔离 CODEX_HOME 与 userData。此证据提供独立实例候选，但尚未证明 Windows 上隔离启动、锁的隔离范围及其他持久状态，不能据此启动或关闭现有窗口；未修改已安装文件。

S5 代码边界：现有 win16-window-uwp/frame_layout.c 明确拒绝缺少 WS_CAPTION 的窗口，且非客户区绘制层依赖已分配的标题空间；直接套用 Terminal 路径不足以给 Codex 无标题栏窗口画标题。只读检查当前包的单实例判定函数表明 Windows 打包应用会请求锁，不能仅凭传入独立 userData 就断言锁隔离；需先验证真实新实例及数据所有权，再验证布局与恢复。

S5 发布版原生模型复测：自建私有桌面下运行既有 chromium-fixture 两次，原始无标题样式 attached=1、客户区顶偏移4、标题蓝色像素0；测试窗口显式添加 WS_CAPTION 后 attached=1、偏移25、蓝色像素8919，两次均退出0。这只证明本模型的发布版行为，不证明真实 Codex、交互或恢复。复用的 s3-private-attach/chromium-fixture 源码和程序暂留，用于后续候选同模型对照，用途完成后清理；本次无新建临时构建。

S5 自有无标题栏模型恢复对照：发布版附着后临时设置 WS_CAPTION，客户区顶偏移25、标题蓝色像素8919；调用移除并恢复原始 style 后，style=0x15070000、顶偏移0、蓝色像素0，与未附着对照一致，两次退出0。该模型由测试主动恢复样式，不证明产品已自动恢复或真实 Codex 行为。首次探针构建因旧夹具缩进触发 -Werror 而失败，修正测试源码缩进后编译成功。测试 scratch 为 build/t23-package/s5-caption-restore1，结果提炼后清理。

S5 自有模型中，临时设置 WS_CAPTION 时 attached=1、标题蓝色像素8919；模拟应用重新写回无标题样式并触发 FRAMECHANGED 后仍 attached=1，但顶偏移0、蓝色像素0。该对照证明单次设置不足以应对模型中的样式重设；未证明真实 Codex 会发生该消息或重设。测试 scratch build/t23-package/s5-caption-change1 已于结果提炼后清理。

S5 生命周期审计：普通 frame 安装读取窗口样式用于布局，但现有移除路径恢复 DWM 策略、菜单及客户区几何，并不拥有或恢复外部添加的 WS_CAPTION；也未处理宿主自行重设该位。故将测试中的 SetWindowLongPtr 直接加到普通安装流程会留下恢复和归属缺口，不能视为正式修复。

S5 临时样式所有权模型：自有窗口在 WM_STYLECHANGING 中保留附加标题位，同时记录应用明确提交的原始标题位；测试中应用删除 THICKFRAME、增加 MINIMIZEBOX 并清除标题，移除原型后两个变更及无标题状态均保留，退出0。首次编译因 WPARAM 与 GWL_STYLE 有符号性比较被 -Werror 拒绝，显式转换后成功。此模型不能区分应用读回适配后样式再写入时的真实意图，也未验证实际绘制、重入或 Codex，不进入产品；测试 scratch s5-style-owner1 已清理。

S5 共享 Chromium 风险复查：先前自有 Chrome 调查发现消息窗口及嵌套系统 IME 的附着错误13传播至创建 scope，最终 GPU/浏览器退出；当前 attachment.c 的 creation_return 仍会将非 ERROR_NOT_SUPPORTED 的安装错误记入 owner->result.failure。旧实验在特定样本跳过这两类辅助窗口后进程可存活，但不能证明根因或 IME 兼容；不得直接将实验性类名排除升级为产品修复，也未证明 Codex 存在同一故障。

S5 辅助窗口故障候选定位：frame_lifecycle.c 在 refresh_font 失败时明确设置 ERROR_INVALID_DATA(13)；frame_font.c 需要创建标题字体及获取菜单字体。这是先前 Chromium 错误13的一个代码路径，不是该次运行的调用栈证明，也不能据此将所有错误13归因于字体；需对自有 Chromium 进程验证具体失败调用。

S5 自有 Chromium GPU 诊断：临时 DLL 在字体分配失败路径输出诊断，并仅对 GPU 进程保留创建失败窗口以便观察；该 GPU 进程两次报告菜单字体获取失败，标题字体未报告失败。浏览器及 GPU 最终退出0；这是诊断候选而非发布 DLL 或功能修复，尚未识别菜单字体内部失败的具体 API，也未证明真实 Codex 相同。核验自有进程退出后清理 scratch build/t23-package/s5-font-diagnostic1 和构建脚本 build/s5-font-diagnostic-build.py。

S5 共用窗口策略检查：现有 frames 测试明确覆盖无标题 WS_POPUP 和 WS_POPUP|WS_BORDER；因而不能简单排除全部无标题或 popup 窗口来绕过 Chromium GPU 的字体失败，否则会丢失既有支持。菜单字体生成包含 DC、位图、字体资源及字体度量多项操作，目前仅确认获取失败，不能推断某一个 API 是根因。

S6 发布版基线：私有桌面和独立 profile 的自有 Chrome 加载 O:/winnt/ntw16s64.dll 并报告附着，约两秒后目标进程以 0x80000003 退出；驱动退出0不代表浏览器测试通过。按测试清理规则核验自有进程并删除 profile build/t23-package/s3-private-attach/chrome-profile-35120；已有浏览器未操作。

S6 自有 Chromium GPU 诊断：菜单字体生成两次在 `AddFontMemResourceEx` 的返回句柄/计数检查处失败（GetLastError 为0，未分别记录句柄和计数），对应先前菜单字体获取失败；该临时诊断仅对 GPU 保留失败窗口以观察，不代表正式行为已修复。浏览器和 GPU 正常退出；独立资料和 scratch build/t23-package/s6-font-reason1、构建脚本 build/s6-font-reason-build.py 在核验进程结束后清理。

S6 开发候选：自定义菜单字体获取失败时使用系统 stock 字体，不改变成功路径；x86/x64 Ninja 增量构建及各4项外框、字体聚焦测试通过。测试首次因未设置 WIN16STYLE_CAPTURE_DIR、第二次因目录未预先创建而在执行前失败；修正后通过。自有独立 Chrome 加载候选 DLL 并附着，4秒观察内窗口保持存活，最大化/还原状态为3/1；离屏截图仍无 Win16 标题，不能计外框通过或恢复通过。候选未发布；测试进程退出后清理 scratch build/t23-package/s6-font-fallback1。

S6 Chromium 非客户区绘制层候选：对带 WS_CAPTION 和 NOREDIRECTIONBITMAP 的 Chrome_WidgetWin_1 复用现有绘制表面；双宽 DLL 增量构建通过。自有 Chrome 候选加载并附着，独立 owner 表面存在且可见，窗口最大化/还原状态3/1；PrintWindow 无法捕获该表面（返回0），输入桌面不可用导致私有桌面实屏验证跳过，不可据此声称视觉通过。测试进程退出后 scratch build/t23-package/s6-frame-probe1 已清理；候选未发布。

登记 scratch build/t23-package/s6-clean1：当前候选同快照双宽干净构建、后续实包回归及发布；交付后依清理规范回收临时构建。

s6-clean1 后台进程已退出且无结果和源码快照，不计构建通过；s6-clean2 因两个变更文件的来源摘要未更新而被 source manifest 拦截，未构建或发布。已更新项目内两个摘要与审计说明并通过源摘要自测；两次失败临时构建按规则清理。登记 s6-clean3 重新执行双宽干净构建。

s6-clean3 同快照双宽干净编译、ABI、来源、资源及公共库边界检查通过；登记 s6-package1 为实际四文件聚焦回归，s6-chrome1 为同包自有 Chrome 对照，测试完成后按清理规则处理。

同批包自有 Chrome 加载候选 DLL、附着并在4秒观察内保持存活；未验证实际屏幕视觉或恢复。s6-package1 在 late-only-x64-x86-w 报告 PRIVATE_DESKTOP_LEAK 并退出8，虽然传播标记通过；原因未知，不能将首轮计作通过。登记 s6-package2 同包重测，保留首轮精简结果；s6-chrome1 自有进程退出后清理。

s6-package2 的 late-only-x86-x64-w 同样报告 PRIVATE_DESKTOP_LEAK，功能标记通过；两处失败桌面稍后仍可打开但无顶层窗口，未枚举到关联线程，原因未知。候选两种位宽单独测试均退出0，旧发布包单独测试 x86→x64 亦退出0，不能据此认定根因或首两轮通过。[失败原始输出](../history/M0-T23-S6-package-failures.md)已提炼，前两轮临时测试输出清理；s6-package3 串行同包聚焦回归，不放宽断言。

s6-package3 串行同包聚焦29项通过、零跳过；前两轮桌面清理失败的根因仍未知。s6-publish1 已发布同批四文件至 assets 和 O:/winnt 并核对摘要；[交付及限制](../history/M0-T23-S6-P1-chrome.md)。实屏 Chrome 标题及交互尚未验证，等待用户验收；S6/T23 保持开放。

发布后审查发现 Chrome 新增表面未包含在 Ntw16sWindowRemove 移除分支，s6-publish1 不应作为验收版本；已添加 Chrome 表面移除分支并更新来源摘要，双宽增量 DLL 构建通过。登记 s6-remove1 对自有 Chrome 验证表面销毁，随后重做干净包、回归和发布；最终产物发布前保留 S6 开放。

s6-remove1 自有 Chrome 实际候选：移除前表面存在，移除返回成功、附着状态清除、表面句柄销毁，客户区顶偏移恢复0，NOREDIRECTIONBITMAP 保持；浏览器和 GPU 退出0。该测试不代表实屏绘制或所有恢复状态，临时资料与探针核验进程退出后清理。

旧 s6-clean3 构建树和 s6-package3 测试输出已清理，测试摘要保留在 s6-publish1；登记 s6-clean4 用于移除修复后的同快照双宽干净构建。

s6-clean4 双宽干净构建、ABI 和来源资源门禁通过；登记 s6-package4 串行实际包回归、s6-remove2 同包 Chrome 移除对照，结束后清理临时产物。

s6-remove2 对同批候选的自有 Chrome 验证：附着且表面存在，移除成功后附着清除、表面销毁、客户区顶偏移恢复0，合成扩展标志保持；浏览器及 GPU 退出0。该对照不证明实屏外观，测试进程退出后清理 s6-remove2。

s6-package4 串行同批实际包聚焦29项通过、零跳过；s6-publish2 已将最终同批四文件发布到 assets 和 O:/winnt 并核对摘要，取代包含移除缺口的前一候选。实屏 Chrome 视觉和交互仍未验证，等待用户验收。

S6 P1 `01e485c4` 已推送并读取远端 main 核对；两处发布文件与测试候选摘要相同。代码变更范围：frame_font.c +3/-0，attachment.c +6/-2；保留原渲染器及移除路径，未引入第二套绘制。正式包、提交和远端核对后，s6-clean4、s6-package4、s6-publish1、s6-publish2 临时构建、测试输出与发布备份已清理；失败原始输出和交付摘要保留在 history。用户确认 Chrome 验收通过，S6 收口；两轮私有桌面未释放原因仍未知，T23 保持开放。

## M0 T23 S7 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New |
| Admission And Approval | 用户准入四个 Windows 程序的盘点、测试和高收益外框修复。 |
| Objective | 测试 Paint、截图工具、设置和设备管理器，复用现有外框，按 ROI 修复并交付。 |
| Non-goals | 不处理内部控件、Codex 或无关长尾；不修改注册表、系统安全设置及已有用户会话。 |
| Reference Baseline | S6 P1 01e485c4，四文件已发布并获得用户验收。 |
| Candidate Proposal | docs/proposals/modern-desktop-window-style.md |
| Files And ABI Surface | 先盘点实际入口、进程和窗口，再按证据修改现有产品适配；ABI 变化另审。 |
| Applicable Rules | EXECUTION、ARCHITECTURE、CODING、DOCUMENT；M0、C17、GCC、Ninja、ccache。 |
| Verification | 自有实例与私有桌面优先；分别核验启动、目标身份、绘制、基本交互和恢复，区分跳过与通过，回归相关已有程序。 |
| Expected Markers | 四个程序逐一说明原生入口、复用情况及真实外框结果；高收益故障修复或明确限制。 |
| Asset Needs | 保留图形及来源；按规范双宽构建、同批四文件测试并发布至 assets 和 O:/winnt。 |
| Reporting Requirements | 按程序列出已验证、修复、失败、跳过和用户验收状态。 |
| Stop Conditions | 不操作无关进程或改变系统安全设置；用户永久授权的自有非 computer-use 测试照常执行。 |
| Exit Criteria | 完成有限范围调查、必要修复、构建测试、发布提交推送，用户验收后收口。 |
| Original Owner Request | Paint、截图工具、设置和设备管理器先盘点测试，按 ROI 修复并发布验收。 |
| Similar-Issue Sweep | 查找相同承载、启动转交、权限和合成机制，优先共性处理。 |

## Historical delivery notes

S42 P1：删除 discover 和 load 两对产品垫片及多组纯转发和公共别名，产品专属适配置于 global/；双宽干净构建、ABI、资源及公共库边界检查通过，双宽各 11 项核心回归及 44 项实际包聚焦通过，其他 window station 子项双宽跳过，恢复摘要与旧版一致但仍不等原始。s42-publish1 因已核实的 sol.exe 占用中断，正常关闭后以同批候选包 s42-publish2 完成两处四文件发布并核对摘要。产品提交 `7301e504` 已推送并逐项复核远端。

P24：完成公共 process-hook GCC/MSVC 兼容适配与可移植测试入口。两种编译器 x86/x64 均编译、链接并通过异常展开、批处理及传播自有进程测试，覆盖重入与容量 0/88/89 入口；88 不代表实际安装 88 项。GCC 干净树双宽各 8 项生命周期回归通过，s41-clean28 双宽干净构建、ABI、来源资源与依赖门禁通过；s41-package28 实包聚焦 32 项通过，双宽恢复摘要与旧版一致但仍不等原始值。四文件发布 assets 与 O:/winnt 并核对（s41-publish24）；ntvdm64 仅作只读对照，逐项导入清单见审计，未导入或验证兄弟产品。真实托盘端到端、管理员 ETW、真实 PID/TID 重用及恢复差异沿用 P23 边界，S41/T21 保持开放。

P23：完成冻结清单逐项复审；公共窗口发现器新增可选 INCLUDE_SELF，产品原默认排除自身不变。双宽干净构建、ABI、资源来源及公共库依赖门禁通过；双宽自有已有/新建窗口独立测试通过。实际包 s41-package27 聚焦 23 项通过，但双宽其他 window station 子项输出 SKIP（exit 12），非管理员控制器未启用 ETW；双宽 manager 绘制改变但移除后摘要与原始不同，不能计作恢复通过。四文件已发布 assets 和 O:/winnt 并核对摘要（s41-publish23）。真实管理员 ETW、真实托盘端到端、绘制恢复差异及真实 PID/TID 重用仍未覆盖或未解决；S41/T21 保持开放。

P22：公开晚期批处理 88 条上限并在入口拒绝超额；产品组合测试加入自有同一窗口 Button 的 WM_PRINT 绘制摘要。双宽干净构建、ABI、来源资源及边界门禁、批处理测试通过；实际包聚焦 20/20 无跳过，含双宽管理器、停止竞态、仅样式及跨宽传播。两位宽实际包应用前后绘制不同，但移除后摘要仍不同于原始（343534586 / 2033954149 / 2875890938）；旧 P20 DLL 同样复现，不点击也复现，原因未确认，不计恢复通过。四文件发布 assets 与 O:/winnt 并核对摘要（s41-publish22）。真实管理员 ETW、完整全局托盘端到端和最终逐项审计仍未完成；S41/T21 保持开放。

P21：晚期钩子批量准备、重试和失败资源保留迁入 process-hook；产品提供注册策略。仅样式测试增加 A/W 自有子进程不传播 DLL 及 A/W 创建入口字节对照，已发布 run16s 的双宽加载负例均返回 42。双宽干净构建、ABI、来源资源及边界门禁和独立晚期批处理/样式测试通过；实际包聚焦 28/28 无跳过，涵盖双宽仅样式、本地钩子及 A/W 同宽/跨宽传播。四文件发布 assets 与 O:/winnt 并校验（s41-publish21）。底层批处理上限 88、公共接口的早期边界验证待补；物理卸载、管理员 ETW 正例、真实全局托盘端到端及最终审计仍未完成。S41/T21 保持开放。

P20：线程窗口钩子的安装、就绪、等待和撤销，已退出同伴跟踪停止机制，以及后台初始化调度与去重迁入 process-global；产品保留会话名、回调、模块固定和样式协议。双宽干净构建、ABI、来源资源及边界门禁通过，独立自有窗口钩子/后台调度和不存在跟踪会话清理测试通过；实际包聚焦 20/20 无跳过，包括双宽本地全局钩子、子孙传播、产品线程、管理器停止竞态及跨宽传播。四文件发布 assets 与 O:/winnt 并核对摘要（s41-publish20）。真实管理员 ETW 正例、完整全局端到端托盘与最终审计仍未完成；S41/T21 保持开放。

P19：进程创建时间、桌面身份指纹和模块文件比较合并到 process-global，产品旧名称为兼容别名；策略和孤儿恢复使用同一身份实现。新增公共组件源码及静态库未定义符号反向依赖门禁并接入打包；双宽干净构建、ABI、来源资源门禁、独立身份/模块/会话测试及四公共库符号检查通过；实际包聚焦 20/20 无跳过，涵盖双宽线程、策略、接入、管理器和跨宽传播。四文件发布 assets 与 O:/winnt 并核对摘要（s41-publish19）。完整全局端到端托盘、管理员 ETW、完整矩阵和最终审计未完成；S41/T21 保持开放。

P18：全桌面及线程窗口/子窗口限时遍历迁入 process-global/window，接入等待、取消与限时重试循环迁入 process-global/transaction；产品提供通知、过滤与接入回调。双宽干净构建、ABI、来源和资源门禁、独立自有窗口与事务测试通过；实际包聚焦 20/20 无跳过，覆盖双宽管理器、停止竞态、接入、发现、窗口及跨宽传播。四文件发布 assets 与 O:/winnt 并核对摘要（s41-publish18）。x86 增量 Ninja 反复提示 premature end of file 和依赖缺失；确认无活动构建后备份原 .ninja_deps 至 build/t21-package/s41-p18-ninja-diagnostic/x86-ninja-deps-original 并重建，后续目标 dry-run 为 no work to do；原始损坏原因未知。完整全局端到端托盘、管理员 ETW、完整矩阵及最终审计仍未完成；S41/T21 保持开放。

P17：CreateProcess A/W 传播事务及异常展开支持迁入 process-hook，产品仅提供会话、目标排除和载荷安装回调；保持启动/晚期现有接入及停用语义，不声称物理卸载。双宽干净构建、ABI、来源资源门禁通过；公共库独立原生测试实际挂钩并验证纯载荷子孙传播、A/W、暂停创建、停用、拒绝和 193 失败子进程退出，无样式 DLL；该独立纯载荷测试为同位宽。产品实际包聚焦 28/28 无跳过，涵盖双宽/AW 的仅传播与组合共 16 项，以及管理器停止竞态。初次增量测试有编译缩进错误已修正；首次实包启动命令括号不完整未启动，自动审批拒绝重复启动后只读核实没有进程或目录，再修正启动一次。四文件发布 assets 与 O:/winnt 并核对摘要（s41-publish17）。真实全局端到端托盘限制、管理员 ETW、实际屏幕像素及最终完整审计仍未完成；S41/T21 保持开放。

P16：公共 process-global/enumerate 协调存量线程去重、首个就绪线程优先和暂未就绪候选回退，产品提供会话、桌面和身份策略。双宽干净构建、ABI、来源资源门禁及扫描独立测试通过；实际包聚焦 16/16 无跳过，包括双宽发现、接入和管理器停止竞态。初次增量命令目标名错误未编译，修正后通过。四文件发布 assets 与 O:/winnt 并核对摘要（s41-publish16）。完整全局端到端托盘限制、独立传播 API、管理员 ETW 和完整矩阵仍未消除。构建使用 Ninja、ccache 和 8 并发；x86/x64 分别约 196/208 秒，命中 593/666 次，各未命中 4 次；后续合并相关改动再进行完整交付构建。S41/T21 保持开放。

P15：公共 process-global/engine 协调目标表与工作线程池的启动、消息泵空闲等待、限时关闭和失败重试，产品提供目标回调并调用管理器；双宽干净构建、ABI、来源资源门禁及自有目标独立生命周期测试通过，实际包聚焦 16/16 无跳过。扩展 global-e2e 在私有桌面因托盘创建错误 2147500037、manager_not_active=2 失败；P14 上一发布包同环境同错误，不能认定为新回归，也不能计作通过。初次增量命令使用不存在的 Ninja 目标，改正后编译通过。四文件发布 assets 与 O:/winnt 并核对摘要（s41-publish15）。发现编排、公共传播接口、真实全局端到端及管理员 ETW 未完成；S41/T21 保持开放。

P14：公共 dispatch 增加可配置工作线程池，产品管理器调用公共启动、限时等待及释放，保留既有目标回调和停止顺序。双宽干净构建、ABI、来源及资源门禁通过；双宽独立调度测试覆盖超时重试和重启，实际包 14/14 聚焦通过、无跳过，含双宽管理器与停止竞态。首次 CTest 因未设置捕获目录、第二次因目录不存在未运行，修正后通过。四文件发布 assets 与 O:/winnt 并核对摘要（s41-publish14）。独立发现协调、公共传播 API、管理员 ETW 和完整矩阵仍未完成；S41/T21 保持开放。

P13：目标关闭、失败保留与重试及管理器进程去重查询迁入 process-global/targets，产品回调负责现有接入事务关闭。自有目标双宽测试覆盖关闭失败重试与重新初始化；仅样式测试对 Button 的 WM_PRINT 像素哈希确认应用变化、移除恢复，并确认 CreateProcessW/CreateWindowExW 入口字节不变。双宽干净构建及门禁通过；同批实际包聚焦 18/18 无跳过，候选 DLL 与测试构建 DLL 摘要一致；四文件发布 assets 与 O:/winnt 并核对摘要（s41-publish13）。屏幕实际显示、完整矩阵、管理员 ETW、公用传播 API 和独立管理器协调仍未完成；物理卸载不受支持。S41/T21 保持开放。

P12：将 PID/TID 目标队列、去重、重试、处理状态、进程锁及分批回收迁入 process-global/targets，产品通过回调执行身份与接入；保持既有发现和停止编排。双宽干净构建、ABI、来源与原资源门禁通过；双宽私有桌面自有子进程仅管理测试通过（已有/新建、去重、未加载样式 DLL）；实际包串行 20/20 无跳过，含双宽管理器、停止竞态和跨宽仅传播。首次沙箱直接探针创建私有桌面返回 5，允许环境重跑通过；初次语法预检发现测试缩进错误并已修正。仅管理未证明目标实际绘制，完整组合矩阵、公共传播 API、管理器独立发现/停止协调和物理卸载仍未完成。四文件发布 assets 与 O:/winnt 并核对摘要（s41-publish12）；S41/T21 保持开放。

P11：保留默认组合模式，增加内部仅传播模式与仅传播载荷解析；不兼容已初始化策略返回 ERROR_ALREADY_EXISTS。双宽干净构建、ABI、来源与原资源门禁通过；双宽载荷测试通过，实际包串行 24/24 无跳过，包含八个父子位宽/AW 仅传播组合和八个原组合，子/孙 DLL 加载、未附着、会话停用及重启均观察到。父进程探针的样式注册使用桩，子/孙使用实际 DLL；像素级原样、公共独立传播 API、管理器独立性、物理卸载及完整矩阵未证明。启动测试首次命令有 PowerShell 语法错误，修正后运行。四文件发布 assets 及 O:/winnt 并校验摘要（s41-publish11）。S41/T21 继续开放。

P10：冻结全局三能力审计清单及迁移顺序，事务状态推进迁入 process-global；产品提供加载、身份、工作线程和样式回调。双宽干净构建、ABI、来源与资源门禁通过；独立事务 pending/失败/无窗口测试双宽通过；实际包串行 16/16 无跳过，含接入与管理器停止竞态。仅管理、仅传播、仅样式及完整组合尚未按新要求证明；跨目录与管理员 ETW 既有限制未消除。四文件发布 assets 与 O:/winnt 摘要一致（s41-publish10）；S41/T21 继续开放。

P9：同名目标模块选择与映像身份校验迁入 process-global；产品提供文件比较和实际导出解析。双宽干净构建、ABI、来源与资源门禁通过；双宽自有进程模块选择及拒绝/回退测试通过，实际包串行聚焦 16/16 无跳过，包含接入、管理器、远端解析和加载。跨目录真实端到端因托盘创建失败 2147500037、manager_not_active=2 未完成；上一发布包同条件同错，不认定是回归亦不算通过。管理员 ETW 和完整矩阵未覆盖。四文件发布 assets 及 O:/winnt 并核对摘要（s41-publish9）；S41/T21 继续开放。

P8：管理器 PID/TID 索引、碰撞链与分批回收遍历迁入 process-global；目标生命周期、关闭及计数由产品回调保持原策略。双宽干净构建、ABI、来源和资源门禁通过；双宽索引碰撞/回收测试通过，实际包串行 20/20 无跳过，包含管理器、停止竞态、拒绝目标及接入；管理员 ETW 正例及完整矩阵未覆盖。四文件发布 assets 及 O:/winnt 并核对摘要（s41-publish8）；S41/T21 继续开放。

P7：会话孤儿身份判定/清理及工作线程事件调度循环迁入 process-global；产品保留严格会话名解析、目标处理和样式策略。双宽干净构建和 ABI、来源与资源门禁通过，独立调度及孤儿身份双宽测试通过；实际包串行 18/18 无跳过，含双宽管理器停止竞态、策略和接入。管理员 ETW 清理实测及完整矩阵未覆盖。过时 s41-clean9 与增量构建已仅终止本任务进程，不计验证。四文件发布 assets 及 O:/winnt 摘要一致（s41-publish7）；S41/T21 保持开放。

P6：目标身份检查和重验证迁入 process-global，进程排除名单经调用方回调保持产品规则。双宽干净构建、ABI、来源及原资源门禁通过；实际包串行 16/16、无跳过，覆盖双宽策略、管理器、接入及远端解析；管理员 ETW 正例及完整矩阵未覆盖。四文件发布 assets 与 O:/winnt 并核对摘要（s41-publish6）；S41、T21 继续开放。

P5：抽离线程启动的数据传递、等待和资源清理，以及参数化远端模块导出定位至 process-global；项目保留事件命名、线程选择、DLL 和导出名称。双宽干净构建、ABI、来源与资源门禁通过；实际包串行 20/20 无跳过，包括双宽导出不存在负例、远端启动和接入；管理员 ETW 正例及完整矩阵未覆盖。四文件已发布 assets 及 O:/winnt 并核对摘要（s41-publish5）。S41、T21 继续开放。

P4：抽离现有管理器 FIFO、DLL 加载事务和系统加载器远端地址校验至 process-global；产品提供 DLL 名、远端入口解析和策略。双宽干净构建、ABI 与来源资源门禁通过；队列双宽通过（首轮因未设置捕获目录未执行）；实际包串行 22/22 无跳过。此前 P2 并行 x64 接入 1460 根因未明，管理员 ETW 正例和完整矩阵未覆盖。四文件已发布 assets 及 O:/winnt 并核对摘要（s41-publish4）；S41、T21 仍开放。

P3：将已有 DLL 文件句柄的映像校验迁入 process-image；加载策略和资源生命周期不变。双宽干净构建及 ABI、来源和原资源门禁通过；同批实际包 16/16 串行通过，无跳过，覆盖双宽远端读取、加载、启动和策略；完整矩阵及管理员 ETW 集成未覆盖。四文件发布 assets 和 O:/winnt 并核对摘要（s41-publish3）。S41 与 T21 保持开放。

P2：令牌用户信息、线程桌面身份比较移入 process-global，远端 PE 头校验、可执行页检查和有界读取移入 process-image；产品排除规则、DLL 名称及导出选择仍在原位。双宽干净构建、ABI、来源和资源门禁通过；实际包 22/22 串行通过、无跳过，覆盖双宽发现、策略、远端、加载、启动与接入。首次并行实际包 x64 attach 返回 1460，窗口已附着但就绪未确认，原因未定位；此前 clean2 与编辑重叠导致来源快照失败。四文件已发布 assets 及 O:/winnt，摘要一致（s41-publish2）。管理员 ETW 集成和完整矩阵未覆盖。S41、T21 继续开放。

P1：窗口枚举及显示事件发现、ETW 进程通知和会话生命周期移入 `lib-win32/process-global`；DLL 保留 Win16S 会话命名、孤儿会话恢复及接入策略。双宽干净构建和 ABI、来源与资源门禁通过；实际包聚焦 15/15 无跳过；双宽进程事件解析及枚举测试通过，ETW 管理员集成双宽因未提权跳过，未做完整矩阵。四文件已发布 assets 根目录及 O:/winnt 并核对摘要（s41-publish）。S41 与 T21 保持开放。

S25 经用户验收收口，交付范围见 [记录](../history/M0-T21-S25-common.md)。

S24 已准入；S23 产品提交 `4eb5c7c` 及远端四文件已复核，详情见 [交付记录](../history/M0-T21-S23-dialog-library.md)。用户进一步要求为后续 Win2000 样式库准备接口；候选边界见库设计，不能将尚未验证的草案宣称为已实现的多主题框架。S24 的库职责盘点、迁移、构建、实际包测试、双位置发布、提交、推送和远端复核均已完成。

P72：完成 S24 的最终职责收束。VB 文本与标签调用者资格迁入 `win16-control/text_policy`；DLL 仅提供报告绘制调用者和启动期直接文本状态。稳定化独立文件夹新建探针：成功投递后立即终止其自有枚举线程，保留真实旧式对话框、创建、返回路径与清理验证。最终审计写入[库设计与依赖审计](../proposals/win16-libraries.md)：`attachment.c`、`controls.c`、`win16_style.c` 及四个对话框/标题接入文件仅剩 HWND/COM/API trampoline、TLS、回调绑定和异常清理，零直接 Win16 绘制调用；两个库无反向 DLL 包含。双宽独立 control/dialog 库运行通过；干净包 `s24-final-library-clean-1` 双宽构建、ABI 和四文件身份通过；实际包 `s24-final-library-focused-2` 的双宽 dialog/control 六项均通过，固定预检中的 demo-pair/automatic/propagation 因当前桌面环境跳过。四文件已发布 assets 根及 `O:/winnt`，摘要由 `s24-final-library-deploy-1` 逐项核对；产品提交 `279bd25` 已推送，远端分支及四个二进制 Git 对象均逐项读取复核。不修改 global_*、跨进程接入、ABI 或原资源。

P73：独立复核 P72：读取远端 `origin/main`，提交树与 `279bd25` 一致，四个 assets 对象逐项与本地已验证提交相等；工作区无未提交改动。S24 关闭；T21 仍保持开放，后续主题接口必须另行准入。

P71：VCL 弹出菜单宿主的身份标记和作用资格判断迁入 win16-control；DLL 保留属性值、线程范围、接入入口和异常清理。首次尝试修改受 retain 约束的原头文件被来源门禁拒绝；已恢复原字节并使用项目自有头文件，来源门禁通过。双宽增量构建通过，候选包 `s24-p71-candidate-test` 聚焦 16/16、无跳过，覆盖双宽普通与 DPI 感知菜单、嵌套、错误宿主、应用自绘及恢复；干净包 `s24-p71-clean-1` 双宽构建及 ABI、独立控件库测试通过；实际包 `s24-p71-focused-1` 聚焦 16/16、无跳过。同批四文件已发布 assets 根及 O:/winnt 并核对摘要（`s24-p71-deploy-1`）；不代表完整菜单矩阵，S24 继续开放。

P70：主题类名安全识别迁入 win16-control，文件对话框快捷方式解析资格迁入 win16-dialog；现有 API 接入和主题跟踪保留原位。双宽增量构建及两个公共库独立测试通过，覆盖无效指针、主题类别和快捷方式选项；独立对话框测试首次因缺少 `WIN16STYLE_CAPTURE_DIR` 与旧包同样提前退出，设置专用目录后双宽通过。候选包 `s24-p70-candidate-test` 聚焦 20/20、无跳过，涵盖双宽主题及 ANSI/Unicode 快捷方式。干净包 `s24-p70-clean-1` 双宽构建及 ABI、双库独立测试通过；实际包 `s24-p70-focused-1` 聚焦 20/20 无跳过。同批四文件已发布 assets 根与 O:/winnt 并核对摘要（`s24-p70-deploy-1`）；未覆盖完整矩阵，S24 继续开放。

P69：VCL CoolBar 窗口身份校验与 VB 字形解码后的可逆编码转换迁入 win16-control，DLL 提供现有窗口查找、解码回调及原生绘制入口；双宽独立库测试覆盖普通字符、不可编码和 RTL 回退；首次增量构建因错误使用产物名作为 Ninja 目标失败，改用 `ntw16s` 后双宽构建通过。增量候选 `s24-p69-candidate-test` 聚焦 10/10、无跳过，包括双宽 CoolBar 嵌套、异常、移除、重新设置父窗口与错误身份回退；追加字形迁移后 `s24-p69-candidate-test-2` 聚焦 14/14、无跳过，覆盖双宽 VB5/VB6 检查列表；干净包 `s24-p69-clean-1` 双宽构建及 ABI 通过，干净树独立控件库双宽通过；实际包 `s24-p69-focused-1` 聚焦 14/14、无跳过。同批四文件已发布 assets 根及 O:/winnt 并核对摘要（`s24-p69-deploy-1`）。该结果不代表完整控件矩阵，S24 保持开放。

P68：富文本/日历绘制见证、box 绘制见证及控件身份规则迁入 win16-control，DLL 保留 TLS、异常保护和接入回调；双宽干净构建及 ABI 通过，实际包聚焦 22/22 无跳过。日历实包测试 x86 通过、x64 出现 39 个边界外差异像素；交替对照中 P67 基线和 P68 候选均曾通过或失败，尚未定位根因。按正式优化选项构建的诊断探针复现 P67 差异（首个像素 0,14：ff0f1011→ff979797），同次 P68 通过；私有桌面记录无可见光标、无焦点插入符；差异为客户区 x=0..2、y=14..26 的 3×13 像素。将测试窗口从屏幕原点移至 (100,100) 后首次旧包和候选均通过，但随后的旧包交替对照原位置 3/3 通过、移动位置 3/3 失败，不能认定屏幕位置是根因。进一步对照在移除样式之前连续重绘并采样：旧包首次与第二次采样出现同样 39 像素差异，第二次之后恢复稳定，4 次严格比较均通过。测试已增加稳定性确认，像素比较区域和断言保持不变。首次干净重建因测试文件来源摘要未更新被拒绝，更新后 `s24-p68-clean-3` 双宽构建及 ABI 通过；`s24-p68-focused-2` 同批实际包聚焦 24/24、无跳过，包括双宽日历。同批四文件已发布 assets 根及 O:/winnt 并核对摘要（`s24-p68-deploy-2`）。此前不同编译选项的诊断探针在外部调用者检查提前失败，不能作为像素证据；原始失败保留，不宣称完整控件矩阵通过。S24 继续开放。

P67：将 MDI、VB/VCL 控件身份及 ANSI 编辑/组合框绘制见证判断迁入 win16-control；DLL 保留窗口查找及观察入口。首次增量构建因未使用参数告警失败，修正后双宽构建及独立链接通过；来源和原资源校验通过。候选 `s24-p67-candidate-test` 聚焦 30/30 无跳过；干净包 `s24-p67-clean-1` 双宽构建及 ABI 通过，实际包 `s24-p67-focused-1` 聚焦 30/30 无跳过，覆盖 MDI、ANSI 见证及部分 VB/VCL 控件。同批四文件已发布 assets 根目录及 O:/winnt 并核对摘要（`s24-p67-deploy-1`）；提交 `7054c8e6d5541f30b493a64d3eaf04d8f9a2e92f` 已推送，远端四文件及库源码读取核对一致。该回归不是完整控件矩阵；日历、滚动条实窗像素及其余样式/对话框抽离仍未完成。

P66：将经典对话框的目录切换、初始化事件协调及其状态清理迁入 win16-dialog；DLL 保留异常作用域及 COM 入口。新增双宽独立库测试使用真实 Shell 项目验证状态、初始化事件与引用清理；首次测试编译因缩进告警失败，修正后双宽通过。实验事件/隐藏分支双宽语法检查通过，完整实验事件运行仍未验证。双宽增量构建、独立链接、来源和原资源校验通过；候选 `s24-p66-candidate-test` 聚焦 22/22 无跳过；干净包 `s24-p66-clean-1` 双宽构建及 ABI 通过，实际包 `s24-p66-focused-1` 聚焦 22/22 无跳过，覆盖默认实际目录切换。同批四文件已发布 assets 根目录及 O:/winnt 并核对摘要（`s24-p66-deploy-1`）；提交 `6f307a32afec2a203961f73490d37623ebb85fad` 已推送，远端四文件及库源码读取核对一致。该回归不是完整对话框矩阵；日历、滚动条实窗像素及其余样式/对话框抽离仍未完成。

P65：将经典 COM 对话框的控件命令、目录/选择变化及事件消息分派迁入 win16-dialog；DLL 保留 COM 入口和异常保护。新增双宽独立库测试覆盖拒绝、导航中保护、确认和隐藏文件刷新；实验事件/隐藏文件分支双宽语法检查通过，完整实验事件运行仍未验证。双宽增量构建、独立链接、来源和原资源校验通过；候选 `s24-p65-candidate-test` 聚焦 24/24 无跳过；干净包 `s24-p65-clean-1` 双宽构建及 ABI 通过，实际包 `s24-p65-focused-1` 聚焦 24/24 无跳过。同批四文件已发布 assets 根目录及 O:/winnt 并核对摘要（`s24-p65-deploy-1`）；提交 `baafa10416ba32da197963a24c2325a4b4145e8a` 已推送，远端四文件及库源码读取核对一致。该回归不是完整对话框矩阵；日历、滚动条实窗像素及其余样式/对话框抽离仍未完成。

P64：将经典 COM 对话框目录结果调整、文件选择结果准备及撤销清理迁入 win16-dialog；DLL 保留 COM 入口与异常作用域。新增双宽独立库测试覆盖路径、结果提交、撤销和无效多选缓冲区；实验事件及隐藏文件分支双宽语法检查通过，完整实验事件运行仍未验证。双宽增量构建、独立链接、来源和原资源校验通过；候选 `s24-p64-candidate-test` 聚焦 24/24 无跳过；干净包 `s24-p64-clean-1` 双宽构建及 ABI 通过，实际包 `s24-p64-focused-1` 聚焦 24/24 无跳过。同批四文件已发布 assets 根目录及 O:/winnt 并核对摘要（`s24-p64-deploy-1`）；提交 `ee85bed8fcc31fc80f4e7481b3a77bde164a6e7f` 已推送，远端四文件及库源码读取核对一致。该回归不是完整对话框矩阵；日历、滚动条实窗像素及其余样式/对话框抽离仍未完成。

P63：将附着窗口的静态与微调绘制资格、报告控件绘制准备、见证重置及绘制分派优先级迁入 win16-control；DLL 保留线程局部作用域、异常清理与实际窗口转发。新增双宽独立库路由边界测试覆盖优先级、撤销、嵌套及微调输入；双宽增量构建、独立链接、来源和原资源校验通过。候选 `s24-p63-candidate-test` 聚焦 28/28 无跳过；干净包 `s24-p63-clean-1` 双宽构建及 ABI 通过，实际包 `s24-p63-focused-1` 聚焦 28/28 无跳过，覆盖部分状态栏、微调、网格、报告及弹出控件。同批四文件已发布 assets 根目录及 O:/winnt 并核对摘要（`s24-p63-deploy-1`）；提交 `506bc548007006292f1daeb3b9f27fdde584631c` 已推送，远端四文件及库源码读取核对一致。该回归不是完整控件矩阵；日历、滚动条实窗像素及其余样式/对话框抽离仍未完成。

P62：将首次附着的控件角色、字体/边框相关状态及 MDI、弹出窗口和报告控件的状态初始化迁入 win16-control；DLL 保留窗口子类安装、观察及失败清理，并通过明确回调提供窗口查找。首次增量构建因遗漏微调控件声明失败，补充头文件后双宽构建及独立链接通过；来源和原资源校验通过。候选 `s24-p62-candidate-test` 聚焦 26/26 无跳过；干净包 `s24-p62-clean-1` 双宽构建及 ABI 通过，实际包 `s24-p62-focused-1` 聚焦 26/26 无跳过，覆盖 MDI 引用、弹出窗口及部分 VB/VCL 控件。同批四文件已发布 assets 根目录及 O:/winnt 并核对摘要（`s24-p62-deploy-1`）；提交 `5e8142bf117c92a34e4630ae9150dab521c5e196` 已推送，远端四文件及库源码读取核对一致。该回归不是完整控件矩阵；日历、滚动条实窗像素及其余样式/对话框抽离仍未完成。

P61：将首次附着的 VCL 分类入口与两种不支持资格判定迁入 win16-control，DLL 保留窗口观察及原协议释放分支。新增双宽独立库边界测试，覆盖两种拒绝分支、树、VCL、特殊边框与窗口框架；首次增量编译因误删仍用于初始化的变量失败，恢复后双宽构建和独立链接通过；更新来源清单时首次遗漏测试文件摘要，补齐后通过。候选 `s24-p61-candidate-test` 聚焦 24/24 无跳过；干净包 `s24-p61-clean-1` 双宽构建及 ABI 通过，实际包 `s24-p61-focused-1` 聚焦 24/24 无跳过。同批四文件已发布 assets 根目录及 O:/winnt 并核对摘要（`s24-p61-deploy-1`）；提交 `c3b382867f7535f40175767e0804496052fc3daf` 已推送，远端四文件及库源码读取核对一致。测试覆盖资格选择及部分控件行为，不是全部控件矩阵；日历、滚动条实窗像素及其余样式/对话框抽离仍未完成。

P60：将窗口过程中的标题栏、菜单、非客户区及输入消息路由迁入 win16-control，DLL 保留子类入口、线程菜单状态及异常清理。双宽增量构建与独立链接通过；候选 `s24-p60-candidate-test` 聚焦 21/21 无跳过。首次干净构建 `s24-p60-clean-1` 因直接包含窗口源码的测试缺少迁移后的消息常量而编译失败，常量移入内部共享头后双宽目标编译通过；直接从 CTest 调用探针时因未配置捕获目录未运行成功，随后直接运行自有隔离桌面探针双宽通过。干净包 `s24-p60-clean-2` 双宽构建和 ABI 检查通过，干净构建的绘制异常与菜单生命周期探针双宽通过；实际包 `s24-p60-focused-1` 聚焦 21/21 无跳过。同批四文件已发布 assets 根目录及 O:/winnt 并核对摘要（`s24-p60-deploy-1`）；提交 `1177d11494e158f04f50e108c9c2d8ca4bf810e3` 已推送，远端四文件及库源码读取核对一致。本次未证明所有应用均无闪烁，日历和滚动条实窗像素差异及其他样式/对话框抽离仍未完成。

P59：将子控件创建作用域的类型、分类、嵌套及安装统计、作用域生命周期迁入 win16-control；DLL 保留线程局部状态、钩子回调和原公开导出。新增原生作用域测试覆盖嵌套、计数、安装回调中结束及清理，双宽独立运行通过；双宽增量构建、独立库链接和原资源检查通过，候选包 `s24-p59-candidate-test` 聚焦 16/16 无跳过。干净包 `s24-p59-clean-1` 双宽构建和 ABI 检查通过，实际包 `s24-p59-focused-1` 聚焦 16/16 无跳过。同批四文件已发布 assets 根目录及 O:/winnt 并核对摘要（`s24-p59-deploy-1`）；提交 `69ac3109153b5f83ad7fd6899109d824afb793e3` 已推送，远端四文件及库源码读取核对一致。此为聚焦回归；日历、滚动条实窗像素及其余样式/对话框迁移仍未完成。

P58：将首次绘制中标准及 VCL 控件的原生窗口过程、调用方模块和所有者见证检查迁入 win16-control；DLL 保留观察入口、线程局部作用域和状态生命周期。双宽增量构建、独立库链接、来源与原资源校验通过；首次候选回归因输入未知用例名在运行前失败，修正后候选分别 24/24 和 16/16、均无跳过。干净包 `s24-p58-clean-1` 双宽构建及 ABI 检查通过，实际包 `s24-p58-focused-1` 聚焦 32/32 无跳过。同批四文件已发布 assets 根目录及 O:/winnt 并核对摘要（`s24-p58-deploy-1`）；提交 `c381b1b59f9a3b8a9c07a25d17eb7b017f1b4e61` 已推送，远端四文件及相关库源码读取核对一致。日历和滚动条实窗像素差异未纳入本次通过范围；其余样式与对话框迁移仍未完成。剩余职责的初步文件级盘点见库设计。

P57 进行中：初次窗口附着的标准控件识别、原生协议绑定和 VCL 文件列表特例迁入 win16-control，DLL 保留窗口观察、附着和状态生命周期。双宽增量构建、独立链接及原资源校验通过；候选 `s24-initial-classify-focused` 聚焦 34/34 无跳过；干净包 `s24-p57-clean-1` 双宽构建通过，实际包 `s24-p57-focused-1` 聚焦 34/34 无跳过。同批四文件已发布 assets 根目录及 O:/winnt 并核对摘要（`s24-p57-deploy-1`）；提交 `11747d8336f082aa9faab0de71fb6dac0a57c927` 已推送，远端四文件及公共库源码读取核对一致。生产源码新增 35 行、删除 19 行、净增 16 行；日历像素问题未纳入本轮通过范围。

P56：报告列表、树、VB 绘制通知的目标判断、作用域编排和原生消息结果处理迁入 win16-control，DLL 保留异常保护、线程局部作用域及绘制回调。首次独立链接发现 VB 绘制入口反向依赖 DLL，改为调用方回调后双宽构建和独立链接通过，原资源身份通过；候选 `s24-report-notify-focused` 聚焦 28/28 无跳过；干净包 `s24-p56-clean-1` 双宽构建通过，实际包 `s24-p56-focused-1` 聚焦 28/28 无跳过。同批四文件已发布 assets 根目录及 O:/winnt 并核对摘要（`s24-p56-deploy-1`）；提交 `19cbd4d7db51ca407f1e73fd3678e2c97c09c07c` 已推送，远端四文件及公共库源码读取核对一致。生产源码新增 73 行、删除 59 行、净增 14 行，异常清理及原生消息顺序保持不变；日历像素问题不在本次测试范围。

P55：窗口样式附着状态查询、滚动刷新、普通移除及终止清理协调迁入 win16-control，DLL 保留公开入口、窗口查找及原有回调。首次双宽编译因机械替换误改函数名失败，修正后双宽增量构建、独立链接和原资源检查通过；候选 `s24-window-lifecycle-focused` 聚焦 32/32 无跳过；干净包 `s24-p55-clean-1` 双宽构建通过，实际包 `s24-p55-focused-1` 聚焦 32/32 无跳过。同批四文件已发布 assets 根目录及 O:/winnt 并核对摘要（`s24-p55-deploy-1`）；提交 `9f8cbae1c2e1a4880e97761ebab23ed39df254f7` 已推送，远端四文件及公共库源码读取核对一致。生产源码新增 81 行、删除 51 行、净增 30 行；日历和滚动屏幕像素问题未纳入本轮通过范围。

P54：报告列表与树控件的通知绑定、VCL 首次绑定及解绑规则迁入 win16-control，DLL 保留通知回调和异常作用域。双宽增量构建、独立链接及资源校验通过；候选 `s24-report-bind-focused` 聚焦 26/26 无跳过；干净包 `s24-p54-clean-1` 双宽构建通过，实际包 `s24-p54-focused-1` 聚焦 26/26 无跳过。同批四文件已发布 assets 根目录及 O:/winnt 并核对摘要（`s24-p54-deploy-1`）；提交 `8fbd34b6f890c6b59a227e2cf3729117db6c1a68` 已推送，远端四文件及公共库源码读取核对一致。生产源码新增 61 行、删除 44 行、净增 17 行；既有日历像素问题不在本次测试范围。

P53：初次附着的标准控件/框架安装分派、主题与滚动条协调迁入 win16-control；DLL 保留窗口观察、子类入口及原生回调。双宽增量构建、独立链接和原资源检查通过；候选 `s24-initial-style-focused` 聚焦 34/34 无跳过；干净包 `s24-p53-clean-1` 双宽构建通过，实际包 `s24-p53-focused-1` 聚焦 34/34 无跳过。同批四文件已发布 assets 根目录及 O:/winnt 并核对摘要（`s24-p53-deploy-1`）；提交 `ad776a61679cdeacdba8eaca1b34507b7f49d00b` 已推送，远端四文件及公共库源码读取核对一致。生产源码新增 60 行、删除 36 行、净增 24 行；日历与滚动屏幕像素问题未纳入本次通过范围。

P52：颜色组合框、日期控件和状态栏的首次绘制识别、裁剪快照及绘制后协调迁入 win16-control，DLL 保留窗口观察、原生身份与绘制回调。双宽增量构建、独立链接和原资源校验通过；候选 `s24-native-paint-post-focused` 聚焦 26/26 无跳过；干净包 `s24-p52-clean-1` 双宽构建通过，实际包 `s24-p52-focused-1` 聚焦 26/26 无跳过。同批四文件已发布 assets 根目录及 O:/winnt 并核对摘要（`s24-p52-deploy-1`）；提交 `92f73d4eefa38b184786febf7392b780f87b6cc3` 已推送，远端四文件及公共库源码读取核对一致。生产源码新增 56 行、删除 34 行、净增 22 行；日历差异不在本次测试范围。

P51：首次绘制后已验证 VCL 工具栏、按钮、滚动条、编辑框、组合框、报告、标签和滑块的安装及失败清理迁入 win16-control，调用方保留原生主题与作用域回调。双宽增量构建、独立链接和原资源校验通过；候选 `s24-verified-install-focused` 聚焦 26/26 无跳过；干净包 `s24-p51-clean-1` 双宽构建通过，实际包 `s24-p51-focused-1` 聚焦 26/26 无跳过。同批四文件已发布 assets 根目录及 O:/winnt 并核对摘要（`s24-p51-deploy-1`）；提交 `94cf56eeae184bfd9609a0a3f4531436bda06bf5` 已推送，远端四文件及相应公共库源码读取核对一致。生产源码新增 99 行、删除 74 行、净增 25 行，调用和失败清理顺序保持不变；既有日历像素问题未纳入本次通过范围。

P50：窗口首次绘制后的边框、VB 用户/数据控件、富文本及网格安装协调迁入 win16-control；调用方保留原生 API 和身份回调、作用域及状态所有权。双宽增量构建、独立链接和原资源校验通过；候选 `s24-post-controls-focused` 非日历聚焦 24/24 无跳过；日历 `s24-post-controls-month` x86 通过、x64 发生既有 39 像素差异失败。干净包 `s24-p50-clean-1` 双宽构建通过，实际包 `s24-p50-focused-1` 非日历 24/24 无跳过；日历 `s24-p50-month-1` x86 通过、x64 同样出现 39 像素差异，完整日历测试未通过且原因仍未确定，不能证明本次无影响。同批四文件按非日历通过范围发布至 assets 根目录及 O:/winnt 并核对摘要（`s24-p50-deploy-1`）；提交 `ec6c98791d9fa0658cd90b0a708d2e9a3c00faad` 已推送，远端四文件及相关公共库源码读取核对一致。生产源码新增 67 行、删除 48 行、净增 19 行，调用顺序及原状态所有权不变。

P49：Win16 窗口附着状态结构以及控件角色、父窗口变化的失效判断迁入 win16-control，DLL 保留观察、线程作用域、窗口查找及撤销回调；不改变全局接入或公开 ABI。双宽增量构建、独立链接及来源/资源校验通过；候选 `s24-attachment-guard-focused` 聚焦 34/34 无跳过；干净包 `s24-p49-clean-1` 双宽构建通过，实际包 `s24-p49-focused-1` 聚焦 34/34 无跳过，覆盖双宽角色变化、MDI、弹窗及重入清理。同批四文件已发布 assets 根目录及 O:/winnt 并核对摘要（`s24-p49-deploy-1`）；提交 `c35c4945eeacd302688bc449ea698d9b51bdc112` 已推送，远端四文件及新增公共库源码读取核对一致。生产源码新增 95 行、删除 61 行、净增 34 行（不含构建），日历像素问题仍未解决。

P48：状态栏字体/面板失效及绘制前文本、图标边框的重入安全快照采集迁入 win16-control，DLL 保留线程局部作用域和附着状态所有权。双宽增量构建、独立链接和原资源校验通过；候选 `s24-status-snapshot-focused` 聚焦 26/26 无跳过；干净包 `s24-p48-clean-1` 双宽构建通过，实际包 `s24-p48-focused-1` 聚焦 26/26 无跳过。同批四文件已发布 assets 根目录及 O:/winnt 并核对摘要（`s24-p48-deploy-1`）；提交 `d91c945713a5378664e940c1f7fe053c61d0e0b4` 已推送，远端四文件和相应公共库源码读取核对一致。生产源码新增 46 行、删除 26 行、净增 20 行；状态所有权与原生消息顺序保持不变。

P47：微调状态借用/释放、滑块与工具栏解绑以及滚动条绘制状态快照迁入 win16-control；DLL 保留子类入口和既有绘制回调。双宽增量构建、独立链接和资源身份检查通过；候选 `s24-control-lifetime-extra-focused` 聚焦 30/30 无跳过；干净包 `s24-p47-clean-1` 双宽构建通过，实际包 `s24-p47-focused-1` 聚焦 30/30 无跳过。同批四文件已发布至 assets 根目录及 O:/winnt 并核对摘要（`s24-p47-deploy-1`）；提交 `6fd629cc0095d37aa75aac7754a9c7cbae619937` 已推送，远端四文件和相应公共库源码读取核对一致。生产源码新增 47 行、删除 30 行、净增 17 行；日历像素问题和滚动屏幕采样未包含在本次通过范围。

P46 日历诊断：针对影响样式抽离验证的既有 39 像素差异，原生临时探针 `s24-month-diag/month_model_layout.c` 在 P45 包 4 次中 2 次复现，目标窗口屏幕客户原点和 DC 原点在解除前后均为 (8,31)，鼠标在 (0,0)，不能归因为坐标变化或鼠标覆盖。`month_model_unstyle_root.c` 移除测试顶层窗样式后 5 次中仍有 1 次复现，不能归因为顶层窗样式。`month_model_noop.c` 保持目标样式并连续重绘采样 6 次均 0 像素变化；该对照故意不解除样式，原探针要求有样式差异而退出 1，不能计为产品测试通过。这些证据表明变化与解除样式前后相关，但尚未确定是绘制、状态或采样机制，不能放宽断言。探针及日志留在 `build/t21-package/s24-month-diag/p46-*`；本阶段无产品或二进制修改。

P45：富文本/日历、CoolBar、ControlBar、VB 用户/数据控件及 VB MDI 的样式候选分类迁入 win16-control，DLL 保留附着状态和现有观察流程；双宽增量构建、独立链接和原资源校验通过。候选 `s24-vcl-extra-classify-focused` 非日历聚焦 26/26 无跳过；候选日历 `s24-vcl-extra-classify-month` x86 出现既往 39 像素差异而失败，`s24-vcl-extra-classify-month64` x64 单独通过。干净包 `s24-p45-clean-1` 双宽构建通过，实际包 `s24-p45-focused-1` 非日历 26/26 无跳过；日历 `s24-p45-month-1` 因 demo 进程快照错误 24 未运行目标，`s24-p45-month-2` x86 通过、x64 出现相同 39 像素差异失败。日历完整组未通过，既往版本亦出现该现象，但尚不能断定本次变更无影响。同批四文件按非日历通过范围发布 assets 根目录及 O:/winnt 并核对摘要（`s24-p45-deploy-1`）；提交 `78d131abb88d5685c70d5fdbcffbf5a857e14647` 已推送，远端四文件及公共库源码读取核对一致。生产源码新增 43 行、删除 20 行、净增 23 行。

P44：17 组 VCL 窗口样式候选分类集中迁入 win16-control，DLL 保留观察、附着和状态所有权；判断顺序及原窗口查询保持不变。双宽增量构建、独立链接及来源、资源身份校验通过；双宽多控件分类、绘制、清理的候选 `s24-vcl-classify-focused` 聚焦 36/36 无跳过；干净包 `s24-p44-clean-1` 双宽构建通过，实际包 `s24-p44-focused-1` 聚焦 36/36 无跳过。同批四文件发布至 assets 根目录和 O:/winnt 并核对摘要（`s24-p44-deploy-1`）；提交 `eb9cb371e8df16b906ecb50d4f8a083cdabd955d` 已推送，远端四文件及相应公共库源码读取核对一致。生产源码新增 64 行、删除 34 行、净增 30 行，原附着状态所有者不变；该聚焦集不代表所有 VCL 控件或应用。

P43：旧式文件对话框驱动器组合框的安装协调及组合框子列表解除所有权规则迁入 win16-control；DLL 保留线程局部作用域及安装、原生查询回调。双宽增量构建和独立链接、来源及资源身份检查通过；双宽组合框重新附着、解除与旧式文件打开/保存的候选 `s24-dialog-combo-focused` 聚焦 20/20 无跳过；干净包 `s24-p43-clean-1` 双宽构建通过，实际包 `s24-p43-focused-1` 串行聚焦 20/20 无跳过。同批四文件已发布 assets 根目录和 O:/winnt，摘要一致（`s24-p43-deploy-1`）；提交 `299255006377d8d8e94aabfc20b36a09127d4da2` 已推送，远端四文件及相应公共库源码读取核对一致。生产源码新增 43 行、删除 29 行、净增 14 行，线程作用域及子列表所有权保持不变。

P42：受管滚动条的状态判定、所有者调色板、启用和撤销、滚动参数读写及重绘协调迁入 win16-control；DLL 保留原生 API、分类和绘制回调。双宽增量构建、独立链接及来源和原资源校验通过；聚焦双宽框架/列表/编辑滚动行为、所有权、网格及文件对话框，首次候选 `s24-managed-scroll-focused` 的 x86 文件对话框超时而中止；`s24-managed-scroll-focused-2` 其余聚焦 18/18、`s24-managed-scroll-file-retest` 双宽串行 10/10 通过，首次超时原因未定。干净包 `s24-p42-clean-1` 双宽构建通过，实际包 `s24-p42-focused-1` 串行聚焦 20/20 无跳过；屏幕像素采样未覆盖。同批四文件已发布 assets 根目录和 O:/winnt 并核对摘要（`s24-p42-deploy-1`）；提交 `a6c072109195c81ef40ca07ac69a7fc87502d425` 已推送，远端四文件及相应公共库源码读取核对一致。生产源码新增 107 行、删除 68 行、净增 39 行，状态所有权及原生调用保持不变。

P41：VB 图形按钮的目标资格、准备及实际绘制协调与清单控件绘制资格迁入 win16-control；DLL 保留当前子类入口及清单绘制作用域回调。双宽增量构建和独立链接、来源及原始资源校验通过；聚焦 VB5/VB6 图形按钮及清单、96/144 DPI、双宽、状态回退与资源清理，候选 `s24-graphical-scope-focused` 聚焦 24/24 无跳过；干净包 `s24-p41-clean-1` 双宽构建通过，实际包 `s24-p41-focused-1` 聚焦 24/24 无跳过。同批四文件发布至 assets 根目录及 O:/winnt，摘要核对一致（`s24-p41-deploy-1`）；提交 `69ce2831f168b98f004a71e8445a36098da90d03` 已推送，远端四文件和对应公共库源码读取核对一致。生产源码统计为新增 47 行、删除 33 行、净增 14 行（含内部声明）；原绘制资格和状态所有者保持不变。

P40：UpDown、滑块、工具栏、进度条、树、网格及图片框安装和所有权检查迁入 win16-control，DLL 保留原有入口、子类、主题及生命周期回调。双宽增量构建、独立库链接、来源和原始资源检查通过；普通沙箱候选 `s24-special-install-focused` 关键桌面用例跳过，隔离桌面 `s24-special-install-focused-private` 聚焦 34/34 无跳过，包含双宽控件安装、回退和清理；图片框未单独覆盖。干净包 `s24-p40-clean-1` 双宽构建通过，实际包 `s24-p40-focused-1` 聚焦 36/36 无跳过（含双宽 VB 控件）；同批四文件已发布 assets 根目录及 O:/winnt 并核对摘要（`s24-p40-deploy-1`）。图片框专属探针及日历完整组仍未覆盖；提交 `8af0eb65a7932da9924b10133cb0273c1434a13b` 已推送，远端四文件及新增源码读取核对一致。

P39：日历标题与导航区的原生查询及验证迁入 win16-control；DLL 保留线程防重入、异常清理与作用域所有权。首次编译因缩进警告失败，修正后双宽构建和库独立链接通过；候选 `s24-month-query-focused` 聚焦 16/16 无跳过，含查询嵌套与异常清理。日历 39 像素波动在新旧包均复现；扩大父窗口、窗口位置、DwmFlush、短暂等待与插入符对照均未确定原因，未放宽断言。干净包 `s24-p39-clean-1` 双宽构建通过；`s24-p39-focused-1` 和 `-2` 的 x64 日历因同一 39 像素差异失败，`-2` 的 x86 日历通过，`s24-p39-month64-retest` 单独复测 9/9 通过；`s24-p39-nonmonth-focused` 14/14 无跳过。诊断 `s24-month-diag` 连续两次采样均读到深色像素、Windows 系统可见区域包含该点，尚不能断定成因；完整组仍不算通过。同批四文件已发布 assets 根目录及 O:/winnt 并核对摘要（`s24-p39-deploy-1`）；提交 `624906dd9a9cc2ebce3fa7b2ae853dc21b231a67` 已推送，远端四文件和代表源码读取核对一致。

P38：日历箭头、标题、选中状态及字体借用的绘制资格和状态迁入 win16-control；DLL 保留线程作用域、异常清理和窗口身份回调。双宽增量构建、独立链接通过，候选 `s24-month-paint-focused` 聚焦 20/20 无跳过。P30 的微调按钮 144 DPI 探针发现父窗口客户高度小于按钮范围；扩大父窗口并检查两个 DC 完整裁剪区域后，上一个发布包双宽实际复测均为裁剪外 0 像素，候选双宽相关回归通过。此前 `s24-spin-full-clip-baseline` 因测试程序 build root 与包不同而被包检查拒绝，并未执行，后续使用原生独立探针完成对照。干净包 `s24-p38-clean-1` 双宽构建通过；`s24-p38-nonmonth-focused` 聚焦 18/18 无跳过，包括双宽 144 DPI 完整采样（裁剪外 0）。完整 `s24-p38-focused-1`、`-2`、`-3`、`-4` 因日历探针偶发 39 像素差异未通过；上一发布包扩大父窗口的诊断四次中两次复现同样差异，候选单测和部分对照曾通过。`s24-month-diag` 的采样范围、窗口位置、DwmFlush 和插入符对照仍未定位成因；不放宽断言。已按阶段范围发布至 assets 根目录及 O:/winnt，并以 `s24-p38-deploy-1` 核对非日历通过包摘要；日历干净包验证仍未通过，不计为通过。提交 `57a2cb4da319468d10f5cbe543fb2f40461fb942` 已推送，远端四文件及代表源码、探针读取核对一致。

P37：RichEdit/日历和控件栏身份规则、位图按钮及控件栏边框绘制资格迁入 win16-control；DLL 保留窗口观察、状态所有权及入口。双宽增量构建和独立链接通过。候选 `s24-vcl-extra-focused`、串行 `s24-vcl-month-retest` 和 `s24-vcl-extra-focused-2` 出现日历非目标区域 39 像素差异；候选 `s24-vcl-month-retest-2` 和上一包 `s24-vcl-month-baseline`、`s24-vcl-month-baseline32-2` 分别通过。恢复原身份函数的 A/B 包 `s24-vcl-ab-month` 仍出现差异；独立探针 `s24-month-diag` 在当前与上一包均复现同一 3×13 深色像素区域（客户区 x=0..2、y=14..26），进一步确认夹具父窗口高度 150 小于日历客户区高度 187，DC 裁剪高度仅 150；扩大父窗口后诊断可完整采样并通过。已修复夹具并要求采样裁剪框覆盖完整客户区；使用修正探针的候选 `s24-vcl-extra-fixed-focused` 聚焦 24/24 无跳过。首次干净构建 `s24-p37-clean-1` 不含探针修正，不用于交付，重新构建中。`s24-vcl-month-baseline32` 因 demo 焦点超时未运行目标，复测基础步骤通过。干净包 `s24-p37-clean-2` 双宽构建通过，实际包 `s24-p37-focused-1` 使用完整采样聚焦 24/24 无跳过；同批四文件已发布 assets 根目录及 O:/winnt 并核对摘要（`s24-p37-deploy-1`），提交 `1a98c3e10905c502a1c9bc3972bf7fed1203d07c` 已推送，远端四文件及代表源码和探针读取核对一致；P30 144 DPI 裁剪问题仍未解决。

P36：15 组 Delphi/VB 控件身份与角色匹配迁入 win16-control，包括组合框、按钮、网格、工具栏、嵌入编辑、导航、弹出列表、复选列表、数据控件及边框；DLL 继续持有原窗口状态与入口。双宽增量构建及库独立链接通过，候选 `s24-vcl-identity-focused` 聚焦 32/32 无跳过，覆盖双宽匹配、父窗口变化、原生回退与清理。干净包 `s24-p36-clean-1` 双宽构建通过，实际包 `s24-p36-focused-1` 聚焦 32/32 无跳过；同批四文件已发布 assets 根目录及 O:/winnt 并核对摘要（`s24-p36-deploy-1`），提交 `030bcd61cef669d655e223a370413e76d66f71ab` 已推送，远端四文件及新增源码读取核对一致；P30 144 DPI 裁剪问题仍未解决。

P35：状态栏主题文本与字体应用资格、原生/VCL 身份及系统模块校验迁入 win16-control；DLL 保留线程作用域、原窗口状态和入口。双宽增量构建和独立库链接通过。首个候选 `s24-status-text-focused` 的 x86 字体句柄复用未构造成功而跳过，单独 `s24-status-alias-retest` 9/9 通过；合并候选 `s24-status-text-identity-focused` 双宽 30/30 无跳过。干净包 `s24-p35-clean-1` 双宽构建通过，实际包 `s24-p35-focused-1` 聚焦 30/30 无跳过；同批四文件已发布 assets 根目录及 O:/winnt 并核对摘要（`s24-p35-deploy-1`），提交 `876d16d1c9f3c67db400acadd91d17de2a02b58b` 已推送，远端四文件及代表源码读取核对一致；P30 144 DPI 裁剪问题仍未解决。

P34：状态栏普通边框、最终面板文本/图标边框的绘制资格与字体缓存借用释放迁入 win16-control；DLL 保留窗口身份、作用域与调用入口。双宽增量构建和库独立链接通过；候选 `s24-status-frame-focused` 24/24、`s24-status-lifecycle-focused` 30/30 均无跳过，覆盖双宽原生/VCL 状态栏、DPI、字体隔离及别名。干净包 `s24-p34-clean-1` 双宽构建通过，实际包 `s24-p34-focused-1` 聚焦 30/30 无跳过；同批四文件已发布 assets 根目录及 O:/winnt 并核对摘要（`s24-p34-deploy-1`），提交 `0ef20cd636b14f578b711b80f8a3f5157d7eb610` 已推送，远端四文件及代表源码读取核对一致；P30 144 DPI 裁剪问题仍未解决。

P33：窗口框架安装、普通撤销、菜单专用所有者、调色板更新及终止清理迁入 win16-control；DLL 保留子类入口、菜单线程状态及调用回调。首次独立链接因 DWM 直接引用失败，改为调用方传入现有 DWM 函数后双宽构建及库独立链接通过。候选 `s24-frame-lifecycle-focused` 的 x86 菜单绘制次数为 6 而打包脚本硬编码要求 3；上一发布包 `s24-frame-menu-baseline` 同样为 6。原生探针实际条件是至少 3 次，已修正脚本匹配该契约而保留实际次数，不证明无闪烁；候选 `s24-frame-lifecycle-focused-3` 聚焦 24/24 无跳过（双宽均 6 次）。干净包 `s24-p33-clean-1` 双宽构建通过；实际包 `s24-p33-focused-1` 聚焦 24/24 无跳过，菜单双宽各 3 次绘制；四文件已发布 assets 根目录及 O:/winnt 并核对摘要（`s24-p33-deploy-1`），提交 `5e0ea77fbcb4cdd032d27e68603176257577b0c5` 已推送，远端四文件及新增源码读取核对一致；此前 144 DPI 裁剪失败仍未解决。

P32：标准控件安装、子组合框字体/所有权、普通移除及窗口终止清理迁入 win16-control；DLL 保留窗口子类入口及既有线程、主题和接入回调。首次移除迁移编译因回调返回类型及未使用函数失败，修正后双宽增量构建通过；安装候选 `s24-control-install-focused` 26/26、合并生命周期候选 `s24-control-lifecycle-focused` 28/28 均无跳过，覆盖双宽组合框、字体、取消、重入及滚动行为。干净包 `s24-p32-clean-1` 双宽构建通过，实际包 `s24-p32-focused-1` 聚焦 28/28 无跳过；同批四文件已发布 assets 根目录及 O:/winnt 并核对摘要（`s24-p32-deploy-1`），提交 `5f9362980fde63c1eaebb28f32ba146f343a9700` 已推送，远端四文件及代表源码读取核对一致；P30 的 144 DPI 裁剪及首次文件对话框超时仍未解决。

P31：COM 文件对话框的 IFileDialog2/主接口操作分派迁入 win16-dialog；DLL 通过回调保留对话框显示、事件订阅、文件夹设置和原有接入/异常清理。双宽增量构建与实验事件/隐藏分支语法检查通过；候选 `s24-proxy-dispatch-focused` 20/20、`s24-proxy-events-focused` 16/16 均无跳过；实验事件完整运行仍未验证。干净包 `s24-p31-clean-1` 双宽构建通过，实际包 `s24-p31-focused-1` 聚焦 28/28 无跳过；同批四文件已发布 assets 根目录及 O:/winnt 并核对摘要（`s24-p31-deploy-1`），提交 `6cfec03688129aa59e6e69b600adebc143fb3826` 已推送，远端四文件及新增源码读取核对一致；P30 记录的 144 DPI 裁剪失败和首次文件对话框超时原因仍未解决。

P30：Delphi 微调按钮的绘制快照、缓冲失效/合成、旧式边框决策，以及标准控件样式适用性判断迁入 win16-control；DLL 保留原线程作用域、窗口状态所有权与接入。双宽增量构建、来源和原资源身份校验通过；微调候选 `s24-spin-scope-focused` 18/18、合并候选 `s24-spin-select-focused` 26/26，均无跳过；非完整控件矩阵。干净包 `s24-p30-clean-1` 双宽构建通过；实际包 `s24-p30-focused-4` 排除 144 DPI 裁剪后 28/28 无跳过。首次 `s24-p30-focused-1` 的 x86 文件对话框超时，`s24-p30-file-retest` 双宽串行 10/10 通过，超时原因不明。`s24-p30-focused-2` 与 `s24-p30-spin-retest` 的 x86 144 DPI 裁剪区外 2 像素、`s24-p30-focused-3` 的 x64 区外 16 像素；上一发布包 `s24-p30-spin-baseline` 和 `s24-p30-spin-baseline64` 同样分别为 2 和 16，候选 `s24-spin-scope-focused` 曾双宽通过，根因不明、仍未解决，不能算通过。同批四文件已发布 assets 根目录及 O:/winnt 并核对摘要（`s24-p30-deploy-1`）；提交 `f86425b66031dd51433d071042f26e03448f3d75` 已推送，远端四文件及代表源码读取核对一致。

P29：控件角色资格、原生 owner-draw 和 VB 回退决策迁入 win16-control；DLL 保留身份判定与原接入回调，原生转发和移除顺序不变。双宽增量构建、来源清单及原始资源身份校验通过；候选包 `s24-control-guard-focused` 聚焦 24/24、`s24-guard-ownerdraw` 聚焦 12/12 均无跳过，后者覆盖双宽 owner-draw 和 VB 自绘回退；非完整控件矩阵。首次干净构建 `s24-p29-clean-1` 的进程句柄消失且无结果，不计通过；干净包 `s24-p29-clean-2` 双宽构建通过；实际包 `s24-p29-focused-1` 聚焦 28/28 无跳过。同批四文件已发布 assets 根目录及 O:/winnt 并核对摘要（`s24-p29-deploy-1`）；提交 `aabdc44acf72c1d97724d6894703c3419b062484` 已推送，远端四文件及新增源码读取核对一致。

P27 后，控件消息收尾中的字体租约释放、重入移除后的字体恢复及微调编辑框布局恢复迁入 win16-control；DLL 保留原窗口回调与状态入口。双宽增量构建通过，候选包 `s24-control-lifetime-focused` 聚焦 18/18 无跳过，覆盖字体布局、微调编辑、组合框及所有权；不是全部控件组合。双宽库链接及生命周期各 2/2 通过；干净包 `s24-p28-clean-1` 双宽构建通过，实际包 `s24-p28-focused-1` 聚焦 18/18 无跳过；同批四文件已发布 assets 根目录及 O:/winnt 并核对摘要（`s24-p28-deploy-1`）；提交 `97d46679ff6817d268ddc24f9f9b390e05b75ac1` 已推送，远端四文件及新增源码读取核对一致。

P26 后，旧式 COM 对话框的实验性目录切换与覆盖提示事件转换迁入 win16-dialog；入口、实验开关与事件所有权回调保持原位。新增独立测试覆盖目录切换允许/拒绝、待处理目录和覆盖提示状态；双宽独立测试及实验宏语法检查通过。候选包 `s24-classic-events-focused` 聚焦 16/16 无跳过，覆盖默认打开、保存、多选与 Shell 文件夹；实验事件完整运行仍未验证。干净包 `s24-p27-clean-1` 双宽构建及独立事件策略测试通过，实际包 `s24-p27-focused-1` 聚焦 16/16 无跳过；同批四文件已发布 assets 根目录及 O:/winnt 并核对摘要（`s24-p27-deploy-1`）；提交 `e374d074935e247a8bd11457c955f83a3d39c670` 已推送，远端四文件及新增源码读取核对一致。

P25 后，COM 旧式对话框代理的窗口附着记录、标签选择及普通/实验事件模式清理迁入 win16-dialog，入口和实验开关不变。新增独立测试验证标签、窗口所有权、COM 引用及模式差异；双宽独立测试及启用实验宏的语法检查通过。候选包 `s24-dialog-hook-state-focused` 聚焦 16/16 无跳过，覆盖双宽打开、保存、多选和 Shell 文件夹；实验事件的完整运行仍未验证。干净包 `s24-p26-clean-1` 双宽构建和独立状态测试通过，实际包 `s24-p26-focused-1` 聚焦 16/16 无跳过；同批四文件已发布 assets 根目录及 O:/winnt 并核对摘要（`s24-p26-deploy-1`）；提交 `bc7c04b8333fd1b4f5991fd2b753dcdc98cde132` 已推送，远端四文件及新增源码读取核对一致。

P24 后，实验性旧式对话框隐藏文件列表筛选和刷新调度迁入 win16-dialog；默认实验开关不变。新增临时目录独立测试验证筛选、去重与清理，双宽对话框库测试通过；启用隐藏文件及事件实验宏的双宽语法检查通过，不代表实验功能完整运行。候选包 `s24-hidden-files-focused-2` 聚焦 16/16 无跳过，覆盖默认打开/保存、多选及 Shell 文件夹；旧布局文件夹用例 `s24-hidden-files-focused` 返回 old=0，P24 基线 `s24-hidden-baseline-folder` 同样失败，不能算通过。首次编译因 COM 宏头顺序及测试缩进失败，修正后双宽通过。干净包 `s24-p25-clean-1` 双宽构建及独立筛选测试通过，实际包 `s24-p25-focused-1` 聚焦 16/16 无跳过；同批四文件已发布 assets 根目录及 O:/winnt 并核对摘要（`s24-p25-deploy-1`）；提交 `3d9400b959df403a2626eb1f66b40594482fa834` 已推送，远端四文件及新增源码读取核对一致。

P23 后，VCL 分组框与面板边框的外观资格、绘制匹配和配对状态迁入 win16-control；附件仍持有唯一绘制范围并提供身份/重入回调。双宽增量构建通过，候选包 `s24-vcl-frame-focused` 聚焦 16/16 无跳过，覆盖双宽分组框、单选组和面板的裁剪、重入、禁用及重新指定父窗口；不是全部 DPI/控件组合。双宽库链接及生命周期各 2/2 通过；干净包 `s24-p24-clean-1` 双宽构建通过，实际包 `s24-p24-focused-1` 聚焦 16/16 无跳过。同批四文件已发布 assets 根目录及 O:/winnt 并核对摘要（`s24-p24-deploy-1`）；提交 `3e0d7df7f4e36ba8ee699efb5dffdca8626700a7` 已推送，远端四文件及新增源码读取核对一致。

P22 后，报告文本可替换字符判断及静态文本字体选择迁入 win16-control；DLL 仍负责原有绘制范围和调用者身份。新增独立策略测试覆盖中文回退、RTL、长度边界和测量标志；双宽库链接及生命周期各 2/2 通过。候选包 `s24-text-policy-focused` 聚焦 18/18 无跳过，覆盖双宽字体布局、静态文本、报告、复选列表和链接控件；不代表所有语言。干净包 `s24-p23-clean-1` 双宽构建通过，实际包 `s24-p23-focused-1` 聚焦 18/18 无跳过，干净包的策略与生命周期双宽各 2/2 通过。同批四文件已发布 assets 根目录及 O:/winnt 并核对摘要（`s24-p23-deploy-1`）；提交 `4b254c7224ecf2b8c40a797852fce24d61413a70` 已推送，远端四文件及新增源码读取核对一致。

P21 后，VCL 内嵌编辑、下拉及数据库/媒体导航按钮的绘制和外观资格判断迁入 win16-control，DLL 借用原附件状态及身份回调。双宽增量构建通过，候选包 `s24-vcl-native-focused` 聚焦 12/12 无跳过，覆盖按钮状态、下拉、裁剪、焦点、外部调用、重新指定父窗口及清理。双宽库链接及生命周期各 2/2 通过；干净包 `s24-p22-clean-1` 构建通过，实际包 `s24-p22-focused-1` 聚焦 12/12 无跳过。同批四文件已发布 assets 根目录及 O:/winnt 并核对摘要（`s24-p22-deploy-1`）；提交 `fe68e65f5d4a5cdcc804d34512e77e076f77b671` 已推送，远端四文件及新增源码读取核对一致。

P20 后，CoolBar 手柄绘制资格和自定义绘制通知决策迁入 win16-control；附件身份和线程状态仍由 DLL 提供。双宽增量构建、候选包 `s24-cool-grip-focused` 聚焦 12/12 无跳过，覆盖嵌套、异常、移除、重设父窗口和回退。上一版与 P20 的独立 x64 日期控件对照均为 GDI 22→22（`s24-date-baseline-compare`、`s24-date-current-compare`）；首次 22→29 的原因仍不明。双宽库链接及生命周期各 2/2 通过；干净包 `s24-p21-clean-1` 构建通过，实际包 `s24-p21-focused-1` 聚焦 12/12 无跳过。同批四文件已发布 assets 根目录及 O:/winnt 并核对摘要（`s24-p21-deploy-1`）；提交 `0667c34b90c9c5c89fb4b9699f3411cda3bb5fa1` 已推送，远端四文件及新增源码读取核对一致。

P19 后，颜色选择组合框和日期控件原生按钮的外观资格检查及绘制协调迁入 win16-control，通过借用视图读取原附件状态；接入和状态所有权仍在 DLL。双宽增量构建通过，候选包 `s24-native-combo-focused` 聚焦 16/16 无跳过，覆盖双宽颜色、驱动器组合框、日期及组合框的选择、裁剪、重入和清理；不是全部控件组合。双宽库链接及生命周期各 2/2 通过；干净包 `s24-p20-clean-1` 构建通过。首次实际包 `s24-p20-focused-1` 的 x64 日期 GDI 资源计数失败（预热 22、结束 29），原因未确定；独立 `s24-p20-date-retest-1` 通过，整组串行 `s24-p20-focused-2` 聚焦 16/16 无跳过，不能证明首次失败无泄漏。同批四文件已发布 assets 根目录及 O:/winnt 并核对摘要（`s24-p20-deploy-1`）；提交 `e07b256304ba6ea2411b96309cb880b85997a805` 已推送，远端四文件及新增源码读取核对一致。

P18 后，窗口菜单键盘、鼠标切换、系统菜单及标题按钮输入的样式决策迁入 win16-control；线程内菜单跟踪和窗口入口仍由 DLL 提供。双宽增量构建、候选包 `s24-frame-input-focused-2` 聚焦 20/20 无跳过，覆盖菜单取消、初始绘制、系统菜单、VB MDI 按钮与待显示框架；完整 MDI 合并菜单导航仍未单独覆盖。双宽菜单分派、框架、库链接和生命周期测试各 4/4 通过。干净包 `s24-p19-clean-1` 双宽构建通过，实际包 `s24-p19-focused-1` 聚焦 20/20 无跳过；同批四文件已发布 assets 根目录及 O:/winnt 并核对摘要（`s24-p19-deploy-1`）；提交 `c201e902c9bf895f5723e31acff74977c0429b4f` 已推送，远端四文件及新增源码读取核对一致。

P17 后，窗口非客户区布局、绘制、激活、命中、DPI 和主题变化等消息协调迁入 win16-control；调用方继续持有原窗口状态并提供接入回调。双宽增量构建及框架、库链接和生命周期各 3/3 通过；候选包 `s24-frame-message-focused-2` 聚焦 22/22 无跳过，覆盖双宽菜单、待显示框架、MDI 按钮、滚动行为及对话框。首次 `s24-frame-message-focused` 指定不存在的对话框测试名而未运行目标，修正后通过。完整 MDI 合并菜单导航、屏幕像素采样及全部框架组合仍未覆盖。干净包 `s24-p18-clean-1` 双宽构建通过，实际包 `s24-p18-focused-1` 聚焦 22/22 无跳过；同批四文件已发布 assets 根目录及 O:/winnt 并核对摘要（`s24-p18-deploy-1`）；提交 `69c08e3a2a223363f6bddc08697dd9f510973421` 已推送，远端四文件及新增源码读取核对一致。

P16 后，菜单项测量、绘制、助记键及弹出菜单初始化/退出消息迁入 win16-control，DLL 仅继续提供帧按钮绘制回调和窗口消息入口。双宽增量构建及候选包 `s24-menu-message-focused` 聚焦 18/18 无跳过，覆盖菜单取消、初始绘制、自定义系统菜单及 VCL MDI 控件；完整 MDI 合并菜单导航仍未单独覆盖。首次来源检查发现被迁入声明的 `menu_scope.h` 属于逐字节保留文件，已恢复原字节并改用库内声明，来源清单通过。双宽菜单分派、控件库链接及生命周期各 3/3 通过。干净包 `s24-p17-clean-1` 构建通过，实际包 `s24-p17-focused-1` 聚焦 18/18 无跳过；同批四文件已发布 assets 根目录及 O:/winnt 并核对摘要（`s24-p17-deploy-1`）；提交 `36eede6ed1014bd8befb773f4bcdd7b2a77517a1` 已推送，远端四文件及新增源码读取核对一致。

P15 后，将标准控件样式消息分派迁入 win16-control：通过调用方提供的状态、绘制和类别回调，继续使用同一窗口所有者和原生消息顺序；接入、资格检查及状态析构入口仍在 DLL。双宽增量构建通过，实际候选 `s24-control-dispatch-focused` 聚焦 28/28 无跳过，覆盖双宽标签、表头、进度条、滑块、组合框、复选列表及框架/列表/编辑框滚动行为；不代表完整控件矩阵或屏幕像素采样。干净包 `s24-p16-clean-1` 双宽构建通过，实际包 `s24-p16-focused-1` 聚焦 28/28 无跳过；同批四文件已发布 assets 根目录及 O:/winnt 并核对摘要（`s24-p16-deploy-1`）；提交 `218ef6e720bee0d68473dac8305da308f73b44fa` 已推送，读取远端四文件与新增源码核对一致。

P14 后，弹出菜单布局与命令派发迁入 win16-control，线程内菜单跟踪及其状态入口仍由 DLL 提供；文件夹浏览的 A/W 参数和结果转换迁入 win16-dialog，现有入口及条件不变。双宽增量构建、菜单分派/两库链接/生命周期各 4/4 通过；新增独立测试覆盖 A/W 转换、禁用与不支持回退、结果保留；增量包 `s24-menu-browse-focused-2` 聚焦 20/20 无跳过。首次干净构建 `s24-p15-clean-1` 因新增测试的来源摘要未更新而在清单门禁停止；第二次因复核重入时序后调整源码而主动停止。清单通过；干净包 `s24-p15-clean-3` 双宽构建通过，实际包 `s24-p15-focused-1` 聚焦 20/20 无跳过，两库链接和生命周期双宽各 3/3 通过。同批四文件已发布 assets 根目录及 O:/winnt 并核对摘要（`s24-p15-deploy-1`）；提交 `80122e59a57bb30d7b814bd1db7c6266dff481c1` 已推送，远端四文件及两个库的代表源码读取核对一致。此前完整 MDI 合并菜单导航和屏幕采样限制仍未解决。

P13 后，菜单栏跨弹出菜单及 MDI 窗口的导航迭代迁入 win16-control；DLL 继续提供原有菜单跟踪入口和借用状态回调。双宽增量构建及菜单分派、库链接和生命周期测试各 3/3 通过；增量包 `s24-menu-iteration-focused` 聚焦 18/18 无跳过，包含双宽菜单取消、首次绘制、系统菜单和 VCL MDI 控件。完整 MDI 合并菜单间导航未单独覆盖。干净包 `s24-p14-clean-1` 双宽构建及实际包 `s24-p14-focused-1` 聚焦 18/18 无跳过；同批四文件已发布 assets 根目录及 O:/winnt 并核对摘要（`s24-p14-deploy-1`）；提交 `0c9b07aa4d77bd3249b69be801f52384943eb472` 已推送，远端四文件与新增源码读取核对一致。

P12 后，标签页、表头、进度条及非客户区滚动条绘制协调迁入 win16-control；DLL 保留调用入口与原状态。双宽增量构建通过，增量包 `s24-native-presentation-focused-3` 聚焦 24/24 无跳过。首轮选择了不存在的 `scroll-frame-32`，未运行目标；改用正确名称后 `scroll-frame-x86` 横/纵返回 5，既往基线也为 5，屏幕像素采样仍未通过。随后使用不依赖屏幕采样的滚动行为用例通过，不能替代视觉覆盖。干净包 `s24-p13-clean-1` 双宽构建通过，实际包 `s24-p13-focused-1` 聚焦 24/24 无跳过；双宽库链接及生命周期各 2/2 通过。同批四文件已发布 assets 根目录及 O:/winnt 并核对摘要（`s24-p13-deploy-1`）；提交 `ee3d995c65a02fd39b85ba0ffe30319e198d0853` 已推送，读取远端四文件及两份新增源码核对一致。

S24 P12：滑块和工具栏进程内绘制通知、组合框编辑区字体持有与消息处理迁入 win16-control；DLL 保留原有子类入口、状态持有和接入流程。双宽增量构建、独立链接和生命周期测试各 2/2 通过；增量候选聚焦 20/20 无跳过。干净包 `s24-p12-clean-2` 双宽构建通过，`s24-p12-focused-2` 实际包聚焦 24/24 无跳过，覆盖双宽字体布局、组合框、滑块和工具栏。同批四文件已发布 assets 根及 O:/winnt 并核对摘要（`s24-p12-deploy-2`）；首次交付后提交检查发现文件尾空行，修正后重新干净构建、测试和发布；提交 `72043afbeb05d68298fd3534a13ebf4ccf0fd0d1` 已推送，读取远端四文件和两份代表源码核对一致。普通沙箱下 `s24-combo-text-focused` 有 15 项因桌面权限跳过；隔离桌面授权后上述 20 项均通过。此前 VB 文件测试与屏幕采样限制仍未解决；本阶段不代表全部样式逻辑迁移或 S24 收口。

S24 增量进展：菜单左右键边界判断迁入 win16-control，颜色对话框色块边界识别迁入 win16-dialog；DLL 保留菜单消息入口和颜色绘制调用入口。双位宽增量构建、ABI、两库独立链接/模板测试各 3/3 通过；增量候选四文件结构检查及双位宽私有桌面菜单键盘、颜色选择和焦点探针通过。上述是局部证据，不证明所有样式逻辑已迁出，也不是干净构建或正式发布。剩余至少包括窗口/控件消息和状态、菜单循环、颜色对话框识别上下文以及 COM 对话框实验分支；需逐项继续审计。候选包 `build/t21-package/s24-menu-color-candidate` 及测试目录 `s24-unit-x86`、`s24-unit-x64` 均为本 S scratch。

滚动条原生参数到 Win16 几何的重复转换已迁入 win16-control，双宽增量构建及几何、链接和 ABI 各 4/4 通过，当前增量包结构检查通过。私有桌面滚动框架探针中当前 x86 候选返回横/纵 5；同环境 S23 基线双宽也返回横/纵 5。探针代码显示 5 对应屏幕像素采样失败，未记录系统错误码；不能断定根因，亦不能算实窗滚动通过。当前 x64 候选实窗未测。相关 scratch：`s24-scroll-candidate`、`s24-geometry-x86`、`s24-geometry-x64`。

来源校验发现 `scroll_layout` 为须逐字节保留的来源文件，已恢复原文件并将新增转换独立放入 `scroll_native.c/.h`；来源清单通过。该调整后的双宽增量构建、几何、链接和 ABI 各 4/4 通过；普通沙箱下菜单分派和原生滚动测试创建私有桌面错误 5，在允许私有桌面的执行环境双宽各 2/2 通过；其中 core/scroll-native 是系统 API 对照，不加载项目 DLL。此后者不验证产品滚动消息或此前屏幕像素采样失败；尚未完成干净包测试或发布。额外 scratch：`s24-scroll-native-x86`、`s24-scroll-native-x64`、`s24-native-priv-x86`、`s24-native-priv-x64`。

标题和菜单字体创建/替换策略、菜单栏屏幕命中及键盘选择运算迁入 win16-control，字体句柄与窗口状态仍由原窗口所有者持有。双宽增量构建、ABI、独立库字体参数/菜单选择和几何测试各 3/3 通过；当前增量包结构检查及私有桌面双宽菜单键盘探针通过。此前候选另有双宽字体框探针通过，不能当作当前快照的单独结果。新增 scratch：`s24-frame-font-x86`、`s24-frame-font-x64`、`s24-font-candidate`、`s24-menu-hit-x86`、`s24-menu-hit-x64`、`s24-menu-hit-candidate`。

窗口非客户区命中判断迁入 win16-control，DLL 保留同线程检查、窗口状态和布局获取；双宽增量构建及 ABI、几何和独立窗口命中测试各 3/3 通过。当前增量包双宽私有桌面菜单键盘和字体对话框探针通过；此为局部验证，后续仍须真实非客户区交互回归及同批干净包验证。scratch：`s24-frame-hit-x86`、`s24-frame-hit-x64`、`s24-frame-hit-candidate`。

窗口框架刷新及客户区屏幕坐标保存/恢复迁入 win16-control，调用时机和生命周期仍归窗口所有者。双宽增量构建、ABI、几何及独立自有窗口缩放后客户区恢复测试各 3/3 通过；尚需现行安装/移除路径回归。scratch：`s24-client-restore-x86`、`s24-client-restore-x64`。

随后以当前增量 DLL 运行自有私有桌面 core/runtime 与 core/frames，双位宽各 2/2 通过，覆盖现行附着、解除和框架路径；并非干净包验证。runtime 各约 53 秒，超出预期，后续无相关修改不重复；原命令行 timeout 35 没有覆盖测试自身较长的超时属性。scratch：`s24-frame-runtime-x86`、`s24-frame-runtime-x64`。

菜单栏选中和退出逻辑进一步迁入 win16-control，以借用视图操作原窗口所有者的状态并通过回调调用布局/绘制；DLL 仍负责菜单跟踪与状态生命周期。双宽增量构建、ABI、几何及独立菜单状态测试各 3/3、私有桌面菜单分派各 1/1 通过；当前增量包结构检查及双宽菜单键盘探针通过，尚未完成全菜单矩阵或干净包。scratch：`s24-bar-state-x86`、`s24-bar-state-x64`、`s24-bar-dispatch-x86`、`s24-bar-dispatch-x64`、`s24-bar-state-candidate`。

菜单鼠标切换、Esc 和方向键消息决策迁入 win16-control；系统消息过滤入口与菜单生命周期仍在 DLL，库借用原活动状态以保留重入顺序。双宽增量构建、ABI 和独立消息测试各 2/2，私有桌面菜单取消及分派各 2/2 通过；测试为当前增量 DLL，尚非正式包。scratch：`s24-menu-input-x86`、`s24-menu-input-x64`、`s24-menu-input-priv-x86`、`s24-menu-input-priv-x64`。

菜单助记键匹配进一步迁入库，包含禁用项、转义 `&&` 和制表符后快捷键文本的既有规则；双宽增量构建、ABI 及独立匹配测试各 2/2 通过。此次新源码尚未完成实窗菜单回归。scratch：`s24-mnemonic-x86`、`s24-mnemonic-x64`。

窗口非客户区光标选择迁入 win16-control；双宽增量构建、ABI 和独立控件客户区/错误目标/边框命中测试各 2/2 通过。尚未做当前源码的真实光标外观检查或干净包回归。scratch：`s24-frame-cursor-x86`、`s24-frame-cursor-x64`。

非客户区更新区域复制、裁剪及缓冲绘制协调迁入 win16-control；窗口消息入口和画笔状态仍由 DLL 持有。双宽增量构建、ABI 和独立绘制回调测试各 2/2 通过；本快照双宽私有桌面菜单取消（包含框架裁剪探针）和消息分派各 2/2 通过。不是完整视觉或干净包验证。scratch：`s24-ncpaint-x86`、`s24-ncpaint-x64`、`s24-ncpaint-priv-x86`、`s24-ncpaint-priv-x64`。

实验性 COM 旧式文件对话框的选择项捕获与释放已迁入 win16-dialog；双宽增量构建、ABI 和使用自有临时文件的选择/清空独立测试各 3/3 通过，事件和隐藏文件实验宏双宽语法检查通过。当前增量包结构检查、双宽 COM 文件框和菜单键盘探针通过；该独立测试不证明实验功能的完整对话框运行时，也非干净包。scratch：`s24-dialog-selection-x86`、`s24-dialog-selection-x64`、`s24-selection-candidate`。

普通系统对话框背景、文字和画刷颜色设置迁入 win16-control；双宽增量构建及独立 DC 颜色状态、ABI 各 2/2 通过；该快照候选的双宽私有桌面颜色键盘、选择及色块探针通过。尚未完成干净包。scratch：`s24-dialog-colors-x86`、`s24-dialog-colors-x64`、`s24-colors-candidate`。

独立滚动条焦点标记的创建、同步与销毁迁入 win16-control，库通过借用视图使用原控件的状态及布局回调。双宽增量构建、ABI 与几何测试各 2/2，私有桌面系统滚动 API 对照各 1/1 通过，但该对照不加载项目 DLL；不覆盖产品消息行为或屏幕像素采样，尚未进行当前快照的干净包测试。scratch：`s24-scroll-caret-x86`、`s24-scroll-caret-x64`、`s24-scroll-caret-priv-x86`、`s24-scroll-caret-priv-x64`。

滚动条箭头禁用状态、列表方向归属与 EDIT 横向范围可表达性判断迁入 win16-control，原生状态读取由调用方回调提供；双宽增量构建、ABI 和范围边界测试各 2/2、私有桌面系统滚动 API 对照各 1/1 通过，但该对照不加载项目 DLL；未覆盖产品消息、之前屏幕像素采样失败或全部控件组合。scratch：`s24-scroll-policy-x86`、`s24-scroll-policy-x64`、`s24-scroll-policy-priv-x86`、`s24-scroll-policy-priv-x64`。

滚动通知分派、计时器停止、捕获归还和手势结束协调迁入 win16-control，通过借用视图保留唯一状态及同步通知后的重入检查。双宽增量构建、ABI/几何各 2/2，通过自有父窗口重入测试验证首条通知改变 epoch 后不再发送后续通知；双宽私有桌面系统滚动 API 对照各 1/1 通过，但该对照不加载项目 DLL，不能证明产品滚动通知、此前屏幕像素采样或捕获组合通过。scratch：`s24-scroll-actions-x86`、`s24-scroll-actions-x64`、`s24-scroll-actions-priv-x86`、`s24-scroll-actions-priv-x64`、`s24-scroll-finish-x86`、`s24-scroll-finish-x64`、`s24-scroll-finish-priv-x86`、`s24-scroll-finish-priv-x64`。

随后用 `s24-scroll-finish-candidate` 实际包在自有私有桌面运行双宽 VCL 滚动 fixture，均报告水平/垂直通知、鼠标、禁用、重入移除及清理通过（96 DPI）；不是全部控件或屏幕采样覆盖。首次驱动调用重复传入 `--wait`，启动器将其当成目标并返回错误 2，目标测试未运行；修正后双宽通过。

独立及嵌入式滚动条的鼠标/键盘输入状态机已迁入 win16-control，DLL 仅以借用视图提供状态、原生查询及绘制回调。增加无屏幕采样的实际 DLL 探针并注册正式包用例；当前增量候选的双宽窗口、列表及编辑框水平/垂直拖动均记录捕获、跟踪位置改变及释放，正式聚焦回归 14/14、无跳过。首次增量候选仍非干净包，不以消息状态测试取代旧屏幕采样失败。scratch：`s24-scroll-input-x86`、`s24-scroll-input-x64`、`s24-scroll-input-candidate`、`s24-embedded-input-x86`、`s24-embedded-input-x64`、`s24-embedded-final-x86`、`s24-embedded-final-x64`、`s24-embedded-candidate`、`s24-embedded-focused`。

列表制表位的字符度量、像素换算及状态替换迁入 win16-control，DLL 保留原始消息转发和状态所有权。双宽增量构建、ABI/独立参数和状态测试各 2/2 通过；实际 DLL 的双宽列表制表位原生测试各 1/1 通过。尚未进行当前快照的干净包测试。scratch：`s24-list-tabs-x86`、`s24-list-tabs-x64`、`s24-list-tabs-priv-x86`、`s24-list-tabs-priv-x64`。

准备 S24 P1 阶段交付：使用 `build/t21-package/s24-stage-clean-1` 干净构建当前已迁移批次，再以 `s24-stage-focused-1` 聚焦验证菜单、对话框、滚动和列表；成功后发布同批四文件并复核。该阶段交付不改变 S24 的完整目标，也不表示迁移结束。

S24 P1 阶段包 `s24-stage-clean-2` 双宽干净构建通过，实际包聚焦 20/20 无跳过，干净树独立库/几何/列表/DC 故障各 5/5 通过；同批四文件已发布 assets 根和 O:/winnt 并核对 SHA。产品提交 `78ad520d2ed914181a17d4cb7f6ffd45f6c8d8ba` 已推送，读取远端四文件及两个库的代表源码与本地一致。未解决的屏幕采样、实验分支与剩余迁移见 [阶段记录](../history/M0-T21-S24-stage.md)。S24 持续开放。

P1 后继续迁移滚动条原生布局查询至样式库；DLL 提供原生查询回调。修正测试中的声明顺序后双宽增量构建及 ABI/几何各 2/2 通过，当前增量候选的双宽窗口、列表和编辑框水平/垂直捕获、移动、释放探针六项通过（96 DPI）；未解决屏幕采样，尚未重新干净构建和发布。scratch：`s24-scroll-layout-x86`、`s24-scroll-layout-x64`、`s24-scroll-layout-candidate`。

窗口主题状态的捕获、提交、启停、恢复和析构已迁入 win16-control；DLL 通过显式回调提供现有原生主题调用并保留公开 ABI，状态只存在库中一份。双宽增量构建、ABI、库独立链接、几何及包含分配失败/嵌套状态/原主题恢复的主题所有权测试各 4/4 通过。主题所有权测试直接编译同一库源码及 DLL 适配器，尚不是实际发布 DLL 回归或 Win2000 样式实现。scratch：`s24-theme-state-x86`、`s24-theme-state-x64`。

当前主题状态增量包结构检查及私有桌面双宽列表滚动和颜色对话框探针通过；双宽库独立链接测试也验证未附着窗口的原生主题回调按参数和 HRESULT 原样传递。仍未完成该增量的干净包/发布，亦不能将这些结果扩张为 Win2000 接入验证。scratch：`s24-theme-state-candidate`、`s24-theme-callback-x86`、`s24-theme-callback-x64`。

标题按钮按下、移动、释放及取消处理迁入 win16-control，库借用原窗口的状态和布局/绘制回调。双宽增量构建、ABI 和独立按钮状态测试各 2/2 通过；当前增量候选在私有桌面双宽 VB6 MDI 最大化按钮及恢复探针通过，仍未覆盖全部按钮和真实视觉。scratch：`s24-frame-button-x86`、`s24-frame-button-x64`、`s24-frame-button-mdi`、`s24-frame-button-candidate`。

菜单栏点击位置与 MDI 合并系统菜单关闭资格判断迁入 win16-control，消息入口和最终命令投递保留 DLL。双宽增量构建、ABI/独立库各 2/2 及私有桌面菜单取消各 1/1 通过；独立测试覆盖无菜单负例，完整 MDI 合并菜单与当前快照的干净包尚未验证。scratch：`s24-mdi-menu-x86`、`s24-mdi-menu-x64`、`s24-mdi-menu-priv-x86`、`s24-mdi-menu-priv-x64`。

上下微调控件的进程内样式适配器已迁入 win16-control，DLL 保留兼容头文件和调用入口；原来源记录随文件迁移并更新摘要。双宽全目标增量构建、来源清单及原图形逐字节检查通过。P2 干净包 `s24-p2-clean-1` 双宽构建通过；`s24-p2-focused-1` 实际包聚焦回归 26/26、无跳过，包含双宽 v5/v6 上下微调、所有权、主题状态、菜单、颜色对话框及滚动行为。同批四文件已发布 assets 根目录及 O:/winnt 并核对摘要（`s24-p2-deploy-1`）；产品提交 `e4313994e382d64a118cff297f08af0ec923d4e8` 已推送，读取远端四文件及两个库源码与本地一致。此前屏幕采样失败与完整 MDI 合并菜单仍未覆盖；该阶段不等于全部业务迁移完成。

P2 后，VB Data 导航箭头样式的调用方身份、目标和绘制几何资格判断迁入 win16-control，DLL 提供既有附着查询回调。双宽增量构建及来源清单通过；实际增量包双宽箭头行为聚焦回归 10/10、无跳过，覆盖四种状态、错误调用方、裁剪、错误所有者、不支持控件和重新指定父窗口。此批尚未干净构建或发布。scratch：`s24-vb-arrow-candidate`、`s24-vb-arrow-focused`。

VB5/VB6 列表复选框绘制的身份与几何验证、重入保护和绘制协调迁入 win16-control；DLL 保留调用入口。对应增量包双宽、96/144 DPI 的 VB5/VB6 聚焦回归 18/18、无跳过（包括双宽 Data 箭头）。随后将 VCL 复选列表绘制范围验证迁入库，附着层保留当前绘制范围状态及身份回调；新候选双宽 VCL 复选列表聚焦回归 10/10、无跳过。两次各自包含包测试框架必选项，不能相加解释成独立控件场景数；它们均为增量候选证据。scratch：`s24-checklist-candidate`、`s24-checklist-focused`、`s24-checklist-target-candidate`、`s24-checklist-target-focused`。

P3 干净包 `s24-p3-clean-1` 双宽构建、来源与资源身份门禁通过；`s24-p3-focused-1` 实际包聚焦 14/14、无跳过，包含双宽 VB6 列表复选框、Data 箭头及 VCL 复选列表。此前 144 DPI 和 VB5 为增量候选证据，不冒充该干净包结果。同批四文件已发布 assets 根目录和 O:/winnt 并核对摘要（`s24-p3-deploy-1`）；提交 `b5ba13a0e9a102068c6e37e45dbe5af1445aea18` 已推送，远端四文件及代表源码读取核对一致。S24 保持开放。

P3 后将滑块和工具栏共享的自定义绘制阶段决策迁入 win16-control，仍由窗口入口持有唯一状态并调用应用原有通知。双宽增量构建和实际候选包聚焦 16/16、无跳过，覆盖 v5/v6 滑块及 VCL 滑块/工具栏；独立库测试双宽通过，额外检查应用跳过、私有阶段和结束状态。首次直接运行 CTest 因缺少 `WIN16STYLE_CAPTURE_DIR` 参数未执行测试；指定登记目录后双宽通过。上述为增量候选证据。scratch：`s24-custom-draw-candidate`、`s24-custom-draw-focused`、`s24-custom-unit-x86`、`s24-custom-unit-x64`。

实验性 COM 旧布局事件分支中的目录列表目标路径解析迁入 win16-dialog，事件分派及原所有者仍由调用层负责。双宽增量构建及独立库测试通过，覆盖系统临时目录的盘符根选择、无选择和不匹配盘符；两个实验宏组合的双宽语法检查通过。尚未验证该实验事件分支的完整运行时。scratch：`s24-dialog-list-unit-x86`、`s24-dialog-list-unit-x64`。

P4 干净包 `s24-p4-clean-1` 双宽构建及资源、来源和治理门禁通过；实际包 `s24-p4-focused-1` 聚焦回归 16/16、无跳过，覆盖双宽滑块、VCL 滑块/工具栏和 COM 工厂；干净树的两个独立库测试双宽各 2/2 通过。同批四文件已发布 assets 根目录与 O:/winnt 并核对摘要（`s24-p4-deploy-1`），提交 `d76ffd52e6a8217591f8e269bfa98c973c08b882` 已推送，远端四文件和两个库代表源码读取核对一致。COM 实验事件分支仍未完成完整运行时验证，S24 保持开放。

P4 后进一步将实验性旧式对话框事件分支的输入目录、驱动器和覆盖目标解析迁入 win16-dialog；调用层仍负责事件和唯一状态。双宽增量构建、实验宏语法及对话框库测试通过；库测试覆盖目录、无效目录、驱动器、绝对文件以及相对文件和扩展名。实验事件回调链未做完整运行时验证；上述为增量候选证据。scratch：`s24-dialog-targets-unit-x86`、`s24-dialog-targets-unit-x64`、`s24-dialog-targets-final-x86`、`s24-dialog-targets-final-x64`。

实验事件监听器的快照及引用持有/释放迁入 win16-dialog；异常清理注册与事件回调仍在调用层。双宽增量构建、实验宏语法和独立库测试通过；测试覆盖快照顺序、列表变化时的引用保持以及释放。完整实验事件运行时尚未验证。scratch：`s24-events-unit-x86`、`s24-events-unit-x64`。

P5 干净包 `s24-p5-clean-1` 双宽构建和干净树双宽对话框库测试通过；`s24-p5-focused-2` 实际包聚焦 16/16、无跳过，包含文件打开、COM 工厂及系统布局的事件/覆盖对照。首次 `s24-p5-focused-1` 的 x86 文件打开超时、日志仅到目标启动；同一用例在 P4 基线及 P5 单独重试均通过，根因未明，不将首次算通过。最终聚焦回归随后通过。同批四文件已发布 assets 根目录及 O:/winnt 并核对摘要（`s24-p5-deploy-1`）；提交 `86dae0c9e17814fe4cded04edbde23719c2f13af` 已推送，远端四文件和两个对话框库源码读取核对一致。实验旧布局事件完整运行时仍未验证，S24 保持开放。

P5 后将实验事件分发、覆盖和共享冲突返回规则迁入 win16-dialog，调用层仍持有异常清理与监听器生命周期；双宽增量构建、实验宏语法及独立库回调结果测试通过。VCL 微调控件的身份、裁剪和输入框子按钮查找迁入 win16-control，调用层保留窗口状态与生命周期；双宽增量构建通过，前一快照的实际包微调聚焦 14/14、后续子按钮迁移快照的 VCL 输入框聚焦 10/10，均无跳过。两次包含必选项，不可相加当作独立场景数。实验事件完整运行时尚未验证；上述为增量候选证据。scratch：`s24-dispatch-unit-x86`、`s24-dispatch-unit-x64`、`s24-spin-identity-candidate`、`s24-spin-identity-focused`、`s24-spin-edit-candidate`、`s24-spin-edit-focused`。

P6 干净包 `s24-p6-clean-1` 双宽构建与来源、资源和治理门禁通过；`s24-p6-focused-1` 实际包聚焦 16/16、无跳过，覆盖双宽经典/现代 VCL 微调及 COM 工厂；干净树双宽两个独立库各 2/2 通过。同批四文件已发布 assets 根目录和 O:/winnt 并核对摘要（`s24-p6-deploy-1`）；提交 `e540576141e2f1704f17f550d10448568b782997` 已推送，远端四文件及两个库代表源码读取核对一致。实验旧式事件完整运行时未覆盖，S24 保持开放。

P6 后将窗口框架状态结构、字体与菜单资源释放、嵌套菜单进入/退出以及框架属性启用/恢复迁入 win16-control；调用层持有同一状态并提供原系统属性函数。双宽增量构建和框架/独立库测试各 2/2 通过，包含字体句柄释放、内层菜单入口失败时保留外层及属性恢复测试。最新实际候选包的双宽菜单取消、隐藏窗口及 VB6 MDI 恢复聚焦 14/14、无跳过。中途测试源曾因局部变量重名无法编译，修正后通过；此前候选同类聚焦也 14/14 通过。上述为增量候选证据。scratch：`s24-frame-policy-unit-x86`、`s24-frame-policy-unit-x64`、`s24-frame-policy-candidate`、`s24-frame-policy-focused`、`s24-frame-state-unit-x86`、`s24-frame-state-unit-x64`、`s24-frame-menu-unit-x86`、`s24-frame-menu-unit-x64`、`s24-frame-menu-candidate`、`s24-frame-menu-focused`。

P7 干净包 `s24-p7-clean-1` 双宽构建、来源、资源和治理检查通过；实际包 `s24-p7-focused-1` 聚焦 14/14、无跳过，覆盖双宽菜单取消、隐藏窗口和 VB6 MDI 恢复；干净树双宽框架及控件库测试各 2/2 通过。同批四文件已发布 assets 根目录及 O:/winnt 并核对摘要（`s24-p7-deploy-1`）；提交 `7abcb8c48dbb985224ac30af2164a59bb345e2cc` 已推送，远端四文件和两个库代表源码读取核对一致。实验事件链和此前屏幕采样仍未覆盖，S24 保持开放。

P7 后将控件状态结构、字体/协议/制表位资源所有权和绘制或字体消息时的字体租约准备迁入 win16-control；窗口入口仍持有唯一状态并处理原有消息顺序。双宽增量构建及控件库、绘图故障、生命周期各 3/3 通过；最终增量候选双宽应用字体、VB 复选列表和 VCL 输入框聚焦回归 14/14、无跳过。此前仅迁移状态时，列表滚动、上下微调及 VB Data 候选聚焦 14/14 通过。上述为不同增量快照，不能相加当作完整覆盖。scratch：`s24-control-state-unit-x86`、`s24-control-state-unit-x64`、`s24-control-state-candidate`、`s24-control-state-focused`、`s24-control-font-unit-x86`、`s24-control-font-unit-x64`、`s24-control-font-candidate`、`s24-control-font-focused`。

P8 干净包 `s24-p8-clean-1` 双宽构建及来源、资源和治理门禁通过；实际包 `s24-p8-focused-1` 聚焦 18/18、无跳过，包含双宽 12 字体变体布局、列表滚动、VB 复选列表及 VCL 输入框；干净树双宽字体、绘图故障、生命周期与控件库各 4/4 通过。同批四文件已发布 assets 根目录与 O:/winnt 并核对摘要（`s24-p8-deploy-1`）；提交 `78db2b345d47400ddaa1e9068b8fbb70e54f6fbd` 已推送，远端四文件及控件状态源码读取核对一致。实验事件链及此前屏幕采样仍未覆盖，S24 保持开放。

P8 后将组合框重绘及标签页、表头、进度条消息中的 Win16 绘制协调迁入 win16-control，消息入口和共享状态仍按原次序调用；双宽增量构建及此前组合框绘制/生命周期各 3/3 通过，其中绘制用例分别约 38 秒和 30 秒，不在无相关修改时重复。当前候选的双宽组合框、标签页、v5/v6 表头及进度条聚焦 24/24、无跳过；上述为增量候选证据。scratch：`s24-combo-message-unit-x86`、`s24-combo-message-unit-x64`、`s24-combo-message-candidate`、`s24-combo-message-focused`、`s24-standard-message-candidate`、`s24-standard-message-focused`。

P9 干净包 `s24-p9-clean-1` 双宽构建、来源、资源及治理门禁通过；实际包 `s24-p9-focused-1` 聚焦 24/24、无跳过，覆盖双宽组合框、标签页、v5/v6 表头和进度条；干净树双宽控件库及生命周期各 2/2 通过。同名工作目录已有运行中的构建，重复启动被拒绝并曾写失败结果；确认原进程和关键源码快照后等待其完成，最终结果为通过，未重复构建。同批四文件已发布 assets 根目录及 O:/winnt 并核对摘要（`s24-p9-deploy-1`）；提交 `9716c1bd5b4ad833e9f1cc3c31996f82a377127c` 已推送，远端四文件和控件库源码读取核对一致。实验事件链及此前屏幕采样仍未覆盖，S24 保持开放。

P9 后将列表框、编辑框、标准按钮/静态控件及 VB Frame 的进程内样式消息协调迁入 win16-control，调用层保留消息入口和借用回调；原生数据、输入和应用图片仍由原控件处理。双宽增量构建及最新标准控件绘制、制表位、独立库与生命周期各 4/4 通过；最新候选的双宽列表/编辑滚动、VCL 文本及 VB 复选控件聚焦 20/20、无跳过。独立剪贴板测试因私有窗口站创建被拒绝（错误 5）双宽跳过，未使用共享剪贴板，不计通过；上述为增量候选证据。scratch：`s24-list-message-unit-x86`、`s24-list-message-unit-x64`、`s24-list-message-candidate`、`s24-list-message-focused`、`s24-edit-message-unit-x86`、`s24-edit-message-unit-x64`、`s24-list-edit-candidate`、`s24-list-edit-focused`、`s24-basic-message-unit-x86`、`s24-basic-message-unit-x64`、`s24-basic-message-candidate`、`s24-basic-message-focused`。

P10 干净包 `s24-p10-clean-1` 双宽构建、来源、资源及治理门禁通过；实际包 `s24-p10-focused-1` 聚焦 20/20、无跳过，覆盖双宽列表/编辑滚动、VCL 文本和 VB 复选控件；干净树双宽控件库、制表位及生命周期各 3/3 通过。同批四文件已发布 assets 根目录和 O:/winnt 并核对摘要（`s24-p10-deploy-1`）；提交 `a72e67810ae9d3d46e43bd65e78aba75c773a26f` 已推送，远端四文件和三个控件库源码读取核对一致。独立私有剪贴板测试仍为跳过；实验事件链及此前屏幕采样仍未覆盖，S24 保持开放。

P10 后将滚动条、框架滚动区及大小框消息中的样式协调，以及 VB 文件列表/公共对话框组合框的绘制协调迁入 win16-control；窗口入口和原始状态仍由调用层提供。双宽增量构建及控件库、几何、生命周期各 3/3 通过；当前候选的双宽普通 COM 文件框、框架滚动和 VCL 滚动聚焦 14/14 通过，前一滚动候选的列表、编辑、框架和 VCL 网格/滚动聚焦 18/18 通过。新增组合框资格独立测试双宽通过。VB 文件列表夹具在创建自有线程探针时错误 126，P10 基线同样失败，均未进入断言；自定义筛选 x86 探针当前候选和 P10 基线均在窗口出现前超时，原因未明，均不计通过。一次聚焦配置使用不存在的 `wide` 编号被拒绝，修正为 `w` 后才执行。上述为增量候选证据。scratch：`s24-scroll-message-unit-x86`、`s24-scroll-message-unit-x64`、`s24-scroll-message-candidate`、`s24-scroll-message-focused`、`s24-fileowner-unit-x86`、`s24-fileowner-unit-x64`、`s24-fileowner-baseline-x64`、`s24-fileowner-link-x86`、`s24-fileowner-link-x64`、`s24-fileowner-candidate`、`s24-fileowner-focused`、`s24-fileowner-focused-2`、`s24-fileowner-focused-3`、`s24-fileowner-baseline`。

P11 干净包 `s24-p11-clean-1` 双宽构建、来源、资源及治理检查通过；实际包 `s24-p11-focused-1` 聚焦 20/20、无跳过，覆盖双宽 COM 文件框、框架/列表/编辑滚动和 VCL 网格/滚动；干净树双宽控件库、几何和生命周期各 3/3 通过。同批四文件已发布 assets 根目录及 O:/winnt 并核对摘要（`s24-p11-deploy-1`）；提交 `eb4b4c0a73b1cfc0f76837ae0d79568162704761` 已推送，远端四文件及两个库源码读取核对一致。此前 VB 文件夹具错误 126、自定义筛选超时、私有剪贴板及屏幕采样未解决，不能从聚焦结果推断其通过；实验事件链仍未覆盖。S24 保持开放。

首次干净构建在 x86 因 DC 故障夹具仍引用旧 `frame_changed` 而失败；随后全目标增量构建又发现独立字体夹具缺少 `GetDpiForWindow` 声明。已更新夹具调用并将框架字体策略移至独立 `frame_font.c`，保持原字体夹具编译条件；双宽全目标增量构建及 ABI、几何、独立链接、DC 故障、列表制表位各 5/5 通过。首次干净构建不计通过，准备使用 `s24-stage-clean-2` 重建。scratch：`s24-all-fix-x86`、`s24-all-fix-x64`。

2026-11-01 发布状态变化：Windows Defender 将 S20 的 `run16s.exe` 检测为 `Trojan:Win32/Bearfoos.A!ml`（ThreatID 2147731250）；只读检测记录显示处理成功。用户报告已恢复文件后复核：`assets/run16s.exe` 已可读取且 SHA-256 与验收版本一致，`O:/winnt/run16s.exe` 仍缺失。用户随后要求删除被映射的 `O:/winnt/ntw16s64.dll`；原路径已移除，旧映像保留在 `build/t21-package/user-retired-ntw16s64.dll`，仍被进程映射。是否误报尚未确认，不关闭防护或添加排除项；后续由用户或维护者向 Microsoft 提交样本核查。S20 的用户验收和曾经的发布摘要保留为历史事实，不代表当前部署完整。

S20 经用户确认验收通过并收口。双位宽干净构建、10 项聚焦实包回归和 LibTray 双位宽消息测试通过；四文件发布至 assets 根目录与 O:/winnt 并核对摘要，产品提交 `8f25892` 已推送并读取远端复核。用户未逐项描述验收操作；自动探针、真实全局接入恢复及失败限制见 [S20 记录](../history/M0-T21-S20-tray-library.md)。旧 ntw16s64 映像仍被系统和当前会话进程映射，保留在 `build/t21-package/s20-release-backup/held-ntw16s64.dll`。

M0 固定；T21 保持开放，S23 与 S22 已收口。S21 的设计和静态审计已交付，按用户要求转入新 S；S21 未做独立产品包验证，相关工作转交 S22，不能记为完整产品收口。S20、S19、S18、S17、S16 已收口。P97 同批干净包已通过 30 项聚焦回归并发布到 assets 根目录及 O:/winnt，产品提交 `33b4638` 已推送；实际读取远端四文件，与测试包及两处发布文件 SHA-256 均一致。该包随后完成双位宽管理员跨权限实测，记录已推送并读取远端复核（`a299eec`）。S13 与 T21 覆盖边界见 [S13 收口审计](../history/M0-T21-S13-vb-global-investigation.md)。

S19 已按用户验收通过而收口；T21 保持开放。最终 39 号徽标包 `82eee1b` 已发布至 assets 根目录与 O:/winnt，且与远端逐项核对一致。用户未逐项列出验收操作，真实 UAC 自动回归、所有窗口恢复和进程内 trampoline 的物理移除不据此宣称通过；详细过程见 [S19 记录](../history/M0-T21-S19-tray.md)。

S18 经用户确认时机，在当前桌面运行已发布 `O:/winnt/run16s.exe --global`：真实 UAC 后驱动记录 `ready=1 elevated=1 sent=1 parent_wait=0 parent_exit=0 session_gone=1`。系统 x64 钩子报告 126 并使用线程后备；启动和停止各向 362 个窗口发通知，各有 7 个超时或失败，具体窗口未记录，不能宣称这些窗口成功接入或恢复。首轮驱动返回 9 且未捕获错误，未计通过；修正驱动标准句柄后第二轮取得上述结果。此轮未设置自有双宽目标，不单凭会话就绪宣称目标样式或传播通过。

随后同一发布包在真实桌面运行自有双宽目标对照：启动前已有 4 个与启动后新建 4 个窗口均附着，停止后 8 个均恢复，隐藏窗口恢复与显示、停止后的 4 个新窗口及 4 个子进程、基本输入均通过；驱动退出 0，记录的窗口 DPI 均为 144。该对照未记录全局活动期间新子进程的传播状态，也未单独记录控制器提权令牌，不能扩张结论。

增加活动期间双宽子孙传播断言后的重试未进入该断言：驱动记录 `manager_not_active error=2`、退出 1，用户反馈没有看到 UAC 提示；自有测试进程已退出。驱动未记录启动器实际退出码或控制台文本，不能据此判定原因；该次不能算传播通过或产品回归。

补充启动器诊断后的下一次真实桌面对照退出 0：32/64 位自有目标的窗口附着、子进程及孙进程 DLL 加载均通过；启动前 4 个和启动后 4 个窗口均附着、停止后 8 个恢复，4 个隐藏窗口显示后仍恢复，停止后新窗口及子进程和基本输入均通过，DPI 144。该次驱动没有记录控制器令牌或其他应用的窗口逐项恢复；之前真实 UAC 启停对照独立确认过提权，不能将这两次观察声称为单次完整令牌/传播证据。

S18 同快照双位宽干净构建及 22 项实际包聚焦回归通过、无跳过；本 S 不改产品源码或发布文件，重建包与部署包 SHA 不同，不能混称。部署文件与远端逐项一致。具体证据和已知边界见 [S18 记录](../history/M0-T21-S18-real-global.md)。

S17 私有桌面独立 UAC 对照未运行全局样式；父进程确认绑定自有桌面，在 `ShellExecuteExW` 处等待，35 秒后驱动返回 99/258 并清理自有进程，未记录子进程桌面。用户反馈未看到提示。该次不能判定系统不支持或产品失败；在当前验证条件下仍不能以原始包安全完成隔离的真实 UAC 组合。此前分项证据仍有效但不能冒充组合通过。

S17 同快照双宽干净构建及 22 项实际包聚焦回归通过、无跳过；本 S 未改产品源码，重建四文件的 SHA 与已发布包不同，不混称或发布。完整组合、完整矩阵与物理输入未覆盖；详见 [S17 证据](../history/M0-T21-S17-uac-isolation.md)。

S16 已完成 `s16-clean2` 双宽干净构建及 22 项实际包聚焦回归，均通过且无跳过。首次 `s16-clean` 后台进程被执行环境回收，没有生成结果，不计入通过。重建包四个 SHA 均与已发布文件不同：两个 EXE 的差异仅为 PE 时间戳和校验和；两个 DLL 的各 PE 节除导出节外逐字节一致，导出节时间戳不同，节之后的数据也存在差异，尚未完成该数据的比较；不能据此断言整文件等价，未覆盖已发布包。私有桌面 UAC 对照曾在提权调用处等待，未得到子进程桌面证据；正式包自动 UAC 与全局接管的组合仍待验证。

进一步检查发现两个 DLL 的导出节差异仅为时间戳；节后的数据是 COFF 符号和字符串表，字符串差异包括各自构建目录的符号名称。该检查解释了至少一部分尾部差异，未证明符号表所有字节等价，也未将新包当作已发布包。

将 COFF 符号名称中的两个构建目录规范化后，双宽 DLL 的符号记录、其余字段及辅助记录作为多重集合比较一致；这不是文件字节一致性证明，测试包和已发布包仍分别按 SHA 标识。

S16 已准备并编译一个自有原生对照：先占用当前桌面的全局会话名，再启动 `O:/winnt/run16s.exe --global`；预期正式程序经真实 UAC 后以已有会话错误 183 安全退出，避免接管用户窗口。它只能验证真实提权入口和冲突失败，不能替代完整接管组合。当时会话再次显示断开，对照尚未运行。

会话恢复后运行两次该对照，正式程序均返回 10，但探针未捕获控制台错误文本；尚不能确认返回原因或真实 UAC 路径，均不计为通过。尝试修正探针的标准输出句柄后结果仍相同，需继续排查观测方式。两次测试均结束，没有将返回码单独视为成功。

再次修正句柄继承后发起第三次对照；等待期间只读查询显示 Windows 会话已断开，90 秒后探针返回等待超时 258，未获得程序退出码，按测试超时处理并清理了自有程序。该次未验证通过，不证明冲突处理失败。

会话恢复后用户同意系统 UAC，第四次对照中原始启动器以 10 退出，但探针仍未读到错误文本，无法验证是否由预期冲突导致，也未证明提权子进程控制台输出正确；该项需继续调查，不计通过。

输出探针正例：相同自有控制台的直接 `WriteConsoleW` 及无提权子进程通过继承的标准错误写入，均可读取唯一标记并返回 0。这证明基础读取及普通子进程输出可用；不证明跨 UAC 控制台输出一定是产品缺陷，仍需单独对照。

补充无提权对照：子进程执行 `FreeConsole` 和 `AttachConsole` 后通过 `fwprintf(stderr,...)` 输出，父探针读到唯一标记且子进程返回 0；此证据不覆盖 UAC 创建子进程的标准句柄状态。

独立真实 UAC 对照复用 `runas`、控制台重新附着和 C 运行库写入路径，提权子进程成功附着且报告标准输出连接控制台，但 `fwprintf(stderr,...)` 返回 0、`ferror(stderr)` 非零（32），`fflush` 返回 0。此结果确认测试原型中 CRT 标准错误写入失败，是正式包未捕获诊断的有证据候选原因；当时尚未独立观测正式进程的 CRT 状态，也未验证修复。

同一独立真实 UAC 对照在重新附着后以 `freopen("CONOUT$", "w", stderr)` 重新绑定时，返回非空、`fwprintf` 返回 22、`ferror` 为 0；子进程正常退出。随后对提权子进程的该路径采用最小修复，并为实际输出增加对照；不影响普通单目标路径或重定向入口。

产品已加入上述绑定并完成双宽干净构建和 22 项实际包聚焦回归（无跳过）。候选包真实 UAC 冲突对照等待 90 秒返回 258、未获得退出码；该次不能作为通过或修复失败证据。当时候选尚未发布。

随后重试的同一候选包对照再次等待 90 秒返回 258、未获得退出码；未将用户聊天中的确认当作系统 UAC 已同意。当时仍缺候选包真实 UAC 输出证据，尚未发布。

用户方便后再次执行候选包对照：真实 UAC 同意，候选程序正常以 10 返回，控制台实际捕获 `run16s: global startup or shutdown failed (183)`，对照退出 0；该测试通过故意制造同桌面会话冲突避免接管，不能当作完整全局接管验证。同批四文件现已发布到 assets 根和 O:/winnt 并核对摘要，交付提交 `73a956b` 已推送，远端四文件与测试包及两处部署摘要一致。独立对照的基线 UAC `fwprintf` 流错误、绑定后写入成功；普通进程重新附着对照也通过。

发布后使用相同摘要包完成管理员私有桌面回归：双位宽、普通权限目标、控制器与宿主故障、孤儿清理通过，x86/x64 图形目标附着及子进程传播均通过。该回归直接以管理员令牌启动控制器，不声称验证普通控制台自动 UAC 与完整全局会话的组合。

为缩小差距，已准备仅在自有私有桌面直接调用发布程序 `--global-elevated` 的对照，拟检查控制台接入、会话就绪、Ctrl+C 和清理；本次启动的系统 UAC 返回取消，测试未开始，不计成功或产品失败。即使通过，该对照也不能替代普通启动器真正跨 UAC 发起的完整组合。

同一探针在普通令牌私有桌面运行的负例记录 `ready=0 sent=0 wait=0 exit=9 gone=1`，驱动返回 6，说明未提权入口未被误计为成功；这不替代管理员正例。

管理员正例直接使用发布程序提权后入口，记录 `ready=1 sent=1 wait=0 exit=0 gone=1`，即全局会话就绪、Ctrl+C 后控制器正常退出且会话映射消失；但私有桌面驱动报告 `PRIVATE_DESKTOP_LEAK` 并返回 8，暂不能作为资源清理完整通过。该对照直接以管理员令牌创建正式程序，仍不能证明普通启动器真实跨 UAC 的完整组合。

后续只读复查中，该私有桌面仍可打开；835 个可查询线程中没有线程绑定该桌面，也没有找到本次自有测试进程。仍可能存在未查询到的引用，不能据此断言持有者或控制器完全无泄漏；未终止其他进程。

进一步只读检查同会话中 174 个候选桌面句柄，158 个可临时复制并检查，未发现指向该桌面的匹配句柄；其余 16 个不可检查，因此仍不能确定剩余引用的持有者或来源，未关闭其他进程的句柄。

超时后复核：候选四文件 SHA 均仍与 22 项测试记录一致，当前 `main.c` 与干净构建快照一致，自有冲突探针已退出。后续恢复时可对该候选继续验证，无需仅因等待超时重新构建。

S15 准入：验证普通控制台启动全局模式时请求管理员权限，能否同时保留原控制台输入输出及 Ctrl+C 停止体验；先用自有隔离原型和权限负例验证，只有方案简单且契约可保持才考虑产品实现。普通单程序模式不改变；不自动修改用户权限或系统设置。

S15 增量证据：自有独立控制台中，不提权的 `ShellExecuteEx` 对照显示父子共用控制台，Ctrl+C 被子进程捕获并正常退出。初版对照误共享了驱动器控制台，驱动器被 Ctrl+C 中断；已改为先断开并创建自有控制台，重测通过。用户批准后，真实跨 UAC 原型发现 `SEE_MASK_NO_CONSOLE` 未使提权子进程共享控制台、Ctrl+C 未到达；改为提权子进程 `AttachConsole` 后，同控制台及 Ctrl+C 对照通过。不接管窗口的候选启动分支对照在会话恢复后真实 UAC 同意、共享控制台和 Ctrl+C 通过；用户选择“否”的对照以 9 退出且未启动全局。候选实现仅在交互控制台发起 UAC，重定向保持原路径；18 项实际包聚焦回归及管理员双宽隔离回归通过。当时真实产品的自动提权与私有桌面全局接管组合尚未验证。

追加组合证据：测试专用构建用新线程切换至自有私有桌面，并显式将该桌面传给 32 位宿主；真实 UAC 同意后全局会话 `ready=1`、控制器令牌已提权，Ctrl+C 后父进程返回 0、会话消失。该测试构建与正式包存在测试隔离差异，不能冒充正式包逐字节实测。随后候选增加父进程句柄监视，避免低权限启动器意外退出后遗留仍运行的提权控制器；自有私有桌面直接调用生产全局入口的父进程退出对照通过，当时最新源码尚待重新干净构建及实际包测试。

`s15-final-clean` 构建快照早于上述父进程回归注册，已核对并停止其自有过期构建进程；不计为交付。最终包须包含最新测试及源码后重新构建。

后续 `s15-final2-clean` 已按最新源码完成双宽干净构建，来源和资源检查通过；其实际包 22 项聚焦测试通过、无跳过，包含父进程退出清理测试。正式包的管理员双位宽隔离回归等待系统 UAC 超过四分钟仍未开始，已结束本次自有测试启动器，未产生结果；当时尚未发布，不能将测试专用组合构建视为正式包的逐字节端到端验证。详见 [S15 证据](../history/M0-T21-S15-global-elevation.md)。

会话恢复后最终包管理员隔离回归通过，双宽目标窗口附着和子进程传播均通过；同批四文件已发布到 assets 根目录和 O:/winnt 并核对摘要。交付提交 `3d26d80` 已推送，远端四文件与测试包及两处发布文件摘要一致；真实自动 UAC 与全局接管的组合仍只在测试专用隔离构建上验证。x64 系统钩子本机返回 126，使用目标线程后备。

先前候选启动分支探针在会话断开时等待 UAC 超时并完成自有进程清理；会话恢复后的对照已通过，不将先前超时作为成功或失败。候选 `build/t21-package/s15-candidate-clean/candidate` 双宽干净构建及来源/原资源门禁通过；`s15-candidate-test/result.json` 的 18 项聚焦回归和 `build/t21-s13/admin-etw/s15-candidate-elevated.log` 的管理员双宽隔离回归通过。该早期候选未发布。

微软 [SHELLEXECUTEINFO 文档](https://learn.microsoft.com/en-us/windows/win32/api/shellapi/ns-shellapi-shellexecuteinfow) 描述 `SEE_MASK_NO_CONSOLE` 旨在继承父进程控制台，但该通用说明及不提权对照均不能代替本机跨 UAC 的实测。

S14 已收口：隔离复现旧 DLL 留存对全局窗口附着及后代传播的影响，明确可行处置和用户文档；不强制卸载或关闭已有进程，不将兼容的所有旧版一律拒绝。自有双宽 P33 映射替换为 P97 同路径后，全局会话已就绪且窗口已附着，后代均未加载 DLL、返回 41；同实验使用 P97 映射时双宽后代均加载并返回 0。四个现有进程的映射 PE 元数据与 P33 对应文件头一致但未逐字节证明，未操作这些进程。当前推荐安全重启受影响应用以加载新版；不声称旧映射是本机 126 的原因。[S14 证据](../history/M0-T21-S14-stale-image.md)：双宽干净构建和 18 项实际包聚焦回归通过、无跳过；无产品源码变更，P97 发布文件未变并与远端再次核对摘要，S14 P1 `2491d25` 已推送复核。

P97 管理员实包验证通过：双位宽普通权限目标实际初始化、子进程传播、窗口接入，以及控制器/宿主崩溃、孤儿会话恢复与清理均通过；独立阶段文件不可用，依据标准输出和退出码判断。管理员 ETW 事件为异步处理，桌面尚未绑定的目标最多等待一秒，短命或无法及时绑定桌面的后台进程不保证接入。P97 普通权限运行不自动提权，S15 已增加交互控制台提权；x64 系统钩子在本机会话仍返回 126，使用异步目标线程后备路径。P95 单独覆盖了停止期间桌面绑定的竞态，但不证明全部停止竞态均已消除，详细证据见 S13 历史记录。

P94 在自有隔离桌面复测实际发布路径：O:/winnt 全局模式与 assets 普通启动混用，双宽已有和新窗口、退出恢复、后续窗口及子进程状态、输入语义均通过；系统报告 x64 hook 126 并使用后备路径。该次为普通权限测试，不替代 P92 的管理员实包结果。

P95 停止竞态专项：自有双宽挂起目标的首次桌面检查均返回 21，管理器跟踪且尚在重试；仅将会话置为 STOPPING 后恢复目标，目标均未加载 DLL。临时移除停止检查的双宽负例均加载 DLL、测试返回 15；正式检查已恢复。同批四文件干净构建，28 项聚焦回归通过且无跳过，已核对并发布两处。测试通过测试专用入口模拟进程发现，不验证 ETW 投递时序；管理员跨权限测试本轮未重新运行，P92 是此前实包证据。

P97 空桌面发现器缺陷：自有空私有桌面上原生 `EnumWindows` 返回 FALSE、错误码 0、窗口数 0；旧发现器双宽均错误返回 31。已发布 P95 的实际 `run16s --global` 在该空桌面正常启动并退出，因此不能声称此前真实入口必现失败。新实现仅将枚举未设错误且回调未中止视为空结果；双宽空桌面启动/刷新/停止与已有、新窗口发现通过，实际 P97 包的空桌面启动停止也通过。30 项实际包聚焦回归通过、无跳过，四文件已发布并核对两处摘要。

P101 用当前部署的 P97 包再次验证用户指定的自有扫雷：先启动 `D:/programs/misc/winmine.exe`，再启动 `O:/winnt/run16s.exe --global`，打开 Help → About 后截图可见经典标题；停止全局模式后扫雷及主窗口仍存活，探针正常清理。截图客户区裁切，不能证明所有控件外观；该次为普通权限私有桌面，不替代管理员跨权限回归。

P102 将自有截图探针设为 DPI 感知后复测相同场景，截图由裁切的 470×414 变为完整的 705×621，能看到经典标题、正文和 OK 按钮；全局停止后扫雷及主窗口仍存活。Windows 标识和链接仍是应用自身内容；截图不代替真实物理输入或任意控件验收，无产品二进制改动。

P82 修复同版跨目录双实例冲突：优先复用目标已有同名模块，检查两处文件逐字节一致及目标映像头和工作入口；系统钩子回调只由首个同名模块处理。实际 assets 普通启动与 O:/winnt 全局启动的双宽、两种先后顺序及退出恢复已通过。映像头与入口检查不是完整内存逐字节证明；不同或不可验证版本保留原行为，不保证全局接管。

P84 用已发布 P82 复测真实扫雷：自有实例先启动，全局模式开启后打开 Help → About，截图可见经典标题；退出全局模式后目标及主窗口存活。截图客户区不完整，不代表完整视觉或物理输入验收。

P85 调查同路径升级：自有 P74 进程仍映射旧 DLL，磁盘同路径替换为 P82 后，P82 全局控制器实际完成了双宽接入及退出。仅按 PE 时间戳或校验和不同拒绝旧映像会丢失已证实可用的覆盖，因此该候选及其过宽负例已撤回，产品未更改。不同历史版本的内部全局协议兼容性仍需明确；不能仅凭公共 ABI 版本号断言。

P86 以双私有桌面实测 P82：全局就绪后，未加载本项目 DLL 的自有启动者创建用户指定的扫雷，随后打开 About，截图可见经典标题；自有进程和桌面正常退出。截图客户区不完整，不代表完整视觉或物理输入验收。

P87 只读检查后台进程事件来源：本机 Kernel-Process Analytic 通道未启用；普通权限读取 Security 的 4688 事件被拒绝，查询进程创建审计策略返回权限不足。未启用日志、修改权限或增加轮询；这不证明所有可能事件源均不可用，也未修复未接入后台链。

P88 修正用户文档和当前架构：全局会话接入过的普通启动目标在停止后也停用样式和传播；此前文档误写为保留。并列明无钩子后台链、异步首帧及既有宿主排除边界；产品二进制未变。

P90 用户允许管理员权限。一次性管理员自有 ETW 探针成功观察到测试后台进程并关闭自有会话，但事件约 2016 毫秒后到达，而目标仅存活约 700 毫秒；不能认为已来得及注入。100 毫秒刷新会话的第二个一次性探针发起后系统 UAC 返回取消，探针未运行；结果不得预先推定。

P91 管理员刷新探针重试成功：自有子进程启动事件约 328 毫秒到达，目标仍存活，取得注入所需进程访问权限，37 次刷新无错误，自有 ETW 会话正常停止。单次结果不保证短命进程覆盖，也未执行注入或验证传播；产品未更改。

P92 实施中：在现有双位宽管理器中试接管理员 ETW 新进程事件，对每个事件按现有同用户、同桌面和位宽策略检查目标线程，再复用已有接入和退出路径；普通权限若无法创建会话则保留既有窗口发现。普通权限下自有双宽后台新目标及其子进程未加载 DLL，负例复现；事件解析和既有管理器双宽各 4 项通过。管理员自有私有桌面正向探针重试后通过：未接入启动者创建双宽后台目标，目标与其子进程均加载 DLL，正常停止后未查询到本项目 ETW 会话；仅凭模块加载不能完全证明每项初始化或视觉行为。微软文档确认控制进程异常退出不会自动停止 ETW 会话；候选增加对端清理，以及再次启动时按 PID 和创建时间回收本项目孤儿会话，均未完成提权实测，不能发布。验证范围仍包括后台进程的真实初始化、子进程传播、停止后行为、双位宽和失败清理；事件异步且不能保证短命进程。

自有异常退出探针 `build/t21-s13/admin-etw/p92-lifecycle.ps1` 已发起管理员请求，启动进程等待系统 UAC；只读 `query user` 显示该提示所在的会话 3 已断开，因此尚无结果，也未重复发起。新孤儿清理测试普通权限双宽均返回 skip 77，不能计为通过。

较低完整性模拟令牌对普通默认安全描述符事件的 EVENT_MODIFY_STATE 打开成功，但真实低完整性自有窗口进程对默认就绪事件返回错误 5；临时标记定位到远程初始化打开就绪事件这一阶段。候选只为该事件授予同一用户 SID 和 SYSTEM 的访问并使用低完整性标签，修正后双宽低完整性目标成功附着并向子孙传播；新测试已纳入包回归，已发布版 x64 同测超时，负例检出。测试桌面及目标均自有；该对照不代替管理员控制器与普通权限目标的实际验证，后者探针已在 scratch 准备，尚未运行。

`core/global-attach` 双宽返回 1460，随后查询显示窗口已附着。以 HEAD 版 `global_start.c` 和已发布 ntw16s32.dll 组装的自有 x86 对照仍返回相同超时；延长临时对照等待后，首次附着约 140 毫秒完成，而原等待仅 100 毫秒。候选将每次控制消息有界等待提高到 500 毫秒，双宽各 7 项聚焦回归通过、管理员孤儿测试各跳过 1 项；新增双宽自有阻塞窗口测试验证超时有界且解除阻塞后窗口可恢复，均通过。原失败保留。

P92 干净包 `build/t21-package/p92-clean3/candidate` 双宽编译、来源和原资源门禁通过；实际包 20 项聚焦回归通过，无跳过。首次包测试因调度器重复加入 `--private` 导致新低完整性用例启动失败；修正调度器后旧包复测通过，之后的快照也重新构建并通过同一回归。权限提示和孤儿会话名称规范化均包含在该包内；双宽解析测试通过、提权集成测试跳过。管理员跨完整性和异常清理仍待验证，不予发布。

其后将链接令牌跨完整性探针纳入现有测试夹具，并添加 `tools/Test-GlobalElevated.ps1`，集中验证双宽后台传播、普通权限目标、控制器/宿主崩溃、孤儿会话及清理。普通令牌运行该入口明确 skip 77；直接运行跨完整性夹具的普通令牌负例返回 1314，不可计作通过。最新测试夹具对 clean3 包的双宽低完整性接入及子孙传播复测通过；assets 与 O:/winnt 四文件摘要仍一致。上述测试源码和工具后续修改不在 clean3 快照中，已由下述 clean4 承接。

包含新增提权测试入口的候选 `build/t21-package/p92-clean4/candidate` 已双宽干净构建、通过来源/原资源门禁及 20 项实际包聚焦回归，无跳过。指定 `D:/programs/misc/winmine.exe` 的自有私有桌面实例先启动，再启用候选全局模式并打开 About；截图 `build/t21-s13/winmine-about-p92-clean4.bmp` 显示经典标题，客户区裁切，停止后目标及主窗口存活。管理员跨完整性综合回归尚未通过，不予发布。

会话恢复并通过 UAC 后，自有管理员探针的双宽控制器崩溃、宿主崩溃和孤儿 ETW 会话恢复均通过；此前等待的进程已结束。跨完整性夹具通过链接令牌启动失败 1346，改为读取本次自有普通权限启动器令牌后可创建目标，但 x86 目标 8 秒未退出（99），不能计通过或推断根因。已增加原生对照和超时诊断；随后的有界管理员回归 UAC 返回取消，测试未运行。测试夹具和工具变更不在 clean4 快照内，已由下述 clean5 承接。

普通权限控制器与低完整性 x86 后台目标的自有对照中，目标正常进入 main、启动子进程并在约 2.5 秒内退出，按预期因无管理员进程事件而未传播。现已给双宽后台探针增加进入 main 和创建子进程前的即时标记，下一次管理员跨权限复测可区分入口前卡住与子进程阶段卡住；该对照不能替代高到中完整性测试。

包含上述最新诊断夹具及测试入口的 `build/t21-package/p92-clean5/candidate` 已双宽干净构建，并通过 20 项实际包聚焦回归，无跳过。管理员诊断仍需新的系统 UAC；该包尚未发布。

为排除跨权限进程标准输出句柄未继承导致的信息缺失，后台测试目标新增可选自有 scratch 文件阶段记录；普通权限 x86 对照已记录 main、创建子进程前、子进程入口和返回。该诊断及管理员测试入口变更晚于 clean5，仍须在管理员复测后纳入最终快照。

管理员测试曾观察到 x86 目标 DLL 已加载但立即创建的子进程未传播，不能把模块存在当成初始化完成。新探针等待目标所属线程的实际就绪事件后再创建子进程；自有后台目标先于全局启动的 x86/x64 对照均观察到 ready 并成功传播，未接入目标的负例返回 ready=0。此证据不保证异步接入之前的子进程。

最近一次管理员跨完整性诊断的原生对照也未进入目标入口（超时、模块未加载），因此尚不能把超时归因于本项目注入。测试私有桌面原先只授权对象所有者；现改为明确授权当前用户 SID 并维持低完整性标签，普通权限下双宽低完整性窗口和子孙传播复测通过。管理员到普通权限仍须复测；该夹具变更晚于 clean5。

会话恢复后的管理员诊断确认：显式授权当前用户 SID 后普通目标能读取全局映射；进程事件到达时，新目标线程尚无桌面句柄且 `GetLastError` 为 0，原策略直接返回 5，未排队接入。候选保持用户、会话、位宽和真实桌面检查，对仅该状态进行每目标最多一秒的有界重试，不做周期性全进程扫描。双宽新建挂起进程对照均观察到首次 `ERROR_NOT_READY` 21、错误位宽 193、恢复并绑定桌面后成功；双宽身份、附着和管理器各 5 项回归通过，首次并发回归的延迟测试计时失败也已保留。管理员实际包正向仍待复测。

自有管理员增量候选复测通过：x86/x64 普通权限后台目标均观察到实际 ready 事件、加载对应 DLL，且各自创建的子进程已传播；双宽原生对照按预期无 DLL 并正常退出。独立阶段文件不可用，标准输出及退出码完整；不能据此断言任意目标或短命进程覆盖。该结果尚非干净实际包验收。

审查有界重试期间的停止竞态时，发现控制器进入 STOPPING 后仍可能再次尝试加载 DLL；候选在每次尝试前重查会话状态。该保护后的双宽低完整性窗口及子孙传播回归通过，停止与重试同时发生的专门端到端测试尚未覆盖。`p92-clean6` 快照早于该修正，已核对并停止其自有过时构建进程，结果不计通过；下述 `p92-clean7` 已包含修正。

`p92-clean7` 已以最新产品源码双宽干净构建，26 项实际包聚焦回归通过、无跳过。最终管理员实际包测试随后通过，四文件已发布并核对摘要；本段之前的待验证表述记录的是各阶段当时状态。

P74 修复普通启动进程在全局运行期间创建的隐藏窗口：即使尚无全局窗口标记，退出时也尝试在窗口所属线程移除已存在的项目附着。旧包负例保留样式；新包的四个 x86/x64 隐藏窗口及常规窗口退出恢复通过。无法接入或无法及时响应的窗口仍不保证恢复。

P76 补充验证：四个普通进程的隐藏窗口在全局退出后实际显示，仍无项目附着；双 DPI 实际包聚焦测试通过，无二进制变化。

P77 窗口快速更替探针：自有 x86/x64 新进程的首个窗口显示 20 毫秒后销毁，第二个窗口均观察到附着；各一次不足以排除接入重试竞态。x86 私有桌面清理检查返回仍存在，随后枚举不到顶层窗口和相关自有进程，桌面仍可打开，原因未明；x64 清理通过。探针只在任务 scratch，未改产品。

P78 对 P77 桌面引用进一步只读定位：当前会话 790 个可查询线程中未发现绑定线程；可复制检查的 139 个桌面句柄中，系统中文输入法 `ChsIME.exe` 持有该桌面句柄，模块查询未发现本项目 DLL。该系统进程未被操作或结束；是否由探针触发该输入法进程及其他不可访问句柄仍未证实。

P79 加载锁窗口更替隔离探针未复现漏接：第二个 x64 窗口附着，但首次 `ShowWindow` 等待约 1547 毫秒、直到自有测试 DLL 的 1500 毫秒加载锁结束后才返回；该实验未真正实现锁持有期间更替窗口，不能排除重试竞态。无产品改动。

P80 用实际 `O:/winnt` P74 包复测用户指定扫雷：自有实例先启动，再启用全局模式并打开 Help → About；截图可见经典标题，退出全局后目标进程及主窗口仍存在。私有桌面测试正常退出；未证明全部控件和恢复像素，无产品改动。

P81 确认跨发布目录的统一退出缺口：同批 `assets/run16s.exe` 普通启动目标、`O:/winnt/run16s.exe --global` 启动全局模式时，目标曾同时加载两处同 SHA 的 DLL，退出后普通实例仍有样式。该同版缺口已由上文 P82 修复；不同或不可验证版本及内存身份检查的边界仍按 P82 所述保留。

P70 候选已干净构建、通过 16 项实际包聚焦回归并发布到 assets 和 O:/winnt，远端实际提交及四文件摘要已复核。普通启动进程在被全局会话接入后跟随该会话：正常停止时移除已接入窗口样式，后续窗口和子进程不再由该进程自动套样式；全局会话未接触的普通进程仍保持普通行为。安装的跳板和 DLL 可能留在目标进程中，停止是策略失效，不声称物理卸载。

P72 为测试补充：普通启动的 x86/x64 自有进程各经历两轮全局启动及停止，每轮核对原窗口、新窗口和子进程传播；已发布的 P70 实际路径和同批包测试均通过。无产品二进制变更；T21/S13 仍开放。

P73 以双私有桌面验证未接入的后台进程创建可见目标：x86/x64 目标均被接入，并向子、孙进程传播；纯无窗口目标仍不保证接入。P70 实际部署路径和同批包回归通过，产品二进制未变。

P63 同源码双宽干净构建及 44 项实际包聚焦测试通过：启动前已有的同桌面进程接入、四种父子位宽传播、窗口与控件、菜单及退出；启动等待上限为每位宽 2 秒，超时可能只完成部分接入。自有阻塞窗口验证了有界等待；长期持续新事件及超时告警仍有未覆盖面。

用户指定的自有扫雷先启动后进入全局模式，Help → About 显示经典标题，退出全局模式后扫雷仍存活。探针截图保存在任务 scratch 的 `build/t21-s13/winmine-about-p63-recheck.bmp`。已接入父进程就绪后创建无窗口子进程的四种位宽组合通过；未接入的其他桌面父进程创建有窗口目标，双宽窗口及首次菜单对照通过，而纯无窗口目标及其子进程未加载 DLL。

本机会话实际部署路径的 64 位系统钩子仍返回 126，产品使用目标线程后备接入；该后备异步执行，不能保证所有首帧。WMI 进程事件和独立 ETW 会话在当前普通用户令牌下均拒绝访问，因此不能据此保证发现未接入父进程创建的纯无窗口目标。Explorer 等共享宿主仍按当前策略排除，UWP/XAML 不在本次范围。

已发现并修复快照误将已退出线程视作活线程导致的 1237；既有跨快照新线程竞态、启动及退出时目标竞态仍有未覆盖面。历史 x64 GDI 总数波动及部分早期首次菜单失败尚无完整根因，不用重试通过抹除；全局停止后部分 DLL/钩子可留在进程内，用户已接受可能残影。详细证据和边界见 [S13 记录](../history/M0-T21-S13-vb-global-investigation.md)。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 23
- Last allocated Td S: 19
- Last allocated T18 S: 64
- Last allocated T19 S: 3
- Last allocated T20 S: 19
- Last allocated T21 S: 44
- Last verified remote delivery: M0 T23 S1 P4，`37d534be`；已实际下载四产品及六个源码/测试/交付文件逐字节复核。P5仅记录复核，产品不变。
- Open task: T23；S1已验收收口，S2待准入；T22已收口。Explorer现有产品排除策略尚未改变。
- 下列较早 P 编号的“尚需”“未发布”等描述是当时的历史状态，当前状态以上方 Current Work 和阶段证据为准。
- S13 P59 阶段包 `72ff492` 已双位置发布、推送并复核远端；后续仍需处理本节列出的时序、资源和线程竞态，S13/T21 均未收口。
- S13 P37 阶段包已发布至 assets 和 O:/winnt；真实部署路径遇到 126 时，双宽已有/新建自有窗口接入及退出恢复通过。目标退出后的管理器记录可在后续窗口事件中分批回收。备用路径仍为异步接入，不能保证所有应用的首帧或全部窗口均被接管，S13/T21 保持开放。
- 发布时旧 DLL 被现有程序持有，已将旧映像移至 `build/t21-package/manager-retire-publish/held-ntw16s32.dll` 和 `held-ntw16s64.dll` 后替换；已运行程序可能继续使用旧映像直至退出，保留这两份备份直到不再占用。
- P31 干净包 29 项串行聚焦通过；此前两次并发批次分别有 demo 新实例模块观测失败和 GDI 增量超限，单项复测通过但根因未确定；详情见[阶段证据](../history/M0-T21-S13-vb-global-investigation.md)。
- 后续并发探针又分别观察到一次 x86 管理器确认失败和一次首次菜单未进入循环；单项及带诊断的八个菜单样本通过，根因均尚未证实，不将重试预算试验当作修复。
- P33 受控繁忙目标原版可双宽复现“已附着但管理器未确认”，新版通过；此前 x86 并发失败是否同因仍未确定。四路包回归再次出现 x64 GDI 22→25 超阈，串行 29 项通过，资源增量原因未明。
- 两组自有双宽窗口各八次启停中句柄数不变，x64 GDI 在 22 增至 23–25 后未见逐轮持续增长；一组末轮仍有 24→25，不能据此排除泄漏或解释既有并发超阈。
- 自有 x64 诊断构建对七个 renderer 源文件中的 GDI 操作记账：64 字体、192 位图、32 画刷、240 DC 均匹配释放，92 区域在成功 GetDCEx 后转交系统；同次总数仍由 22 波动至 26，其他来源、系统释放时间及未跟踪代码不能据此排除。
- 双宽自有目标每轮 `GdiFlush` 前后计数一致；x64 临时手动裁剪并显式删除区域的对照仍从 22 增至 24。这些有限对照不能确定总数差异来源，正式绘制未改。
- P59 完整统一样式钩子增量候选在双宽两轮启停后，新建窗口不再自动换肤，已有及动态控件统一解除；增量包聚焦 25 项通过。尚需同快照干净构建、实际包验证、风险核查和发布。[证据](../history/M0-T21-S13-vb-global-investigation.md)。
- P58 全局候选已接入晚传播：双宽已有/新建目标、同进程两轮启停与模拟系统回调通过；增量候选包聚焦 25 项通过，旧候选误用导致一次失败保留记录。尚未覆盖完整普通样式钩子，线程冻结/创建竞态及系统钩子 126 等风险未解决，未发布。[证据](../history/M0-T21-S13-vb-global-investigation.md)。
- P57 实验载荷绑定全局会话：八种 A/W 双宽组合在停止后由父及已注入子进程创建新后代均不再注入；候选包聚焦 16 项通过。实际管理器尚未调用，窗口恢复和并发停止仍未验证，未发布。[证据](../history/M0-T21-S13-vb-global-investigation.md)。
- P56 将晚传播初始化整理为内部接口并复用普通 CreateProcessA/W 注册；双宽八组合及重复调用通过，普通候选双宽传播通过。接口尚未链接产品或绑定全局会话，真实普通钩子共存和停止待验证。[证据](../history/M0-T21-S13-vb-global-investigation.md)。
- P55 测试适配器将晚批处理接到项目正式 CreateProcessA/W 传播实现；八种 A/W 与父子位宽组合、八项强制回归共 16 项实际包测试通过。全局管理器尚未接入，停止和并发边界未解决。[证据](../history/M0-T21-S13-vb-global-investigation.md)。
- P54 多钩子实验增加统一检查与写入前保护准备；双宽第二目标冲突及保护失败负例保持两个函数不变，正常批次通过。写入后的保护/缓存/恢复失败仍可能留下已安装状态，未接入产品。[证据](../history/M0-T21-S13-vb-global-investigation.md)。
- P53 新增单钩子晚安装实验构件，双宽测试覆盖暂停失败、目标字节冲突及正常跳板；不支持多钩子事务或卸载，尚未接入产品，既有线程竞态仍在。[证据](../history/M0-T21-S13-vb-global-investigation.md)。
- P52 实验构件在暂停后再次核对线程集合；双宽测试在两次枚举之间创建线程时返回重试并恢复，仍不能阻止第二次快照之后的新线程，未接入产品。[证据](../history/M0-T21-S13-vb-global-investigation.md)。
- P51 已在仓库增加未接入产品的线程暂停/恢复实验构件；双宽忙区拒绝、暂停故障与恢复测试通过，线程创建竞态和在途调用仍待解决。修正 P40/P42 三项过期来源收据并通过来源门禁。[证据](../history/M0-T21-S13-vb-global-investigation.md)。
- P50 更正：先前消息框按钮未附着系探针错误；按实际 Button 类定位后，双宽普通和全局对照均附着。新增延迟消息框回归；用户现场问题及无窗口子进程传播仍未解决。[证据](../history/M0-T21-S13-vb-global-investigation.md)。
- P49 忙线程地址检查在双宽原位循环中拒绝安装并恢复线程，正常传播对照通过；另一含系统调用的 x64 负例未被拒绝，故该检查不足以证明调用栈或停用安全。[证据](../history/M0-T21-S13-vb-global-investigation.md)。
- P48 自有运行中进程晚钩住 CreateProcessW，并调用项目真实注入器；四种双宽组合均观察到子、孙进程入口加载 DLL。仅为隔离原型，尚未接入全局管理器，停用及线程竞态未解决。[证据](../history/M0-T21-S13-vb-global-investigation.md)。
- P47 自有双宽原型模拟暂停第二个线程失败，安装返回错误且已暂停线程恢复；正常安装/停用对照通过。线程上下文回滚、线程创建竞态及实际传播仍未解决，未用于产品。[证据](../history/M0-T21-S13-vb-global-investigation.md)。
- P46 官方 MinHook v1.3.4 临时原型在自有双宽多线程函数上通过晚安装及停用；上游遇无法暂停的线程仍可能继续，尚不能安全用于既有进程，也未验证实际传播。[证据](../history/M0-T21-S13-vb-global-investigation.md)。
- P45 双宽探针证实全局已附着父窗口创建无窗口子进程时，子进程入口 DLL 未加载（41）；普通模式下子及孙进程均在入口加载并成功退出。新探针已纳入构建，普通对照已注册；全局负例仍待修复。[证据](../history/M0-T21-S13-vb-global-investigation.md)。
- 用户更新契约：全局模式应尽力为可接入进程安装与普通启动统一的可传播钩子；退出无需特判保留普通模式。现有启动期代码不支持对运行中进程安全重跑，尚未实现统一晚接入。扫雷 About 在自有隔离样本中标题呈现经典样式，不能据此否定用户现场复现；此前消息框按钮未附着的报告是探针误用控件 ID，修正后双宽普通及全局对照均观察到附着。
- P43 正式包测试未通过：并发可见窗口用例及随后串行原用例均在 x64 GDI 22→25 时超过 +2 门槛；前者另有一次启动窗口通知超时或失败。原因仍未查明。[证据](../history/M0-T21-S13-vb-global-investigation.md)。
- P42 新增直接以 `WS_VISIBLE` 创建的双宽顶层窗口回归，自有四线程三轮启停及原显式显示对照通过；未证明所有窗口类别及首帧。[证据](../history/M0-T21-S13-vb-global-investigation.md)。
- P41 四项键盘用例经正式实际包测试入口验证，连同八项强制回归共 12 项通过、无跳过；演示程序验证耗时 44.3 秒。产品二进制未变。[证据](../history/M0-T21-S13-vb-global-investigation.md)。
- P40 新增双宽菜单键盘命令及原生负例，四项自有私有桌面对照通过；保留原鼠标消息用例，物理输入和先前失败原因仍未覆盖。[证据](../history/M0-T21-S13-vb-global-investigation.md)。
- P39 八个并发私有桌面菜单样本再次出现一次 x86 未进入菜单循环；相同投递消息探针在无样式对照中也未进入，不能仅凭此判定真实点击或绘制故障。十二个带诊断样本通过，失败根因仍未知。[证据](../history/M0-T21-S13-vb-global-investigation.md)。
- P38 自有 x64 八轮启停诊断中，GDI 总数 22→24 后稳定，内部表将增加的两个对象归为区域；无全局管理器的不同负载对照总数保持 9。内部结构非公开且两组工作量不同，原因及是否泄漏仍未确认。[阶段证据](../history/M0-T21-S13-vb-global-investigation.md)。
- 打印当前页已纳入受限 PrintDlgEx 转接；跨 DPI、实际打印和本地化仍待覆盖。
- COM并发：已观察的双多选并发由系统原生路径止损；旧框目录串用根因、非多选并发仍未查明，详见[S3证据](../history/M0-T20-S3-com-safety.md)。
- COM多选：默认回退系统新框，O: 长名、单项、取消、链接及双宽并发经有限验证；旧框实现及其他参数仍未全面解决。
- Shell筛选：受影响的带回调组合保留系统新树及原筛选接口；旧树中完整支持该回调仍未实现，其他标志组合未全面验证。详见[S3证据](../history/M0-T20-S3-shell-filter.md)。
- P253 已发布：干净包 58 项通过；系统打印宿主自然更替后，已发布包及无注入对照的双宽 A/W Print/Apply 实际虚拟打印各八项通过。此前 32 位 EndPage 失败的原因未明；详见阶段证据。

## Technical Baseline

三个组件、四产品文件；纯C、GCC、Ninja、ccache，原图形资源及来源保持。发布位置为项目assets根目录和O:/winnt根目录。当前有限控件契约见[界面设计](../design/UI.md)，启动/架构见[架构](../design/ARCHITECTURE.md)，具体覆盖及限制见[收口审计](../history/M0-T18-scoped-closure-audit.md)。T19 S1双宽干净包、两处部署摘要、18项聚焦回归和真实程序记录见[交付证据](../history/M0-T19-S1-canon-ownerdraw.md)。历史完整活动包保存在[归档](../history/M0-T18-S64-current-archive.md)，只作历史记录。

用户否决S2两次试用；S1 P208源码和原始四文件已恢复并经实际远端复核。见[撤回记录](../history/M0-T19-S2-rollback.md)。

S3调查结果见[收口记录](../history/M0-T19-S3-dialog-closure.md)：当前原版真实点击正常；历史异常根因未查明，本S没有产品代码或二进制修改。

T20 S1 的阶段包已发布部分公共对话框和 COM 桥接，P231 默认文件夹新建保留旧式布局并修正选中目录及输入路径后的确认问题；P234 处理旧式列表系统主题动画覆盖项目滚动条，同源码增量候选的双宽真实拖动通过，干净包的双宽聚焦消息驱动测试及 x64 真实拖动通过，干净包 x86 真实鼠标复测因近期输入跳过。按用户最新指示暂停COM布局桥接及其长尾。事件桥接实验默认关闭，定制、隐藏目录及完整矩阵仍未完成；公开新式对话框 API 与 demo 路径已纳入阶段包。详见[阶段证据](../history/M0-T20-S1-preliminary.md)；S1已收口；T20此后按用户要求有限范围收口。

## Recent Closure

- M0 T21 S44：同步公共交集及通用测试；[记录](../history/M0-T21-S44-shared-refresh.md)。

- M0 T21 S43：公共组件极简契约修正；[记录](../history/M0-T21-S43-public-contracts.md)。

- M0 T21 S42：产品全局适配五项清理；[记录](../history/M0-T21-S42-global-cleanup.md)。

- M0 T21 S41：公共全局进程能力抽离及跨工具链验证；[记录](../history/M0-T21-S41-process-global.md)。

- M0 T21 S40：公共线程快照遍历；[记录](../history/M0-T21-S40-process-global.md)。

- M0 T21 S39：移除公共 file 组件及测试；[记录](../history/M0-T21-S39-remove-file.md)。

- M0 T21 S38：共享文件原语及本地独立测试；[记录](../history/M0-T21-S38-file.md)。

- M0 T21 S37：导入 handle 并接入现有句柄调用；[记录](../history/M0-T21-S37-handle.md)。

- M0 T21 S36：共享 process-launch 导入及启动器事务归属；[记录](../history/M0-T21-S36-process-launch.md)。

- M0 T21 S35：全局架构查询复用 process-image；[记录](../history/M0-T21-S35-global-machine.md)。

- M0 T21 S34：控制台重绑定归回启动器；[记录](../history/M0-T21-S34-console-return.md)。

- M0 T21 S33：原样导入隔壁 process-hook，双宽构建与包回归通过；[记录](../history/M0-T21-S33-import.md)。

- M0 T21 S32：完成本项目现有公共映像、启动及 Shell 机制提取；[记录](../history/M0-T21-S32-common-completion.md)。

- M0 T21 S31：console 和 security 基础机制提取；[记录](../history/M0-T21-S31-console-security.md)。

- M0 T21 S30：独立 module-import；[记录](../history/M0-T21-S30-module-import.md)。

- M0 T21 S29：原生命令行尾部解析迁入 process-launch；[记录](../history/M0-T21-S29-process-launch.md)。

- M0 T21 S28：process-image PE 校验和位宽查询提取；[记录](../history/M0-T21-S28-process-image.md)。

- M0 T21 S27：同位宽纯载荷入口与双宽自有目标验证，跨位宽纯载荷和隔壁适配未验证；[记录](../history/M0-T21-S27-payload.md)。

- M0 T21 S26：提取现有纯 C 进程挂钩原语，双宽实际包及四条真实传播边通过；不新增物理解钩，隔壁兼容性未实测。[记录](../history/M0-T21-S26-process-hook.md)。

- M0 T21 S20：用户确认验收通过，LibTray 拆分和有限测试证据见 [记录](../history/M0-T21-S20-tray-library.md)。T21 保持开放。

- M0 T21 S19：用户确认验收通过；39 号 32×32 徽标、托盘控制、UAC 启动修复的交付及验证范围见 [记录](../history/M0-T21-S19-tray.md)。T21 保持开放。

- M0 T21 S18：真实桌面 UAC 启停及自有双宽窗口、运行时子孙传播和停止恢复已验证；7 个窗口通知超时或失败的对象不明，T21 保持开放。详见 [记录](../history/M0-T21-S18-real-global.md)。

- M0 T21 S17：私有桌面真实 UAC 对照等待超时，用户未看到提示；双宽干净包及 22 项聚焦回归通过，完整组合未验证。见 [记录](../history/M0-T21-S17-uac-isolation.md)，T21 保持开放。

- M0 T21 S16：提权控制台输出修复已发布并远端复核；管理员双宽回归通过。私有桌面仍有未明引用、真实自动 UAC 与完整会话组合未覆盖，详见 [记录](../history/M0-T21-S16-console-output.md)；T21 保持开放。

- M0 T21 S15：交互控制台自动 UAC、控制台保持和父进程退出清理；双宽干净包、22 项聚焦测试及管理员双宽隔离回归通过，四文件双位置发布，远端 `3d26d80` 摘要复核一致。组合验证范围及限制见 [记录](../history/M0-T21-S15-global-elevation.md)。

- M0 T21 S12：动态标准按钮双宽附着、点击和退出恢复；13 项实际包回归，远端 P1 7cf6481 及四文件摘要已复核。[记录](../history/M0-T21-S12-late-controls.md)。

- M0 T21 S11：双宽干净构建、18 项实际包回归及双位置发布；[记录](../history/M0-T21-S11-restart-timeout.md)。

- M0 T21 S10：双宽自有 Shell 视图的外框及滚动条附着/解除已验证；DirectUI 内部未样式化，远端 6760373 已复核。[记录](../history/M0-T21-S10-shell-view.md)。

- M0 T21 S9：Explorer 三种私有桌面启动均未产生自有窗口，不修改排除规则；远端 3947c81 与两处四文件摘要一致。[记录](../history/M0-T21-S9-explorer-isolation.md)。

- M0 T21 S8：双宽 96 DPI 虚拟化及 144 DPI 对照通过，远端 be2e5c4 与两处四文件摘要一致；[记录](../history/M0-T21-S8-dpi-unaware.md)。

- M0 T21 S7：简单窗口所有权语义保留，144 DPI 双宽受控输入与 18 项聚焦包测试通过；远端 782d2c3 与两处四文件摘要一致。[记录](../history/M0-T21-S7-input-dpi.md)。

- M0 T21 S6：双宽四线程、八窗口和三次启停通过，远端 4a88f58 及两处四文件摘要一致；[记录](../history/M0-T21-S6-threads-resources.md)。

- M0 T21 S5：普通与全局共用窗口附着实现，远端 35bcfaa 及两处四文件摘要一致；[审计](../history/M0-T21-S5-coverage-audit.md)。

- M0 T21 S4：实际应用和双宽混合顺序已验证，宿主故障检测已修复并在远端 8f21d36 复核；[记录](../history/M0-T21-S4-real-lifecycle.md)。

- M0 T21 S3：手动全局模式已交付，远端 a9c178c 与两处四文件摘要一致；正常 run16s 效果独立保留。[记录](../history/M0-T21-S3-global-controller.md)。

- M0 T21 S2：同线程窗口接入基础已交付；全局模式仍在实施。[记录](../history/M0-T21-S2-global-window.md)。

- M0 T21 S1：双位宽全局钩子隔离可行性和恢复限制已验证，用户接受残影；未实现产品全局入口。[证据](../history/M0-T21-S1-global-prototype.md)。

- M0 T20 S19 / T20：按用户指令有限范围收口；系统菜单与首次绘制交付，真实桌面边界留TODO。[收口审计](../history/M0-T20-closure-audit.md)。

- M0 T20 S18：颜色框基本色及自定义色块恢复细黑框平面外观，双宽包验证和发布；[记录](../history/M0-T20-S18-color-swatches.md)。

- M0 T20 S17 / T20：最终双宽干净构建及 18 项精简包回归通过；当前发布包和剩余边界见[交接](../history/M0-T20-S17-final-handoff.md)。

- M0 T20 S16：普通文件快捷方式的受限默认解析，32 项包回归、双位置发布及远端复核；[记录](../history/M0-T20-S16-file-link-resolve.md)。

- M0 T20 S15：显式保留快捷方式的打开请求转接旧框，32 项实际包聚焦回归及双位置发布通过；[记录](../history/M0-T20-S15-file-link-flag.md)。

- M0 T20 S14 / T20：常用对话框有限范围审计、双宽干净包 23 项聚焦回归；首次超时及回退如实记录。[任务级审计](../history/M0-T20-S14-completion-audit.md)。

- M0 T20 S13：32 项干净包聚焦回归及远端复核，错误交接原型不进入产品；[记录](../history/M0-T20-S13-shadow-coverage.md)。

- M0 T20 S12：真实API错误原型及28项包回归；ChooseFont过期错误缺口有负例，未进入产品。[记录](../history/M0-T20-S12-shadow-prototype.md)。

- M0 T20 S11：扩展错误双宽对照及28项干净包回归；嵌套取消保留内层错误。[记录](../history/M0-T20-S11-commdlg-error.md)。

- M0 T20 S10：导航后选择原型、IDABORT修正及68项干净包回归；错误接口未实现。[记录](../history/M0-T20-S10-file-selection.md)。

- M0 T20 S9：双宽目录/筛选原型及24项干净包回归；尚未结合文件确认。[记录](../history/M0-T20-S9-file-navigation.md)。

- M0 T20 S8：固定两项原型和40项干净包回归；未进入产品，导航与筛选未验证。[记录](../history/M0-T20-S8-win32-multi-prototype.md)。

- M0 T20 S7：Win32文件多选双宽对照及24项干净包回归；长名多选仍回退Explorer。[记录](../history/M0-T20-S7-win32-multi.md)。

- M0 T20 S6：探针修订、双宽干净包26项及远端复核；临时目录旧Shell对照超时仍未解决。[证据](../history/M0-T20-S6-folder-probe.md)。

- M0 T20 S5：最终交接、双宽构建、部署包18项及实际路径探针通过；P1 87216a1 已推送并读取远端对象复核。T20 保持开放。[记录](../history/M0-T20-S5-handoff.md)。

- M0 T20 S4：双宽干净构建、实际重建包40项通过；部署包字节未改，旧探针超时保留。[记录](../history/M0-T20-S4-common.md)。

- M0 T20 S3：Shell筛选与COM多选已安全回退，双宽构建、实际包测试及远端复核；[Shell](../history/M0-T20-S3-shell-filter.md)、[COM](../history/M0-T20-S3-com-safety.md)。

- M0 T20 S2：入口与回退矩阵已交付、双宽构建及实际包代表回归通过；[证据](../history/M0-T20-S2-dialog-audit.md)。

- M0 T20 S1：按用户指令阶段收口；已交付、已知缺口及后续承接见[记录](../history/M0-T20-S1-closure.md)，T20保持开放。

- M0 T19 S3 / T19：用户确认当前对话框恢复正常并要求收口；历史异常原因不明，字体试验已撤回。

- M0 T19 S2：按用户要求撤回全部字体试验并恢复P208原始四文件；[活动包归档](../history/M0-T19-S2-rollback-current-archive.md)。

- M0 T19 S1：Canon自绘按钮图像恢复，实际远端源码和四文件已复核；范围与限制见[交付记录](../history/M0-T19-S1-canon-ownerdraw.md)，[活动包归档](../history/M0-T19-S1-current-archive.md)。
- M0 T18 S64 / T18：按用户确认的常用可见控件范围收口；非视觉、应用自绘不处理，安全契约不足及长尾留TODO。首次焦点回归失败、重入负例不足、物理输入和完整矩阵未覆盖均保留记录。

2026-10-29 阶段包 P218 已构建、聚焦测试及双位置发布；完整矩阵和公共对话框桥接仍在进行，不代表 T20/S1 收口。失败、跳过与已知回退见阶段证据。

## T22 S1 implementation status

当前候选通过显式 `--app <AUMID>` 激活新应用，以@38定向回调适配x64
ApplicationFrameHost外框；不启用宿主传播，不改XAML客户区，不接入复用实例。
适配撤销不等于DLL卸载；模块固定至宿主退出。三组件四产物保持。

- 最大化按monitor工作区剪裁；visual3真实144DPI截图确认标题按钮完整及撤销恢复。
- 原生私有桌面激活复现GDI31->44->42；正式测试保留原生预热及首轮适配基线，
  后续无增长断言。原失败及精确对照已保留，不声称所有系统USER缓存物理卸载。
- host-menu8双宽各3项通过，无skip（ABI、布局、生命周期、真实菜单取消）。
- 真实presenter诊断证明动画临时WS_EX_LAYERED导致原路径跳过；已删除呈现hook、
  TLS和诊断属性。现用WM_ENTERIDLE及单次TPM_NOANIMATION，不改系统动画设置。
- visual11：自有Calculator21380、宿主10220/TID38944、144DPI；菜单在标题按钮下
  完整显示，取消、应用/撤销、几何通过。客户区不改。visual10因目标不在前台作废，
  截图已删除，原生截图探针增加前台身份检查。
- clean1来源门禁、clean2链接、clean3版本字符串门禁失败均保留；clean3双宽编译
  和ABI通过，但未发布。版本字符串已统一M0 T22/0.22.0.0。
- clean4已冻结当前源码，后台双宽重建中，工作器PID26424；实包测试、发布、
  提交及远端复核尚未完成。实际菜单命令完整矩阵、物理拖动/缩放、跨屏DPI及
  其他UWP仍未认证。S1/T22及目标保持开放，不以阶段包代替最终验收。

本S scratch均位于build/t22-package，证据提炼后由本S清理；已映射DLL保留：
`s1-frame-buttons1`、`s1-clean1`、`s1-clean2`、`s1-clean3`；
`s1-window-audit`、`s1-frame-core1`、`s1-frame-core2`、`s1-frame-core3`；
`s1-frame-core4`、`s1-frame-menu1`、`s1-frame-product1`、`s1-frame-launch1`；
`s1-frame-launch2`、`s1-frame-launch3`、`s1-frame-motion1`、`s1-frame-motion2`；
`s1-frame-motion3`、`s1-frame-motion4`、`s1-frame-motion5`、`s1-link-fix1`；
`s1-frame-clip1`、`s1-frame-clip2`、`s1-frame-clip3`、`s1-frame-host-menu1`；
`s1-frame-host-menu2`、`s1-frame-host-menu3`、`s1-frame-host-menu4`、`s1-frame-host-menu5`；
`s1-frame-host-menu6`、`s1-frame-host-menu7`、`s1-frame-host-menu8`、`s1-package1`；
`s1-publish1`、`s1-clean4`。
详见[原始失败与最终验证](../history/M0-T22-S1-feasibility.md)。

用户指定库边界：本 S 将绘制、布局、应用及撤销迁入
`lib-win32/win16-window-uwp`，以便未来并列实现 `win2k-window-uwp`；
激活、宿主定位、线程钩子及模块生命周期留在产品层。clean4快照早于此迁移，
继续完成构建作为基线，但不发布。后续scratch `s1-library1`、`s1-clean5`
分别用于增量验证及最终同快照包；沿用已登记实包/发布目录。

迁移首轮静态链接声明缺失导致palette导入符号链接失败，已改为静态声明；
`s1-library2`为修正后增量验证，不将首轮失败算通过。

迁移结果：s1-library2双宽各3项通过，无skip；静态库未定义符号审计仅依赖
Windows/C运行库及已有Win16绘图库，无产品私有实现。clean4双宽干净构建和
包门禁通过，因早于库迁移不发布。clean5后台PID12668是当前交付快照；
首轮scratch编译探针漏_WIN32_WINNT定义已修正，不影响产品源码。

用户永久授权：发布本项目文件时可随时结束实际占用者，不再重复请求确认。
执行前仍核对PID、映像路径、启动时间及被映射的准确发布路径；不按名字批量
结束或处理无关进程。当前只读检查assets/ntw16s64.dll占用者为ApplicationFrameHost
PID10220，启动2026-11-03T07:30:25.1227778-08:00；实际执行前重新核实。

P4本地交付：clean5双宽干净构建、ABI/资源/边界门禁通过；UWP库双宽各2项
通过，实包14项聚焦通过无skip。真实计算器144DPI激活、按钮及撤销几何通过，
visual12菜单屏幕截图通过。s1-publish1完成assets根目录与O:/winnt四文件发布，
摘要完全一致。S1/T22保留开放；完整限制及来源摘要见release.json及本S研究记录。
提交推送和实际远端复核进行中；不得将此开发包称为所有UWP支持完成。

P5交付复核：产品P4 `c06ea3edbcb2f3146a278ee9297deab438112997` 已推送，
重新fetch并读取33个变更blob逐字节比对；远端四文件及两处八个部署文件均与
实包测试SHA256一致。仅更新复核记录，无二进制变化。S1/T22保持开放，
后续仍需解决/验证系统菜单状态矩阵、原生缩放光标、标题子窗替换及跨屏DPI。
当前可验收命令：`O:\winnt\run16s.exe --app Microsoft.WindowsCalculator_8wekyb3d8bbwe!App`，
需新计算器实例；不要将此入口扩张为普通calc别名或global已支持。

P6继续：审计菜单状态与原生缩放光标。先在真实TrackPopup菜单循环读取普通/
最大化状态的系统项目，以旧实现负例确认断点，再做最小修复；不扩展客户区。
本S新增scratch `s1-menu-state-negative1`、`s1-menu-state1`供增量验证；
生命周期及图形来源要求不变，后续相关修复合并后再做完整交付。

`s1-menu-state2`验证真实菜单键盘选择Close并由原窗口收到SC_CLOSE，限时取消兜底；
同时检查普通/最大化Restore、Maximize、Move、Size状态和应用禁用Close的保留。

私有桌面PostMessage键盘选择未产生菜单命令，s1-menu-state2失败，不计通过；
移除无效的伪键盘验证，保留菜单状态断言，真实选择转当前桌面自有目标探针。
`s1-menu-state3`为该测试修订后的增量验证。

`s1-cursor-diagnostic1`仅用于记录自有适配层命中及SetCursor结果，临时诊断属性
不得进入最终发布。visual16真实键盘菜单最大化/撤销通过，边框光标仍待定位。

P6增量有效证据及输入失败已提炼到本S研究记录。临时T22Cursor属性已删除；
新增scratch `s1-menu-state4`最终增量检查、`s1-clean6`完整快照、`s1-package2`
实包回归、`s1-publish2`双位置交付。系统可见光标未验证，不能称完整输入验收。

用户确认可暂不移动鼠标后，visual22实际SendInput移动/缩放各一次、菜单键盘
最大化、撤销及几何恢复均通过（自有PID40740，144DPI）；已自动恢复指针/窗口
并关闭自有实例。可见边框光标仍显示箭头，记录为已观察限制，不计通过。
s1-menu-state4双宽各3项通过无skip；clean6后台PID30928继续完整构建，
当前已发布仍为P4，不能把未发布P6称为交付。S1/T22和目标保持开放。

P6本地交付完成：clean6双宽干净构建及ABI/资源/依赖门禁通过，最终库双宽各2项通过；s1-package2共14项聚焦实包通过，36.72秒，无skip。初次测试命令误写Task22参数未执行，改为Task 22；沙箱后台无结果退出后确认已结束，再以获准的后台工作器完成。
最终包真实计算器PID5240的激活、最大化/还原、最小化/还原及撤销几何通过；visual23初次沙箱激活E_ACCESSDENIED，获准环境重跑PID16800/宿主12040/线程5924，144DPI标题、菜单截图、撤销与几何通过。未改系统设置。
s1-publish2已将同批四文件发布assets根目录及O:/winnt并校验摘要。原生输入visual22移动/缩放各一次及键盘菜单最大化通过，但可见缩放光标仍为箭头；该失败不能被其他通过项替代。S1/T22保持开放。产品差异仅frame.c及对应测试，31/1与25/4行增删；无第二套渲染路径、无资源变更。当前提交推送及远端复核待完成。

P7复核：P6 b1db5d2aa0f4349f8ef978aa6d6be7d724f1e7de 已推送；重新fetch后读取10个变更blob逐字节一致，远端四产物与两处八个部署文件均匹配实包SHA256。仅登记复核，无二进制变化；S1/T22保持开放，可见边框光标失败及其他有限覆盖边界不变。

P8光标审计：未换肤计算器同坐标原生HTLEFT/SIZEWE通过；覆盖层真实WM_SETCURSOR及NC鼠标消息已到达，线程内SetCursor正确但系统可见光标仍为箭头。正确NC命中、移动消息后设置、自有窗口类及HTTRANSPARENT对照均未解决；全部只在已登记s1-window-audit中，不合入无效产品修改。未变更二进制，沿用P6包；详细PID/线程/限制见研究记录，S1/T22保持开放。

P9对照：原窗口已收到真实HTLEFT光标消息并设置SIZEWE；完全鼠标透传的分层窗也未解决。移除探针WindowFromPoint观察仍复现。同一clean6适配库在自建普通Win32窗口上的native/styled/restored光标三态全部通过，故不能称适配库普遍失效；当前断点属于本机UWP宿主场景，具体输入所有权仍未证实。仅更新研究记录，P6产品包不变，S1/T22继续开放。

P10覆盖扩展：闹钟与录音机外框激活/按钮/撤销通过，但截图均为更新占位页，不计正常应用完整支持。扫描应用返回50，断点为两个可见标题时误选69像素左侧标题；完整标题为1920像素。附加菜单不在标准系统菜单内，不能仅改选择后覆盖丢失入口。未合入不完整修复；研究记录含版本、SHA、PID及几何。安装后立即解除临时线程钩子仍复现光标问题。P6产品包不变，S1/T22开放。

P11双标题原型：扫描应用左侧入口实测为Share/Settings。选择完整标题后，正常扫描主界面外框绘制/撤销通过；追加Application menu命令并异步转交原按钮可打开原菜单，截图visual41-aux证明。仅scratch原型，固定命令ID/重复安装/清理尚需正式化，未发布；下一步实现并测试公共库可选附加标题动作。新宿主18304下光标仍复现，不是仅旧宿主状态问题。S1/T22开放。

P12实施中：可选附加菜单动作进入win16-window-uwp；菜单项目仅在跟踪期间分配无冲突命令并删除，产品提供原生附加标题按钮回调。标题选择完整可见区域，不复制宿主菜单实现。新增scratch s1-aux-action1用于双宽增量ABI/库测试，后续实包与交付待完成；不以源码修改声称已发布。

P12增量：s1-aux-action1双宽各3项通过无skip；正式增量DLL真实扫描PID12420/宿主18304/TID23204，144DPI键盘UP/ENTER实际选中eff0，原生Share/Settings菜单截图visual42-aux通过，撤销/几何通过。新增scratch s1-clean7、s1-package3、s1-publish3用于本批完整交付；冻结源码后不再改产品输入。

P12交付：clean7双宽干净构建、ABI、原图形资源及依赖门禁通过；最终库各2项通过。s1-package3共14项聚焦通过、无skip、33.97秒。最终包ScanPID11928/宿主18304/TID16224、144DPI实际UP/ENTER选择eff0后原Share/Settings打开，visual43截图及移除几何通过；CalculatorPID38488的窗口按钮、恢复/撤销几何通过。s1-publish3两处四文件摘要匹配测试包。静态库增加apply_action，不新增产品导出/第五产物；src三文件69增6删，测试26增1删。公开库无产品私有依赖，原资源未改。原生附加菜单不重新绘制，缩放光标仍未修复；S1/T22保持开放。当前提交推送复核待完成。

P13复核：P12 3c334b3b8ecf0aef9bdddbf259e3a15edc536703 已推送；重新fetch并读取12个变更blob逐字节一致，远端四文件及两处八个部署文件与实包SHA一致。无新增二进制变化，S1/T22保持开放。

P14验证进行中：原生Scan截图另有全屏按钮，当前覆盖层未提供对应入口；
不能将P12附加应用菜单称为保留全部标题操作。fullscreen-audit1原生无障碍
探针因输入桌面非Default跳过，未激活目标。先补双宽私有桌面的标题隐藏/恢复
生命周期检查，scratch s1-title-visibility1；实际全屏转接仍待原生接口证据。

P14验证完成：s1-title-visibility1双宽各3项通过、无skip；构建分别11.97/6.48秒，
测试4.60/4.35秒。仅测试新增7行及审计文档更新，产品源码和资源未改变；
assets与O:/winnt四文件仍逐项等于clean7实测包，无需重新发布相同产品。
全屏入口缺失已确认；真实调用探针因屏保桌面跳过，未计通过。S1/T22开放。

P15原生证据：屏保下只读MSAA未暴露全屏；UIA观测到EnterFullScreen/
ExitFullScreen及非空InvokePattern。自有Scan原生进入/退出全屏S_OK，几何恢复；
叠加已发布clean7 DLL时覆盖层隐藏/恢复、撤销也通过。无屏幕/物理输入验证，
未新增产品菜单入口。详细PID/接口/下一步线程边界见本S研究记录；产品不变。

P16宿主原型：独立MTA线程调用原生全屏/退出成功，Scan几何及同一覆盖层恢复，
撤销成功；临时原型模块随后零映射，不代表固定产品DLL物理卸载。计算器原生
标题没有全屏能力，不能无条件添加菜单项。产品接入最小方案及未验证边界已
记研究记录；仍仅研究scratch改动，无新产品发布，S1/T22开放。

P17实施中：公共外框支持复制附加动作数组；产品按协调方UIA能力查询决定是否
增加全屏切换，菜单回调启动独立MTA原生动作线程，保留原应用菜单及现有动作API。
不扩展XAML客户区、不新增导出/产物。scratch s1-native-actions1用于双宽增量；
正式整包和发布待实现/测试合并后进行，当前不得把源码称为已发布。

P17首轮x86编译及3测试通过，PE门禁因新增Windows自带OLEAUT32导入未登记而拒绝。
同步精确依赖清单（DLL/启动器增加OLEAUT32，demo不变）；无第三方运行库或第五
产物。续跑scratch s1-native-actions2；保留首轮失败，不改安全设置或放宽外部依赖。

P17 actions2双宽各3项通过；新增标题选择/GUI线程拒绝测试，scratch s1-native-actions3继续增量回归。

P17 actions3标题模型测试首轮受隐藏启动参数影响未显示父窗而失败；补显式ShowWindow，与既有私有桌面fixture一致。追加菜单容量不足的部分插入回滚检查，scratch s1-native-actions4续跑。

P17最终增量双宽各4项通过无skip；真实协调方计算器/Scan激活及按钮/撤销通过，
生产native.c独立线程全屏往返通过。屏保下菜单消息/无障碍替代均未完成真实选择，
不计通过。冻结s1-clean8整包，后续s1-package4/s1-publish4；仍须正常桌面菜单验收。

P17本地交付：clean8双位宽干净构建、ABI/原资源/来源/依赖门禁通过；最终UWP
测试双宽各3项通过无skip。s1-package4共14项聚焦通过无skip，32.06秒。
最终DLL计算器PID36124与ScanPID18888、144DPI的API接入/按钮/撤销通过；
全屏工作线程的生产源码诊断对照通过，但菜单实际选择仍未验证（屏保下替代
消息/无障碍均未证明状态变化）。s1-publish4两处四文件与测试摘要相同。
本次没有正常桌面截图，不以API验证替代外观/真实菜单选择验收；S1/T22保持开放。

P18远端复核：P17 621d2beeb1e18da801fa88577603d5f23b81d0e3 已推送；重新fetch逐字节比对全部21个变更blob，
远端四产物及两处八文件与测试SHA一致。仅登记复核，产品未变；S1/T22开放，
全屏菜单真实选择及正常显示仍未验证。

P19验证中：新增同线程双外框隔离、菜单嵌套循环内标题销毁及显式重新附着回归。
scratch s1-host-lifecycle1；mouse-final1当前桌面探针在输入桌面非Default时跳过，未移动鼠标或激活计算器。
不将显式重附着称为产品自动追踪标题替换；产品二进制未变。

P19首轮测试夹具标题宽度未覆盖客户区右沿，按现有契约返回50；改为实际客户宽度。
scratch s1-host-lifecycle2续跑，不计首轮通过，不修改产品几何规则。

P19验证完成：s1-host-lifecycle2双宽各4/4通过无skip；同线程双外框隔离、菜单期间
标题销毁的清理及显式重新附着通过。仅测试增加35行，生产源码/资源不变；
assets与O:/winnt均与clean8实测四文件一致，无重复发布。详细边界见研究记录。
S1/T22保持开放，不能据此宣称自动标题替换或全局UWP已实现。

P20桌面复测：Default/Active并不代表输入可用；自有计算器接入/撤销/几何通过，
前台为空导致真实输入跳过。独立普通Win32可见窗口同样前台0、SetForegroundWindow失败，
排除仅UWP适配导致本次前台缺失。具体会话原因待确认；两类自有实例均清理。
仅记录证据，产品未改；S1/T22开放。

P21：正常桌面恢复，正式clean8 DLL计算器真实移动/缩放/菜单最大化/撤销几何通过，
稳定边缘采样仍为箭头（失败保留）。Scan全屏菜单真实UP/ENTER往返及有限等待后
原矩形恢复通过，截图已检查；更新release验证状态。产品未改，S1/T22开放。

P22实施：用户明确要求run16s calc.exe，无需手写AUMID。仅匹配解析后的系统
calc路径且存在已安装计算器包时使用激活；同名其他程序保留原路，参数转交，
--wait保留等待。允许本次激活API返回的复用实例，created_pid仍仅标记新建；
不启用global或客户区换肤。scratch s1-calc-entry1用于增量验证，正式发布待完成。

P22首轮增量x86四项/x64五项通过无skip；补真实正式launcher calc.exe对自有
计算器撤销后重新附着验证，scratch s1-calc-entry2。

P22正式命令夹具因桌面条件skip（未发现已有Calculator）；增加显式--activate-api
仅API模式，不发送真实输入、不声称显示验证。scratch s1-calc-entry3继续增量。

P22本机重复激活创建第二个新PID而非复用；旧测试同PID假设失败，已清理探针
明确新建实例。正式命令测试改为无现有计算器前提、核对新PID创建时间并自行清理，
不承诺本机一定复用。scratch s1-calc-entry4。

P22 entry4双宽4/5项通过无skip；正式候选run16s calc.exe实际退出0，自有计算器
PID21776在144DPI接入、按钮、移除几何通过（API-only，不计新截图）。冻结
s1-clean9，同批实包s1-package5、发布s1-publish5；后续不改冻结产品输入。

P22环境核对：O:/winnt/calc.exe与System32/calc.exe SHA均621ba393...f829d8。
增加系统启动器逐字节副本识别，避免O:/winnt工作目录命中副本后遗漏。clean9
继续作为早期基线，不发布；最终快照改s1-clean10，增量s1-calc-entry5。

P22副本识别补充临时复制正例及修改首字节负例；scratch s1-calc-entry6作为最终
增量快照验证，随后冻结clean10，clean9保留为不发布基线。

P22 entry6双宽4/5项通过无skip；O:/winnt工作目录真实calc副本入口退出0，
自有PID6532接入/按钮/撤销几何通过。clean9基线已完整通过但不发布；冻结clean10。
另按用户要求实测系统notepad：打包FullTrust/WinUI桌面程序，首层PID32228有DLL，
最终/SESSION子进程31464无ntw16s DLL，传播未通过；已关闭自有文档及进程，
不将其称为UWP支持或追加名字映射来掩盖。

P22交付：clean10双宽干净构建及来源/资源/ABI/依赖门禁通过，最终外框双宽各3项、
启动器识别1项通过无skip；s1-package5实际四文件14项聚焦通过（31.16秒）。
最终真实计算器命令因已有PID27072而skip，非通过；该实例时间与旧负例吻合但
归属链未证实，未关闭。s1-publish5完成assets和O:/winnt四文件发布、摘要一致。
记事本最终进程无DLL、resize cursor及既有其他边界仍开放，不收口S1/T22。
后台verify-final PID26748只等待既有构建并执行测试，无重启或产品修改。

P23分类与交付复核：P22 58b9841288b8f1b29fa30359697af1796747640b已推送，
fetch读取15个远端变更blob全部一致；两处八文件SHA与实包一致。
用户要求先不处理WinUI桌面；Notepad、Terminal、Task Manager、Explorer归入
未编号的[后续候选](../proposals/modern-desktop-window-style.md)，不新增T/S。
原生只读分类探针使用已登记s1-window-audit；Explorer/Task Manager均无包且
非AppContainer；Terminal清单为FullTrustApplication。Task Manager模块枚举
错误5保留，不计框架版本或换肤验证通过。当前继续UWP外框，S1/T22开放。

后续光标审计：旧宿主采样flags=2是CURSOR_SUPPRESSED；外部箭头句柄采样没有
记录flags，故不能证明箭头实际可见，也不能判定已修复。s1-window-audit内
新增纯C cursor-baseline对照，当前因输入桌面/前台不可用返回77，在创建窗口
及移动鼠标前跳过。记录保留，产品未改；待可显示环境继续验证，不改系统设置。

P24验证中：仅扩展frame_layout原生测试，覆盖96/120/144/168/192/240/288 DPI，
八方向缩放边缘、原生隐形边缘与XAML客户区边界、最大化及固定尺寸禁止缩放。
scratch s1-layout-boundaries1，双位宽增量构建；不把布局计算测试算作跨屏显示验收。

P24布局测试双位宽通过无skip；仅测试/证据变更，产品输入和发布包保持P22，
不重复发布。当前桌面光标对照skip仍保留；S1/T22不收口。

P25验证中：原生launch-uwp-live增加显式测试AUMID/映像名参数，仍在目标已有
实例时跳过，产品入口不增加应用名映射。scratch s1-multiapp-fixture1用于双宽
增量与自有Windows Scan API回归；不执行扫描/预览，不计正常桌面绘制验收。

P25增量双宽3/4项通过无skip；正式P22 DLL的Scan自有PID14156激活等待外框
超时1460，后续观察CoreWindow自身为root、无surface，FAIL。探针和自有目标
均已退出；无扫描操作。输入桌面同样不可用，但尚未证实因果。产品不变，
保留失败证据，已询问正常桌面可用时机；S1/T22开放。

P26验证中：用户恢复桌面后原生SIZEWE对照通过。Scan首次复测仍1460；无样式
激活对照随后250ms出现宿主外框，最终P22 DLL复测PID23452在144DPI附着、
最大/最小/恢复、撤销几何通过。早先超时原因未证实，不改写为通过。
自有Scan真实移动/缩放及撤销通过；mouse-scan-p22稳定边缘光标flags=1且为箭头，
确认失败。移动后的立即截图为原生标题，mouse-scan-p22-delay延后500ms截图
呈现Win3.x标题，不能称持续回退；该轮鼠标移离采样点，光标结果无效。
Scan多一个菜单项，旧夹具固定UP/UP未选中最大化，取消后无错误命令执行；
不计产品菜单失败。scratch均位于已登记s1-window-audit；产品未改。

cursor-order/mouse-scan-order1定向线程消息观察未取得前台（当前前台另属PID19692），
输入/光标/截图跳过，附着/撤销几何通过且观察钩子及自有实例已清理；不推定
光标覆盖来源。P26仅记录当前证据，边缘光标问题未修复。

P27纠正P26光标结论：旧scratch重复使用CURSORINFO，首次调用后cbSize变0，
后续返回87却继续报告旧箭头。逐次初始化后正确；正式测试增加显式
--activate-pointer AUMID image.exe，最多500ms采样并恢复指针和原topmost。
scratch s1-pointer-regression1双宽增量3/4项通过无skip；使用已发布P22 DLL，
Scan自有PID29168、144DPI的真实可见SIZEWE、窗口按钮、撤销几何通过，实例清理。
原先光标失败判定作废，不改产品renderer；此前激活超时原因仍未确定。
产品及两处四文件保持P22；S1/T22开放。
P28验证准备：登记build/t22-package/s1-wait-audit，自有纯C探针运行已发布
run16s --wait calc.exe，核对新建PID/外框/覆盖窗，关闭外框后观察启动器退出；
任何已有Calculator实例则skip，不终止宿主。不改产品，限时并清理本次实例。

P28正式夹具增加--wait-api launcher，同一既有uwp.c复用目标发现及外框识别，
无新产品/构建依赖。登记s1-wait-regression1用于双宽增量与已发布程序实际测试。

P28首轮正式等待夹具失败：frame为空时owned误匹配无owner分层窗，未发送关闭；
目标PID16112已由测试超时清理。修正空frame保护及等待期间ApplicationFrameWindow
校验；登记s1-wait-regression2，沿用既有产品，不将夹具失败视为产品故障。

P28最终验证：s1-wait-regression2双宽3/4项无skip；正式夹具调用O:/winnt发布
程序，自有Calculator PID22048的样式窗口存续时启动器等待，关闭后应用及启动器
退出，返回0。测试和证据已修正，产品输入及两处四文件SHA不变，无新二进制发布。
--wait真实关闭验证完成；不将宿主尚存HWND视为销毁或物理卸载。S1/T22开放。

P29验证准备：s1-reactivate1用于正式原生夹具--repeat-api，首次自有激活、
样式撤销后再次调用产品激活；明确记录复用同PID/同CoreWindow或新建PID，
关闭自有实例并验证覆盖层不再可见。无产品修改，不把模块卸载作为成功条件。

P29完成：s1-reactivate1双宽3/4项通过无skip。正式P22 DLL：Scan PID33664，
第二次created_pid=0、同PID/CoreWindow/外框，撤销后样式重新应用通过；计算器
14144->24496实际新建，第二实例接入通过，不计复用。自有进程均退出；两者
覆盖层关闭后仍有HWND但已隐藏，仅证明不显示，不证明资源释放/宿主复用安全。
产品输入及两处四文件不变；S1/T22开放，后续审计宿主缓存窗口的附件生命周期。

生命周期审计继续使用已登记s1-window-audit：纯C系统激活Scan，不加载样式DLL，
检查新进程承载外框是否继承上一自有实例的覆盖层。仅观察并关闭本次创建实例，
不终止共享宿主，不修改系统配置。

P30验证准备：无样式Scan自有PID27204激活后无覆盖层；宿主18304后续只读枚举
不再存在P29自有窗口。先前夹具一隐藏就结束采样，不能推断长期保留。
登记s1-retire1，重复激活测试改为最多1500ms等待覆盖HWND实际销毁；产品未改。

P30完成：s1-retire1双宽3/4项通过无skip；正式P22 DLL的Scan8488复用及
Calculator18880->18008新建再次通过。两者关闭后覆盖层均在1500ms内实际销毁。
P29隐藏即停止的采样不足以认定长期缓存/资源泄漏；本轮未发现需改的产品故障。
无样式Scan27204无覆盖层、正常退出；仅本机样例，不扩张为所有宿主复用保证。
产品输入及两处四文件SHA不变，S1/T22开放。

全局接入审计：沿用已登记s1-window-audit，原生只读WinEvent时序探针观察自有
Scan激活/外框关联/关闭。暂不改变global名单或启用全桌面样式；现有目标表按
PID/TID去重，UWP共享宿主需按外框关联，不能直接放开普通传播路径。

P31全局接入审计：Scan23684原生事件证明CoreWindow关联完成后再次SHOW，
现有公共窗口观察器可作为该样例发现入口。不能直接放开宿主进程过滤：会进入
普通控件及传播路径。后续在同一S按外框/CoreWindow身份单独排队，复用公共
调度/队列及单目标外框协议；冻结顺序和判据见研究记录P31。仅研究文档变更，
尚未实现/发布全局UWP，产品与P22不变；观察探针及自有实例已清理。

P32实施中：先提取产品侧uwp/target共享协调器，启动器调用同一外框发现/接入
路径；新增按窗口发现及撤销接口供全局适配调用，不扩展公开DLL导出。保持
原启动行为、既有标题资源和协议。s1-global-coordinator1用于增量验证；尚未发布。

P32增量记录：coordinator1的x64链接失败为宿主18304映射增量DLL；已只读核实
路径/启动时间，将该DLL同目录保留为ntw16s64.p32-pinned.dll后重链，未结束宿主。
coordinator2双宽3/4项通过。新增产品uwp/global按外框/CoreWindow排队，公共调度
执行；经典进程过滤保留。s1-global-coordinator3加入真实定向适配夹具与经典manager
回归，不启用全桌面样式；产品集成仍待实测与完整打包发布，不声称已交付。

coordinator3夹具GCC缩进警告修正后，coordinator4双宽7/8项通过；真实定向Scan
PID10904原先无surface，调度后单个可见surface，停止移除成功。尚非正式托盘
全桌面端到端或最终发布。coordinator5补UWP就绪等待及入队立即停止竞态验证。

coordinator5双宽7/8项通过；定向Scan10776接入、停止移除、立即停止再入队均通过。
manager1扩为实际生产manager观察路径，测试构建过滤到自有PID；x86原有manager
首次失败为cleanup=0（自有子进程关闭超时），其余字段通过，未认定产品回归。
manager2串行复测并覆盖启动管理器后才创建UWP的真实SHOW路径，不依赖手动refresh。

manager2串行双宽7/8项通过；生产manager定向已有/新建Scan、已有Calculator通过，
新建Calculator未出现覆盖层，失败保留。manager3增加UWP队列/活跃/失败统计及
测试过滤诊断，双宽7/8项通过；新建Calculator25148复测通过（active=1 failed=0）。
此前失败原因尚未确定，不以重跑通过宣称修复；validation目录再核对完整四组合。

P32桌面恢复复测：manager3/desktop-restored四组合通过，实际自有Scan35748/36896、
Calculator24856/25284均active=1 failed=0，停止后surface实际销毁。此前validation
新建Calculator24012为failed=1，不是纯发现遗漏；未保留具体错误，原因未确定。
登记s1-global-visual1用于增加测试专用错误码和可选正常屏幕截图，继续增量双宽回归。

s1-global-visual1双宽7/8项通过；自有Scan28240及Calculator36768接入/撤销通过，
error=0、144DPI。屏幕截图被其他窗口遮挡，不计绘制通过。登记s1-global-visual2，
仅修测试截图时自有窗口短暂置顶后恢复；不修改产品行为。

s1-global-visual2实际Scan15928及Calculator27280外框绘制可见，停止均销毁surface；
计算器截图仍有激活动画，不计稳定客户区画面。登记s1-global-visual3仅在截图采样前
泵消息等待1秒，不延长/更改原接入验证条件；保持正常显示与附着状态证据分开。

s1-global-visual3：自有Scan8068、Calculator608，144DPI，接入返回0；正常客户区
显示后的截图可见经典标题/按钮/边框（屏幕另有IME候选层，不属于本项目窗口）。
两实例停止返回0且surface实际销毁，重复启动立即停止通过。夹具双宽编译及各1项
布局回归通过；完整聚焦沿用visual1双宽7/8项。未运行扫描/打印等应用操作。
当前P32尚未完成干净同批打包、实际包验证、发布及提交；既有发布仍P22，S1/T22开放。

P32交付冻结：登记s1-clean11，同一快照双宽干净构建；四文件实包验证将另用
s1-package6及s1-global-package1，发布用s1-publish6。沿用已有实际桌面证据，
此前新建Calculator偶发失败仍为未解决项；不将开发验收包等同T22收口。

P32本地交付：s1-clean11双宽干净构建通过（256/274秒）；s1-package6首次经典x86
cleanup超时，原断言同包串行s1-package7复测20/20通过、无skip。真实同包已有/新建
Scan与Calculator四项通过，正常144DPI截图通过，撤销surface销毁；calc-new统计
failed=2但目标结果0，失败记录身份尚未确定，不宣称全局零失败。普通calc命令
及--wait同包通过。已核实并关闭唯一旧DLL占用宿主18304，s1-publish6两处四文件
摘要一致。此前偶发接入失败和清理超时保留未解记录；未完成托盘端到端及并存矩阵。
当前待提交推送/远端复核，S1/T22保持开放。

P33交付复核：P32提交56b5c0e85de753b7e7bb1f6fda2e2ebae6e8582f已推送；
fetch后18个变更blob逐项相同，远端四产物及两处八文件摘要与s1-package7一致。
P32统计18文件，文本+768/-136，净+632；目标协调所有权已移至产品uwp适配层，
启动器重复发现/安装路径删除；窗口renderer/原图形资源不变。本次P33仅记录复核，
无新二进制变化；s1-clean11/governance2自测13项拒绝通过，S1/T22继续开放。

P34诊断准备：登记s1-global-diagnostic1，测试构建记录经典目标失败的PID/TID、
错误码和AppContainer属性；不修改生产分支或准入策略。用已发布P32 DLL运行
最多三次新建Calculator，区分经典策略拒绝统计与外框接入失败，不无限重试。

s1-global-diagnostic1双宽编译/布局测试通过；实际三次计算器11584失败、15760/9220通过。
11584经典目标返回5、AppContainer=1，外框最终存在但没有UWP_TARGET_RESULT，定位为
未进入外框接入。登记s1-global-diagnostic2，增加自有目标SHOW/PARENTCHANGE/UNCLOAKED
只读事件诊断，先验证顺序再改发现逻辑。

s1-global-diagnostic2三次新建计算器均失败；29964/1152日志显示CoreWindow SHOW时
外框未就绪，宿主在后续47/31ms发出UNCLOAKED，旧观察器只处理SHOW，UWP路径未执行。
最小修复：公共window观察器增加可选UNCLOAKED订阅，x64管理器开启，既有默认及x86
不变；复用原准入/队列/renderer，无轮询、不放开AppContainer经典路径。登记
s1-global-uncloak1增量双宽公共观察器、UWP和经典管理器回归；随后同旧夹具实测。

uncloak1产品编译通过，公共测试未定义Win8+头版本导致UNCLOAKED常量不可见；
测试显式使用Win10 SDK声明，登记s1-global-uncloak2续编，不改变产品实现。

P34修复对照：uncloak2双宽8/9项通过无skip；真实Calculator30432/38888/37300
三次全部通过。30432重现Core SHOW后31ms宿主UNCLOAKED，新增订阅触发UWP接入0，
停止surface销毁。旧diagnostic2三次失败形成实测负例；经典error5/AppContainer=1
解释同类failed计数，不改策略或将其算作UWP失败。登记s1-clean12冻结同批构建，
s1-package8实包回归及s1-global-package2真实UWP；待通过后s1-publish7发布。

P34交付验证：clean12双宽323/318秒完成干净构建及门禁；package8实包20/20
无skip。global-package2已有/新建Scan和Calculator四项通过，144DPI截图正常，
停止surface销毁。calc-new14632两条经典error5明确AppContainer=1，UWP接入0。
发布前未发现实际占用；publish7已更新assets及O:/winnt，同包八文件摘要一致。
待提交推送及远端复核；并存/托盘端到端仍待验证，S1/T22开放。

P35复核：P34提交12bfe7e943d37dd17904704614cf65d2e008597e已推送并fetch，13个变更blob、
远端四产物及两处八文件SHA均与package8相同。文本+197/-71净+126；普通默认
观察器不变，x64产品选择新增通知，样式资源及renderer不变。P35仅记录复核，
无二进制变化；治理自测13项拒绝通过。并存及托盘端到端仍待验证，S1/T22开放。

P36验证准备：登记s1-global-pair1，测试专用白名单扩为两个自有PID；真实Calculator
与Scan同时承载于同一宿主，验证不同外框各有surface及停止分别销毁。另测首窗
先通过共享单目标协调器应用样式后再由全局管理器接入；不是启动器CLI全链证明。
新增pair/pair-styled仅在测试夹具，产品行为不改。使用P34发布DLL，无新发布。

P36验证结果：pair1双宽各4项管理器回归通过、无skip；P34发布DLL下，计算器34104/
Scan24552共享宿主37040，两个不同surface同时可见，active=2 failed=0，停止均销毁。
预先经单目标API换肤的计算器13084与Scan30396也同结果；重新启动立即停止均无surface。
生产manager预处理结果双宽与P34完全一致；两处八文件摘要未变，无新binary发布。
本轮是同宿主并存/API预置样式证明，不冒充完整CLI/托盘端到端。待提交推送复核。

P37验证准备：P36提交7049bfa9已推送复核（pair1/remote.json），发布摘要未变。
登记s1-global-cli1：pair-cli由真实发布run16s --app启动第一个自有计算器，核对
新PID出生时间和初始surface，再与Scan一同进入生产manager夹具；区分真实启动器
和测试过滤的全局manager，不冒充托盘/UAC完整路径。本轮仅测试变更。

cli1真实启动器组合通过：Calculator7164与Scan22092共享host37040，active2/failed0，
撤销及重复启停通过。登记s1-global-cli2，修正夹具在CLI非零但已激活新实例时，
先取得并验证出生时间再走自有清理；仅测试变化，未宣称该失败分支已实测。

P37验证完成：cli2双宽编译及各1项布局测试通过，无skip；发布P34真实启动器创建
Calculator14540并预先换肤，与Scan12208共享host37040，active2/failed0。两surface
可见、停止均销毁，重复启停清理通过。测试专用失败清理分支未故障注入。两处八文件
摘要仍与P34一致，无新binary发布；完整Explorer托盘/UAC仍未验证，S1/T22保持开放。

P38验证准备：P37已推送8fa027f1并复核。登记s1-tray-native1，原生探针运行正式
run16s --global，经托盘通知打开真实TrackPopupMenu，用键盘选择启用/停用，观察
自有UWP窗口；不使用测试导出，不把通知模拟算Explorer图标真实点击。管理员探针
需要系统UAC同意；不改产品实现。

tray-native1双宽编译成功，默认交互用例明确skip；管理员真实运行失败：图标rect
查询0、菜单初始unchecked，通过选择后未观察到UWP样式，控制器退出。尚无子进程
错误输出不能归因。登记s1-tray-native2，仅增加正式控制器stderr和退出码诊断。

tray-native2实测失败，但controller在清理前仍STILL_ACTIVE(259)，再次打开菜单
仍unchecked且stderr为空，说明输入尚未触发Enable，不能归因于接入。登记
s1-tray-native3，测试改用GetMenuItemRect+原生鼠标点击并恢复指针，保留上述负例。

tray-native3鼠标输入返回2但菜单仍unchecked，controller仍259、stderr空。尚未证实
命令触发，不算UWP接入失败。登记s1-tray-native4：DPI设置前移至任何窗口/COM之前，
增加WindowFromPoint命中及实际DPI awareness证据；这是探针修正，不改变产品。

tray-native4 DPI设置成功(awareness2)，但菜单项坐标WindowFromPoint命中其他HWND，
因此拒绝点击，未启用全局。登记s1-tray-native5：记录popup几何并按菜单窗口DPI
转换坐标；先以--menu-only普通权限只点Exit验证输入，不创建UWP或启用全局。

tray-native5只点Exit对照证实菜单为96DPI，其项目rect为逻辑坐标，popup物理rect
按当前144DPI放大；点转换API未成功。登记s1-tray-native6，测试在菜单自身DPI
上下文读取项目/窗口逻辑rect，再映射到原上下文物理rect并验证WindowFromPoint。

tray-native6菜单前台校验未通过，未发送点击。登记s1-tray-native7：改由原生
菜单消息队列选择Home/End+Enter，走真实TrackPopupMenu/action，不使用产品测试
命令；不宣称物理输入或Explorer图标点击通过，停止扩展输入自动化。

tray-native7向popup HWND发送按键未触发Exit；登记s1-tray-native8，最后对照原生
菜单owner HWND的消息路径，不将上述队列发送成功算作命令执行成功。

tray-native8 owner按键队列同样未执行Exit。登记s1-tray-native9，采用标准MSAA
IAccessible::accDoDefaultAction执行真实菜单项，不增加产品测试入口，不依赖焦点
或物理坐标；先仅Exit对照，失败仍不算产品回归。

tray-native9的MSAA Exit已通过。正式管理员Enable回调成功，已有Calculator21860
及新Scan15044均出现surface；checked菜单项MSAA执行返回80020003，撤销未通过，
Exit按原产品语义未恢复其他窗口。登记s1-tray-native10：通过同一菜单action使用
的request事件停用，验证owner身份、事件退出、surface销毁及菜单unchecked后Exit。
明确不将此算成勾选菜单项MSAA/物理点击已通过；失败清理也复用该受控停止路径。

tray-native10真实Calculator21476/Scan30972均换肤，request完成，两surface销毁，
controller退出0；夹具随后等待托盘菜单而失败。代码审查确认外部request未设置
托盘state.stopping，worker正常退出时controller按现有语义随之退出；不是Uncheck
后继续驻留。登记s1-tray-native11，仅修测试为正式Enable+外部shutdown契约，
明确Uncheck驻留UI未验证，不改变产品语义或冒称完整托盘端到端。

P38当前：native11双宽构建成功；默认交互测试各skip，不计运行通过。最终管理员
启动仍在等待UAC（尚无探针PID/日志），保持同一启动，不重发。已有native10实际
Enable、已有/新建UWP、外部停止及surface销毁证据；修正后的最终用例待执行。
src产品无diff，两处八文件SHA仍与P34一致。遵循用户新指示：UAC发起后监控，
只有超过1分钟未批准才提醒，不立即反复询问。

P38更新：上述同一次UAC最终返回取消，未创建探针（不得算pass）。native11最终
x64 menu-only通过；管理员Enable/external-shutdown最终复测尚未运行。工作保留
未提交，详情见研究记录P38。M0/T22/S1继续开放，无产品或部署变更。

P38最终复测通过：native11管理员probe39232启动正式controller15932，已有Calculator
12284及新Scan37796均换肤；request停止完成、两surface销毁、controller/probe退出0。
实际144DPI，图标rect查询0；start/stop各5条窗口通知失败，不宣称全桌面完全恢复。
默认双宽交互CTest仍skip，真实运行仅x64；Uncheck驻留、Explorer点击及启动器自身
UAC路径未验证。本轮仅测试/构建入口/记录，无产品变更，待提交推送复核。

P39诊断准备：P38提交cf807fad已推送复核。登记s1-menu-msaa1，先用自有原生
checked/unchecked标准菜单对照MSAA默认动作，不启用全局、不请求UAC，不改产品。

menu-msaa1原生unchecked/checked均返回invoke0、command1001。登记s1-menu-msaa2，
自有自定义顶层窗口同一二进制比较普通启动与正式run16s --wait启动，隔离样式因素。

menu-msaa2同一自有程序普通及run16s --wait启动均通过两个菜单状态；不凭此推断
正式托盘失败原因。登记s1-tray-toggle1，在现有实包探针加--toggle分支，记录真实
菜单类型及MSAA名称/动作/状态后执行Uncheck，验证controller驻留和surface销毁。

toggle1两UWP已换肤，但Uncheck菜单disabled=1，探针拒绝执行，清理使用正式stop
事件完成。登记s1-tray-toggle2：等正式session.ready后再核对菜单，不凭surface出现
推断全局启动完成。未改产品；也尚不能据此解释此前80020003。

P39待验证：toggle2修正两条编译缩进警告后双宽增量构建成功。管理员启动在等待
超过1分钟提醒后由系统返回取消，未产生actual.log或探针PID，不重复发起。
普通/样式自有菜单对照通过；真实toggle1发现菜单disabled=1，ready等待修正尚待
实测。未改产品或发布文件，P39工作保留未提交，M0/T22/S1开放。

P39审计续作：第二次toggle2管理员启动也由系统返回取消，未创建探针，不重发。
已复核发布截图、原始日志及P22至HEAD renderer无diff，形成[有限范围收口审计](../history/M0-T22-coverage.md)。
剩余真实托盘Uncheck/再次启用驻留验证；其余范围和限制逐项列明，不临时扩张。
本轮可交付审计文档，ready-aware测试及其receipt继续保留未提交，等待实际复测。

P40验证准备：登记s1-tray-window-events1，探针--window-events显式使用产品现有
非交互普通权限窗口发现路径，无UAC。只验证当前用户自有UWP/托盘Uncheck，
不声称管理员ETW或跨权限通过，不改变产品权限策略。

P40：登记s1-tray-window-events2，真实普通权限生产控制器39840对已有Calculator39528、
新Scan34520完成Enable、Uncheck驻留、再次Enable/Uncheck驻留及Exit0；144DPI，
原生MSAA菜单动作，精确surface销毁。session.ready等待修正了测试过早操作；
不将普通权限窗口发现算作管理员ETW或物理Explorer点击通过。两宽测试编译通过，
x86 Ninja缓存恢复警告后成功；真实运行x64。每轮92条通知失败如实保留，不声称
全桌面完全恢复。仅测试及记录变更，P34产品保持，两处摘要复核；M0/T22/S1开放。

P41验证准备：登记s1-tray-admin-repeat1，复用P40双宽已构建探针的--toggle，
验证管理员正式控制器两次启停和驻留。单次UAC启动，监控同一进程，不重复发起。

P41结果：同一次管理员启动最终返回Windows UAC取消，未创建探针PID或actual.log，
不能算测试通过；不重复发起。普通权限P40两轮真实菜单启停证据保持有效。
修正QUEUE/proposal仍称未发布的过时描述；产品未变，S1/T22开放，管理员重复启停
仍待系统授权实际完成。治理SelfTest13负例通过，文档交付后复核远端。

P42验证准备：用户确认现在方便批准UAC；登记s1-tray-admin-repeat2，复用P40已验证
探针（SHA256 256a3eb38b880034e4c8d0b1c49df141d4feba0c922a3d2e7e17acc76904ff0c），
执行--toggle管理员两轮启停。单次启动监控，不重复发起。

P42：管理员probe11248、正式controller10284，已有Calculator21548/新Scan34716，
144DPI，真实MSAA菜单两轮Enable/Uncheck、两次驻留和最后Exit全部通过（退出0）。
每次精确surface销毁；自有应用清理。start/stop各5条通知失败，未声称全桌面恢复。
先前UAC阻塞已解除。此次只记录最终证据，P34产品不变；交付复核后进行最终收口审计。

S2 实包验证 scratch：build/t23-package/s2-package1；使用 s2-clean2 同快照四文件与对应 clean-source，聚焦 demo-system-menu、startup-errors，保留其他任务改动。
S2 干净构建 s2-clean2 通过双位宽 ABI，生成同快照四文件；s2-package1 聚焦实包 10/10 通过、skip 0（不是完整矩阵，未验证真实 Task Manager）。clean-source 双位宽 launch/self-elevate-unit 与 core/frames 各通过；首次运行因未设置 WIN16STYLE_CAPTURE_DIR 在测试框架入口失败，指定自有目录后重跑通过。真实窗口接入、绘制、恢复仍未证明，未发布。
S2 clean-source 双位宽 Shell 句柄、无效目标及带句柄无效目标对照共 6/6 通过；该对照不验证 runas 成功句柄或 Task Manager。未发现残留的自有 self-elevate 探针进程。
S2 当前只读枚举未发现 Taskmgr.exe，登记 s2-live1 为候选标准命令自有实例测试；不操作其他实例。
S2 s2-live1 无旧 Task Manager 情况下候选 --wait taskmgr 新建 PID24412、启动器 PID26224；两分钟内未观察到后续实例或输出，未确认 UAC 结果，按身份核验后回收自有目标及启动器；不计通过。
S2 登记 clean-source/build/t23-package/s2-core1：候选同快照双位宽 Quick 核心矩阵，后台运行。
S2 同快照 Quick 矩阵 s2-core1：x86 5/6 通过，core/geometry 退出 0xc0000139（入口点未找到，原因未证实），脚本因此未运行 x64；矩阵未通过，需诊断，不得计为通过。
S2 s2-geometry-fix：core/geometry 链接的 GetWindowSubclass 在系统默认 comctl32 v5 不按名称导出；复用现有 control_extensions.rc v6 清单，双位宽增量验证登记于 s2-geometry-fix。
S2 s2-geometry-fix：复用 v6 清单后 geometry 双位宽各 1/1 通过；原 s2-core1 仍为失败记录。登记 s2-quick2 执行修正后的双位宽 Quick 矩阵。
S2 s2-quick2：修正测试清单后 Quick 矩阵通过，x86 6/6、x64 8/8，skip 0；这是增量工作树验证，不替代最终同快照干净构建。
S2 登记 s2-clean3：纳入 geometry 测试清单修复的同快照双位宽干净构建，后台运行。
S2 s2-clean3 同快照双位宽干净构建通过；登记 s2-package2 对该候选四文件做实包启动、菜单与传播聚焦回归。
S2 s2-package2：s2-clean3 同批四文件聚焦实包 10/10 通过、skip 0；physical input 未测，完整矩阵和真实 Task Manager 未完成，不视为交付通过。
S2 登记 s2-clean3-core：对 s2-clean3 干净快照直接执行双位宽 Quick 测试，不重新配置或重建候选。
S2 s2-clean3-core：干净快照 Quick x86 6/6、x64 8/8 通过；补充 self-elevate-unit、frames 双宽及 shell 六项共 10/10 通过。均不代表实际 Task Manager 提权、绘制或恢复通过。
S2 登记 s2-package3：干净候选普通双宽传播及系统菜单隔离实包回归。
S2 s2-package3 未通过：x86/x64 global-system-menu 启动全局控制器时 cannot create tray icon (2147500037)，隔离桌面无托盘结果未确认；普通传播用例未执行，不计通过。登记 s2-package4 单独测试普通传播。
S2 s2-package4：普通传播 x86/x64 和必需实包测试共 10/10 通过、skip 0；不覆盖 self-runas 真正提权或 Task Manager。s2-package3 全局托盘错误保留待排查。
S2 登记 s2-tray-baseline：私有桌面使用已发布 O:/winnt/run16s.exe 与自有目标，对照全局系统菜单托盘启动；不使用真实桌面。
S2 s2-tray-baseline：同一私有桌面测试用 O:/winnt 已发布启动器也得到 cannot create tray icon (2147500037)、target_exit=99；证明候选与发布版在该测试条件下均失败，不证明具体原因或真实桌面托盘回归。
S2 会话检查：query user 显示会话 1 为 Disc；s2-live1 的两个自有 PID 均不存在。本次无法观察可交互 UAC 与窗口；无证据确认此前超时由会话断开造成。
S2 s2-clean3 四文件 SHA 与 s2-package2 实包记录一致；check_package 原始资源表和来源声明校验通过。Windows 会话仍 Disc，真实目标等待会话恢复。
S2 会话已恢复 Active，未发现现存 Taskmgr；登记 s2-live2：干净候选 run16s --wait taskmgr 自有实例及有限等待。
S2 s2-live2：Windows 会话 Active，无既有 Taskmgr；候选启动器 PID25684 创建 Taskmgr PID352。约一分钟后仍无后继，稍后两者均已退出，无 stdout/stderr，未捕获退出码或 UAC 结果；未观测目标窗口，不能计通过。
S2 用户反馈 s2-live2 未看到 UAC；登记 s2-trace2，复用现有自有原生跟踪器跟踪 s2-clean3 进程创建、Shell 请求及退出，不操作现有进程。
S2 s2-trace2（用户反馈上次未见 UAC）：候选启动器 PID21992 创建 Taskmgr PID11252，加载 s2-clean3 ntw16s64.dll；90 秒内仅观察到隐藏 OleMainThreadWndClass，未捕获 ShellExecuteExW、主窗口或最终提权进程；探针结束后两个自有 PID 已不存在。跟踪器使用调试断点，不能排除跟踪影响，也不证明初始化或绘制。
S2 登记 s2-live3：无调试断点的自有候选启动和只读线程等待状态对照。
S2 s2-live3 无调试断点：候选 --wait 启动器 PID26200 创建 Taskmgr PID7572，90 秒内启动器未退出且没有 stdout/stderr，线程采样显示 UserRequest/EventPairLow 等待；无法据此确定具体等待对象或是否进入 Shell。超时后核验身份并清理自有进程。
S2 登记 s2-native-control：相同交互桌面无候选 DLL 的 Taskmgr 启动对照；仅处理自有实例。
S2 s2-native-control：直接启动 Taskmgr 首 PID25844 创建后继 PID20616；后继出现 TaskManagerWindow，但 visible=0。对照发现 Start-Process -WindowStyle Hidden 可影响目标显示，因此 s2-live2/3 无可见窗口不构成样式失败证据。后继按身份关闭。登记 s2-visible1 以正常窗口显示方式重测。
S2 s2-visible1：正常显示候选启动后 TaskManagerWindow PID3448 HWND0x210684 可见，用户截图证实滚动条已换肤而标题仍原生。只读管理员模块探针确认加载 s2-clean3/candidate/ntw16s64.dll SHA256=ebe2e38aec3a4cb89c7909d08e472a2a8b595ea02015fba44445ab12d1f21e97。144 DPI 客户区从外框顶部开始并含 XAML bridge；尚不能判断顶层附着失败还是应用覆盖绘制。用户正在观察该窗口，保留不关闭。
S2 s2-visible1 附着查询探针仅对自有 PID3448 的窗口线程查询 Ntw16sWindowAttached，临时消息和属性在结束时清理，不调用附着或移除。
S2 s2-visible1 顶层附着只读探针的管理员启动返回取消（1223），未创建探针或查询结果；不据此判断附着状态。用户反馈前次 UAC 未见，不能推断本次是否显示。
S2 用户反馈附着探针 UAC 未看到。静态审计：本机 Taskmgr.exe 同时导入 CreateWindowExW 和 CreateWindowInBand；后者调用点 RVA 0x134d40 传入 style=0x00cf0000，与可见目标的创建样式部分一致。当前窗口创建拦截只注册 CreateWindowExA/W，临时 CBT scope 又由这些调用包围；CreateWindowInBand 绕过是强相关假设，尚未运行时证明该调用创建 TaskManagerWindow，也未排除附着后覆盖。
S2 s2-visible1 静态反汇编：TaskManagerWindow 类名地址存入栈变量，随后原样作为 CreateWindowInBand 第二参数，style=0x00cf0000；与实际顶层类和样式一致。自有纯 C band=0 夹具经 s2-clean3 run16s 启动创建成功、候选 DLL 已加载、Ntw16sWindowAttached=0；这是创建入口遗漏的运行时负例，但不单独证明 Task Manager 顶层运行时附着状态。
S2 登记 s2-band-build1：CreateWindowInBand 复用现有创建期作用域的双位宽增量构建及自有夹具；保留此前候选。
S2 s2-band-build1：可选 CreateWindowInBand 复用现有创建作用域，x86/x64 构建与 ABI 通过；自有 band=0 原生/候选夹具双宽 2/2 通过，旧 s2-clean3 x64 负例 attached=0 被测试检出。来源摘要校验通过；尚未验证真实 Task Manager 外框、交互或恢复，不能宣布修复完成。
S2 登记 s2-band-build2：band 自有窗口增加移除后未附着的双位宽验证。
S2 s2-band-build2：双宽自有 band 窗口创建、附着、移除和移除后状态测试均通过；登记 s2-band-clean1 作同快照双位宽干净构建。
S2 s2-band-clean1 双位宽干净构建通过；登记 s2-band-package1 进行同批实包启动、传播、菜单回归。
S2 为避免任务管理器单实例复用，核验 PID3448、父 PID24940、出生时间及名称后关闭本任务上次创建的目标；登记 s2-band-live1 测试新干净候选。
S2 s2-band-package1：同批四文件聚焦 12/12 通过、skip 0；干净快照 band x86/x64 创建/附着/移除均通过。s2-band-live1 新真实实例首进程 PID25440 等待超过两分钟，未出现后继或输出；按身份核验清理，UAC 状态未确认，真实窗口未验证。
S2 登记 s2-band-visual2：私有桌面 band 自绘客户区夹具，验证可见状态的客户区偏移及移除恢复；不代表真实应用绘制。
S2 用户反馈 s2-band-live1 未见 UAC。s2-band-visual2：私有桌面双位宽真实 CreateWindowInBand 自绘客户区窗口，原生 top=0、样式后 top>0、移除后 top=0 均通过；这验证几何恢复，不证明像素或真实 Task Manager 通过。
S2 登记 s2-band-clean2：包含私有桌面几何与恢复夹具的同快照干净构建，后台运行。
S2 s2-band-clean2 双位宽干净构建通过；登记 s2-band-package2 同快照实包回归及 s2-band-clean2-tests 原生测试。
S2 s2-band-clean2：干净快照 band 创建/附着/几何恢复 x86/x64 通过，Quick x86 6/6、x64 8/8 通过；s2-band-package2 同批四文件聚焦 12/12 通过、skip 0。仍未验证真实 Task Manager 或发布。
S2 登记 s2-band-errors：双位宽 band 无效窗口类原生/候选错误码对照。
S2 s2-band-errors：无效窗口类的 native 与 styled 对照 x86/x64 均返回 NULL、ERROR_CANNOT_FIND_WND_CLASS=1407；创建、附着、移除和几何恢复的双宽夹具通过。
S2 登记 s2-band-live2：无已有 Taskmgr，使用 s2-band-clean2 正常显示标准命令 run16s taskmgr；只观测和回收自有目标。
S2 s2-band-live2：标准 run16s taskmgr 创建首进程 PID19308，超过 110 秒无后继窗口和日志，UAC 状态尚未确认；核验身份清理自有首进程，不计真实通过。
S2 登记 s2-band-print：私有桌面自有 band 窗口 WM_PRINT 非客户区像素验证及恢复。
S2 s2-band-print：私有桌面 x86/x64 band 窗口 WM_PRINT 非客户区均检测到黑、白、灰像素并通过附着/几何移除及错误码测试；该 WM_PRINT 证据不替代正常屏幕合成像素或真实 Task Manager。
S2 s2-band-live2 后只读桌面诊断：OpenInputDesktop 返回 Default，前台 HWND 非零；当前探针令牌为 Medium，Administrators 为 deny-only，当前无 consent.exe。该状态是事后采样，不能反推先前等待时是否出现 UAC。
S2 登记 s2-uac-observe1：实时只读观察标准候选启动时 Taskmgr/consent 进程及输入桌面；只清理自有目标。
S2 s2-uac-observe1：实时看到 consent.exe PID26348，OpenInputDesktop 返回5；用户确认“是”后，目标 Taskmgr PID8552 HWND0x10b0a74 正常可见，用户截图可见 Win16 标题，但标题下方出现疑似重叠/透明残影；144 DPI 只读几何 outer=(0,92,774,830)、clientorigin=(6,127)，XAML bridge=(6,127,768,824)。该结果证明可见外框，不证明交互、恢复或残影修复。
S2 s2-uac-observe1：尝试普通权限 PrintWindow 捕获真实 PID8552 的目标窗口返回 error=5，未生成截图；无法用本次捕获判断用户看到的标题下方残影是否持续。未操作或关闭目标。
S2 s2-uac-observe1 进一步截图：普通权限 PrintWindow 返回5；管理员截图探针启动返回取消（1223），没有截图结果，不能判断残影是否持续或推断 UAC 是否显示。用户截图和真实窗口几何仍是当前证据。
S2 styled 真实窗口 DWM 查询：NCRENDERING_ENABLED=0，USE_IMMERSIVE_DARK_MODE=1，SYSTEMBACKDROP_TYPE=2；原因未判定。为原生对照按 PID8552、父12884和出生时间核验后关闭自有样式实例。登记 s2-dwm-native。
S2 s2-dwm-native：原生真实窗口 DWM NCRENDERING_ENABLED=1、dark=1、SYSTEMBACKDROP_TYPE=2；候选窗口分别为0、1、2。背景类型两者相同，不能认定是残影原因；自有原生对照 PID8328 经身份核验关闭。
S2 登记 s2-screen-native：正常桌面自有原生 Task Manager 前台屏幕像素采样，只截取窗口矩形。
S2 s2-screen-native：自有原生 Task Manager 前台屏幕截图 native.bmp，顶部应用标题区域正常深色不透出其他程序；以身份核验后关闭对照目标。登记 s2-screen-styled 做同机候选对照。
S2 s2-screen-styled：同机候选标准启动时实时观察 Taskmgr PID23660 和 consent PID23424，普通进程访问输入桌面 error5；约两分钟后两者消失、桌面恢复 Default，未观察到目标窗口，UAC 结果未捕获，未生成候选截图。
S2 登记 s2-screen-auto：同批候选自有目标前台原生 BitBlt 截图，按 PID 和窗口类核验，不捕获其他区域。
S2 s2-screen-auto：候选首进程 PID23960 和 consent PID9660 出现，随后约两分钟内均消失；未发现候选目标，未截图，UAC 结果未捕获。前台窗口检查为零，不能从中推断残影。
S2 s2-screen-auto/watch：会话恢复后使用已登记自有窗口截图探针与 110 秒受限监控；只对本次启动后的前台 TaskManagerWindow 截取。
S2 s2-screen-auto/watch：用户恢复后自有候选最终 Taskmgr PID27148 出现，前台检测后 BitBlt 误采到非目标画面，已立即删除该截图；随后 PID27148 已退出。Application 日志查询未发现该时段 Taskmgr/ntw16s 崩溃记录；未捕获退出码，不能判断退出原因，也未验证残影。
S2 登记 s2-screen-auto2：截图前等待稳定并检查前后台和标题颜色，只在确认目标图像时写出文件。
S2 s2-screen-auto2：自有首进程 PID7500 与 consent 出现，110 秒内无后继；观察脚本超时写结果时因数字键 JSON 序列化失败，现已修正并将自有目标清理提前；复查首进程和启动器均不存在。本次无截图，UAC 结果未捕获。
S2 截图判定阈值离线负例：原生截图顶部蓝色像素334，用户提供样式截图17535；原阈值100错误接受原生，已改为窗口宽度3倍（原生阈值2322，用户截图阈值2316），并重建探针。该判定仅筛选，不证明截图无其他遮挡。
S2 登记 s2-band-clean3：纳入 band 像素、错误路径测试后的双位宽同快照干净构建，后台运行。
S2 s2-band-clean3 双位宽同快照干净构建通过，登记 s2-band-package3 及 s2-band-clean3-tests 执行实际包与核心回归。
S2 s2-band-clean3/tests：最新快照双位宽 band 创建、像素、恢复、失败路径通过；Quick x86 6/6、x64 8/8 通过；s2-band-package3 同批四文件聚焦 12/12 通过、skip 0。真实 Task Manager 残影和恢复未完成，尚未发布。
S2 用户回复 retry；登记 s2-screen-auto3，使用最新干净候选和稳定/像素校验的自有窗口截图探针。
S2 用户提供同一 Task Manager 标题时有时无及白块截图；前台目标 PID23388 的屏幕像素探针在标题应有区域多点读到 RGB(32,32,32)，和 Win16 蓝色标题不一致，复现不可见状态。随后目标退出，单次重绘探针身份检查未通过并未执行重绘；未捕获退出码，不能推断原因。
S2 登记 s2-repaint1：准备自有目标非客户区重绘前后像素统计探针，不保存屏幕图像。
S2 s2-repaint1：重绘前/300ms/1300ms 标题蓝色像素统计探针已编译；错误 PID 负例返回2，未触碰其他窗口。尚未在真实目标运行；交互会话长时间无输入，未重新弹出 UAC。
S2 用户要求 retry；登记 s2-repaint2，标准候选启动并在自有目标前台时自动做一次重绘像素对照，110秒上限。
S2 用户补充稳定复现：点击 hamburger 后 Win16 标题消失并变黑。登记 s2-repaint3，使用单次 BitBlt 批量像素读取的快速重绘探针对自有目标对照。
S2 用户补充：点击 hamburger 后标题消失；失焦或最大化可恢复绘制，但随后更正贴边不能恢复。上述为用户观察，自动对照待完成。
S2 用户更正：最大化后原本可见的标题消失、还原也不恢复；贴边也不能恢复，不再将这些视作恢复手段。
S2 s2-repaint3 管理员重绘对照：重绘调用成功，前/300ms/1300ms 蓝色像素均8693；该次标题本来可见，不覆盖消失状态。
S2 s2-repaint3：管理员重绘在标题可见状态前后均8693蓝像素，未覆盖汉堡触发；新增自有目标坐标和PID核验、恢复鼠标位置的一次点击探针，错误PID拒绝。
S2 s2-repaint3 自动点击：目标前台时标题蓝像素8693，点击 hamburger 后95；RedrawWindow 成功后300ms与1300ms均8693。此证明一次重绘可恢复至少1.3秒，但触发消息尚未定位。
S2 登记 s2-repaint-prototype1：自有目标窗口线程临时探针仅在顶层 WM_ERASEBKGND 后投递 WM_NCPAINT，验证异步重绘是否能避免汉堡后覆盖；探针不作为产品实现。
S2 s2-repaint-prototype1：WM_ERASEBKGND 后直接 PostMessage(WM_NCPAINT) 原型未令标题在600ms内恢复（clicked=95），主动 RedrawWindow 后恢复；不能把该原型作为修复。
S2 登记 s2-repaint-prototype2：自有窗口短时延迟合并重绘原型；仅验证，不引入产品或轮询。
S2 s2-repaint-prototype2：基于顶层 WM_ERASEBKGND 的异步合并重绘仍未在600ms内恢复（clicked=95）；手动重绘后恢复。后续失焦使最后一帧不可比较，不能将该原型作为修复。登记 s2-repaint-prototype3 追踪自定义消息是否到达。
S2 s2-repaint-prototype3：顶层 WM_ERASEBKGND 后注册消息合并原型未在600ms内恢复，且记录未见注册消息送达；主动重绘仍恢复。尚未采用。登记 s2-repaint-prototype4 检验擦除返回后的同步重绘。
S2 s2-repaint-prototype4 对照无效：驱动仍安装 WH_CALLWNDPROC，但回调读取 CWPRETSTRUCT，导致未获得消息；像素仍仅证明手动重绘可恢复。登记 s2-repaint-prototype5 纠正钩子类型。
S2 s2-repaint-prototype5：纠正 WH_CALLWNDPROCRET 后，目标 WM_ERASEBKGND 返回后同步 RedrawWindow 原型使点击前、点击后600ms和重绘后1300ms蓝像素均8693；真实目标一次正例，不证明全部XAML或最大化。
S2 登记 s2-xaml-repaint1：将验证过的擦除后同步重绘限定于带 XAML composition bridge 的顶层窗口，增加自有宿主和重入边界测试；尚待双宽及真实目标验证。
S2 登记 s2-xaml-repaint2：XAML 宿主擦除后同步重绘的双位宽正例、普通窗口负例和重入边界；仅为增量测试，真实应用与菜单回归未完成。
S2 s2-xaml-repaint2：双宽宿主正例、无宿主普通窗口负例及重入界限通过；为新候选实测核验并关闭自有 PID19516。登记 s2-xaml-candidate1 为增量实测包。
S2 登记 s2-xaml-live1：新增量候选标准 run16s taskmgr 真实窗口多次操作对照。
S2 用户确认 hamburger 和最大化标题保持，但标题下有残影且缩放时闪烁；登记 s2-backdrop1，仅对自有实例暂时更改单窗口 DWM 背景属性并立即恢复，检查残影关联。
S2 s2-backdrop1：真实候选窗口的 SYSTEMBACKDROP_TYPE 由2临时切为1并成功恢复2；前后原生屏幕截图中白块仍在且关闭背景后部分区域透出更多文字，故不采用背景属性调整。用户另报告拖动缩放时标题闪烁，须查重复绘制。
S2 登记 s2-ncpolicy1：对自有候选窗口临时启用 DWM 非客户区渲染后截图，并恢复本项目原先显式禁用状态；只作机制对照，不作为产品方案。
S2 s2-ncpolicy1：自有目标临时启用 DWM 非客户区渲染后白块仍存在，恢复显式禁用返回成功；标题形状变化是预期对照，不能把白块归因于 DWM NC 开关。
S2 s2-ncpolicy1 白块区域只读 WindowFromPoint：左侧命中 TaskManagerWindow，右侧命中 GlassWindow；不能认定为普通编辑框。登记 s2-resize-trace1，针对自有目标一次放大还原统计消息。
S2 s2-resize-trace1：自有真实目标一次扩大并还原，标题像素保持且尺寸已恢复；顶层窗口在两次尺寸变化期间产生多次 WM_ERASEBKGND / WM_PAINT / WM_NCPAINT（日志保留），支持同步强制全窗重绘可能引入缩放重复绘制，但此对照不等于真实拖拽闪烁的视频证据。
S2 登记 s2-xaml-coalesce1：按宿主背景擦除合并30ms非客户区重绘，移除/销毁取消；双宽夹具验证突发擦除、普通窗口负例与重入，真实窗口和缩放仍待测。
S2 登记 s2-xaml-coalesce2：补充待处理重绘时移除窗口样式的计时器清理验证。
S2 s2-xaml-coalesce2 双宽通过：五次擦除合并、普通窗口负例及移除计时器清理；为实测经身份核验关闭自有 PID16528，登记 s2-coalesce-candidate1。
S2 登记 s2-coalesce-live1：标准候选启动，核对汉堡菜单、最大化和真实缩放绘制。
S2 s2-coalesce-live1：候选首进程 PID28896 和 consent PID13748 出现，约两分钟后均消失，未捕获 UAC 结果或最终目标；重绘合并真实窗口回归未运行，用户此前报告白块及缩放闪烁仍未解决。
S2 用户要求重试合并重绘候选；登记 s2-coalesce-live2，自动观察自有目标与标题截图，110秒上限。
S2 s2-coalesce-live2：用户要求 retry 后，合并重绘候选最终 Taskmgr PID22652、父21732 可见，原生前台采样证实 Win16 标题存在，但标题下方能看到其他窗口内容透出，残影仍复现；截图含非目标背景内容，已删除原图。汉堡/最大化/缩放尚未完整回归。
S2 登记 s2-coalesce-resize：复用自有目标 PID22652 的单次扩大还原和只读附着查询；根窗口已附着，查询到的 GlassWindow、FilterControlGlassWindow 和 composition bridge 未附着。顶层非客户区重绘 4 次，原同步候选同类测试为 20 次；不代表连续拖动无闪烁。
S2 登记 s2-client-refresh：对同一自有目标进行一次完整客户区重绘前后有界像素差异采样，不保存屏幕图像；用于检验残影是否只是未失效的客户区。
S2 s2-client-refresh：首次未在前台，未执行；第二次自有目标前台，含子窗口的完整 RedrawWindow 返回成功，500ms 后标题下方采样区域 50310 像素中仅 20 个变化。该次对照不支持简单的失效区域遗漏，亦不能独立证明透明成因。
S2 登记 s2-client-occlusion：在自有目标后短暂放置自有纯色窗口，采样目标标题下方纯色像素数量后立即销毁；不保存其他窗口图像，以验证是否存在底层画面透出。
S2 s2-client-occlusion：首次目标区域纯色采样为零，但没有背景可见的正对照，结论不充分；补正对照的第二次因目标未在前台而未执行，不据此确认或排除透明。
S2 s2-client-occlusion 正对照重试：目标标题下方纯色像素前后均为0，相邻正对照区域3900像素均呈纯色，目标保持前台并自动销毁背景窗口。该次不支持标题下方实时透出紧邻其后的窗口；旧画面残留与应用合成内容仍待定位。
S2 s2-client-occlusion 补测采样区域平均 RGB 前后均为(36,37,37)，正对照3900纯色像素；不支持整条采样区域存在可观的即时颜色透出。该汇总不能排除局部透明或合成滤镜，未解决残影。
S2 登记 s2-bridge-resize：仅对自有任务管理器的 XAML composition bridge 临时改变宽度一像素并立即还原，比较标题下方的采样变化；验证合成表面尺寸变化能否刷新残影，不保存截图。
S2 s2-bridge-resize：改变并恢复调用均成功，最终尺寸与原值相同；但结束时目标不在前台，像素对照无效，不能据此判断残影。
S2 登记 s2-coalesce-hamburger：自有候选目标前台时复用单击探针，采样单击后600ms的标题颜色，随后恢复鼠标并进行手动重绘对照；不操作其他目标。
S2 s2-coalesce-hamburger：第一轮等待前台超时，未点击；第二轮系统提权等待超过一分钟后返回取消，未创建单击探针日志，不计通过或失败。
S2 文档和源码审计：frame_message.c 处理 WM_NCCALCSIZE 调用原过程后不保留其返回值；Microsoft 文档说明 TRUE 参数返回0会保留并左上对齐旧客户区。是否造成真实残影尚无证据。登记 s2-nccalc-fixture，以自有窗口对照重算时返回标志与旧像素保留，不修改目标应用。
S2 s2-nccalc-fixture：私有桌面双宽原生窗口同样扩大20像素时，返回0仅使新增20像素区域失效；返回 WVR_REDRAW 使304像素客户区全宽失效。首次 x86 编译因使用错误目录失败，改用项目已配置的 x86 GCC 后通过。该夹具证明返回值影响无效区域，不证明 Task Manager 的原始返回值或残影原因。
S2 登记 s2-nccalc-return：对模拟 XAML 宿主检查应用要求 WVR_REDRAW 时安装外框前后返回值，先复现再决定最小修正；不将此夹具结果等同真实 Task Manager。
S2 s2-nccalc-return x64 增量负例证实原生返回 WVR_REDRAW、安装外框后返回0。候选仅传递原过程明确请求的 WVR_REDRAW 位；VALIDRECTS 使用原几何，仍不传递，避免错误复用。登记 s2-nccalc-fix 双宽增量回归；真实应用返回值和残影关联未证实。
S2 s2-nccalc-fix 双宽外框回归和 ABI 检查通过；登记 s2-nccalc-fix2 补测水平、垂直和 VALIDRECTS。s2-nccalc-fix2 双宽外框回归和 ABI 检查通过，均为自有夹具，尚未验证真实目标或发布。
S2 登记 s2-nccalc-candidate：此前候选启动器/demo 与增量新双宽 DLL 组成的开发验证包，四文件摘要已核对；不是干净同快照交付包，未发布。
S2 登记 s2-nccalc-live：对经身份核验的自有旧候选目标关闭后，以管理员探针启动增量候选并核验真实外框和汉堡操作；此为提升后运行时对照，不替代标准命令的完整交付验证。
S2 登记 s2-nccalc-private：针对增量候选运行私有桌面 demo 系统菜单及 x86/x64 普通传播聚焦回归；不计作正式同批交付测试。
S2 s2-nccalc-private：demo 系统菜单及普通传播 x86/x64 三项均退出0；为增量包聚焦回归，不覆盖完整矩阵。s2-nccalc-live 同一次系统提权等待超过一分钟后返回取消，管理员 worker 未启动；旧测试实例未由该 worker 关闭，真实目标和残影不计通过。
S2 s2-nccalc-live 用户重试后，已关闭旧目标；提升后启动器 PID31112 创建真实 Taskmgr PID1924，加载增量候选 ntw16s64.dll SHA256=75149e3616be970e4de69139d1b562f458321096a171b7070ee8943b82ace259。单击探针命中启动器的可见 ConsoleWindowClass 而非目标，返回5，未点击；标题、残影和交互不计通过。登记 s2-nccalc-unobscured，临时隐藏该测试自有控制台并恢复。
S2 s2-nccalc-unobscured：临时隐藏自有启动器控制台后，目标前台且命中 PID1924；真实单击前和600ms后标题蓝色像素均8693，手动重绘后与1秒后亦8693，控制台已恢复。证明本次候选单次汉堡操作标题保持，不证明连续缩放或残影修复。
S2 s2-timer-queued x64 负例：WM_TIMER 已排队后移除样式，应用窗口过程接收到内部计时器，原测试失败；须在交付前消除消息泄漏并考虑排队回调与 DLL 卸载生命周期。
S2 登记 s2-nccalc-visible：只截取当前自有候选 Taskmgr 前台窗口以检查残影，临时隐藏并恢复自有控制台；如包含非目标内容，提取结论后删除截图。
S2 s2-nccalc-visible 截图探针返回2，目标不在前台，未产生截图；不据此判断残影。登记 s2-timer-isolated：将延迟重绘计时器隔离到内部消息窗口，验证排队后移除及双位宽回归；仍需真实应用测试。
S2 s2-timer-isolated 双宽外框和 ABI 回归通过；登记 s2-timer-destroy 并验证排队消息后移除/销毁根窗口，内部消息窗口均销毁、应用过程未收到该计时器；双宽测试与 ABI 通过。该改动尚未在真实 Task Manager 测试或发布。
S2 登记 s2-timer-unload：自有进程直接加载相应位宽 DLL，制造已排队计时器后移除样式并卸载 DLL，再派发队列，验证无过期回调；不涉及远程进程或物理卸载产品注入。
S2 s2-timer-unload 私有桌面双宽原生测试均返回0：样式移除后内部消息窗口已销毁，DLL 实际卸载，再派发队列未崩溃且根窗口未收到内部计时器；只覆盖本地直接加载接口，不代表远程注入的物理卸载。
S2 登记 s2-timer-candidate 和 s2-timer-live：增量测试包包含新计时器隔离 DLL；真实管理员目标重测将核验新 DLL 摘要、前台与单次汉堡操作，不作为最终同批交付。
S2 s2-timer-live：管理员启动器 PID25188 创建自有 Taskmgr PID26904，实际加载候选 ntw16s64.dll SHA256=727429b4ded0485a2bb85428be3968164f139bdf1f6c5c3f398413cf2159d808。临时隐藏自有启动器控制台后，单击前、后600ms、主动重绘后及1秒后标题蓝像素均8693；控制台已恢复。此为单次汉堡正例，不证明残影、连续缩放或普通权限启动路径。
S2 登记 s2-timer-visible：对上述自有目标前台截图核查残影，排除并恢复测试控制台遮挡；检查后删除包含非目标内容的图像。
S2 s2-timer-visible：前台截图标题蓝像素17487，标题下方仍显示非目标界面片段；确认 WVR 标志透传及计时器隔离未单独修复残影。截图含非目标内容已删除。登记 s2-nocopy：自有目标上一次不移动不缩放的 SWP_FRAMECHANGED|SWP_NOCOPYBITS 对照，前后临时截图核查并删除。
S2 s2-nocopy 管理员启动在等待后返回取消，worker 未启动，未生成截图；对照未执行。登记 s2-timer-topology：仅读取现有自有目标的 DPI 感知窗口层级，以定位残影与客户区边界。
S2 s2-timer-topology：144 DPI 时顶层 outer=(28,150,802,877)、client=(34,185,796,871)，可见主 composition bridge 填满客户区；GlassWindow 与内层 bridge 到屏幕 x=517。对照 s2-timer-visible 截图中右侧残图片区约为窗口相对 x=492..770，与该内层右边界接近；窗口位置已变化，此为跨时刻近似比较，不能单独认定残影由该边界导致。
S2 s2-nocopy 当前权限尝试：前后截图采集成功，但 SetWindowPos 返回 FALSE，未捕获错误码；不计重算对照，不能断言失败原因。两张临时截图已删除。
S2 登记 s2-residue-hit：只读采样自有目标标题下方多点的实际屏幕窗口和进程，检查外部置顶窗口遮挡；仅当前台身份匹配时使用结果。
S2 s2-residue-hit：目标前台且截图再次显示同一残图片段；在标题下方45/55/70/85像素高度，多点命中均为目标 PID26904 的 TaskManagerWindow 或 GlassWindow。与前次窗口不同屏幕位置的截图相比，残图片段仍处于相同窗口相对位置，支持内容被保留而非简单屏幕位置残留；WindowFromPoint 无法排除透明命中穿透，因而不作为完整根因证明。含其他界面片段的临时截图已删除。
S2 s2-nocopy 当前权限补测：截图脚本因目标不在前台停止；独立只记录返回值的探针对自有 PID26904 取得 SetWindowPos=FALSE、GetLastError=5。该权限下未执行重算，真实 SWP_NOCOPYBITS 效果仍未验证。
S2 s2-nocopy 管理员重试：自有目标 SetWindowPos(SWP_FRAMECHANGED|SWP_NOCOPYBITS) 返回 TRUE；首次后截图因前台变化失败，随后临时隐藏测试控制台、确认目标前台补拍，残图片段仍在。前后截图均已删除；该方案未解决真实问题，不引入产品。
S2 登记 s2-bridge-visibility：在自有目标中短暂隐藏再显示原本可见的主 composition bridge，核验句柄、进程和恢复状态，以区分可见性重建与普通客户区重绘；此为诊断，不作为产品方案。
S2 s2-bridge-visibility：主 bridge 短暂隐藏并恢复可见，前后目标均为前台且标题蓝像素17487；残图片段仍在。前后临时截图已删除；不采用该动作作为修复。
S2 登记 s2-print-client：对自有目标使用真实 PrintWindow 客户区采样，与正常桌面截图对照；区分应用绘制与屏幕合成，失败不计结论，含非目标内容图片核验后删除。
S2 s2-print-client：管理员 PrintWindow(PW_CLIENTONLY|PW_RENDERFULLCONTENT) 返回 TRUE、客户区762×686，结果顶部同样含相同非目标界面残图片段；因此该现象也出现在目标窗口打印结果，不只是截图时可见置顶窗口遮挡。PrintWindow 可使用合成内容，不能仅凭本次结果断言具体渲染层。含非目标内容图像已删除。
S2 登记 s2-frame-toggle：对自有 Taskmgr 窗口在其 UI 线程临时移除 Win16 外框、采样，再立即恢复附着；记录移除/恢复返回值和窗口身份，比较残影并验证恢复。不会操作其他实例或持久修改系统。
S2 s2-frame-toggle：真实 PID26904 上 Ntw16sWindowRemove 和再次 Ntw16sWindowAttach 均成功，状态由附着→未附着→附着；PrintWindow 显示移除及再附着后原有非目标界面残影消失。随后自有控制台遮挡排除、目标前台 BitBlt 也未显示该残影，但应用图标区域出现重复，不能计视觉完全通过。截图已删除。该结果支持附着时机或状态转换相关假设，尚不能确定具体原因或支持自动重附作为修复。
S2 创建时机源码对照：隐藏的普通目标在创建期安装时 pending_frame=TRUE，首次 WM_SHOWWINDOW 才激活外框；已经显示的再次附着则立即激活。实际 Task Manager 的首次 WM_SHOWWINDOW、XAML bridge 创建和首次合成相对顺序尚未实测，不能把此差异直接认定为根因。
S2 登记 s2-creation-timeline：只对新建自有 Taskmgr 记录 WinEvent 窗口创建/显示事件与候选启动顺序；事件只反映可访问窗口事件，不等同内部 WM_SHOWWINDOW 或首次合成。
S2 s2-creation-timeline：管理员启动自有 Taskmgr PID31728，记录根窗口 CREATE=140ms、bridge CREATE=281–375ms、根 SHOW=875ms、bridge SHOW=1437–1468ms；时间为事件回调相对启动的观察值。说明桥接窗口显示晚于根显示，不等同实际 WM_SHOWWINDOW 或首次合成时间。
S2 登记 s2-late-frame-prototype：自有普通 Taskmgr 先按原生方式显示，等直接 composition bridge 的 SHOW 事件后仅在其 UI 线程附着候选 Win16 外框；对照首屏残影、交互和恢复，不改产品启动行为。
S2 s2-late-frame-prototype：自有原生启动 PID7864 在直接 bridge SHOW 后由目标 UI 线程附着，成功加载 s2-timer-candidate ntw16s64.dll SHA256=727429b4ded0485a2bb85428be3968164f139bdf1f6c5c3f398413cf2159d808。PrintWindow 与真实前台屏幕采样均未见先前非目标界面残影，标题蓝像素17487；客户区两侧仍有细白条。汉堡单击前/后蓝像素均8693；一次扩大/恢复蓝像素8693/9233/8693；一次最大化/恢复状态正常、蓝像素8693/16979/8693。截图已清理。此仅为晚期附着原型，未覆盖标准 run16s 启动、连续拖拽和所有 WinUI 应用。
S2 登记 s2-defer-build：候选在隐藏创建阶段保留原生外框消息；首次显示时若已存在 XAML bridge，等待直接 bridge 可见再激活，最长2秒后回退激活。此代码尚未验证，先做双宽回归和真实创建时序夹具，不据原型声称产品通过。
S2 s2-defer-build 双宽 core/frames 和 ABI 检查通过。s2-defer-fixture 双宽真实启动注入夹具通过：隐藏 bridge 期间原始客户区顶部0、显示后25、移除恢复0；普通窗口立即应用25，隐藏/再次显示、2秒回退及待处理定时器销毁均通过。登记 s2-defer-candidate 和 s2-defer-live：仅用于真实 Taskmgr 启动对照，非交付包。
S2 s2-defer-live：管理员启动器 PID28332 创建 Taskmgr PID31172，加载候选 ntw16s64.dll SHA256=23e2b8580095e13eb6aa0d1ffed3c348f46a82284752b943e239ecd182c2f9bd。首屏真实截图中未复现原外部界面残影，但仍有图标重复及右侧细白条，临时截图已删；第一次点击后失焦，采样无效。s2-defer-interaction 遮挡排除后重测，目标持续前台，汉堡单击前后蓝像素均8693。登记 s2-defer-motion 做一次扩大及最大化恢复；登记 s2-defer-tests 将自有夹具纳入项目并测试。
S2 s2-defer-tests：项目内 launch/deferred-frame-x86/x64 两项通过，覆盖实际注入、XAML 等待/显示、普通窗口、隐藏恢复、2秒回退和待处理窗口销毁。s2-defer-motion 系统提权等待超过一分钟后返回取消，运动对照未执行；不计通过。
S2 s2-defer-private：新增量候选私有桌面 demo 系统菜单与普通传播 x86/x64 三项均退出0；本地 source manifest 校验通过。以上非干净同批交付测试。
S2 登记 s2-defer-nocopy：针对自有延迟激活候选 PID31172，比较无复制重算前后 PrintWindow 客户区中的重复图标及细白条；不改变位置尺寸，截图提炼后删除。
S2 s2-defer-nocopy：目标处于汉堡菜单展开状态，SetWindowPos(SWP_NOCOPYBITS|SWP_FRAMECHANGED) 成功，前后 PrintWindow 均成功；右侧白条及顶部裁切图标仍在，未解决。截图已删除。此不能排除其他显示状态的差异。
S2 s2-defer-motion 管理员重试：自有 PID31172 在前台，一次扩大/恢复标题蓝像素8693/9233/8693，一次最大化/恢复状态正常、蓝像素8693/16979/8693。运动前菜单展开、之后菜单收起；之后 PrintWindow 白条未见，但顶部图标仍重复，无法将白条消失归因于某次运动。截图已删除；连续拖动和闪烁视觉仍未验证。
S2 登记 s2-activation-nocopy：仅对存在直接 XAML bridge 的目标，在外框首次激活改变客户区几何时尝试 SWP_NOCOPYBITS；此前对已完成激活窗口的事后重算无效，此次检验旧像素复制发生时机。未证明有效，若真实对照无改善则撤回候选。
S2 s2-activation-nocopy 双宽 core/frames、ABI 与 deferred-frame 夹具通过；真实 Taskmgr PID10068 加载原型 DLL SHA256=c16a1dd0f03adb167d032ff32ac26e275cb1ff6285ff4d229acf5ac4f53e644d，标题持续可见，但首屏出现额外的整条残留标题栏，菜单操作后仍局部存在。原型比先前更差，frame_layout.c 已原样撤回；截图已删除。此结果不作为交付。
S2 s2-activation-revert：撤回几何原型后双宽增量 core/frames 和 ABI 检查通过，source manifest 校验通过；PID10068 是仍在运行的自有失败原型目标，后续测试前按身份清理。
S2 登记 s2-layer-target 和 s2-layer-capture：重启自有延迟激活候选后分别采样根客户区和可见合成子窗口，定位重复图标属于哪个绘制层；只用于研究，临时截图核对后删除。
S2 s2-layer-target：自有 Taskmgr PID32472 已创建；自动点击探针命中其他 PID 后拒绝点击，worker 无完整结束报告，不计交互通过，临时截图已删除。s2-layer-capture 使用同一目标的 PrintWindow：根、可见主 composition bridge、GlassWindow 均返回成功；主 bridge 结果本身包含重复图标及右侧白条，GlassWindow 采样为黑色。PrintWindow 可包含合成后内容，说明现象已存在于 bridge 采样结果，不能据此断言精确的 XAML 内部元素。图片已删除。
S2 登记 s2-late-layers：对同一新建自有原生 Taskmgr 在可见 bridge 显示后，采样附着前客户区，再在同一 UI 线程附着并采样；对比重复图标与边缘细条。临时截图核查后删除，不代表标准启动路径验证。
S2 s2-late-layers：PID7536 的首个 bridge SHOW 后立即采样时，原生客户区仅出现图标，主要内容尚未绘制；随后附着成功，500ms 后出现主内容和顶部裁切图标、边缘白条。该对照说明 bridge 可见不等于应用首屏已完整绘制，但不同时间状态不能独立解释重复图标；图片已删除。登记 s2-settled-layers，在同样的自有原型中等待1秒后再附着以作对照，不将固定延时作为产品方案。
S2 s2-settled-layers：自有 PID18540 的 bridge SHOW 后等待1秒，附着前 PrintWindow 已显示完整主内容及单个图标；随后 UI 线程附着成功、500ms 后 PrintWindow 出现原图标及顶部裁切的第二份图标。两次采样为同一目标、但探针未设置 DPI awareness，不能用于精确坐标比较；结果不支持简单延迟1秒消除重复，不引入固定延时。图片已删除。
S2 登记 s2-xaml-outer：对已有直接 XAML bridge 的窗口，原型在附着与移除时保持外框位置尺寸，让客户区重新布局；用于验证原有“保留客户区屏幕位置”导致的几何移动是否造成图标残影。普通窗口保持原路径；效果不佳则撤回。
S2 s2-xaml-outer 双宽 core/frames、ABI 与 deferred-frame 夹具通过；真实自有 Taskmgr PID33444 加载原型 DLL SHA256=4086be5bd68ae8a87be3c6466e5fbf2ad559f4e1a567e682bde3349f079836d9，PrintWindow 仍出现顶部重复图标及右侧白条。保持外框尺寸未解决问题，frame_layout.c 与相应试验断言已撤回，截图已删除；此原型不交付。
S2 s2-xaml-outer-revert：撤回几何原型后双宽 core/frames、ABI 和 source manifest 通过。PID33444 为自有失败原型目标，后续按身份清理；图标和白条仍待定位。
S2 登记 s2-caption-edge：真实候选 PID33836 的 DPI 感知几何为 outer.top=-35、client.top=0，标题被移出屏幕；构造原生客户区顶到工作区边缘的 XAML 窗口，先验证附着后标题可见与移除后外框恢复。
S2 s2-caption-edge 负例复现顶部越界；修正只在原本位于工作区内的 XAML 外框将被接管动作推过顶部或左侧边界时保留外框几何，移除时保持对应外框。s2-caption-edge-final 双宽顶部/左侧和恢复夹具、core/frames 与 ABI 通过；s2-caption-edge-injection 双宽延迟创建注入回归通过。登记 s2-caption-edge-live：自有真实 Taskmgr 原生窗口先置于工作区顶部，再在 UI 线程附着当前候选并核对实际外框位置。
S2 s2-caption-edge-live：自有原生 Taskmgr PID4236 已验证加载候选 DLL SHA256=bb7607bdf1c77a50181029761d501267ff5b5232eb89a7c94fa0aee505313ec1；144 DPI 工作区顶部0，原生外框定位 top=0、UI线程附着后 top=0，未再被推至负坐标。客户区打印仍有顶部重复图标，截图已删除；这是晚期附着原型，标准启动、实际屏幕标题及真实移除恢复仍需验证。
S2 登记 s2-caption-edge-remove：对上述自有目标在 UI 线程移除并重新附着，核验 DPI 感知的前/移除/再附着外框坐标；不操作其他目标。
S2 s2-caption-edge-remove：真实自有 PID4236 的移除和再次附着均成功；144 DPI 外框前、移除后、再附着后均为(976,0,1750,937)，未出现标题越界或外框漂移。此覆盖工作区顶部的晚期附着与恢复，不代表标准启动、多屏或跨 DPI 全覆盖。
S2 登记 s2-xaml-dwm-initial：已知原生目标 DWM 非客户区启用、当前首次附着会禁用；仅对存在直接 XAML bridge 的目标原型保留原 DWM 状态，检查首次状态切换是否造成图标重复。若外框或效果不正确则撤回，不改变系统级设置。
S2 s2-xaml-dwm-initial 双宽 core/frames、ABI 与 deferred-frame 夹具通过。真实目标 PID33888 加载原型 DLL SHA256=a69383337692aa1c293aebb119286fa4db26bb0fa67179c5177e4a09d10ba9e6；首次屏幕采样误采其他窗口，已立即删除，不计结果；经身份校验的 PrintWindow 客户区仍有重复图标与右侧白条。原型未改善，frame_layout.c 已撤回，客户区临时截图已删除；非客户区实际绘制未获有效证据。
S2 s2-dwm-revert 双宽 core/frames 和 ABI 通过。登记 s2-capture-guard：屏幕采样前后检查九个点所属进程和根窗口；原私有桌面 WindowFromPoint 对照因不可见返回0，不计通过。纯 C 模型验证无覆盖、同进程其他窗口和异进程覆盖的分支，均符合预期；采样不能保证发现所有遮挡或透明穿透，真实目标尚未验证，优先使用身份校验的 PrintWindow。
S2 s2-defer-focus-before2 双宽注入负例：等待 XAML bridge 时收到 WM_NCACTIVATE 后，激活的标题像素为0；已在等待期间保留活动状态，并在 WM_DPICHANGED 后刷新字体但不提前改变外框。s2-defer-focus-final 双宽注入夹具验证活动/非活动标题及既有延迟、清理场景通过，双宽 core/frames 与 ABI 通过。尚未完成真实跨 DPI 变更验证及新候选实包测试。
S2 登记 s2-timer-reentrant：验证应用在内部计时器窗口创建过程重入销毁根窗口时，是否遗留持有已释放状态的消息窗口；先做私有桌面负例。
S2 s2-timer-reentrant4 证实根窗口在 CBT 创建回调中被销毁后，内部消息窗口仍存活；已在创建及子类化后检查脱离状态并销毁未完成窗口。s2-timer-reentrant-fix 双宽 core/frames 和 ABI 通过；s2-reentrant-injection 双宽实际启动注入的同一重入场景及延迟激活全套夹具通过。未重新实测真实 Task Manager，也未发布。
S2 登记 s2-nocopy-fixture：私有桌面自有窗口对照 FRAMECHANGED 与 NOCOPYBITS 在不变尺寸和扩大尺寸时的客户区失效范围；不替代真实 XAML 合成验证。
S2 s2-nocopy-fixture 双宽原生对照一致：不变尺寸且无 NOCOPYBITS 时无客户区重绘，有 NOCOPYBITS 时全客户区失效；扩大20像素时无该标志仅新20像素失效，有该标志则全宽失效。此前真实目标完整重绘未消除残影，所以不能据此推断该标志能修复 XAML 合成残影，也尚未变更产品位置标志。
S2 登记 s2-timer-injection：使用同一增量候选 run16s 及双位宽自有私有桌面窗口，验证实际创建钩子路径中 XAML 计时器窗口的创建、排队、移除与回收；不声称真实 WinUI 视觉通过。
S2 s2-timer-injection：x86/x64 实际启动注入均显示根窗口和内部 STATIC 消息窗口被附着；移除根样式后内部窗口销毁、应用未收到计时器，两项退出0。s2-timer-class 专用类别对照仍被附着，尝试未解决额外附着，已撤销并保留系统 STATIC；s2-timer-final 双宽外框和 ABI 回归通过。消息窗口会获得一次额外附着，已验证销毁回收；真实目标完整生命周期仍需验证。
S2 登记 s2-timer-queued：在私有桌面构造重绘计时器消息已排队后移除样式的情况，验证内部计时器是否泄漏到应用窗口过程。
S2 s2-caption-edge-screen：用户关闭旧任务管理器后，对先前自建 PID4236 重试管理员屏幕探针，身份和前台检查通过；实际标题蓝色像素17487（阈值2322），但标题下方仍有重复图标。截图提炼后删除。s2-defer-tests 增量双位宽实际注入与工作区边缘附着/移除检查通过；不代表真实启动或连续拖动视觉通过。
S2 登记 s2-defer-nocopy/bridge-refresh：对已核验身份的自建 PID4236 的直接 XAML bridge 暂时缩窄一像素后恢复，前后采样客户区，检验合成层尺寸刷新是否消除重复图标；只操作该窗口，截图检查后删除。
S2 s2-defer-nocopy/bridge-refresh：枚举找到 PID4236 的可见最大直接 XAML bridge，宽752高926；临时缩窄1像素及恢复均成功，前后客户区截图的顶部重复图标仍在。该对照不支持用子窗口尺寸刷新解决；未修改产品代码，截图提炼后删除。首次调用 FindWindowEx 的托管探针未匹配目标，未执行尺寸变更，后改用直接枚举。
S2 登记 s2-defer-nocopy/layer-sample：只对自建 PID4236 的四个可见合成子窗口按各自 HWND 与 PID 采样，定位重复图标所属层；临时截图检查后删除。
S2 s2-defer-nocopy/layer-sample：四个直接可见合成子窗口的 PID 校验与 PrintWindow 均成功；顶部重复图标出现在覆盖完整客户区的 bridge 3805284 采样，不在标题辅助 bridge 5705590 或内容 bridge 1118378 采样中。PrintWindow 可能包含合成子层，不能据此断言具体 XAML 元素；临时图片已删除。
S2 s2-defer-tests/edge-workarea：实际注入夹具改为查询主显示器工作区顶边放置窗口，避免硬编码 (10,10) 造成任务栏位置变化时测试失真；双位宽增量构建与 launch/deferred-frame-x86/x64 均通过，验证该夹具中的工作区边缘接入及移除后外框保持。真实窗口连续拖动与残影修复仍未通过。
S2 登记 s2-defer-tests/frame-repaint：原生合成窗口夹具先检查延迟外框重绘是否意外触发客户区 WM_PAINT，作为减少闪烁候选的负例；如调整实现，须重新验证真实 Taskmgr 汉堡、最大化、拉伸及普通窗口。
S2 s2-defer-tests/frame-repaint：负例证明原计时重绘会触发应用客户区 WM_PAINT；候选改为同一线程直接请求 WM_NCPAINT，双位宽 core/frames 通过且不触发额外客户区绘制。登记 s2-caption-edge-live/repaint-worker：关闭已核验的自有旧 PID4236，以当前候选 DLL 重建自有 Taskmgr 实测；真实拉伸与残影仍待验证。
S2 s2-caption-edge-live/repaint-worker：新建 Taskmgr PID28260、HWND1052852 加载候选 ntw16s64.dll SHA256=4d09bd679d39ef928ada6e39ea5c747a490f9ff7d633fcd8d80bbed586a60ea6，工作区顶边 late attach 成功。登记 repaint-check：仅自有目标前台时一次汉堡点击与扩大/恢复，采样标题并恢复指针和前台；不能代替连续拖动及标准启动。
S2 repaint 候选验证：x86/x64 core/frames 和 launch/deferred-frame-x86/x64 通过；真实自有 PID28260 标题绘制采样蓝像素17487（阈值2322），但顶部重复图标及右侧细白条仍可见。汉堡输入后的即时蓝像素8693，但随后前台丢失，尺寸变化探针退出2未执行；该点击对照不能证明完整交互或闪烁已解决。候选未发布，临时截图提炼后删除。
S2 登记 repaint-resize-alone：对仍存活的自有 PID28260 在前台独立进行一次扩大及恢复，检验前一测试中的前台失效；含身份与前台校验。
S2 repaint-resize-alone：脚本生成时替换表达式报错，误保留汉堡探针；命中校验 PID=0，探针拒绝输入。窗口扩大/恢复 API 均成功，但屏幕标题采样全0，不计视觉通过。frame-repaint-verify 新增线程消息探针，证明合成窗口补画确实发送 WM_NCPAINT，且不额外触发客户区 WM_PAINT；x86/x64 core/frames 通过。真实拉伸闪烁及标题残影仍未解决。
S2 登记 s2-caption-edge-remove/geometry：对已核验自建 PID28260，单线程移除并重新附着候选，对比原生和接管状态的根客户区及直接可见子窗口坐标，并采样客户区；finally 恢复附着，截图核验后删除。
S2 geometry 首次目标最小化、客户区高度0，PrintWindow 不可用，未执行移除；登记 geometry-wrapper 临时恢复目标，测试后恢复其最小化状态。
S2 geometry 对照：自有 PID28260 最小化后临时恢复，原生/样式切换及再次附着成功，测试后恢复最小化。144 DPI 下外框保持 (976,0,1750,937)；原生客户区原点 (987,0)，接管后 (982,35)，主要 bridge 随客户区移动 (−5,+35)，子窗口之间相对坐标不变。PrintWindow 原生显示单个图标，再附着后出现顶部裁切图标；这支持残影与非客户区接管有关，但不足以确定 XAML 合成内部根因。部分采样出现外部画面痕迹，不据此评价内容；图片均删除。
S2 登记 geometry-only：对自建 PID28260 暂时移除 Win16 外框，仅以可移除子类调整 WM_NCCALCSIZE 至相同客户区，采样前后；finally 解除该子类并恢复 Win16 样式和原窗口最小化状态。该探针仅研究几何，不进入产品。
S2 geometry-only 对照：自有 PID28260 先移除 Win16 样式，仅按已测偏移调整 WM_NCCALCSIZE，不绘制经典标题；原生客户区图标单个，几何调整后复现顶部裁切的重复图标。探针所产生的根客户区及可见子窗口坐标与样式状态一致；可卸载子类解除成功、Win16 再附着成功、原最小化状态恢复。证据说明改变客户区几何本身足以复现现象；不证明内部合成实现细节，也未解决。采样已清理。
S2 登记 geometry-axes：只对自建 PID28260 分别测试单独顶部与单独侧边/底部客户区偏移，比较重复图标；每次移除临时子类，finally 恢复样式和最小化状态。
S2 geometry-axes：自有 PID28260 的原生基线图标单个；仅将客户区顶部下移35像素时复现顶部裁切图标，仅调整左右及底部（顶部不变）未观察到该重复。两次临时子类均成功移除，Win16 样式与最小化状态恢复；PrintWindow 样本有限，不能推断所有 WinUI 或证明修复。采样已删除。
S2 登记 dwm-notice：对自有 PID28260 的已接管窗口发送一次 WM_DWMCOMPOSITIONCHANGED 并对比客户区，检验宿主是否自行重新协调 DWM 扩展区域；恢复最小化，截图核查后删除。
S2 dwm-notice：向自有 PID28260 发送 WM_DWMCOMPOSITIONCHANGED 成功；前后 PrintWindow 均仍见顶部裁切图标，未支持此通知能修复。采样含其他画面痕迹，已删除；恢复最小化。
S2 登记 dwm-margins：对自有 PID28260 对照 DwmExtendFrameIntoClientArea 顶部边距0、35及−1，采样客户区；原扩展边距无法读取，测试结束按身份结束该自建实例而不声称恢复其边距；不修改系统设置。
S2 dwm-margins：DwmExtendFrameIntoClientArea 顶部边距0、35、−1均返回 S_OK，自建 PID28260 已结束；四张 PrintWindow 采样顶部含其他画面，不可用于判断残影是否消失，结果标记 inconclusive，未引入产品改动，图片已删除。
S2 登记 s2-standard-repaint：用现有双宽增量产物组成隔离候选包，由普通权限执行 run16s taskmgr，观察自有进程、UAC 和目标窗口；此不是干净交付包，不发布。
S2 s2-standard-repaint：普通权限 run16s taskmgr 经 UAC 创建 PID31868，真实载入隔离包 ntw16s64.dll SHA256=4d09bd679d39ef928ada6e39ea5c747a490f9ff7d633fcd8d80bbed586a60ea6，窗口附着查询成功；前台屏幕标题蓝像素17487，实际画出经典标题但顶部重复图标及右侧细白条仍可见。登记标准启动目标单次扩大/恢复；增量包非正式交付。
S2 s2-standard-repaint/resize：扩大和恢复 API 返回成功，标题采样蓝色像素8693/6391/6306；结束时前台已变化且捕获守卫返回2，采样受遮挡或焦点变化影响，不计标题持续显示或无闪烁通过。初始标准启动截图已提炼并删除。
S2 登记 s2-size-notify：从隔离增量包标准启动自有 Taskmgr，检查已知真实窗口位置变化日志中主窗口未出现 WM_SIZE 的现象；如观察到目标，单次受限尺寸通知对照后清理，仅研究不进入产品。
S2 size-notify 首次探针进程未设置 DPI 感知，在144 DPI 下将虚拟化尺寸508×597作为 WM_SIZE 参数；前后顶部重复图标仍在，但参数与实际尺寸不一致，不计有效对照。已修正为 Per-Monitor V2，登记重试；首次截图已清理。
S2 size-notify 重试：Per-Monitor V2 下查询实际客户区762×896，WM_SIZE 发送成功，前后身份与遮挡检查通过，标题蓝像素均16029；顶部重复图标仍在，单独补发正确尺寸通知未修复。屏幕采样已删除，目标随后按身份结束。
S2 登记 s2-dwm-visible：对照微软 WinUI 自定义标题文档，使用身份及前台遮挡校验重新检验窗口级 DWM 顶部扩展边距；只在自建目标上运行，结束目标而不声称能恢复不可读取的原边距。
S2 s2-dwm-visible：标准启动已创建自有 PID35576；后续管理员对照的 consent 曾出现并结束，但 worker 无任何输出，未运行 DWM 对照，不计结果。按 PID、启动时间核验并结束自建目标。微软 WinUI 标题设计资料说明存在 glass 子窗口与 composition bridge；这只提供研究方向，不证明本机残影根因。
S2 登记 s2-dwm-visible/combined：一次受限管理员探针启动增量包自有 Taskmgr 并校验 DLL 身份，再逐项采样 DWM 边距；最终清理自有目标。提权前不启动任何对照。
S2 登记 frame-regression：UAC 等待期间，对当前增量候选运行双位宽菜单取消、菜单分派与延迟外框所有权回归；不替代真实 WinUI 显示验证。
S2 s2-dwm-visible/combined：同一次管理员请求等待后返回系统取消（1223），worker 未启动、无 DWM 结果；不重复发起。frame-regression 双位宽 core/menu-cancel、core/menu-dispatch、core/pending-frame-owner 共6项通过；不覆盖真实 WinUI 残影与拖动。
S2 登记 frame-repaint-destroy：在私有桌面模拟合成窗口收到延迟 WM_NCPAINT 时重入销毁，核对根窗口与内部计时器均释放；不触碰现有记事本。
S2 frame-repaint-destroy：x86/x64 私有桌面测试均通过，合成窗口在计时器发送 WM_NCPAINT 的调用钩子中重入销毁后，根窗口和内部计时器均不再存活；仅覆盖该受控路径，不代表真实 WinUI 残影或动态视觉通过。
S2 combined 重试：输入桌面检测为 Default；重试同一受限自建管理员 DWM 对照一次，若未获提权不计通过。
S2 combined 重试：同一次系统 UAC 等待后返回取消（1223），探针未启动，无任务管理器对照结果。现有 Notepad PID27948 为用户 softpc.ini 窗口且未发现 ntw16s DLL，未操作；保持用户数据和进程。
S2 非提权替代目标审计：本机存在 Windows Notepad、Windows Terminal 和 Paint 包，但当前 Notepad PID27948 打开用户 softpc.ini；尚无证据证明再次激活可避免复用或影响其文档。未启动或操作这些应用，也未把它们视为 Taskmgr 对照；继续保留 S2。
S2 用户请求 retry：保留现有改动，重试一次受限管理员 DWM 对照；如未执行不计通过。
S2 用户 retry：同一次 UAC 等待后返回取消（1223），受限管理员 worker 未启动，未产生对照；未重复发起。
S2 用户反馈上次看不到 UAC 并要求再试；核验发起 pwsh 与活动控制台均为 session 1，重试一次受限管理员探针。
S2 用户 retry 实际提权成功：自有 Taskmgr PID14596 加载增量 DLL SHA256=4d09bd679d39ef928ada6e39ea5c747a490f9ff7d633fcd8d80bbed586a60ea6；基线真实屏幕标题可见，重复图标与右侧白条仍在。DWM top=0 返回 S_OK 后目标被 PID9248 遮挡，守卫拒绝截图，效果不计；目标已清理。登记 combined 重新获取目标前台后再采样，不操作遮挡窗口。
S2 combined 前台重试：PID35092 加载正确 DLL，但无法取得前台，未做 DWM 调整并已清理；登记自有目标短暂置顶后对照，结束目标时消除置顶。
S2 combined 置顶对照：自有 Taskmgr PID8004 加载增量 DLL SHA256=4d09bd679d39ef928ada6e39ea5c747a490f9ff7d633fcd8d80bbed586a60ea6；真实屏幕基线及 DWM 顶部边距0、35、−1 的前台/遮挡守卫均通过，三次 API 均 S_OK，顶部重复图标与右侧细白条均仍在，部分边距产生额外黑色背景；未引入 DWM 产品改动。实例已退出，截图提炼后删除。
S2 登记 s2-force-wvr：在自有 Taskmgr 的 UI 线程先临时设置仅返回 WVR_REDRAW 的下层子类，再使用当前 DLL 正常附着，核对标记及前台屏幕；试验只存在于目标进程，结束后关闭自有目标，不修改产品源码。
S2 s2-force-wvr：自有 PID34912 的下层子类确认实际返回 WVR_REDRAW、DLL 附着成功；屏幕守卫返回2未取得目标前台，不能评价真实屏幕。PrintWindow 试验样本中首行汉堡及图标被裁切，不能证明可用，且呈现潜在内容裁切；不引入该原型。自建目标已退出，采样已清理。
S2 登记 s2-drag-check：自有 Taskmgr 前台和右边框命中校验后进行一次30像素短拖拽、采样标题像素、恢复外框和鼠标；非逐帧录像，不能单独证明绝无闪烁。
S2 s2-drag-check：同一次系统 UAC 最终返回取消（1223），探针未启动，无拖拽或视觉结果；不计通过。
S2 drag 探针预检：PowerShell 解析无错误，HWND 参数实测传给原生程序为数字；drag.exe 为 x64 且无参数时退出2，候选 DLL SHA256 与前述增量验证一致。此预检不代表拖拽运行或视觉通过。
S2 用户请求拖拽探针 retry：确认无任务管理器、UAC 和构建进程，重试一次自有目标短拖拽对照。
S2 用户重试 drag：UAC 成功，自有 PID36800 加载目标 DLL，但窗口未取得前台，按保护条件退出，未发送鼠标输入，目标已清理。调整探针临时连接前台线程输入队列并在 finally 解除，再重试；仅作用自建目标。
S2 drag 前台重试：自有 PID18708 仍无法取得前台，未发送输入并已清理。登记标题栏命中探针：仅 WindowFromPoint 属于自有根窗口且 WM_NCHITTEST=HTCAPTION 时点击，并恢复鼠标；随后仍须通过现有前台和右边框命中校验。
S2 用户 retry drag 实测：自有 PID31940 标准候选，目标前台且命中右边框，一次短拖拽宽774→803像素；标题蓝像素7次采样为8693,8774,95,8782,95,8963,8917，确认中途丢帧。登记 interactive-repaint：先以合成窗口夹具验证交互缩放中擦除后即时重绘的负例，再测试候选。
S2 interactive-repaint：负例在擦除后无即时 WM_NCPAINT；候选在 WM_ENTERSIZEMOVE 至 WM_EXITSIZEMOVE 的直接 XAML 宿主擦除后即时补画，其他阶段仍合并计时重绘。x86/x64 core/frames 通过；登记同一自有 Taskmgr 短拖拽实际对照，非交付。
S2 interactive-repaint 实测失败：自有 PID34332 加载候选 DLL SHA256=3f092a3e721ededb52932b0783afa192e5bc54320076a5e9b61e9b9a263da493，真实短拖拽宽774→804，蓝像素8693,8774,95,8782,95,8963,95；与基线相比仍多次丢帧。已撤回同步擦除补画原型与其期望断言；此前独立 NC 补画候选保留。登记撤回后双宽回归。
S2 interactive-revert：撤回无效同步重绘后 x86/x64 core/frames 均通过，来源清单通过。登记 drag-trace：一次30像素自有短拖拽同步记录窗口消息与蓝像素采样时刻，确定丢帧相对于 WM_ERASEBKGND、WM_NCPAINT 与进入/退出尺寸循环的时序；非长时间压力测试。
S2 drag-trace：自有 PID27676 加载撤回后 DLL SHA256=e2f976251ca81a65554622d7af19b8e5937803ff200174958f14eea68a4f4276，前台命中短拖拽宽774→805。蓝像素/采样毫秒为8693/63、8728/141、95/219、95/313、8909/391、95/453、95/516；低样本与 WM_NCCALCSIZE 在219、313、516ms 同时或接近，相关 WM_NCPAINT 在235、313、532ms。消息和采样来自不同进程，不构成因果证明，支持进一步检查尺寸变化期间非客户区呈现；目标与指针已清理恢复。
S2 登记 interactive-position：基于短拖拽时序，先以夹具验证尺寸循环中应用处理 WM_WINDOWPOSCHANGED 后是否即时补画非客户区；负例确认后再测试候选，不作为已解决。
S2 interactive-position：负例证实应用处理位置变化后没有即时 WM_NCPAINT；候选只在直接 XAML 宿主的交互移动/尺寸循环期间，于 WM_WINDOWPOSCHANGED 完成后立即补画。x86/x64 core/frames 通过；登记实际短拖拽对照，失败则撤回。
S2 interactive-position 实测失败：自有 PID4092、候选 DLL SHA256=5e9e53eaedcd51c879f2a0ef74bab6705c139d957daa3a50424105a2d35901c4，短拖拽宽774→804，标题蓝像素8693,8774,8782,95,95,95,95；即时位置补画未改善，已撤回对应产品代码与测试原型。用户要求优先私有桌面；登记验证真实 Taskmgr 能否在私有桌面启动和绘制，不切换用户桌面。
S2 私有桌面可行性：构建原生探针，以独立 CreateDesktop/STARTUPINFO.lpDesktop 启动自建 Taskmgr，不切换输入桌面；枚举目标、检查 DLL、PrintWindow 采样并清理自有进程。采样若为空或黑屏计失败，不等同真实前台绘制。
S3 只读基线：共享 Explorer PID9248 同时拥有桌面、任务栏和两个现有 CabinetWClass 文件夹窗口；两个文件夹窗口直接包含 Microsoft.UI.Content.DesktopChildSiteBridge、TITLE_BAR_SCAFFOLDING_WINDOW_CLASS 和 ShellTabWindowClass，未操作已有窗口。当前通用桥接识别只匹配 Windows.UI.Composition.DesktopWindowContentBridge，登记 s3-explorer1 用双位宽自有夹具先验证现代桥接类的启动/清理负例。
S3 s3-explorer1 负例：双位宽实际启动注入夹具中旧桥接类通过、现代 DesktopChildSiteBridge 均退出15（未建立预期的延迟清理计时器）；候选将两种直接桥接类统一供延迟激活、可见性、重绘及边缘几何判定复用，不按 Explorer 进程名特判。该夹具不代表真实 Explorer 已接入。
S3 s3-explorer1 首轮候选：x64 两种桥接通过；x86 仍旧失败，经日志确认脚本仅构建夹具未重链 x86 DLL，使用了旧 DLL，不计候选失败或通过。脚本已修正为双宽均构建 ntw16s 后复测。
S3 s3-explorer1 修正脚本后，旧及现代桥接类在 x86/x64 实际启动注入夹具均通过；双宽 core/frames 通过。当前仅证明识别、延迟激活、绘制夹具和清理，不证明共享 Explorer 的窗口接入、真实绘制或标签交互。
S3 登记 s3-explorer-handoff：私有桌面启动同批已发布 run16s explorer /n,自有空目录；记录启动树、Shell 窗口 PID 与 DLL，不接入共享 Shell。仅当 HWND 原先不存在且实际目录仍精确匹配自有目录时关闭该新窗口；仅结束已验证的本次启动器子进程，不结束现有 Explorer。该带参数对照不代替无参数标准命令。
S3 s3-explorer-handoff 首轮：启动器返回0，但观察脚本在启动日志为空时 regex 报错，未确认目标窗口，不计通过；修正空日志处理后使用新自有目录复测。期间新出现 /factory Explorer PID14508，无法证明它为本次独占或可安全终止，未操作该进程。

S3 隔离桌面转交复核：Shell.Application.Windows 只观察调用方桌面，先前 not_observed 不能证明无窗口。通过 OpenDesktopW/EnumDesktopWindows 在两个各自创建的私有桌面确认 CabinetWClass 窗口分别由新建 Explorer PID14508、PID8544 持有，均为系统 COM factory 启动，未加载 ntw16s DLL；已发布 run16s 返回0不代表最终窗口已附着。两窗口已关闭（句柄身份复核为无效），两个 COM 进程仍存活，未强制终止。该 /n 加自有目录试验不代表无参数命令，也不证明实际绘制。
S3 登记 scratch：build/t23-package/s3-private-attach，独立桌面内针对新建 Explorer 文件夹窗口做单窗口附着及撤销原型，检查进程身份和句柄，不使用共享 Shell 或系统设置；这不等同正式启动接入或实际屏幕视觉通过。
S3 s3-private-attach：在未切换的独立桌面启动 run16s explorer /n,自有空目录，枚举新 CabinetWClass PID35832/TID32192；针对该窗口线程的临时 WH_CALLWNDPROC 原型加载增量 ntw16s64.dll，Ntw16sWindowAttach 返回成功、Attached=1；随后 Ntw16sWindowRemove 返回成功、Attached=0，并请求关闭该自有窗口；整个试验无 UAC。仅证明目标 UI 线程调用附着及撤销，不证明正式 run16s 转交接入、绘制、交互、恢复的视觉效果或无参数命令。
S3 私有桌面 PrintWindow 对照：Explorer 自有目录窗口附着后采样可见 Win16 标题，但采样前画面未完成初始化、XAML 区域呈黑且非真实输入桌面；不计完整视觉或交互通过。截图留在登记 scratch 供本任务提炼，未发布。
S3 产品级线程入口候选：新增 DLL 序号39 Ntw16sWindowThreadHook（双宽 ABI 清单同步），短期线程钩子仅按明确 HWND 调用既有附着或撤销并返回 Win32 错误；成功附着会固定 DLL，不等于物理卸载。双宽 GCC/Ninja 增量构建、core/abi、core/window-thread、core/frames 各3/3通过；窗口夹具验证不同 HWND 拒绝、目标附着、重复附着、撤销及第二窗口保持未附着。首次 CTest 因未设置登记 capture 目录未执行，设置后复测通过。
S3 产品级入口私有桌面实际验证：目标 Explorer PID29008 的自有窗口通过 DLL 序号39 附着和移除返回成功，PrintWindow 附着后可见标题，XAML 区域采样不完整；尚未接入正式 run16s，也没有完成真实桌面、标签、拖拽及恢复验收。后续 /separate,/n 对照仍由既有测试创建的 COM 进程 PID29008 处理，故 PID 新旧、独立桌面或 /separate 均不足以唯一关联某次启动；未操作用户已有桌面窗口。
S3 标准命令隔离对照：私有桌面无参数 run16s explorer，启动器 PID31428；有限等待约10秒未在该私有桌面枚举到 CabinetWClass，探针返回6、未进行附着。尚不能区分跨桌面复用、启动失败或其他系统转交，不计标准启动通过；没有据此修改用户既有窗口。
S3 标准命令负例根因已定位：空私有桌面中的 run16s explorer 返回0，但其子进程 PID3876（父 PID31852，出生时间与系统 explorer.exe 路径已核对）创建 Progman、Shell_TrayWnd 等私有 shell 窗口，未创建文件夹窗口；此前 PID35608 同样由探针启动器 PID31428 创建。两个自有私有 shell 均按身份核验后结束；现有用户 shell PID9248 未操作。登记同一私有桌面先启动自有 shell，再运行无参数 run16s explorer 的对照。
S3 已有 shell 的私有桌面对照：先启动本任务自有 shell PID15756，再执行无参数 run16s explorer（启动器 PID22072，退出0）；该私有桌面出现 Home CabinetWClass，实际由同一 shell PID15756 承载。只对新窗口调用 DLL 线程入口，附着及撤销返回成功；随后关闭新窗口并结束自有私有 shell。与空桌面不同，这提供了标准命令转交的隔离证据，但仍未证明正式启动器定位或真实屏幕交互。
S3 COM 精确句柄候选：参考微软 Developing with Windows Explorer 文档所述 CLSID_ShellBrowserWindow / IWebBrowser2，新建自有私有 shell 后在该桌面调用 CoCreateInstance 返回 E_FAIL 0x80004005，未取得窗口句柄；该结果只说明本机该隔离上下文失败，不概括其他系统，未改注册表。
S3 登记 scratch：build/t23-package/s3-explorer-candidate，用双宽增量 DLL 和候选启动器建立非交付隔离包；在先有自建 shell 的私有桌面运行无参数标准命令，独立查询最终窗口是否由候选自动附着，再清理自有实例。候选使用启动前窗口快照及唯一新增窗口条件，尚未完成并发归属、真实绘制或正式包验证。
S3 启动器候选私有桌面实测：先建立自有 shell 和已有文件夹，再以非交付增量包运行无参数 run16s explorer。最终新 Home 窗口由同一 shell PID35088 持有；独立线程查询新窗口 Attached=1，先前窗口 Attached=0，两者属于同一 Explorer 进程；启动器退出0，自有窗口和 shell 已清理。另一先前窗口在独立 Explorer PID33232 的对照也显示其未加载 DLL、新窗口已附着。快照筛选不能证明与其他进程并发启动时的因果归属，仍需并发负例及真实桌面验证。
S3 并发负例：同一私有 shell 在候选启动同时创建两个新 Home 窗口，两个独立查询均显示未附着（目标 DLL 未加载），原有窗口也未附着；本次候选拒绝了多个新增目标。候选增加约200ms稳定窗口，仍无法从唯一新增窗口本身证明它一定由本次请求引起，若另一应用恰在此时单独打开窗口而本次请求未创建窗口，仍有误归属可能；这是未解决的关联限制。
S3 重复启动对照：自有私有 shell PID32048 中，已有窗口未附着；连续两次候选标准启动各取得一个新窗口，独立查询均 Attached=1，均在同一 shell 进程；自有实例已清理。已加载 DLL 时，候选检查模块路径和回调前64字节一致性以避免明显旧版本错位；此检查不是完整映像摘要或所有加载竞态的证明。
S3 隔离桌面 DPI 感知对照：自有 shell PID23700，标准启动新窗口附着=1、同进程先前窗口附着=0，最大化 showCmd=3 与恢复 showCmd=1 后附着仍为1，重复启动的新窗口附着=1。Per-Monitor V2 的 PrintWindow 采样可见完整 Win16 标题及原有 Explorer 标签、导航区；这不是物理输入桌面，不证明鼠标菜单、拖拽闪烁或标签操作。采样包含用户 Home 快捷访问列表，仅留登记 scratch，不纳入提交。
S3 launch/explorer-selection：x64 原生夹具在独立桌面检查已存在窗口不附着、唯一新增窗口附着以及两个新增窗口的歧义拒绝，1/1通过；产品候选另在真实隔离 Explorer 上完成上述正例与并发负例。
S3 回归：双宽 core/abi、core/window-thread、core/menu-cancel、core/frames 通过；x86 core/menu-dispatch通过。x64 menu-dispatch与x86 UI 测试并行时一次在断言97失败，单独复测1/1通过；并行干扰是可能原因，未证明根因，保留该失败记录。
S3 补充目标选择测试：私有桌面 x64 夹具验证已有隐藏窗口显示后仍不被当作新窗口、两个新窗口不附着，以及子进程已映射不同路径 ntw16s64.dll 时返回 ERROR_REVISION_MISMATCH 并正常关闭子进程，1/1通过。线程代码前64字节相同仍不能保证整个已映射 DLL 是相同构建，保留为限制。
S3 登记 scratch：build/t23-package/s3-clean1，用同一源码快照进行双宽干净 Ninja/ccache 构建及包身份检查，作为后续实际包测试候选，不等于用户验收。
S3 s3-clean1：同一源码快照双宽干净构建、包结构及 ABI 检查通过（GCC 16.1、Ninja、ccache 4.13.6）。该实际候选包在先有自建 shell 的私有桌面运行无参数 run16s explorer：目标加载路径为 s3-clean1/candidate/ntw16s64.dll，已有同进程窗口 Attached=0、新窗口 Attached=1；144 DPI 下系统/最小化/最大化命中分别3/8/9，系统菜单打开并采样，最大化/恢复状态3/1后仍附着，第二次启动也附着。PrintWindow 能见经典标题及系统菜单；未覆盖真实鼠标交互和导航、标签操作。自有 shell 已结束。
S3 登记 scratch：build/t23-package/s3-package1，使用该干净包及其干净构建树运行上次已选29项相关实际包聚焦回归；后台单任务执行以减少UI测试竞争。
S3 s3-package1：同批四文件实际包29项聚焦回归全部通过、无跳过，单任务约61秒；覆盖首次注入、双位宽传播、标题菜单与相关全局回归，不替代实际 Explorer 物理交互。干净构建树 launch/explorer-selection 1/1通过。另以同路径但自有子进程回调首字节被修改的内存映像对照，候选返回版本不匹配，子进程正常清理；仅覆盖这一模拟差异。
S3 登记 scratch：build/t23-package/s3-publish1，备份并发布 s3-clean1 同批四文件至 assets 根目录和 O:/winnt，逐项核对测试摘要；保持 S3 开放待用户验收。
S3 s3-publish1：已发布 s3-clean1 同批四文件到 assets 根目录与 O:/winnt，两处摘要均与 s3-package1 一致；未改系统设置。准备提交和远端复核，用户实际桌面验收仍待完成。
S3 发布后补充私有桌面验证：使用 O:/winnt 实包的自建 Explorer 窗口，以 UI Automation 调用 Add New Tab，观察 tab 项由3增至6；调用 Close Tab 恢复3；选择 This PC 后窗口标题变为 This PC 且附着查询仍为1，既有窗口未附着；自有 shell 已结束。此为对应辅助功能路径，不证明物理鼠标、拖拽或所有导航。ShellWindows 集合未返回私有窗口，未执行该方式导航，不计失败或通过。P1 4b9f164a 已推送，实际远端提交与四文件及关键源码摘要已复核。
S3 发布包私有窗口重绘与移除补测：标签新建、关闭及选择 This PC 后分别用 PrintWindow 采样，三次仍可见 Win16 标题；同一自建窗口调用线程移除入口返回成功，附着查询由1变0，采样显示原生 Explorer 标题恢复；DLL 仍固定于目标直到该自有 shell 退出。三次菜单 WM_MOUSEMOVE 消息后菜单保持可见，但菜单项高亮均为0，悬停交互不计通过，也不能证明物理鼠标无闪烁；自有 shell 已清理。
S3 P4 映射身份修正：自建 DLL 占用探针显示直接覆盖失败，改名后可安装新文件，但旧映射文件在进程退出前无法删除；GetModuleFileNameEx 仍显示旧名称，GetMappedFileName 显示改名后的真实映射。旧 P1 源码在改名再用相同内容替换的原生负例中返回0（错误接受），目标子进程已正常清理；候选增加目标与本地实际映射名比较，原有64字节检查保留，负例返回1306且完整选择测试通过。
S3 登记 scratch：build/t23-package/s3-map-candidate1，增量启动器和现发布的双位宽 DLL 组成非交付包，用自有私有桌面复测标准启动、同进程旧窗口和重复启动。
S3 P4 负例与实测：上一发布版源码在自建子进程先加载 DLL、再将映射文件改名并以相同字节替换原名时返回0，错误地接受了旧映射；更正后返回 ERROR_REVISION_MISMATCH=1306，完整私有桌面选择测试通过，子进程及临时文件清理。增量候选在自有 Explorer shell PID37680 的标准启动与重复启动均附着，已有同进程窗口未附着，144 DPI 标题按钮命中、菜单采样、最大化和恢复通过；该增量包不是交付包。
S3 登记 scratch：build/t23-package/s3-map-clean1，对 P4 同一快照后台执行双位宽干净构建和包检查；下一步使用同批四文件实际包测试。
S3 登记 scratch：build/t23-package/s3-map-replace1，以自有隔离 Explorer 先加载测试 DLL，再在同一 scratch 内改名并替换同内容文件，复测第二次标准启动应拒绝旧映射；不涉及用户共享 shell 或发布目录。
S3 P4 真实旧映射对照：私有 Explorer shell PID7732 加载测试包 DLL，改名旧映射并以同字节替换后，第二次标准启动的新窗口独立查询未附着；初次新窗口已附着、先前窗口未附着，私有 shell 结束后旧 scratch DLL 可删除。P4 s3-map-clean1 双宽干净构建和包身份检查通过；干净树 launch/explorer-selection 1/1通过，另以该干净包在自有 shell PID19464 验证标准启动、已有窗口不附着、144 DPI 标题按钮、系统菜单、最大化/还原及重复启动通过，不代表物理鼠标验证。
S3 登记 scratch：build/t23-package/s3-map-package1，对 P4 干净包运行29项同批实际包聚焦回归，单任务以避免菜单测试竞争。
S3 s3-map-package1：P4 同批干净包29项实际包聚焦回归通过、无跳过；干净树窗口选择负例通过。发布前只读查询用户共享 Explorer PID9248，未发现 ntw16s64.dll，未改变或结束该进程。
S3 登记 scratch：build/t23-package/s3-map-publish1，备份并发布 P4 同批四文件至 assets 和 O:/winnt，核对摘要；S3 保持开放，物理交互及用户验收待完成。
S3 s3-map-publish1：P4 同批四文件已发布至 assets 根目录与 O:/winnt，两处摘要与 s3-map-package1 一致；共享 Explorer PID9248 未结束。屏保输入桌面下未执行物理鼠标测试；S3 保持开放。
