# M0 T16 S1 验证登记（已收口）

本资料保留最终结论与历史失败；当前状态以CURRENT为准，最终发布SHA见release。

## 冻结覆盖全集

每行是一个任务审计单位；允许处置只有实际通过、明确原生回退、明确未覆盖或
用户延后。完成谓词为所有承诺项有实际证据，未覆盖/回退不计实现通过，最终
完整包及发布远端证据齐全。审计由主会话在实现后另切换复核角色完成。

| 单位 | 契约与证据入口 | 当前处置 |
| --- | --- | --- |
| 原始依据 | outline-original.json：原USER/GDI/VGA SHA、调用边界、55/AA、ROP及四条带 | 已核实；非完整VGA帧缓冲模拟 |
| 独立栅格 | core-outline-move --raster：软件真值与原生GDI PATINVERT/区域；双次擦除 | offline-final3双宽通过 |
| 默认过程与否决 | 启动策略DefWindowProcA/W；应用SC_MOVE返回777优先 | full-final1四显示配置通过 |
| 被动API | 无载荷LoadLibrary+显式install；默认过程字节不变 | offline-final3双宽通过 |
| 移动/提交 | 真窗口不动、仅最终一次定位、提交区域RGB及最终位置等于预览 | full-final1四配置/256场景通过（消息/API探针） |
| 取消/残影 | Esc、完整截图RGB恢复、overlay销毁 | full-final1四配置/256场景通过（消息/API探针） |
| 键鼠入口 | NC标题、SC_MOVE、9/1键盘步长、A/W各路径 | full-final1四配置/256场景通过（消息/API探针） |
| 位置约束 | WM_MOVING修改、重复鼠标不积累、完全离开工作区不更新 | full-final1四配置/256场景通过（消息/API探针） |
| 生命周期 | 捕获、失活、隐藏、销毁、detach、应用定位、跨线程sent cancel | full-final1四配置/256场景通过（消息/API探针） |
| 消息泵 | 同线程重入、WM_QUIT73重投、GetMessage取消后不派发旧目标 | full-final1四配置/256场景通过（消息/API探针） |
| DPI | 当前硬件单屏144；PMv2与logical96；负方向及绝对量化 | 单屏physical144/aware144及unaware96通过；物理96/120/192及多屏未覆盖 |
| 明确原生 | 对话框、MDI、max/iconic、RTL、禁用Move、被动API、多屏unaware | 代码保守回退；不计轮廓支持 |
| 既有视觉 | 原资源门禁；旧/新同场景标题与菜单截图对照 | 原资源门禁通过；旧/新双宽同位置各614400像素RGB0差异 |
| 负例 | 当前探针运行旧0.15包，必须检出无轮廓 | full-final1四配置/256场景通过（消息/API探针） |
| 架构/传播 | ABI3、37/40 detours、四原生传播边及原有回归 | 干净双宽/ABI已通过，实际包集成见下方 |
| 交付 | 同源200已核对输入、四文件、完整99包、双发布八SHA、实际远端/恢复/清理 | 99完整包及双发布八SHA、远端218文件、归档/204恢复/1400映像清理通过 |

## 历史迭代与限制（以下旧候选状态不是当前状态）

lifecycle-matrix5早期实现曾双宽及四种显示组合通过，实际物理144DPI；每组合
64场景。但之后加入独立GDI对照、负方向、最终位置/提交取消RGB比较及菜单。
早期结果不能替代最终候选验证。final-focused2屏幕BitBlt错误5；原生读取input
桌面为Screen-saver，非产品通过。之后用户明确表示暂不方便，先完成无需显示
工作，不自动恢复可见用例、不主动解锁或关闭屏保、不调整系统设置。

offline-final1双宽ABI/被动/栅格各3例通过；后续窗口身份和测试更改须重新跑。
原生量化探针：physical144/logical96，SetWindowPos逻辑43,75最终物理65,113；
临时PMv2请求64,112仍经应用logical43,75回到65,113。候选匹配原生可达位置。

原始VGA默认几何和输入步长有二进制依据；真彩RGB反相、现代DPI和分层窗口是
明确现代映射。不宣称原调色板、所有配置、所有窗口框架、鼠标Aero Snap或物理
键鼠验收。原图形头/声明零修改，现有标题/字体/菜单/滚动条保持。

## 执行入口

