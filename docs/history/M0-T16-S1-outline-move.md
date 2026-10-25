# M0 T16 S1：Win3.x 局部轮廓移动与任务审计

用户准入队列首位并最新要求当前拖拽T做到收口；M0固定，T16 S1，基线
96e47212da877bcb4e16178abea093b006bb343f（0.15.0.0）。本记录当前仍在执行，
完整包/发布/推送/远端/恢复/清理事实只在实际完成后追加，不用早期成功替代。

## 原始依据、最小实现与职责

原始Win3.1 3.10.0.103的USER/GDI/VGA身份、SHA、段偏移和执行链见
[原始证据](../etc/evidence/outline-original.json)。原USER指令执行到GDI边界，
原VGA FASTBORDER执行到BITBLT边界，冻结55/AA、PATINVERT 0x005A0049、旧窗口
左上角画刷原点、四边几何及VGA默认厚3/细1、键盘9/Ctrl1。没有执行真实VGA
端口/平面锁存器/16色调色板，不称为完整原机帧缓冲复现。现代真彩RGB反相与
DPI整数格映射明确属于现代实现；原系统二进制/模拟器不是构建或核心测试依赖。

在现有ntw16s DLL内增加私有move.c/h；启动策略的DefWindowProcA/W两处拦截只
接管已附着、同GUI线程且应用已交到默认过程的SC_MOVE。应用返回777的否决
保留，被动LoadLibrary/显式API不装新拦截。标题鼠标及键盘入口共用局部事务。

每事务仅自建透明分层窗口和位图/DC预览，真实窗保持原位；松开/Enter一次
SetWindowPos，Esc/取消/捕获丢失/失活/隐藏/销毁/卸载/应用定位/环境变化退出。
保留WM_ENTERSIZEMOVE/MOVING/EXITSIZEMOVE、应用约束、重入与WM_QUIT73。
隐藏/flush自己的上个预览后才采样，避免DWM把预览再反色；无全局SPI、屏幕
绘画锁、全局hook、既有进程注入或新增产品。三组件四产物、ABI3/31命名+
ordinal1保持；私有detours35增加到37，容量仍40，两宽导出门禁通过。

## 冻结覆盖全集与审计方法

全集已先冻结于[验证登记](../etc/evidence/outline-validation.md)的16个单位。
主会话在实现后切换复核角色，逐项读取原始请求、实际源码/日志/包；不另开
agent，不凭自身摘要替代证据。允许处置为实际验证、明确原生保留、明确未覆盖；
未覆盖不能算实现通过。各单位的最终处置在本节交付记录中列出。

| 单位 | 有限契约与证据接收者 |
| --- | --- |
| 原始依据 | 原始证据JSON：身份/CPU/GDI/VGA边界；非完整VGA帧缓冲 |
| 独立栅格 | 双宽raster：原始四条带/55AA软件真值与native GDI/双次擦除 |
| 默认过程 | 实际A/W默认入口与应用否决；本次包的outline-display系列 |
| 被动边界 | 双宽passive，DefWindowProc代码字节保持、原veto返回 |
| 移动提交 | 预览中真实RECT不变、一次提交、原生最终RECT/显示严格对照 |
| 取消残影 | Esc/cancel截图对照底图，捕获和窗口生命周期 |
| 键鼠入口 | NC标题/SC_MOVE/9或Ctrl1，消息探针而非物理输入 |
| 工作区/约束 | WM_MOVING改位置、同鼠标重复不漂移、完全离开工作区拒绝 |
| 生命周期 | capture/deactivate/hide/destroy/detach/reposition/sent cancel |
| 消息泵 | 同线程重入拒绝、终结后不派发陈旧目标、WM_QUIT73原码重投 |
| DPI | 实际单屏144/aware与logical96；厚/细边框、正/负方向 |
| 原生边界 | dialog/MDI/max/iconic/RTL/禁用Move/既有捕获/被动等保留 |
| 视觉保留 | 原五头/声明和每DLL18表；同场景旧/新双宽截图全图RGB0差异 |
| 旧包负例 | 0.15两个位宽未进入新事务而被当前探针检出，Job全回收 |
| 架构/传播 | 双宽ABI、首次/四传播边、真实cmd及原有经典控件/菜单等完整包 |
| 交付恢复 | 200输入、同批四文件、实际包、双发布、远端/归档/恢复/精确根清理 |

