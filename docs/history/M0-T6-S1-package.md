# M0 T6 S1 — independent candidate package

T5关闭和T6准入已推送于60bb624。S1有限全集：剩余来源角色、必要补充契约、
四文件同批版本/PE/声明/哈希、干净复制树重建、明确自动与物理验收边界。
实际成功结果在运行后补入，不提前关闭T6或M0。

## Remaining source roles

本表记录S1最初MSVC阶段的导入状态；后续GCC机械适配已将部分retain转为adapt，
最终处置以source-manifest和回执为准，不以本表冒充当前逐字节保留。

固定commit/blob/size/SHA的124项及294 includes不改变；不从来源脏工作区取文件。
以下五份固定测试进入本地，前四个私有故障/菜单/剪贴板契约不由常规smoke替代：

| 固定tests/app文件 | 本地处置/实际输入 |
| --- | --- |
| win16_style_control_present.c | byte-exact retain，编译当前controls.c，SaveDC/pattern失败像素不变、每次present及自有GDI精确平衡；负例要求故意泄漏被拒绝 |
| win16_style_dc_failure.c | byte-exact retain，编译当前win16_style.c，借用DC/clip保持、原生菜单准备/恢复、嵌套作用域和回调销毁 |
| win16_style_menu_dispatch.c | byte-exact retain，链接真实本地DLL，UAH draw与measurement转发、菜单点击damage及GDI循环 |
| win16_style_edit_clipboard.c | adapt，仅环境不可用77分支；四种native/styled×single/multi copy/paste/cut/undo/readonly/font恢复断言保留，不自动使用共享剪贴板 |
| win16_style_scroll_api.c | byte-exact retain，纯原生API边界：system/custom消息、frame、隐藏bar、horizontal/list/edit与宽位置；不冒称DLL外观测试 |

前两种测试共用本地生产C文件静态库，分别只替换一个painter的故障边界；不是
新增产品renderer或DLL。menu/clipboard使用真实DLL；fixture不进入四文件包。
长路径补验使用真实CLI和两宽固定entry fixture，明确extended目标路径>300字符。

剩余九个旧候选改reference，destination为空、固定元数据保留，逐行理由在
source-manifest。旧ntw16s_dll由ABI/passive/runtime/terminal等本地DLL契约承接，
旧demo/gallery单体由本地frame/font/tabs/lifetime/default/sizebox与产品pair承接；
不保留旧public binding/28页demo或原Win3.1像素相等假设。native_title_hook及其RC
由实际CLI+间接模板的八Dialog/六MessageBox矩阵承接，旧nthook_install由
payload/staging/failures及四边创建/回滚承接。三个旧observation脚本由本地注册集、
PE/demo边界和干净打包工具承接，不复制服务/源路径/旧注册。不是用空文件填数。
全部实际目标有回执后，import_status改imported仅表示来源角色收口，不表示产品已验收。

## Native clipboard capability failure

s1-contracts PID5140、扩大权限诊断PID3012均在原生CreateWindowStation返回5，
没有执行任何剪贴板操作；最初将CHECK行错误定位成SetProcessWindowStation，
逐行复核后已更正。独立原生诊断同样创建失败，job UI flags为0；不臆测具体
系统策略或绕过权限。恢复原请求掩码，仅将该确定原生能力缺失返回77并由CTest
标记skipped，不能计入成功。共享opt-in没有自动执行。S2/必要人工验收接收该项；
最终发布/M0关闭前仍需结果，不能以四个其他新契约通过替代。

