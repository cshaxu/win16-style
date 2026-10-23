# T1 独立化设计与后续证明责任

本文件记录已选设计，不是产品实现或注入通过证明。产品设计权威为 design；
逐文件输入为 source-manifest，下面每个决定由对应后续任务实现并复验。

## ABI 与唯一实现

[abi-decisions.json](abi-decisions.json) 从固定源码公开声明逐项列出决定。
核心 public 名称/参数形状保留，使用本项目 ABI version 3，区别于来源 ABI 2；
不宣称对旧 NTVDM 二进制兼容。ABI 版本与构建版本 T 是两条独立标识。
公开 C ABI 保留 __cdecl、固定 Windows 类型、opaque scope、同 GUI 线程约束，
无跨 CRT 堆分配释放。x86/x64 各有导出核验，不能把 x64 导出名当作 x86 结果。

Ntw16sBindNativeApi 与 Ntw16sBindComboInfoApi 不再作为对外绑定：renderer 与
Hook 同 DLL，原 scroll/theme trampoline 通过 DLL 私有接口单点绑定；不导入
WOW 自有 combo 私有状态投影。其余接口保留以支持真实 DLL 的核心测试；demo
不消费这些 API。新增 Ntw16sFinishHelper 为 CALLBACK、ordinal 1 NONAME，
供 OS rundll32 安装事务使用；不保留 NthookAnchor/ContextFlags 等旧消费者 ABI。

Detours 源只编译一次到注入私有构建对象：DLL 使用窗口/进程拦截和 installer，
run16s 使用 installer；编译复用不是第五个产品或另一份 renderer。
显式列源，uimports.cpp 由 creatwth.cpp 包含，不再独立编译造成重复定义。

## 三种加载情形与时序

1. 无复制载荷的普通 LoadLibrary：仅供显式核心 API 消费，不自动影响进程。
2. Detours helper 进程：先 DetourIsHelperProcess 检查，跳过应用自动拦截，
   ordinal 1 只执行一次安装回调，不创建 GUI 或传播到 helper 的子进程。
3. 启动器/传播安装的新目标：恢复 Detours imports、验证有界载荷后，执行只涉及
   已导入 API 地址和内存事务的最小 detour 引导，登记当前线程；失败拒绝 DLL
   初始化，绝不“成功但无 hook”回退。字体/GDI、窗口附着、主题及 renderer
   binding 在后续拦截入口的 GUI 线程进行，不能在 DllMain 调用 USER/GDI。

采用 Detours 官方的 DllMain 最小事务模式，而不是在 DllMain 初始化 renderer。
此处的 API 入口改写是有限引导，不是额外线程/远程执行框架；不使用远程线程、
全局 CBT hook 或驻留服务。第一次真实窗口/对话框 API 入口先完成私有 binding
与创建作用域，然后调用原 trampoline，再完成附着与原错误值恢复。
CreateProcess/窗口/scroll/theme 的 trampoline 各只有一处原函数所有者；
不能通过再次 LoadLibrary 自己或另一个 renderer 来解决递归。

原 Hook DllMain 调用 DisableThreadLibraryCalls；合并后保留 renderer 的静态
TLS 且用 /MT，因此明确删除该调用，保留 CRT 所需线程通知。GUI 对象只在 owner
线程销毁释放；自动注入的 DLL 生命周期随进程，不支持应用任意 FreeLibrary
后继续运行挂钩代码。普通显式消费者必须在最后 scope/HWND/font/callback 结束
后释放引用。进程退出不在 DllMain 等待或补做 GUI 清理。

上述模式不承诺兼容另一个 DLL 在自己的 DllMain 中调用 USER/GDI 创建 GUI 的
程序；不得用私有 loader-lock 探测或轮询来掩盖此边界。后续启动任务验证早期
窗口、新 GUI 线程、对话框及 DLL 首次加载/销毁时序。

## 复制载荷与安装事务

独立载荷 version 1，固定 32 字节头（八个 uint32_t）：bytes、version、machine、
flags、directory_offset、directory_bytes、reserved0、reserved1；尾部是一条
以 NUL 结束的包目录 UTF-16。总长上限 65536，offset 从头大小开始，计数为偶数，
目录只允许完整绝对 DOS/UNC/扩展路径，不传借用指针、frontend/execution 句柄、
broker 身份或 session 语义。flags 只允许样式及传播位；保留位为零，位宽与目标匹配。
解析同时验证溢出、重叠、终止 NUL 及实际长度，不能 memcpy 后相信远端数据。
目标从固定包目录派生两个 DLL 文件名；不能从目标 CWD/PATH 搜索 DLL。