厚/细、ANSI/Unicode和16种消息/生命周期组合在每显示配置64场景中覆盖；四个
配置共256场景。普通DPI提交/取消保留原严格截图比较；legacy重定向的原始
平移截图在纯SetWindowPos控制中也差7像素，而product_vs_native=0。因此仅该
场景以同最终坐标的直接原生定位作零容差控制，原始差异保留另报。采样矩形
重采样差异、失效的presenter/flush假设及自动审查过程均如实记入验证登记。
独立live oracle捕获完整frame并使用native GDI，生产为四条带/软件求值，不
共用栅格生成器；离屏原VGA真值不变，不引入像素容差。生产一次提交在原生
控制之前检查。新旧标题/菜单/客户区在同位置各614400像素RGB0差异。

## 保留状态、代码统计与检索

move_loop和surface由GUI线程栈事务唯一拥有；thread-local tracking只借用栈
指针。取消唤醒阻塞GetMessage，不启动线程。style_window.calls持有到模态返回；
属性代际/valid校验防止移除、销毁、HWND复用后操作旧节点。分层窗口在创建时
绑定surface身份，仅身份匹配才销毁；DC/位图/类恰好在清理路径释放。捕获只
释放本目标仍持有的部分；应用字体、图像、菜单及其他窗口的资源不接管。

统计范围为CMake、生产C/RC/私有头、新原生C测试和实际包runner；排除docs与
二进制。git diff --numstat加新文件逐行统计：增加859、删除18、净841。没有
旧源码路径删除或移动；原图形表和声明零修改。未变源码由200输入/归档及
原图形门禁接收，四发布物由各自SHA接收。统计只对应本次代码，不把日志计入。

检索包括SC_MOVE/WM_NCLBUTTONDOWN、DefWindowProcA/W、移动/取消/捕获/终结/DPI
各消息，CreateProcess回滚，以及move.c中的SPI_SET/LockWindowUpdate/全局hook。
生产命中处置为默认过程接入、既有style_proc取消和私有move模块；没有新全局
机制。原frame/font/menu/scroll/控件算法文件保持，Win16Style改动限取消和私有
默认移动桥接。Actor真实VB6 CheckBox缺口已提炼为独立证据，用户新要求的
[VB5/VB6专项](../proposals/vb5-vb6-controls.md)仅列队列第3位，不夹带产品修复。

## 已知边界与失败保留

普通轮廓移动不提供鼠标边缘Aero Snap，键盘Snap仍系统路径。对话框、MDI、
max/iconic/RTL、应用自处理移动等不扩张为支持；unaware多屏及system-aware
物理/逻辑DPI不一致明确回退；动态跨屏、物理96/120/192/多屏、物理键鼠/IME/
无障碍没有本次实际验证。不称任意程序或全部Win3.x像素兼容。

T11已有并行rollback Job计数债务再次出现active3/member2；原断言与生产创建
代码完全不改。串行旧/新各3轮通过；原断言并发旧6通过/新1失败5通过；仅
scratch的有界计数诊断旧/新各6通过但未复现初始3，不能据文档归因为某个
宿主/工具。失败保留在验证登记/TODO，不宣称修复或用重试删掉历史失败。
最终完整包须独立通过后才能交付，早期89+补5+独立rollback不拼接为整批通过。

当前真实HD Tune管理员UI保持用户默认skip，无新UAC；非本T承诺面。最终停止
于本T交付验收，不启动排队的文件夹、全局模式或VB专项。


## P1实际交付（远端复核尚待执行）

0.16.0.0，200输入SHA256 072ff97b429028f463b63b9d95e0d244ef384cc23d112f644629caa82c4e3240。
最终clean-final3独立干净双宽all/ABI/PE通过；实际包99/99，497.7582246秒，
无deferred/skip；补充经典core24/24，双安装目录base14/14及实际DLL路径的
移动4/4（各64场景）通过。两处四文件与测试包八SHA一致，无占用者，未结束
用户进程。实际四SHA及工具链由[release](../etc/release.json)唯一承载；
[交付证据](../etc/evidence/outline-delivery.json)接收保留指纹、native图像及实际路径。

安装探针首次未显式转发STD句柄，45秒上限回收active0；修正测试驱动器后
维持原45秒期限，4个安装路径场景通过，未改产品或完整包结论。
最终源码采用Git规定LF，新C/RC回执已同步；原资源零字节改变。
此前各候选/失败保留于验证登记，不冒充本批通过。P1源码/四文件须推送并实际
读取远端，随后进行归档/恢复/安全清理与P2关闭；此时S1/T16仍开放。
