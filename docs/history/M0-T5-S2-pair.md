# M0 T5 S2 — external demo pair

S1 9d94cdf已推送并实际复核。两路均运行同一复制到自有测试包的demo.exe，
直接启动或真实run16s --wait启动；不使用两份改编例、不向demo注入测试代码。
observer只在自己的私有桌面监听SHOW，按类名和自有job归属接收主窗口；
Toolhelp核对实际demo模块路径、原生无样式DLL、注入只加载同包64位DLL。

冻结观察单位：同标题/11个标准控件与六种类名、初始edit/check/radio/list/combo/
scroll数据；BM_CLICK触发Apply/复选/单选真实通知，原生控件WM_KEYDOWN触发
列表/组合选择通知，应用状态label必须对应；WM_HSCROLL业务消息和菜单Reset/
Exit、WM_SYSCOMMAND最小/最大/恢复。此处未把合成业务消息说成物理输入。

PrintWindow路由WM_PRINT捕获两路自有窗口，保存BMP；核对渲染哈希、注入标题配色和edit字符
位置实际advance差异，证明模块加载之外的真实绘制/排版变化。observer不读取
renderer私有内存、不跨线程调用样式ABI；实际附着以绘制/排版及生产路径作证，
不伪造Ntw16sWindowAttached的跨进程返回值。截图须人工目视检查后提炼结论。

每路只管理新建树，启动前挂起并交给KILL_ON_JOB_CLOSE；正常关闭核对根退出0，
失败/observer退出时只回收自己树。WinEvent仅在测试期间存在，最后解除；
DC/bitmap/file/process/thread/job/event均有本observer唯一所有者。

首轮s2-pair PID36312在裸启动路径因observer跨进程写文本失败，诊断轮PID36920
显示实际仍为Native text、Apply正确回显该原始值。SetDlgItemText换为显式
WM_SETTEXT并立即读回原预期；直接跨进程传HDC的空白图改PrintWindow（OS路由
WM_PRINT），不更改产品、不降低断言。s2-pair-marshalled PID29752最终exit0，
x86 ABI1/1、x64 pair+ABI2/2，pair约1.17秒（总1.34秒）。

已人工目视两张实际BMP：同标题/菜单/文本和六类数据，原生为系统主题边框/
控件；注入为Win16标题、几何按钮/边框/滚动条及位图文字，内容没有裁切。
原生/注入edit五字符advance为41/43；指定标题色像素为0/26386，画面hash不同。消息驱动的复选/单选、
编辑Apply、列表/组合通知、滚动、Reset/Exit和系统窗口命令全部断言通过。

真实输入/菜单展开和选定DPI
仍由独立包任务接收。demo没有新增测试开关、样式接口或环境开关。
交付统计（本行加入前）：6文件+191/-22，测试116行、CMake+7，生产源码零变更；旧路径无移动，92回执及23/13负例通过。
