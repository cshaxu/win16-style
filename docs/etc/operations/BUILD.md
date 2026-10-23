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
不能将PID视为成功。精确scratch为build/t6-package/candidate-new；RunName必须未存在。
它复制当前源码（含尚未提交的当前文件），按snapshot.json记录全部输入，再在
clean-source中使用全新build/.ninja双宽构建，复制四文件到candidate，package.json
记录PE/版本/imports/SHA；每个build/test worker预算600秒。候选构建不自动发布。

完整矩阵在该干净树运行，仍需使用新RunName：

```powershell
cd build/t6-package/candidate-new/clean-source
pwsh -NoProfile -File tools/Build-Core.ps1 -TaskRoot t6-package -RunName full-new -TestPattern '^(core|launch|propagate|demo)/.*$' -TimeoutSeconds 600
```

结果区分passed、passed-with-skips和failed-or-inconclusive；逐架构JUnit摘要列出
跳过名称。原生私有window-station权限不足时剪贴板77不可用，保持待验收，不等于通过。
所有失败/必需跳过均须有处置。每个产品S的同批包测试通过后发布、提交推送并
复核；开发包不等同M0最终通过。真实桌面验收及跨任务审计完成后才关闭M0。
T6 S1已从干净复制树通过双宽全构建及同包测试；当前发布状态和验收边界见
[发布清单](../release.json)。完整矩阵与必要真实交互仍由后续S验证。

三组件四文件须始终同目录：run16s.exe、ntw16s32.dll、ntw16s64.dll、demo.exe。
普通使用：

```powershell
.\demo.exe
.\run16s.exe .\demo.exe
.\run16s.exe --wait <application.exe> [arguments]
```

默认同时启用样式和后代传播；--wait等待根目标并返回它的实际退出码，不等待全部
后代。无--wait在暂存/恢复成功后返回0；初始化失败可由--wait观察到loader退出码。
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


同包运行验证使用独立观察/控制器，将候选四文件作为实际参数；默认9项聚焦测试：

```powershell
pwsh -NoProfile -File tools/Test-Package.ps1 -RunName package-test-new -Package build/t6-package/candidate-new/candidate -BuildRoot build/t6-package/candidate-new/clean-source/build/.ninja -SourceRoot build/t6-package/candidate-new/clean-source
```

传入`-FullIntegration`另外执行64字段对照、回滚、八对话框及六消息框；它仍不替代
完整core矩阵或物理验收。工具记录每例参数/退出码/必要标记和四文件前后哈希，
隐藏worker总预算1800秒，单例120/320/520秒。发布时只复制被测四文件，不换包。

原图形资源检查：`python tools/verify_resource_identity.py --self-test`。两DLL须包含
全部18个原始栅格表及RCDATA5完整来源说明；禁止构建时生成替代图形。
