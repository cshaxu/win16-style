# Build and candidate package

在x64 Windows准备独立x86/x64 MinGW-w64 GCC、匹配windres/dlltool/as、ccache、
CMake 3.25+、Ninja、Python 3及PowerShell 7。产品及当前测试使用C17，不需要C++或
MSVC。MSYS2可提供构建工具；选用目标为i686-w64-mingw32 / x86_64-w64-mingw32的
原生编译器，不能选会引入msys-2.0.dll的MSYS gcc。源码不需要ntvdm64、softpc、Git、驻留服务或系统DLL安装。
工具自动从PATH、ccache邻接目录及常用MSYS2目录寻找两个编译器；可用
WIN16STYLE_GCC32 / WIN16STYLE_GCC64环境变量或Build-Core -Gcc32 / -Gcc64
指定。实际路径、目标、版本和编译器哈希写入该次toolchains.json。静态链接GCC
支持库和线程库，包仅依赖Windows系统DLL（含系统MSVCRT/UCRT），不附带
libgcc_s、libwinpthread、libstdc++或MSYS运行库DLL。
从仓库根运行：

```powershell
pwsh -NoProfile -File tools/Build-Package.ps1 -RunName candidate-new
```

命令返回隐藏worker PID、最终result.json和候选路径。须读取最终结果和worker日志，
不能将PID视为成功。精确scratch为build/t9-package/candidate-new；RunName必须未存在。
它复制当前源码（含尚未提交的当前文件），按snapshot.json记录全部输入，再在
clean-source中使用全新build/.ninja双宽构建，复制四文件到candidate，package.json
记录PE/版本/imports/SHA；每个build/test worker预算600秒。候选构建不自动发布。

完整矩阵在该干净树运行，仍需使用新RunName：

```powershell
$projectCompilerCache = Join-Path $PWD 'build/.ccache'
cd build/t9-package/candidate-new/clean-source
pwsh -NoProfile -File tools/Build-Core.ps1 -CacheRoot $projectCompilerCache -TaskRoot t9-package -RunName full-new -TestPattern '^(core|launch|propagate|demo)/.*$' -TimeoutSeconds 600
```

结果区分passed、passed-with-skips和failed-or-inconclusive；逐架构JUnit摘要列出
跳过名称。原生私有window-station权限不足时剪贴板77不可用，保持待验收，不等于通过。
所有失败/必需跳过均须有处置。每个产品S的同批包测试通过后发布、提交推送并
复核；开发包不等同M0最终通过。真实桌面验收及跨任务审计完成后才关闭M0。
T6 S1曾从干净复制树通过双宽全构建及同包测试；当前发布状态和验收边界见
[发布清单](../release.json)。完整矩阵与必要真实交互仍由后续S验证。

三组件四文件须始终同目录：run16s.exe、ntw16s32.dll、ntw16s64.dll、demo.exe。
普通使用：

```powershell
.\demo.exe
.\run16s.exe .\demo.exe
.\run16s.exe --wait <application.exe> [arguments]
```

默认同时启用样式和后代传播；--wait等待根目标并返回它的实际退出码，不等待全部
后代。默认等待Console目标；GUI目标无--wait在暂存/恢复成功后返回0。等待时可观察到loader退出码。
Console目标默认等待根目标并返回实际退出码；GUI目标默认恢复后返回0，--wait则等待。
CLI阶段错误2/3/4/5/6/7/8分别表示用法、解析、PE、创建、安装、恢复、等待。
Windows错误码在stderr，失败不继续裸运行。原始参数尾串、环境/cwd/继承保持原生语义。

包目录需ACP可往返或有效短路径，不支持时明确失败1113；不承诺任意Unicode包目录。
显式extended目标长路径单独验证，不把它扩张为所有API/命名空间支持。目标为普通
x86/x64原生程序；DOS/Win16二进制、ARM64、服务/受保护/提权目标及未覆盖创建API
不在已验收支持面。自动DLL生命周期随进程，不支持应用任意FreeLibrary后继续用hook。
字体使用本机System CP1252/FNT数据，未随包携带字体；五个图形头与固定ntvdm64原资源逐字节相同；宿主字体/调色板/DPI仍分别验收。

项目MIT与MinHook/HDE原始BSD声明保留于源码，二进制内嵌完整RCDATA声明：
RCDATA 3为四产物的项目LICENSE，RCDATA 4为两个DLL的完整MinHook/HDE LICENSE.txt。
旧Detours源码/声明只保留在其历史交付中，当前产品不使用Detours。


同包运行验证使用独立观察/控制器，将候选四文件作为实际参数；默认31项聚焦测试（含真实交互式cmd四位宽边、6场景等待契约、双宽15场景菜单取消及字体布局探针）：

```powershell
pwsh -NoProfile -File tools/Test-Package.ps1 -RunName package-test-new -Package build/t9-package/candidate-new/candidate -BuildRoot build/t9-package/candidate-new/clean-source/build/.ninja -SourceRoot build/t9-package/candidate-new/clean-source
```

传入`-FullIntegration`另外执行64字段对照、回滚、八对话框及六消息框；它仍不替代
完整core矩阵或物理验收。工具记录每例参数/退出码/必要标记和四文件前后哈希，
隐藏worker总预算1800秒，单例120/320/520秒。发布时只复制被测四文件，不换包。