安装器仅借用已创建且本层尚未交付的挂起目标，使用 IsWow64Process2 返回的
实际 machine 选择同位宽 DLL；native machine AMD64 且 process machine UNKNOWN
才是 x64，process I386 才是 x86。错误/其他 machine 明确拒绝，不猜父进程位宽。
不引入原 nt.h/SEC_IMAGE 分类器。必要 PE 文件校验仅用 SDK 结构并严格验证边界。
创建请求原有 A/W、参数、环境/cwd、STARTUPINFO/EX、继承及 CREATE_SUSPENDED
保持原样；临时增加的挂起只在原调用者没要求挂起时抵消一次。

installer 的“成功”含义是 imports/payload 已正确暂存，不等于 DLL 已执行。
正常恢复后 Windows loader 先执行 DLL 引导，再到应用入口；若 DLL 拒绝初始化，
目标启动失败，不允许继续以无样式运行。原调用者要求挂起时不为等待 ready 而
偷偷恢复；此时只能返回已暂存状态。目标初始化失败的进程退出与创建前安装错误
分开报告，CLI wait/退出码由启动器任务明确。验收必须观察目标入口标记和实际
窗口附着，不能仅凭创建成功宣称自动初始化通过。

## 跨位宽选择及失败预算

同位宽用 DetourUpdateProcessWithDll；跨位宽使用来源已改造的有限
DetourProcessViaHelperDllsW，系统现有 rundll32 加载目标宽度的同一个产品 DLL。
x64->x86 选择 Windows/SysWOW64；x86->x64 用 Windows/Sysnative。路径从系统
查询所得，不硬编码 C 盘，不附带 helper EXE。不调用无限等待的 A helper 或
会代替创建者控制目标的 DetourCreateProcessWithDll 包装函数。

固定来源 W helper 的 10000 ms 等待后，在 TerminateProcess 后仍有
WaitForSingleObject(INFINITE)。此处不能原样保留：采用同一绝对 10 秒事务截止
时间，目标先退出和 helper 退出分开判定；终止 helper 后的回收使用剩余有界预算，
无剩余预算只关闭本方句柄并返回明确超时，不继续无限等待。安装器不终止目标；
失败由创建者中止它仍拥有且尚未交付的那个目标，不能影响父、兄弟或已交付后代。

Detours imports 的 DLL 名是 LPCSTR。包路径先做可往返的 ACP 转换，必要时尝试
存在且指向同文件的短路径；不能无声 best-fit 替换。不可表示且没有有效短路径
时返回 ERROR_NO_UNICODE_TRANSLATION；目标自身的 Unicode 路径/参数仍以 W
原样保留。完整任意 Unicode 包目录并未因此得到支持，必须作为发布说明中的
明确限制，或由后续启动任务取得有证明的改进后消除。MAX_PATH 相关 source
helper 缓冲在适配时改为与实际路径/命令上限一致并检查，不能靠截断“成功”。

## 来源支持面及接收者

| 面 | 已知来源边界 | 后续证明 |
| --- | --- | --- |
| frame/menu | 自编几何/命中、原生系统命令；素材改为自编 | 核心任务 normal/pressed、激活、最大/最小、菜单与 DPI |
| button/static | checkbox/radio/三态、push/default、label/group | 核心状态、焦点/键盘/通知及资源移除 |
| edit | 原始 border 必须在创建时捕获；原生文本/选择/undo | 核心及自动附着任务验证，不猜晚附着 consumed bit |
| list/combo | 字符串列表；tab state 无公开 getter，owner-draw/no-data 不泛化 | 核心明确已知 tab 状态和原生回退；自动附着任务保留边界 |
| scrollbar | 自编 fixed thumb；部分 16-bit native position 与 accessibility 限制 | 核心/自动附着覆盖已支持范围，宽范围不截断 |
| font | 宿主 System 生成 20px；CP1252 来源覆盖有限 | 核心复验实际度量/生命周期，不声称全 Unicode fidelity |
| 透明传播 | 四种父子位宽边、A/W、第三代 | 启动及传播任务真实生产路径和失败矩阵 |

## Primary references

- [DLL best practices](https://learn.microsoft.com/en-us/windows/win32/dlls/dynamic-link-library-best-practices)：最小 DllMain 与禁止操作。
- [Using Detours](https://github.com/microsoft/Detours/wiki/Using-Detours)：有界引导模式、trampoline、helper 检查和 ordinal 1。
- [DetourCreateProcessWithDlls](https://github.com/microsoft/Detours/wiki/DetourCreateProcessWithDlls)：imports 在加载器之前修改；本文采用底层 installer，保留创建者事务责任。
- [DisableThreadLibraryCalls](https://learn.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-disablethreadlibrarycalls)：静态 CRT/TLS 约束。
- [IsWow64Process2](https://learn.microsoft.com/en-us/windows/win32/api/wow64apiset/nf-wow64apiset-iswow64process2)：UNKNOWN/native machine 与 WOW machine 的区别。

这些资料是设计依据，不是本项目 Detours 安装或真实应用成功的实测证据。
