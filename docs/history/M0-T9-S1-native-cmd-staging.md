# M0 T9 S1: native nested cmd and external imports

本S接续e6d6b55准入，承接用户追加的真实嵌套cmd测试；未分配新S或覆盖其他活动包。
M0固定，T8保持开放。当前交付仍在验证，远端复核和清理完成后才收口。

## Confirmed defect and minimum fix

最后正常节点是两钩子均初始化的外层原生PE cmd。旧包在创建内层原生cmd时失败：
子映像实际为PE，不是NE；目标退出193，目标独有内层标记缺失。仅ntw及仅nthook
对照能完成嵌套。挂起子进程现场另证实nthook/Detours把有效导入描述符放在可读的
MEM_PRIVATE分配内，RVA超出原SizeOfImage；旧ntw staging在读取前拒绝并返回193。
这解释本次cmd故障，不把它追写成此前未稳定复现的NE间歇故障根因。

产品仅改staging.c的导入描述符寻址：允许真实可读外部分配，保留32位RVA/本机
地址末端溢出检查、1024条目终止限制、ReadProcessMemory权限和原失败回滚。
不改两钩子trampoline、兄弟协议或全局状态。版本0.9.0.0/M0 T9，ABI3保持。
标题、字体、菜单、标准控件、应用自绘及六原资源源码均无改动。

## Finite test design

- 自包含launch/staging：双宽复制真实导入表到SizeOfImage外，真实运行初始化并
  创建附着窗口；另测NOACCESS、无终止条目和RVA末端越界。无需安装后端。
- 新纯C cmd-nested驱动真实cmd交互标准输入输出，子/k仅设置独有环境变量，
  再由目标echo组装唯一标记；完整标记不由驱动器写入stdin。内层exit /b 37，
  外层恢复errorlevel37，再exit /b 23，验证目标和包装器原生退出码。
- 四边32→32、32→64、64→32、64→64；32→64明确Sysnative。实际映像路径、PE、
  PID/创建者、DLL路径/SHA、载荷/commit/hook数和实际子代传播分别观测。
- 必需propagate/cmd-nested仅本包；run16 cmd与run16s run16 cmd为独立可选兼容。
  每例新登录AuthenticationId、预检空RPC端点、私有桌面及自有Console，不碰用户
  服务/VDM/Console。所有实例归job，有界等待并检查内外两层job归零。
- 复用现有隔离桌面、staging契约和实际四文件包矩阵；不增加产品测试开关、
  第五产物、C++或MSVC。入口与复跑命令见[tests/README](../../tests/README.md)。

## Negative detection and test diagnostics

相同增强staging fixture链接e6d6b55旧生产源码，x86/x64均在有效外部表处以193
检出；原有基础挂起契约仍通过。最终cmd探针对旧T8发布包及其逐字节相同备份的nthook四组合通过，
双钩子首例失败，真实原生PE子cmd退出193、成功标记缺失，且自有job清零。
root CreateProcess成功错误码0、子进程退出193与staging原生错误193分别记录；
一轮最终负例的子cmd退出太快未入快照，只检出标记缺失；对同SHA备份用最终
探针复核又采到原生子cmd退出193，分轮记录不合并。未直接采样cmd内部GetLastError，不伪称该退出码就是所采样API错误。

调试中两个测试前置问题独立处理：headless Console不满足本机run16 frontend，
曾返回87；只在新登录worker内FreeConsole/AllocConsole后解决。模块快照后短命
helper退出造成RPM 299，也曾提前打断观测；现仅在确认该自有进程已退出时记为
初始化未确认并继续，存活进程读取失败仍失败。实测记录helper exit0/read299后
目标cmd成功；没有放松目标初始化/传播断言。

## Verification and delivery

最终包、构建输入、工具链、各用例结果及原始证据SHA提炼于
[release.json](../etc/release.json)。最终同批四文件19/19通过，含完整创建/回滚/dialog/message集成；仅ntw四边和可选
nthook/双钩子各四边均通过，目标独有标记、37/23退出及内外job归零。双宽
staging正负例均通过；不存在后端时另实测77/SKIP。已先备份后发布assets根及
O:/winnt/system32，8摘要与实际包相同，8份before可从e6d6b55 assets恢复。
P1尚待实际远端复核和清理。

相关注册全集用`^(core|launch|propagate|demo|compatibility)/.*$`运行：x86为26项、
25通过/clipboard跳过；x64为40项、38通过/clipboard及默认可选后端跳过，共63通过、
3跳过、0失败。该轮使用稳定构建树；随后仅测试观测/清理源码继续修订，最终冻结
版本另作聚焦与同包测试，不能把不同构建身份混称一轮。最终源码/测试输入128项。

统计范围为src/tests/tools/CMakeLists.txt相对e6d6b55：13路径，增加609/删除23/
净增586行（含两个新增C测试文件）。状态所有权仍归创建事务和GUI线程，测试只
拥有本次job/管道/桌面/新登录Console；无源码迁移，旧判界由有界可读寻址取代。

缺陷面搜索为`rg 'ERROR_BAD_EXE_FORMAT|SizeOfImage|IMPORT_LIMIT'
src/ntw16s-dll/injection`：生产描述符循环只有staging.c一处。PE头本身的边界、
近地址分配范围、installer映像验证和helper位宽校验仍各司原职，不放宽它们。
同宽/跨宽helper、载荷、回滚和调用者挂起语义通过实际包矩阵核对。

构建尝试如实处置：worker未启动、来源回执未同步、干净树缺少旧assets时相对下载
链接被治理拒绝，均未算通过；更新本地作者回执，并将README下载链接改为本项目
固定远端四文件。其余已通过候选被后续测试观测/清理修订取代，只有
`s1-release-final`为最终待发布包；不混入任何旧候选文件。

## Limits

原始完整PID/路径/模块/转录仅在任务scratch，公开记录脱敏并保留摘要。仅验证
本机Windows及安装后端的有限组合；无任意版本后端、ARM64、真实输入桌面、
任意DPI承诺。默认可选后端缺失返回77，不能计作通过。私有桌面clipboard权限
5/77跳过独立报告，不通过用户共享剪贴板绕过。

本S新包NE冷/热及VDM复用未重跑；此前NE间歇失败仍未证实根因；既有裸NTSRV复用无传播是用户接受边界。本S不
关闭T8、不修改兄弟仓库、不注入或结束用户已有进程。
