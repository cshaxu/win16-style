# M0 T21 S40：公共线程快照遍历

将 global_manager 两处重复的 Toolhelp 线程快照遍历迁入 `lib-win32/process-global/enumerate`，提供全量或按 PID 过滤的同步遍历、调用者回调和提前停止。启动时的桌面、进程去重与身份筛选，新进程通知后的权限、身份与延迟处理仍由管理器执行；PID/TID 是快照，调用者仍重新验证。ETW 新进程通知、项目专属会话命名和孤儿清理仍在 DLL 的 global_process，未宣称该监控器已抽离。`rg Thread32 src/ntw16s-dll/global_manager.c` 无命中。

clean1/package1 17 项通过并阶段发布；复核后将枚举器从安装器传递依赖改为管理器消费者直接链接，最终使用 clean2/package2/publish2。GCC 16.1.0、Ninja、ccache 4.13.6 双宽干净构建、ABI、资源和来源门禁通过；core/process-global 双宽通过自有 PID、线程、过滤、提前停止、无匹配与句柄计数。实际包聚焦 17/17、无跳过，含双宽 policy、manager、unsupported 和 cmd 嵌套传播。完整管理员 UAC、全部目标进程/桌面组合及动态线程竞争未测；Toolhelp 是瞬时快照，不保证遍历期间线程保持存活。

publish2 同批四文件发布到 assets 根及 O:/winnt，摘要一致：

- run16s.exe `2113f11423cf9951b036e60743c452ca208a25cc266022ea5f7d2a25fee8aec2`
- ntw16s32.dll `dee60222c146a70990136434ccf9aa116b34f63deec0e2ddc068c2685b69cb2d`
- ntw16s64.dll `e475cea0e23bcba9d82e1ec566c9453034bd68c12fdbc9bdd4d30b3d62fa4520`
- demo.exe `67d16bb202f85dc4a0db68de8d78cec51d9211fc8ebda820a73a6b0c0ac7eb85`
