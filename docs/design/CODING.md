# Source Layout

导入任务按需建立目录；当前不放置虚假实现或空构建目标。

```text
src/run16s-exe/       x64 CLI / 首次启动
src/ntw16s-dll/       双位宽 renderer / interception / injection
src/demo-exe/         x64 原生示例
src/lib-win32/        内部 Win32 静态库：托盘、进程管理和挂钩等基础能力
src/lib-style/        各年代样式组件及可跨风格共享的样式机制
src/lib-style/common/  不依赖具体样式的窗口输入、生命周期和对话框基础机制
tests/run16s-exe/     启动契约测试
tests/ntw16s-dll/     双位宽 DLL 单元及生命周期测试
tests/demo-exe/       原生 demo 边界测试
tests/integration/    真实注入、跨位宽链及 UI 观察
tools/               构建、打包、检查与观测工具
assets/       四产物的发布包
build/               全部生成物（忽略）
```

内部静态库不产生第四个产品组件；启动器和 DLL 通过明确的内部构建接口使用共享代码。
`lib-style/common/menu_input.h` 由 Win16 与 Win2000 窗口层共同使用：菜单命中路由、助记键、键盘动作和指针切换判断统一，菜单几何、绘制和跟踪状态由各样式持有。
demo 只依赖 Windows API。产品 ABI 与私有头分离，消费者不得
通过包含 renderer 私有头绕过责任边界。

目标构建基线：纯 C17、MinGW-w64 GCC + Ninja + ccache，警告作为错误，
x86/x64 使用独立工具链与稳定 build/.ninja/x86、build/.ninja/x64；CMake 可作
Ninja 生成器。MSYS2 是可选构建环境，发布物不依赖其运行时。T6 S1已完成C/GCC迁移、干净重建及
同包聚焦测试；完整回归和M0验收以CURRENT为准。旧MSVC结果不是新基线证据。不沿用 ntvdm64 的个人
路径、历史 T 编号脚本、服务发布目录或外部生成图。构建工具实际可用性由首个
实现任务验证。T2 已完成双位宽核心 DLL 构建，T3已有x64启动器；T5已有纯原生x64 demo。

build/.ninja/ 与可选 build/.ccache/ 可复用；其他 scratch 必须在活动包登记
精确目录、用途与回收责任。证据先提炼到跟踪文档，任务结束后回收不再需要的
scratch；每次测试完成即按执行规则清理已不再需要的临时构建及副本，
仅为未完成的后续验证或发布暂留有明确用途的最小集合。不得清理用户目录、
系统目录、其他任务正在使用的内容或跟随 reparse point。
产品包四文件均在同一目录，构建输出与正式发布相互区分。

T1曾以独立MSVC C17/C++17、/MT双架构探针验证旧基线；该探针及旧入口
已在T6 S1退休，历史可从T1提交复取。当前工具链直接构建实际产品和C测试，
入口为tools/Build-Core.ps1，不能用历史探针结果替代当前GCC整包验证。
本机双架构 SDK/DLL/TLS 实测已通过，结果及证明局限见
[工具链记录](../etc/import/TOOLCHAIN.md)，不等于产品构建成功。

## UWP window style library (T22 S1 candidate)

`src/lib-style/win16-window-uwp/frame.{c,h}` owns same-GUI-thread apply,
remove and surface lifetime; `frame_layout.{c,h}` owns geometry and hit testing.
Matching tests are in `tests/lib-style/win16-window-uwp/`. Public functions use
`win16_uwp_frame_*`, without product names. The library reuses `win16-window`
and `win16-common`; it does not call the launcher, process hooks or activation.

The caller supplies the root and native title HWND, runs on their GUI thread,
and retains the containing module until callbacks have returned after removal.
`ntw16s-dll/uwp/hook` owns the exported thread callback, message protocol and
module pin; `run16s-exe/uwp` owns AUMID activation and target/host selection.
A future `win2k-window-uwp` can provide a parallel style implementation at this
boundary. No unimplemented theme-dispatch framework is introduced here.

T22 S1 native title actions: the frame library copies an optional action array;
labels and contexts are borrowed until removal. Menu commands are temporary,
collision-checked and removed before callbacks. Product `uwp/native` selects
the witnessed native title and queries/invokes its full-screen UIA provider.
Coordinator queries run outside the target GUI thread; menu invocation uses
an independent MTA worker with a module reference. OLEAUT32 is an OS dependency,
not an additional product. This does not style XAML controls or enable global UWP.

T22 S1 P22 launcher maps resolved System32/SysWOW64 calc.exe and byte-identical copies to the installed
Calculator package, forwarding activation arguments and optional wait. Other
same-name images retain native launch. Activation-returned reused instances are
accepted; created_pid remains zero for reused processes. This is explicit target
activation, not global enumeration or host propagation.

T22 S1 P32 candidate: product `uwp/target` shares frame discovery and targeted
apply/remove coordination between explicit activation and the global manager.
Product `uwp/global` tracks individual CoreWindow/frame pairs using public
process-global identity, queue and worker primitives. The x64 manager observes
existing and new windows; the ordinary host exclusion and propagation policy
remain unchanged. The shared host receives only selected frame adaptation,
not client-control styling or propagation. Stop removes surviving adaptations;
the pinned DLL is not physically unloaded. Native fixture coverage and remaining
intermittent activation failure are recorded in CURRENT; not universal UWP support.
