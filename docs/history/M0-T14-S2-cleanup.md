# M0 T14 S2：清理旧映像二次Shell启动

状态：最终实际包/双发布及安装验证通过，P1实际远端复核待完成；Owner-Reopen；准入48b5715时工作树干净，用户明确要求清理S1多余二次启动。
M0固定，T14追加下一S2，不消耗新T。首次校验逻辑恢复97999c0原main字节。
产品T14按固定版本契约保持0.14.0.0，S2由Git提交及包摘要区分。

## 清理边界

删除legacy_shell.c/h、legacy_shell_unit.c、shell_errors.c，撤回CMake/实际包的
专用模块和测试入口；删除当前README/UI/测试说明里的Shell二次启动能力声明。
首次MZ/NE校验失败仅stderr诊断193、退出4，不再调用Shell补一次启动或错误UI。
现有740提权自身的run16s_elevate/ShellExecuteEx分支原样保留，不扩大为其他错误重试。
全部DLL/demo源码、原资源/声明、标题/字体/菜单/滚动条及ABI3保持原字节。
S1历史及证据保留并标明撤回，不改写历史事实。没有兄弟仓库修改或依赖。

## 原生验证

新增launch/legacy-rejection纯C，自包含合成MZ/NE，经实际启动器直接和真实系统cmd
两条路径，四个独立用例。stderr使用继承文件句柄捕获，要求固定193诊断恰好一次、
退出码4、无自有Shell_Dialog/#32770额外UI。IOCP记录每个自有Job新进程实际映像，
要求启动器恰好1个、cmd恰好0/1个、其他只能是实际系统目录conhost.exe，未知0；
所有计数之和必须等于Job总进程数，等内核ACTIVE_PROCESS_ZERO后再严格核对活动0。
观察仅在自有私有桌面/Job内；超时有限，失败只回收自有树，不操作用户现有窗口。
新fixture替代强制系统UI的旧fixture并纳入实际包，包仍24项；注册70项不等于全跑。

早期测试假定Job直接只有1进程、cmd只有2进程而失败；没有放宽为任意数量。
原生创建通知实际识别额外进程为C:/Windows/System32/conhost.exe后，将其完整绝对
映像和全部进程身份纳入精确计数。最终四用例通过，未知映像0，整Job回收0。
旧S1发布实际运行产生额外Shell UI，被新测试检出（退出1）；旧版stderr也能捕获193。
因此本S不声称已定位用户此前“首次连命令行报错都没有”的原因，不把管道/文件捕获
等同于物理Win+R下持久可见，不补弹窗/暂停或猜测系统根因。

## 交付

独立冻结150输入，干净双宽构建完成；最终实际包24/24，无失败或跳过，4槽并行
245.18秒。资源/来源/治理自测通过，原ABI3及31命名导出保持。
四文件同批双发布，八摘要等于测试包，八备份等于48b5715资产，RM0占用，未关闭用户进程。
已部署system32启动器再次通过四场景首次拒绝，双宽已部署DLL ABI验证通过。
[发布清单](../etc/release.json)与[精选证据](../etc/evidence/legacy-retry-cleanup.json)记录
原始摘要及未覆盖范围；推送后做实际远端复核。scratch登记为build/t14-s2-cleanup
及build/t14-package（仅s2-*），提炼证据后回收，稳定build/.ninja与build/.ccache保留。

统计范围CMakeLists/src/tests/tools：10路径，+112/-195，净-83；删除4个旧文件，
新增一个纯C回归。原main恢复，不改变原生PE/UAC事务或GUI状态所有者。