原图形资源检查：`python tools/verify_resource_identity.py --self-test`。两DLL须包含
全部18个原始栅格表及RCDATA5完整来源说明；禁止构建时生成替代图形。

run16s保持Console子系统，交互cmd自然等待启动器；控制台目标默认同步，GUI目标
默认异步，显式--wait始终等待。GUI子系统试验已被用户否决且从未发布。

用户指定的发布步骤：每S若有binary change，将已测试的同批四文件发布至assets根目录
及O:/winnt/system32；系统目录原四文件备份至本S scratch，再逐项核对三处SHA。
仅更新本项目四文件。占用时按EXECUTION核对实际PID/映像/启动时间，正常关闭、有限等待后可结束该已确认进程并继续替换；权限失败记录并停止，不扩大权限。无binary change记录摘要比较。
部署入口（先完成同包测试，再运行）：

```powershell
pwsh -NoProfile -File tools/Publish-Package.ps1 -Package build/t9-package/candidate-new/candidate -TestResult build/t9-package/package-test-new/result.json -RunName publish-new
```

`-ValidateOnly`只检查被测哈希、四文件集合和目标路径，不写入；正式执行保存两处
旧文件及before/after摘要到发布scratch，失败保留incomplete记录，不标为完成。
当前交互验收使用原生探针，不用computer-use；私有剪贴板权限跳过和物理输入未覆盖
如实单列，不再要求用户反复完成整套手工操作。

打包、包测试和发布入口提供-Task参数，当前默认9；Task用于版本校验及build/t9-package
记录目录。未来任务传入实际编号，不复用旧任务scratch。

字体策略以[UI](../../design/UI.md)为准：菜单System原比例按DPI缩放，控件保持既定原度量适配；本页不复制字号/字形配置。

## 快速开发与持久缓存

开发期优先使用稳定树，只编译quick-tests依赖并运行quick标签：

```powershell
pwsh -NoProfile -File tools/Build-Core.ps1 -TaskRoot td-s9-efficiency -RunName quick-new -Profile Quick
# 自选共同目标及测试模式；跨宽fixture仍需先准备双宽输出
pwsh -NoProfile -File tools/Build-Core.ps1 -TaskRoot td-s9-efficiency -RunName focused-new -Targets ntw16s,core-abi,launch-staging -TestPattern '^(core/abi|launch/staging)$'
# 冻结后完整集合（含显式skip的可选后端），不把quick当full
pwsh -NoProfile -File tools/Build-Core.ps1 -TaskRoot td-s9-efficiency -RunName full-new -Profile Full -TimeoutSeconds 600
```

工具链配置属于build/.ninja/<arch>/toolchain.cmake，不再依赖任务scratch。写入内容
未变时不更新时间；清理任务日志后直接cmake --build仍可用。编译器缓存项缺失时
读取CMake已有编译器记录，仅确认编译器确实变更才使用--fresh，不能因正常变量
没有缓存项就清空对象目录。编译默认8 jobs，CTest
默认4 slots，可用-BuildJobs/-TestJobs调整；重UI占2 slots，不共享输出目录。

ccache DIR/TEMPDIR/MAXSIZE=5G/BASEDIR写入生成的编译命令，直接cmake/ninja也
使用同一项目build/.ccache。干净复制树显式共享原项目缓存，保留独立构建目录；
不清空全局缓存，不使用sloppiness绕过输入检查。-CacheRoot用于受控的干净构建
调用。每宽记录cache-before/cache-after以及configure/build/test阶段耗时。

每个CTest由tools/run_test.py将原生命令放到WIN16STYLE_CAPTURE_DIR下的唯一
工作/捕获目录；原始命令、参数、返回码（含77）保持，原超时和断言保持。调用者
须提供自有scratch。包测试已有每case目录，使用相同的有界并行原则，-Jobs默认4、
重UI占2 slots，保留所有必需标记和四文件测试前后摘要。-Jobs 1可做同集合串行对照。
开发中先quick/focused，测试源码收敛后一次最终冻结、完整回归与同批包验证。


原生超类回归入口：`core-superclass.exe <absolute-run16s.exe>`（分别运行x86/x64构建），
自有私有桌面中覆盖裸/注入与96/144 DPI、初始化器重设窗口过程、实际经典填充/卸载、
输入协议和清理。`core-theme-stage.exe`编译生产theme.c并核对暂存/提交/取消/终结、
失败和所有私有分配归零。Test-Package两种位宽各收一例，FullIntegration总计35例。

可选真实DoubleKiller检查：使用与实际目标相同位宽的
`doublekiller-driver.exe <absolute-run16s.exe-or-bare> <absolute-target.exe> <absolute-doublekiller-probe.dll> <owned-output-prefix>`。
从登记scratch运行；只操作本次Job中新建实例，四页只读检查及实际Run按钮绘制/卸载，
截图在卸载之前生成；不点击业务扫描/删除/移动/注册表命令。该检查需要用户提供的真实应用，
不加入核心必需测试、不构成构建依赖。实际覆盖及限制见CURRENT和本S证据。
