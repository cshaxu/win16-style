# M0 T16 S2：原样撤回局部轮廓移动

用户明确“追加一个单独T16的S任务”，因移动很卡决定撤回整个效果。Owner-Reopen，
顺序S2，M0固定，不新开T；S1成功历史保留，但不能代表流畅性验收。

## 诊断与撤回边界

先用纯C/O3编译S1相同move.c，QPC分段，两自有表面各12次更新：640×400平均
154.663ms，1280×700平均178.211ms，约6.5/5.6次每秒。主要是四条带屏幕读回
118.881/143.765ms，其次DwmFlush31.963/30.635ms；ULW仅0.624/0.837ms。
CPU循环跳过内部区域，不能错误归因为逐像素扫描全部客户区。诊断为同源微基准，
不冒称每个目标的完整DLL性能剖析；GDI8前后相等。worker已signalled/exit0，
即刻Job计数仍1后关闭kill-on-close Job，不把该瞬时计数写成active0。

系统原生移动模式由[系统设置SPI_SETDRAGFULLWINDOWS](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-systemparametersinfoa)
控制；[DrawFocusRect](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-drawfocusrect)
仅是XOR矩形绘图，不是单HWND移动模式开关。本S不改变全局设置，不继续替代实现。

原样恢复96e47212da877bcb4e16178abea093b006bb343f的CMake、三份版本RC、
window_titles.c、win16_style.c及Test-Package.ps1；删除move.c/h、outline_move.c。
导入回执/UI描述恢复基线。全196个CMake/src/tests/tools路径按Git规范LF等于基线，
没有新增代码/测试；原五头/来源声明未写入。196代码路径加LICENSE为197产品输入。
代码统计该10路径：+18/-859，净-841；删除路径由基线行为接收，不留下孤立注册。

用户原样撤回包括版本，产品恢复0.15.0.0/M0 T15；治理仍为T16 S2，Task15只用于
校验恢复产品元数据，参照T13 S2先例。ABI3/31命名+ordinal1，私有detours恢复35/40。
标题、字体、菜单、滚动条和既有框架适配均回到同一已确认代码，三组件四产物不变。
S1研究/历史、Actor只读证据及用户要求的VB候选保留，队列顺序不动，不自动准入。

## 有限验证与实际发布

GCC16.1.0、Ninja、ccache4.13.6，独立干净x86/x64 all构建及双ABI门禁通过；
四文件同批、原资源18表/依赖/版本门禁通过。按用户要求只跑短聚焦实际包：ABI32/64、
demo边界/裸样式对照、首次注入32/64、原生cmd嵌套四位宽边、每宽一次原生定位。
8/8通过，4.51992秒，每例有90秒Job限额，退出0/active0。两次定位使用SetWindowPos，
不冒称物理鼠标拖动或完整原生模态循环测试。源码精确撤回证明本项目不再提供该循环。

两处四文件发布到assets根目录和O:/winnt/system32，八SHA等于实际测试包。
Restart Manager报告八文件owners0，未关闭用户进程。安装system32绝对路径两宽ABI
再验通过，两个Jobactive0。未重复91/99全集、未重测所有DPI/物理输入；既有T11
并行Job计数债务保持，不称修复。不是将历史S1结果复用成恢复包测试。

具体输入/四SHA、构建工具及聚焦记录由[release](../etc/release.json)和
[撤回证据](../etc/evidence/outline-withdrawal.json)承载。实际提交推送、远端/恢复及精确scratch清理已完成，S2/T16关闭（撤回），M0开放。

## P2实际复核与全任务撤回审计

P1 `07d752fc4b580ef3ddd7596f49523c7d1a8c5938`已推送，实际读取213份远端文件
（197输入+四发布物+本次文档等）逐字节匹配Git；三个退休路径直接请求返回404。
切换协调复核角色核对实际diff/原请求与基线：整个196代码路径零差异，197输入
SHA快照亦逐字节等于T15基线1f196f850d88d876b3e18e14616c95220c5f8bed07ffd82b9afbe0b1c776ba73。
不把仅撤回主模块而遗留默认过程hook/测试注册算完成；这些入口均已完整撤回。
源码/版本、35/40私有拦截、公开ABI、原资源/既有UI和队列各有明确接收者，T16的
全部产品/测试改动均撤回，来源研究和历史证据保持可追溯，没有新产品替代实现。

用单次git cat-file批读取197源+四产物，201对象恢复通过；发布前八备份等于P1前
Git资产，均可从Git恢复后才清理。独立干净快照已重新构建全部双宽，未重复另建
归档树或重跑长矩阵；不把省略的重复验证写成通过。197输入/八发布物清理前复核。

原生NtCreateFile阳性检查自身EXE/加载DLL拒绝C0000043、卸载DLL成功；登记的
build/t16-perf、build/t15-package/withdraw-clean及withdraw-publish共169映像全部
无加载占用，祖先/后代reparse0。先提炼证据，再核对绝对路径仅删除这三个根，
已确认不存在；临时外置guard也移除。稳定缓存、其他T15工作及49审查源码、早期
外部截图保留，不把失败证据藏入缓存。治理13/源码23+9/原资源6自测与diff检查通过。

P2为仅文档关闭，197产品输入与八部署摘要不变。S2/T16按用户要求整体撤回收口，
M0开放；不继续优化，不重新排队或自动启动下一T。最终不声称修复后的自绘轮廓更快，
而是完整取消该效果、恢复原生移动。
