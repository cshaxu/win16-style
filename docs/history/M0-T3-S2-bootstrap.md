# M0 T3 S2 — same-DLL bootstrap and automatic native windows

S1 2cc5d4d 已推送并复核关闭。本 S 改造固定来源 entry/intercept/window_titles，
合并到现有两个 DLL；核心 ABI3 的31名字及 ordinal2..32 保留，新增真实
CALLBACK ordinal1 NONAME，供 OS rundll32 安装；没有额外 helper 产品。

## Loading and ownership

普通加载无载荷，仍为显式同线程核心。helper 在 DetourIsHelperProcess 后跳过
应用拦截，仅由 ordinal1 完成安装。载荷目标验证版本/目录/位宽，恢复暂存的
imports，事务只登记当前线程和改写已有 API 地址，失败返回 DLL 初始化失败。
静态CRT/TLS线程通知保留。DllMain 不调用 USER/GDI、LoadLibrary、等待或创建线程；
所用 DetourUpdateThread(GetCurrentThread()) 实际直接返回，不挂起其他线程。
载荷/恢复使用内存枚举和读写，未使用 Detours 符号搜索/DbgHelp 加载路径。

第一次真实 GUI API 入口在 loader-lock 外执行 InitOnce，私有绑定同 DLL 的
三条 scroll/theme 原 trampoline，然后进入唯一创建作用域。移除原 LoadLibrary/
GetProcAddress 绑定第二 renderer、WOW combo binding 和服务/RPC 路径。
CreateWindowEx A/W、八 Dialog APIs、六 MessageBox APIs 共用公开门面的真实
创建作用域；错误值在原调用后保存，scope 清理后恢复。原 renderer 拥有状态和
销毁，当前未拦截 CreateProcess，传播留给下一 T。

## Finite runtime corpus

目标是原生 x86/x64 exe，无样式链接、LoadLibrary、安装或策略配置；只读诊断
查询已加载 DLL 的 Ntw16sWindowAttached。同位宽模块存在、真实 HWND 附着、
实际私有字体、原生数据/标题/消息和退出均分别断言，避免仅凭模块出现判通过。
控制器的包目录带空格，目标复制为中文加空格文件名；保留原始两词、引号、
尾反斜杠、空字符串、中文/希腊字符参数。每种位宽均有裸启动原生对照。

| Matrix | 有限范围及完成标记 |
| --- | --- |
| launch/automatic | 两宽×原生/CLI；每目标初始和新GUI线程，A/W四root及28个动态六类控件、原生状态/自定义消息/标题、字体；两个线程完成及入口/末尾 witness |
| launch/dialogs | 同四目标，每目标两线程各八 API：named/indirect、A/W、modeless/modal；共64调用，modal返回47，根附着与观察标记 |
| launch/messages | 同四目标，每目标两线程各六 MessageBox APIs；共48调用，根及单按钮附着，公开返回IDOK，12条观察/两个线程/目标末尾标记 |

全部运行于控制器自有私有桌面；观察 hook 只关闭本夹具自有标题的消息框。
GUI线程等待使用消息分派，不能以阻塞 USER 的等待制造假的窗口故障。
这些是自动生产入口验证，不是用户物理鼠标/键盘验收，也不是产品 demo。

## Failure disposition

s2-bootstrap 首轮编译拒绝原对话框宏的常量条件 C4127；对 modeless 销毁分支
改 C++17 if constexpr，保持 /W4 /WX。s2-automatic 实际 CLI 双宽基本矩阵通过。
s2-dialogs 扩展入口/末尾 witness 后，x86 2/2、x64 5/5，dialog矩阵93.25秒通过。

s2-message-matrix 中基本/对话框通过，新增消息框原生32位对照失败；聚焦
diagnostic/native-detail 显示本机原生“OK”按钮实际内部ID为2，仍是唯一按钮。
夹具错误地假定GetDlgItem(IDOK)有效；改为枚举唯一BUTTON并验证附着，保留
公开 MessageBox 返回IDOK断言。生产代码未为该原生实现细节修改或降低契约。

## Verified results

s2-message-matrix（PID38156）中的中文文件名基本矩阵3.58秒、八对话框矩阵
94.24秒通过；消息框失败只用于上述定位，不将整轮计为通过。
s2-messages-verified（PID40388）聚焦 payload/messages：x86 payload1/1，
x64 payload及messages2/2，消息框矩阵95.22秒，exit0。四个目标各有12条
消息框观察、两个GUI线程完成和入口/末尾标记，不把CLI零退出单独作为证据。
s2-passive（PID14236）两宽各ABI/frame2/2，普通加载仍允许原生窗口后显式安装。
无生产行为变更，已通过的基本/对话框矩阵不为诊断文件输出重复运行；S3完整复验。

DLL SHA256：x86 be94a4a83782318e0f13852c203359f937c7a4c44bddbc0796403ec84ec5d32f；
x64 f32d7016f55f67d6dcdfc54bbdadac437bff2890914c2254d6b5522e22f0e718。
两宽31名字/32实际ordinal（1 NONAME）、静态CRT、六系统DLL imports核验通过，
版本0.3.0.0/M0 T3。77本地回执、来源23/治理13负例与diff --check通过。

P1 fc12e3c已推送；实际20文件+629/-54，git show --check通过。协调角色
读取实际entry事务、窗口scope/私有绑定、ABI/helper及夹具/回执差异，未发现
超出T3范围或未处置S2失败。关闭S2，进入S3；整个T3仍待任务级审计。

实际 staged numstat：src +209/-9（含组件说明），tests +218/-1（含8行资源），
CMake +22/-0，PE工具 +3/-3；净新增439行。窗口/字体/清理仍由唯一renderer
所有，新层仅负责原API事务/作用域，没有旧本地hook家族或第二renderer待保留。