- `tools/Build-Core.ps1 -TaskRoot t16-outline -RunName <fresh> -TestPattern '^core/(abi|outline-passive|outline-raster)$'`：双宽离屏。
- `tools/Build-Package.ps1 -Task 16 -RunName <fresh>`：干净快照、全部双宽构建与包身份。
- `tools/Test-Package.ps1 -Task 16 -RunName <fresh> -Package <candidate> -BuildRoot <clean-build> -SourceRoot <snapshot> -FullIntegration -DeferDisplay`：只延后4个outline-display用例，其余仍执行；结果必须是display-deferred，不能发布。
- 用户方便后去掉DeferDisplay，另取fresh运行完整99用例；旧包负例及snapshot对照单独登记，不把部分结果拼成完整通过。

所有显示探针只新建自有窗口和有超时的Job实例，使用同线程测试消息，不使用
computer-use/全局hook/SendInput，不把整套测试交给用户。HD Tune管理员测试按
用户原决定默认skip，不重新弹UAC。


## 当前最终源码的离屏执行记录

- offline-final2在沙箱中CreateDesktop返回错误5，非产品结论；与此前输入桌面
  Screen-saver导致屏幕捕获错误5是两项不同限制。仅为自有私有桌面测试在沙箱外
  重跑offline-final3：x86/x64各ABI/被动/独立栅格3项全通过（共6项，无skip）。
- clean-offline1同源快照双宽all干净构建、两宽ABI/PE/导入导出、四文件版本及
  每DLL18原表通过。200产品输入逐字节等于当前工作区；没有兄弟依赖。编译
  x86 32.856455s、x64 39.6695135s；Ninja8构建并发，ccache4.13.6直接命中142/156，
  两宽miss均0；GCC16.1.0。候选版本0.16.0.0，尚未发布。
- offline-package1以FullIntegration+DeferDisplay运行95个非可见用例；已记90项：
  89项通过、rollback失败，另5项被中止/尚未启动，4显示项明确延后。保留原始
  failed-or-inconclusive结果，不能改成整包通过。四边真实cmd嵌套已通过。
- rollback失败：32位parent、A路径，before.ActiveProcesses=2，after=3，成员列表
  仅parent40020和sibling16172（均rollback32.exe）。第62行原断言失败；该现象
  已在T11债务中登记，本次未修改断言/生产回滚路径，不凭计数差推定进程泄漏。
- offline-followup1以同一fixture对部署0.15旧包与当前候选各串行3轮：六轮全部
  exit0，每轮4个A/W×parent位宽组合。并发失败未在串行复现，原因仍未确定，
  不能据串行通过抹掉原批次失败或断言版本回归。另补跑被中止的5项全部exit0并逐项复核原标记：dialog-show-32
  101.0549476s、dialog-show-64 87.3633697s、creation-contract 10.5454939s、
  dialogs 75.6586111s、messages 79.066221s。
- deferred-gate-negative为明确标记fixture_only的独立门禁负例：Publish-Package
  ValidateOnly拒绝display-deferred状态，未进入部署步骤、没有发布。该fixture
  不是实际包测试结果。

候选四摘要（不是发布基线）：

| 文件 | SHA256 |
| --- | --- |
| run16s.exe | e9fc6f300538cd349c8b340732f52be92477cbe8a9ca909c1d2e082de5163ba4 |
| ntw16s32.dll | fd1fca3b0d32de61a2f53b62bc29bf02d9ac1d7517a4b7d25f5189e7af32dce8 |
| ntw16s64.dll | 65c240a7654192b427207be68a8826c2793c4ec5330373476da247a41f09017c |
| demo.exe | 34f44675cd14c5149c0393671001c40d564ee053664b713a824a5088d4eb9013 |

项目assets与O:/winnt/system32八摘要仍匹配原0.15.0.0，备份在已登记t16-package/
baseline。没有提交推送或新发布，S1/M0/T16保持开放。显示、负例、视觉对照、
完整包与回滚失败处置、发布远端及最终清理仍是关闭前条件，不将早期成功替代。


offline-coverage-review独立核对11个补跑/对照记录及原标记：原89通过，加5项补跑，
再加单独rollback通过证据，共95个不同非可见用例均有单项通过证据；不是把多次
记录合成完整批次通过。首次并发rollback失败仍未解释，完整99包仍须重新通过。
补跑协调器及全部自有实例已退出；八部署摘要再次确认保持0.15基线。没有因新
Actor发现或VB专项提案改变200候选产品输入。新增提案仅排队，当前S/T不收口。