Build-Core新增JUnit结果摘要，顶层passed-with-skips区分全部成功，逐架构记录
测试数/成功数/跳过名称，保留原失败退出和600秒预算。Microsoft关于站点连接和
访问的说明仅用于诊断参考：[进程连接](https://learn.microsoft.com/en-us/windows/win32/winstation/process-connection-to-a-window-station)、
[站点创建](https://learn.microsoft.com/en-us/windows/win32/winstation/window-station-and-desktop-creation)。

## Candidate build and licensing

Build-Package只复制本仓库显式src/tests/tools/docs及根构建/声明文件，不复制
.git、build缓存、旧产品或用户目录；逐文件SHA snapshot可复核。干净树先过
来源/几何/治理门禁，再双宽全构建/ABI；Ninja实际依赖只能在复制树及MSVC/SDK。
不移动/修改兄弟仓库，也不把依赖追踪说成OS ACL不可达证明。构建源码自包含，
Git或来源仓库不是运行该流程的前提；构建工具/Windows系统库仍是正常宿主依赖。

候选目录严格四文件，逐一对照同批fresh输出，机器/GUI-CUI/DLL、ABI3/精确导出/
系统imports、数值0.6.0.0和M0 T6、SHA均由check_package检查。project MIT声明
以RCDATA 3嵌入四文件，Detours完整MIT以RCDATA 2嵌入launcher及两DLL；检查器
核对实际原字节。没有第五个license运行文件，源码原声明仍保留；不嵌入原像素/字体。
MSVC/链接器时间及环境可导致跨构建哈希不同，不承诺逐位可复现；本批复制必须完全一致。

候选包只进入build/t6-package，完整矩阵、真实输入及M0审计前不加入assets/binary。

## Focused results and path-gate correction

s1-contracts-bounded PID27144两宽各4通过+clipboard 1 skipped。DC借用状态/像素、
控制present/GDI精确平衡、menu转发/生命周期及纯原生scroll均通过；不将skip算成功。
s1-edges首轮负例包装器把临时绘制对象打印计数误写死1，实际含DC/bitmap为3，
真实painter已拒绝泄漏。包装器改为要求明确正计数泄漏标记、退出1及真实assert失败。
s1-edges-verified PID16848最终passed-with-skips：x86 1成功/1不可用，x64 2成功/
1不可用，两个long目标均>300字符、入口文件及退出42真实核对。生产行为未修改。

s1-clean PID15656在治理前置失败：不是缺复制文件，源/glyph门禁已通过；旧检查器
对FullName匹配build，把位于build下的整个副本都排除。修正为相对传入Root的
排除，仅改文件收集路径，不改任何规则或13负例。原根和真实嵌套副本各13负例
均通过；s1-clean-verified PID20604重新执行干净重建，结果待结束后记录。

## Owner toolchain and delivery correction

旧候选s1-clean-verified PID20604构建/PE/声明检查通过，仍是MSVC/C++，
不作为用户新增纯C/GCC目标的交付。用户同时要求每S编译、产物发布/测试、提交推送，
此前只有构建测试及源码推送的交付缺少随行二进制，保持历史原状，不补造发布事实。
T6 S1继续活动，先完成纯C/GCC迁移及治理同步再按新闭环收口。
本机WinLibs x86_64 GCC 16.1和独立i686 GCC均以C17/-Wall/-Wextra/-Werror
编译运行纯Win32探针成功；Ninja及ccache4.13.6可用。x64 -m32因无multilib失败，
后从ccache同目录找到真正i686工具链并验证，不能误判为无32位GCC。


## 纯 C 异常清理迁移（未交付）

新增私有 injection/unwind.c/.h，Windows 原生 unwinder 继续传播异常，仅在正常
退出或系统异常展开时执行一次清理。x64 直接使用函数 UHANDLER，以实际
EstablisherFrame 匹配线程当前 scope；不共享 C-specific handler 的 scope 游标。
x86 使用当前栈上的 SEH registration record，不改变 ESP，不借用 C++ 异常机制。
调用清理前先弹出状态，正常清理保留 GetLastError。要求 facade 帧指针、禁用
sibling-call 优化及 LTO；模块仍未接入产品，新工具链整包尚未通过。

直接链接生产模块的 tests/run16s-exe/unwind.c 在本机双宽 GCC C17 -O2、
-Wall -Wextra -Werror、-fno-omit-frame-pointer -fno-optimize-sibling-calls
-fno-lto -static-libgcc 下编译运行均退出0：正常清理、两层原生异常展开按112
顺序清理、同线程再用及两个工作线程隔离通过；正常路径LastError=1234保留。
输出 C UNWIND PASS width=32/64。探针/日志在已登记 build/t6-package/gcc-probe。
这证明清理模块；不代表拦截/注入替换或 GCC 四文件发布完成。


## 纯 C 私有补丁后端与 facade（未交付）

MinHook v1.3.4固定14项来源已逐项记录；保留完整MinHook/HDE BSD2声明，
12项原字节保留、buffer.c仅标记x86不用pOrigin以通过Wextra、hook.c适配为
startup_hooks.c。固定32槽事务只为初始载荷引导使用，无锁/等待/线程枚举或冻结，
先建立全部trampoline再改写，失败恢复已修改的本进程导出。
本机x86/x64 GCC C17 -O2 Wall/Wextra/Werror真实补丁测试退出0，验证KERNEL32
GetCurrentProcessId导出替换、原始trampoline、abort恢复及commit状态边界。
第一次x86测试暴露GCC静态dllimport指针可指向EXE跳板；改为运行时IAT取址后
双宽都通过。该发现同步用于产品facade，不能只让测试绕过。

create_process.cpp/window_titles.cpp已用git mv迁为.c，__try/__finally改为生产
私有C unwind scope，decltype/constexpr移除，全部原A/W窗口、8类对话框、6类
消息框、滚动/theme和创建字段路径保留，双宽GCC对象编译警告作为错误通过。
C类型查询使用GCC __typeof__，原生unwind metadata使用GCC内联汇编；没有C++
编译。尚未接入新引导/installer及根构建；当前整包不能构建发布，T6 S1保持active。
来源门禁保留旧124/294覆盖，另核验MinHook固定身份/声明/回执及7个拒绝用例。


复验补充：私有补丁接口使用memcpy读写函数指针槽，避免void**写入的C严格
别名问题；双宽真实补丁测试及两个facade对象编译再次通过。新增来源回执负例
拒绝MinHook目的路径冒充authored及漏回执，第三方拒绝用例扩为9。
版本核查另发现DLL ProductVersion字符串仍是0.4.0.0；统一四文件字符串为
0.6.0.0，并要求包检查器同时检查固定版本与两个版本字符串；DLL资源新增完整
MinHook/HDE LICENSE.txt。四份MinGW windres资源对象编译通过；Windows路径
传给windres预处理器须用正斜杠并显式-E/-xc/-DRC_INVOKED，本地首轮失败后修正。

GCC默认emulated TLS会引入libwinpthread-1.dll；原模块探针不证明独立运行包。
改为-static -static-libgcc后双宽unwind生产模块测试仍退出0，objdump实际imports
仅KERNEL32和Windows系统MSVCRT/UCRT API sets，不含libwinpthread、libgcc_s、
libstdc++或msys-2.0。后续根构建及包门禁须采用该链接约束。源/资源检查通过
不能替代待完成的新context/installer/entry引导、整包链接和全回归。


## 纯 C staging / helper 迁移设计准入

保持新建挂起目标限定，同位宽仅用SDK VirtualQueryEx/ReadProcessMemory读取该目标
的MEM_IMAGE PE头，找到唯一主EXE；不猜测私有PEB偏移，不扫描其他进程。
在主映像之后DWORD RVA可表示的空闲区域分配单个有界块，保存原import/bound/IAT
/checksum、固定头/目录策略和新增ordinal1导入；全部数据写入后才改目标PE头。
新DLL在初始引导中验证载荷并恢复原目录，不释放loader仍可能访问的块；由目标
进程退出回收。普通显式加载没有有效stage头，仍为被动模式。

跨宽维持OS rundll32 + 同包匹配DLL ordinal1，不增加helper产品。父端先以借用
目标句柄向低地址写入有界交接块，拒绝不足权限句柄；命令只传PID/块地址/创建
时间，helper核对固定头/目标创建时间/包路径及载荷，然后使用同宽stager。
不传前端句柄，不使用环境策略、broker或持久IPC。父端target/helper等待共用
10秒截止，目标先退出为1067，仅终止并关闭自有helper，绝不提前恢复原目标。
设计不构成实现或通过证据，四宽边/字段/回滚及helper负例须完整重新运行。


## GCC整包集成与首轮完整矩阵（仍未交付）

根CMake已改为仅C/RC，GCC C17 Wall/Wextra/Werror、静态GCC/线程支持库、Ninja及
ccache；新增SDK PE staging与OS helper，context/installer/entry及失败测试全部迁C。
借用句柄扫描自身新建目标的映像时，ReadProcessMemory权限错误必须原样返回，
不能吞成193。该缺陷由受限句柄用例发现并修正，拒绝码5和挂起数契约复验通过。
载荷、两宽ABI、原生异常展开、补丁事务、同宽staging及失败矩阵双宽focused均通过，
x64 helper保留约10秒timeout/目标先退出1067及句柄回收断言。

两个真实加载问题已定位并修正：MinGW importlib按名称导入GetWindowSubclass，
OS v5 COMCTL32仅在ordinal411提供它；对照本项目旧MSVC产物确为ordinal410..413，
新增私有导入库保留411契约，不把所有目标强制激活为v6。直接编译renderer的
绘图测试也使用该导入库。rundll32 ordinal回调不猜A/W参数，改读Unicode进程命令
行的严格父端格式。首次跨宽又暴露rundll32较小的线程栈；路径/策略/导入scratch
移到有界自有堆，减少嵌套栈占用。实际x64 CLI启动x86/x64原生目标均退出0。

保留demo/terminal原生Common-Controls v6 manifest，用显式RC代替MSVC pragma；
WinLibs默认manifest会与显式manifest冲突，按所选GCC -dumpspecs仅移除其endfile
中的default-manifest.o，保留该编译器其他CRT endfiles。两宽导出、版本字符串、
完整项目/MinHook声明和仅Windows系统依赖的四文件包核验通过。

首轮临时全量runner多退一级目录，两个worker已实际终止且没有JUnit，不算产品
结果。修正runner后的full-gcc-verified真实执行x86 24项（19过/4失败/1跳过），
x64 35项（29过/5失败/1跳过），x64总479.21秒。成功面包括实际demo两路观察、
四位宽边A/W三代、64创建字段原生/注入对照、四故障回滚、两GUI线程的八对话框/
六消息框、所有既有生命周期/字体/卸载面和helper。真实输入仍未验收。

失败均保留：绘图测试未带411导入（三项）已在宿主上下文双宽复验通过；受限
shell中的CreateDesktop错误5另记为环境不可执行，不伪称绘图失败。旧demo静态
依赖检查不允许系统CRT，按新profile允许Windows MSVCRT/UCRT、继续拒绝编译器
运行库DLL；boundary复验通过。长路径错误由MinGW CRT默认通配符展开造成，
含\\?\的359字符参数被改为20字符；启动器明确_dowildcard=0后保留原始参数，
双宽long-path实际CLI/install/entry/exit42复验通过。x64五个失败用例合并focused
5/5通过。x86 styled-runtime首轮90秒wrapper超时；保留全部断言/预算并增加测试
侧阶段日志，复验54.40秒通过，GDI43/43、USER39/38；不把并行调度推测当证明。
私有剪贴板77仍为明确跳过，未触碰共享剪贴板。

旧Detours的8文件及5个旧MSVC探针文件逐一确认无用户改动后退休；来源清单转
reference且回执移除，历史提交保留原源码与完整声明；当前src/tests无C++源。
新Build-Core已移除vcvars/MSVC，记录实际GCC目标/版本/hash，支持显式编译器及
每架构独立预算；复用既有GCC树的ABI/PE双宽通过。首次toolchain.cmake生成被
PowerShell数组拼接为单行，旧缓存忽略了它，因此该次不证明干净配置；已修正，
仍须从新复制树验证。工具/包及最终源码快照尚未交付，不关闭T6 S1。


首次新GCC干净包s1-gcc-clean在来源/治理通过后未开始编译：同步-Worker入口未
创建scratch，最早写run.log失败，故没有内层result.json；外层明确失败，不算
干净构建通过。已恢复Worker自建scratch，与异步父入口兼容。同期补齐近址分配
边界的显式ERROR_NOT_ENOUGH_MEMORY，避免任何NULL分配路径返回陈旧成功码。
后续必须以全新运行名复验，不覆盖首轮失败目录。


## 独立GCC干净复制树构建通过（尚未S1发布）

s1-gcc-clean-verified，worker24840已实际完成，result.status=passed、exit0。
从显式源清单复制的clean-source无.git/旧缓存/兄弟源码，源/几何/治理门禁均通过；
新生成四行toolchain.cmake实际完成x86 107步、x64 124步全产品及全部fixture编译，
各架构core/abi 1/1，PE/31名称+ordinal1..32、0.6.0.0双版本、完整项目/MinHook
声明及候选四文件与构建树SHA一致均通过。Ninja实际头依赖只在副本和所选GCC
工具链根中；未以旧MSVC候选或缓存树结果代替。GCC16.1.0两套独立目标为
 i686-w64-mingw32 / x86_64-w64-mingw32，ccache4.13.6、Ninja；32位使用系统
UCRT API sets，64位使用Windows msvcrt.dll，没有额外编译器运行库DLL。

候选相对目录 build/t6-package/s1-gcc-clean-verified/candidate，恰四文件：

| 文件 | SHA-256 |
| --- | --- |
| run16s.exe | 785b5081171cfa370e5aad3b32703b7f6fa8a5caeafec3f38c8bd2f6c938c29a |
| ntw16s32.dll | 0afa2a72e27f2a1f24cf527a1559a1be41458484b5d9f0a01ab63173c7f17499 |
| ntw16s64.dll | 278d0481d0e5b7fba72dc706b5e0a91ee30bf0c6067d502a4bdcccbd47f145a4 |
| demo.exe | 794ec074e0d0439fb395afede38335daa5e5821ef30c9eaadf7dc777e1c257ec |

构建后已逐项核对当前src/tests及根CMake/LICENSE输入与snapshot.json，无差异；
候选四文件哈希再次匹配package.json。支持资料补充不改变这些编译输入。
该副本的完整矩阵和物理验收仍pending。下一步必须直接测试该四文件包：至少
实际CLI/双宽自动附着、传播、同文件demo两路观察、长路径及必要负例，并核对
测试前后包哈希，不能只以构建目录ABI关闭S。然后发布assets/binary四文件、
提交推送及实际复核，才可收口S1并自动准入S2完整矩阵/真实交互。


## S1同包验证及发布（P1待推送）

s1-package-test，worker36392已实际完成，9/9通过。输入均为干净副本产出的同一
candidate：双宽ABI、demo源码/PE边界、同文件demo两路实际模块/外观/业务、CLI、
两GUI线程自动附着、四位宽边A/W三代及调用者挂起、353字符extended目标路径
双宽entry/exit42、坏包/stdio/exit失败路径。每例记录实际参数/退出码/必要标记；
四文件在测试前后哈希一致，与上表及构建树一致。demo native字体advance41、
styled43；caption像素0/26386，仍是私有桌面合成消息与PrintWindow，不是物理输入。

新增tools/Test-Package.ps1保存此可复验流程；默认9项，并可显式加入创建字段/
回滚/八对话框/六消息框扩展。工具不替代core完整矩阵；本轮只声称默认9项实际通过。
四个被测文件逐一复制到assets/binary，复制后哈希再次相等，没有换包或混包。
docs/etc/release.json记录当前交付标识、121个产品/测试输入的摘要、工具链及
完整四文件SHA/PE/imports和pending范围。新测试工具与交付说明不改变已编译输入。

S1达到独立四文件包、纯C/GCC、来源/声明/构建/同包聚焦验证边界；后续接收者
M0 T6 S2负责最终快照完整矩阵、私有剪贴板77处置及必要真实桌面交互，S3负责
跨任务与M0完成审计。未完成项不被本S发布消除。推送成功且实际复核后才能关闭S1。


暂存审计发现此前未跟踪startup_hooks.c末尾多一个空行；修掉并更新回执，另为
manifest显式指定LF，保证Windows检出字节与回执一致。虽然不改C行为，仍按新输入
执行全新干净重建及同包测试，旧候选保留证据，不把不同源指纹混称同批发布。


## 最终待推送批次

s1-final的干净双宽构建、ABI/PE/依赖/声明核验通过；s1-final-package-test worker10952
实际完成9/9，同包前后哈希一致。最终发布采用这一批，不使用前述旧候选的哈希。
121个src/tests/CMake/LICENSE输入均与工作区及最终构建快照匹配。暂存Git blob复核：
120项原字节相同，LICENSE是既有Git text=auto的LF到CRLF检出转换，MIT文字完全相同；
release.json同时保留其构建原字节SHA及canonical Git blob SHA，不冒充二者逐字节相等。
所有C/头/RC/manifest/测试源码均逐字节匹配。源与二进制不混批。

| 最终文件 | SHA-256 |
| --- | --- |
| run16s.exe | d9ada2ee4b6b271b17141a36b7d765a71fcdbfe50b602c2d85f83d8783af4ae6 |
| ntw16s32.dll | 9a40c4dd01752e7ea79dadb0dfc1c20db8e47df0d6997cc22974bed1ed7a75c7 |
| ntw16s64.dll | 959e22036792351e0c789ada6e605c09fd58504f794ee5b9555e5cca29eb8ea2 |
| demo.exe | 0818903eb19d5708f05061c95000846c46e227c6d19e6b7fa830630d6200f5fd |

改动统计范围CMakeLists.txt、src、tests、tools（不含docs/元数据/四个二进制），
git diff --cached --numstat为93个路径、+4546/-13283、净-8737。主要删除为已退役
Detours/旧MSVC探针；五个拦截/注入CPP与失败fixture按git mv迁为C，原职责保留。
当前保留状态所有者为各窗口GUI线程、创建事务、私有启动补丁及载荷生命周期；
未引入broker、外部repo运行依赖或第五产品。全部工作区变化均属本T6 S1，未改sibling。
源码/几何/治理门禁、暂存空白检查、公开ABI/四文件及必要负例已通过。S1只在推送
并读取实际提交复核后关闭；S2仍需最终快照完整自动矩阵、剪贴板77处置与物理验收。
