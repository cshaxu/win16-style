# M0 T26 S1: shared UWP target dispatch

`lib-style/uwp/dispatch` now owns the existing target-association revalidation,
frame host process bitness inspection, caller-supplied action selection, module
image validation and target-thread dispatch transaction. It uses the existing
shared process-image and window-dispatch components; it does not implement a
second hook transport. The caller supplies the expected machine, protocol
names, export, timeout and action callback. Win16's title/fullscreen capability
policy and its message names remain in the product adapter.

The launcher, DLL and global manager use that adapter, which now directly calls
the public transaction. Existing order and x64-only Win16 host policy remain
unchanged. The standalone test creates its own hidden synthetic frame and core
windows and exercises the real fixture hook: success, target error, caller
cancellation, architecture mismatch, identity mismatch and result cleanup.
This is not an actual UWP application; real host activation, visual behavior,
access-denied conditions and alternate-style consumption remain unverified.
T26's wider audit and extraction remain open.

The clean x86/x64 MinGW GCC, Ninja and ccache build passed from snapshot
`2f92eb91d650b6b43bd02058924f4899cb5d7a8e9b5b6db546117ec676a881b4`.
All eight files were published to assets and `O:\winnt` with matching hashes
before testing. Both widths passed the new synthetic-target transaction test,
16 component tests, two published-launcher probes and two direct protocol
workers: 42 focused checks. The transaction fixture uses a real hook on the
current test thread; the separate existing window-dispatch test covers a
test-owned child process. The clean shared dispatch archives have no unresolved
Win16, Win2000 or product-private symbols. Real UWP host, cross-architecture
targets and alternate-style integration remain unverified.

## 后续分阶段对照

当前工作区探针在 32/64 位均显示 DLL 加载和导出解析成功；向同进程线程传模块句柄时 `SetWindowsHookExW` 返回 126，空句柄可成功，故共享分发器已仅对同进程目标选空句柄。同线程及同进程另一 GUI 线程的分发均通过。自建跨进程探针在映射盘、沙箱临时目录及沙箱外本机临时目录均返回 126；沙箱外映射盘对照亦返回 126。以上只能排除这些单一因素，不能断言操作系统策略或真实 UWP 宿主的根因。沙箱外临时探针已清理。当前修复未发布。
