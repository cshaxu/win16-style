# M0 T1 S2 独立化设计与工具链

准入依据为用户整个 T1 请求，S1 e26b444 推送并实际复核后继续；S2 closed。
没有产品源码导入。选定方案见 import/INDEPENDENCE.md；34 个来源公开声明逐项
记录在 abi-decisions.json：32 保留 public、2 合并后转 private，新项目 ABI 3；
helper 单独以 ordinal 1 NONAME 保留 OS 安装边界。

来源集合保持 124 文件不扩张。ABI 决策将 api.h 从 retain 改为 adapt：
版本宏需要修改，这是新增研究结论引起的明确处置细化，不是静默变化。
五个原图资源头继续 replace/零原字节导入，产品导入状态仍 not-imported。

## 覆盖与设计审计

冻结单位为：ABI/导出、普通/目标/helper 三加载模式、最小载荷、原生创建事务、
跨位宽路径/超时/Unicode 约束、线程/资源所有权、双宽工具链、支持面接收者。
每个单位在 INDEPENDENCE 或 TOOLCHAIN 有决定/实测及后续证明责任。
DllMain 有限 detour 引导与 renderer 延后 GUI 初始化分离；不能复制原 Hook 的
DisableThreadLibraryCalls，因为静态 CRT/TLS 必须保留线程通知。

相似问题搜索固定源码 entry.cpp/context.cpp/installer.cpp/window_titles.cpp/
creatwth.cpp：原 helper 的 W 清理仍存在 INFINITE 等待，规定同一 10 秒截止预算；
A helper/创建包装器不调用。MAX_PATH/ACP 转换显式失败或可验证短路径，不能
静默截断/替换；复制载荷去除 NTVDM 身份/句柄，原生目标位宽用 SDK 查询。
保留挂起时只证明暂存完成，不用恢复目标等待 ready；后续实际目标入口/附着
测试接收这个区分。34 条 ABI 清单的分类与两宽导出核验由核心/启动任务接收。

## 工具链实测与局限

tools/Probe-Toolchain.ps1 启动拥有日志/退出结果/硬超时的后台任务；首次受限
worker 无结果不计成功，允许工具执行后重跑 PID 26304 已完成，exit 0。
MSVC 19.43.34809，SDK 10.0.22621.0，CMake 4.3.3，Ninja 1.13.2。
实际两宽均 /MT /W4 /WX，C17/C++17 DLL/EXE 编译链接及 CTest 1/1 通过；
SDK 位宽查询和 TLS 多线程隔离正确。tests/toolchain 断言未被 NDEBUG 取消。
只有工具链探针通过，未声称 renderer/Detours 或真实 UI 通过。

治理正向/13 负例、git diff --check 通过。后台 worker 已结束；scratch 在
100 MB 预算内，待 T1 精炼证据后统一回收。没有外部系统配置/安装变化。
生产代码 0；新增测试/工具行数在实际提交统计复核记录。

P1 9cf7183 已推送并实际复核，15 文件 +662/-21，git show --check 通过。
测试 C/C++ 为 +68/-0；测试 CMake +28/-0，工具 PowerShell +52/-0；
产品代码仍为 0。实际提交包含设计/ABI 结论与双宽真实探针，没有产品导入。
源 api.h 的 adapt 细化有明确研究理由与接收者；S2 无阻塞，继续 S3。
