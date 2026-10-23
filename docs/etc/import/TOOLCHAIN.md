# T1 S2 工具链实测

本机实际环境：Visual Studio Build Tools 2022 17.13.5，MSVC 19.43.34809，
工具目录版本 14.43.34808，Windows SDK 10.0.22621.0，CMake 4.3.3，Ninja 1.13.2。
由 vswhere 发现 VC 环境；工具安装路径不成为仓库约定。PATH 中的 CMake/Ninja
可来自其他工具包，本次 C/C++ 编译和链接实际均为 MSVC，不是 GCC 产品构建。

可重现入口：`pwsh -NoProfile -File tools/Probe-Toolchain.ps1`。
它启动隐藏、拥有 PID 的 worker，返回 result.json 路径；worker 分别 vcvarsall
x86/x64、独立 Ninja 配置、Release 构建、CTest，全部输出到 build/t1-toolchain。
总预算 120 秒，超时终止自己启动的子树并记未定结果；不复用已有 scratch。

第一次受限环境的后台 worker 未留下结果，不计测试结果。改用允许执行工具链
的权限后，带 worker stdout/stderr 日志重跑，PID 26304 完成，result status
passed、exit_code 0。没有自动审批拒绝；不把首次没有结果的尝试计为通过。

| 架构 | 实际构建 | 实际运行 |
| --- | --- | --- |
| x86 | C17 DLL + C++17 SDK 单元 + C17 EXE，Release /MT /W4 /WX | toolchain/x86/sdk-dll-tls 1/1，通过，width=32 |
| x64 | 同一探针源码与 flags，独立配置 | toolchain/x64/sdk-dll-tls 1/1，通过，width=64 |

探针验证 SDK/Kernel32 的 IsWow64Process2、C DLL 导出与 EXE import、线程本地
计数的主/子线程独立性和主线程状态保留。断言是显式运行分支，不受 Release
NDEBUG 禁用。只有 probe DLL 与 fixture EXE，不是本项目四个产品。

证明局限：这证明当前工具链能够构建和执行匹配 ABI 的两架构本地测试，
不证明 renderer、Detours、跨位宽安装、任意宿主版本或 Win16 UI 已通过。
完整日志/缓存不提交；精炼结果晋升本页后，由 T1 执行者停止/回收 scratch。
