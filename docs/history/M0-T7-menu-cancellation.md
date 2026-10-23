# M0 T7 S1 — 菜单取消修复

用户报告点击菜单后无法取消，随后鼠标移到哪个顶层菜单就打开哪个。以89fe6b9
为基线准入Corrective T7 S1，版本0.7.0.0；只有本S，永久M0。交付及复核状态见CURRENT。

## 失败契约与修复

旧open_bar用select_bar完成鼠标跨项切换，内部进入bar_keyboard模式，却未在鼠标
取消后清理。键盘popup又把所有无命令返回当作Escape，外部点击也保留了模式。
因此菜单已经退出，焦点仍在frame，后续WM_NCMOUSEMOVE又打开popup。

保留初始鼠标来源，结束迭代时leave_bar并恢复焦点；menu_frame只在根菜单记录
实际Escape，键盘根Escape仍返回选中菜单栏，第二次Escape返回原焦点。外部点击、
同项二次点击、WM_CANCELMODE及失活则取消；F10只选中菜单栏时点击空白也退出。
system-menu、助记键及嵌套菜单分别回归，不把子菜单Escape当作整次取消。

状态仍由原GUI线程style_window拥有。tracking_menu保留栈式重入、owner-draw
scope及s->calls固定生命期，detached后不继续读写；没有全局hook/轮询或新ABI。
三组件/四文件、Console等待/传播、原资源、字体和滚动条修复不变。

## 有限覆盖与证据

原生C探针显式LoadLibraryEx实际候选DLL、调用公开安装/移除API，在owned私有
desktop驱动真实TrackPopupMenu队列。WM_ENTERIDLE投递原生鼠标/键盘消息；
不使用computer-use/SendInput，不冒称物理输入。2500ms watchdog仅失败保护，
不能把超时当通过。每例检查菜单loop进出匹配、capture释放、焦点归还、取消后
hover没有新loop、命令数、菜单项owner-draw恢复及卸载销毁。

| case | 情形与完成谓词 |
| --- | --- |
| 0 | 鼠标切换另一个顶层菜单后Esc，恢复闲置 |
| 1 | 鼠标切换后客户区点击，恢复闲置 |
| 2 | 重复点击当前顶层项，取消 |
| 3 | 切换后WM_CANCELMODE，立即退出 |
| 4 | 切换后激活另一个owned窗口，自然取消且不抢回焦点 |
| 5 | 键盘popup后外部点击，退出菜单模式 |
| 6 | 键盘根Escape回菜单栏，第二次Escape归还焦点 |
| 7 | 键盘选择命令，准确发送一次 |
| 8 | 鼠标切换后选命令，准确一次且恢复闲置 |
| 9 | 子菜单Escape回父级，再取消根，层级保留 |
| 10 | 键盘system-menu Escape及第二次Escape |
| 11 | 键盘system-menu外部点击取消 |
| 12 | 助记键打开，Escape两级退出 |
| 13 | F10空闲选择后客户区点击，无popup并恢复焦点 |
| 14 | F10空闲选择后菜单栏空白点击，无popup并恢复焦点 |

旧实际assets两个DLL均在case0被最终探针拒绝：entries=2/exits=2、phase=2、
commands=0、focusRestored=0、timeout=0、exit1。不是合成mock，也不是超时假负例。
开发阶段初始probe的EDIT通知计数曾计入命令，修为只接受ID1/2且HIWORD0；
probe-complete扩充case13期间产生未冻结构建树的新探针/旧DLL混合失败，未计通过。
源码冻结后probe-final双宽各3/3通过（menu-cancel十五场景、menu-dispatch、dc-failure）。
final初批保留为开发证据；随后统一本次修改文件LF换行并刷新回执，release从新快照
重建，成为唯一发布候选。此后产品/测试输入冻结，原资源六文件未改。

相似问题搜索：`rg -n 'tracking_menu|bar_keyboard|SC_KEYMENU|WM_CANCELMODE|EndMenu|open_bar' src/ntw16s-dll`。
生产命中按menu_filter、open_bar_once/open_bar、SC_KEYMENU普通/空格/助记键、
NC/client点击及CancelMode逐项处理；其余EndMenu和键盘迭代保留原所有者/重入语义，
由menu-dispatch与dc-failure覆盖移除、销毁、嵌套loop及DC失败回滚。

## 最终交付与任务级审计

最终release干净双宽全构建及ABI各1/1通过。125输入摘要绑定快照，所有产物/测试
来自同批；外部依赖trace只允许本仓库快照和所选工具链。相关回归选择完整core、
demo及launch wait/payload/staging/unwind/startup-hooks/failures，不重建：x86注册25、
24通过/1跳过；x64注册28、27通过/1跳过；0失败、注册与JUnit执行名字集合相同。
两个跳过均为隔离clipboard CreateWindowStation权限5/exit77，不计通过且不碰共享
clipboard。本次不重跑全部propagate contract/dialog/messages；T6完整矩阵是历史证据。

实际候选包12/12通过：双宽ABI、裸/注入demo、六场景等待契约、双宽15场景菜单取消、
CLI、自动注入、四位宽边三代传播、长路径及stdio/错误退出回滚。最终两份菜单日志
各15 CASE PASS及总标记；DPI144/150% demo滚动条26物理像素、焦点/控件/通知/
About/移动均通过。四文件发布assets根目录及O:/winnt/system32，先备份再复制，
8条before/after与已测包SHA一致。原文件为T6 S5 P1 5b5ee58，可由Git assets恢复。
产物、工具链、125输入及原始证据SHA见release.json；实际远端复核待P1推送后记录。

代码统计基线89fe6b9，范围src/tests/tools/CMakeLists.txt，git diff --numstat：
10文件+173/-31，净+142。分组：CMake 1文件+8/-1；src 4文件+49/-18；
tests 1文件+99/-0；tools 4文件+17/-12。无旧资源/renderer替换路径；旧菜单状态
错误清理分支已替代，栈式所有者保持，旧二进制先备份再同批替换。
本任务的完成全集为：取消/切换15项、键盘/命令/层级保留、生命周期/资源清理、
启动/传播与demo回归、原资源/ABI身份、四文件双发布及远端复核六单元；每项须有
实际证据，host clipboard权限跳过不能计通过，不要求用户重复手工验收。

| 任务级覆盖单元 | 处置/完成依据 |
| --- | --- |
| 取消/跨项 | 双宽15场景实际包通过，双宽旧DLL负例失败，hover无新loop |
| 键盘/命令/层级 | case6..14及menu-dispatch通过，根/子Escape与命令数量准确 |
| 生命周期/资源清理 | 双宽完整core、dc-failure/lifetime、loop/capture/focus/item恢复通过 |
| 启动/传播/demo | 同包12项与相关launch/demo通过；未修改传播/注入源码 |
| 原资源/ABI身份 | 六原文件及双DLL18表、完整声明、ABI3/31名称不变；source23/MinHook9/resource6负例门禁 |
| 发布/远端/卫生 | 双发布8条摘要一致；P1推送实际复核及清理后再由P2关闭 |