## 恢复显示后的测试契约复核

用户明确要求T16收口，输入桌面Default，恢复显示验证。display-resume1背景窗
exStyle=0，动态桌面进入截图；背景改为创建时TOPMOST|NOACTIVATE并严格核对
标志/角点后隔绝背景。窗口物理初始RECT改由PMv2 GetWindowRect直接取得，避免
LogicalToPhysicalPoint对已经舍入的逻辑边界二次转换造成一像素尺寸错误。

在logical96/physical144负方向用例，完整画布源捕获的像素700,179为282828，
独立完整frame源矩形的SRCCOPY/CAPTUREBLT均为292929，排除了两ROP不同的假设。
DPI重定向屏幕源矩形的重采样可依采样矩形而变；live oracle改为单独捕获完整
frame，原生GDI区域/PATINVERT，生产仍四条带/软件求值，互不共用栅格实现。
离屏原VGA四条带真值不变。独立参考经分层窗呈现、仅加DwmFlush等尝试均未
解释差异，没有保留无效presenter，也没有引入像素容差或系统设置调整。

native-position-diag1保持原严格断言，证明product_vs_native=0，而纯SetWindowPos
也有native_vs_translated_origin=7。旧位置截图简单平移不是unaware重定向窗口
有效的零误差契约；普通DPI/cancel仍原严格比较，只有该unaware映射场景增加
同一最终坐标的直接原生定位控制，要求逐像素零差异，原始translation差异另报。
生产一次提交计数在独立控制之前检查，不把测试控制的后续定位记为产品提交。
自动审查曾拒绝缺少此对照证据时替换断言；保留断言取得上述原生日志后，才
准入有限对照修订。未把原始7像素差异改写为0或宣称任意DPI位图平移恒等。

rollback-stress1原断言不变：旧/新各6轮四并发，旧6轮通过，新第7轮在parent64/W
再次出现active3/member2。rollback-accounting1仅scratch诊断增加有界计数等待，
旧/新各6轮通过，但first_count均2，没有取得收敛时差，不宣称根因或修好。
tracked rollback_contract.c完全不改；该T11债务保留，必须另跑完整包验证。
官方[Job计数定义](https://learn.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-jobobject_basic_accounting_information)
与[完成端口通知](https://learn.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-jobobject_associate_completion_port)
解释不同观测边界，不能仅据文档归因于MXC/DWM或其他未证实机制。

## 最终独立批次与任务审计

用户最新收口指令恢复显示验证，原生桌面Default。clean-final3最终200输入快照
独立干净双宽构建并打同批四文件；full-final1完整99/99、497.7582246秒，无deferred
或skip；原严格rollback测试全部四边通过，早期偶发失败仍是未定位债务。
经典core额外双宽各12共24项通过；两处真实安装base各7共14及实际安装DLL移动
四配置各64场景通过（18安装检查）。四最终display配置每配置64场景，共256；
每配置独立native GDI oracle四项、提交/取消像素八项，结束GDI19=基线19，Jobactive0。

旧0.15负例双宽均exit1，MOVE_RESULT mode0/stage0/enters0/moving0，未进入新事务，
Job全回收；仅证明新探针检出该消息契约差异，不冒称旧版本物理移动必然失效。
旧/新标题菜单客户区同位置各614400像素RGB0差异；18整文件及10绘制函数指纹
等于基线，原五头/声明/每DLL18表保持。四张正常显示的原生PNG已保留。

实际发布assets根目录和O:/winnt/system32八SHA与最终包一致。安装驱动初次缺少
STD句柄转发曾45秒超时/active0；只修探针转发后同45秒限额四例通过，产品未变。
实际远端P1取回218文件通过；归档双宽和四文件门禁、204 Git对象恢复及旧八备份
恢复通过。登记两个根1400映像原生占用检查和reparse0后精确清理，四PNG/稳定缓存/
其他任务源码保留。冻结16单位已按实际通过、原生边界及明确未覆盖完成独立审计。
S1/T16已关闭，M0开放，不启动下一候选。唯一发布SHA及最终证据见
[release](../release.json)和[交付记录](../../history/M0-T16-S1-outline-move.md)。

## 后续撤回

用户实际体验卡顿，已明确准入独立T16 S2撤回全部产品/测试改动。本资料的S1功能/
像素通过不代表流畅性通过，也不代表当前仍启用；以[T16 S2撤回](../../history/M0-T16-S2-outline-withdrawal.md)
为准。
