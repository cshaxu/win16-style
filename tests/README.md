# Product verification

当前构建使用独立x86/x64 MinGW-w64 GCC、Ninja、ccache、CMake、Python及PowerShell 7。
不需要C++、MSVC、来源仓库、NTVDM或系统样式DLL安装。配置与独立包步骤见
[构建说明](../docs/etc/operations/BUILD.md)；当前验证状态见
[CURRENT](../docs/states/CURRENT.md)，不能以注册数量或旧测试结果声称通过。

```powershell
pwsh -NoProfile -File tools/Build-Core.ps1 -TaskRoot t9-package -RunName full-new -TestPattern '^(core|launch|propagate|demo)/.*$' -TimeoutSeconds 600
```

入口返回隐藏worker PID及结果位置；必须读取最终result.json、逐架构JUnit、
run.log/run.err。RunName必须全新。每架构独立600秒预算，依次构建并测试，
总上限1200秒；超时为未定结论。稳定树为build/.ninja/x86、x64，ccache为
build/.ccache；TestPattern可选择focused测试，匹配零项失败。

注册全集以实际CTest -N和本次报告为准；历史T6数量不是当前覆盖。包含生产C异常清理与原生补丁事务、ABI、
实际DLL窗口/字体/控件及资源生命周期、绘图/DC故障、菜单转发、原生滚动对照、
私有剪贴板、真实CLI/双位宽首次加载/八类对话框/六类消息框、四位宽边三代传播、
64组原生/注入创建字段、四组失败回滚、helper超时/目标先退出及长路径。

core/geometry直接编译生产几何函数；control-present/dc-failure直接编译并仪器化
生产绘图路径，并使用同样的v5 COMCTL32 ordinal导入契约。原生对照与样式结果
分别报告，失败不改断言来放行；不可用的私有window-station剪贴板返回77，列为
跳过及待验收，绝不自动改用用户共享剪贴板。

demo/boundary检查真实x64 GUI PE、Windows系统imports及无样式消费源码；
demo/pair由独立observer在私有桌面对同一个exe观察模块、PrintWindow、排版、
业务通知及窗口操作。demo不含测试开关。自动消息、截图和私有桌面不代表物理
鼠标/键盘、输入桌面焦点或任意DPI的真实交互验收。

GDI/USER检查同时断言已知窗口、最后字体lease和同负载预热后的逐轮资源计数。
原生对照记录资源，不将冷启动缓存当成DLL泄漏。证据先提炼到history，任务结束
按活动包回收scratch与Testing日志，保留稳定构建缓存。T1旧MSVC探针已退休，
历史代码及结果保存在其实际提交中，不作为当前GCC构建入口。


## 真实交互式嵌套 cmd

新增纯C `tests/integration/cmd_nested.c` / `cmd_support.h`，不是单层cmd /c smoke。
从标准输入驱动外层原生cmd，再启动交互内层cmd（/k只初始化本例独有变量），
成功标记由内层echo组装，驱动器不把完整标记写入输入管道。验证实际cmd.exe绝对
路径/PE位宽/PID/创建者、DLL路径/SHA及ntw载荷/commit/hooks，内层exit /b 37、
外层恢复的errorlevel37、外层exit /b 23、包装器实际退出码及owned job清零。
日志中的nthook初始化结论来自实际原生子代继承，不能把模块存在单独算成功。

必需自包含回归为 `propagate/cmd-nested`，四边32->32、32->64、64->32、64->64；
32->64使用Sysnative进入真正System32 cmd。CTest从双宽构建输出复制校验三文件
到自有scratch包；实际包测试 `tools/Test-Package.ps1` 直接使用同批candidate。
`launch/staging`另在双宽覆盖可读外部导入表/真实初始化窗口、NOACCESS、1024条
未终止表及RVA末端越界，不需要Detours、run16、NTVDM或兄弟仓库。

在已双宽构建后，选择全新scratch（原始PID/路径/转录只保留本地，不公开提交）：

```powershell
$env:WIN16STYLE_CAPTURE_DIR = Join-Path $PWD 'build/t9-package/cmd-local-new'
New-Item -ItemType Directory $env:WIN16STYLE_CAPTURE_DIR
ctest --test-dir build/.ninja/x64 -R '^propagate/cmd-nested$' --output-on-failure
# 或对实际四文件发布包运行
& ./build/.ninja/x64/cmd-nested.exe --core (Resolve-Path assets).Path
# 可选本机兼容：真实 run16 cmd、run16s run16 cmd，四位宽边分别报告
& ./build/.ninja/x64/cmd-nested.exe --compat (Resolve-Path assets).Path 'O:\winnt\system32\run16.exe'
```

可选CTest名称 `compatibility/cmd-nested`；默认WIN16STYLE_OPTIONAL_RUN16为空，
明确返回77/SKIP，不计通过。可配置该CMake FILEPATH或按上例直接运行。可选模式
每例用CreateProcessWithLogon NETCREDENTIALS_ONLY创建新AuthenticationId，检查
端点原先不存在/未监听，再只对本次自有headless Console做FreeConsole/AllocConsole
以满足后端Console窗口前置条件；用户Console/服务/VDM完全不动。每case独立job、
私有桌面、超时；缺少后端或无法创建隔离登录/Console时明确skip。

输入输出记录、原生root创建错误、实际子进程退出码和cmd ERRORLEVEL分别报告，
不凭“不兼容16位”弹窗推断NE。失败后可记录并关闭仅属于本次job的模态警告以收集
ERRORLEVEL，case仍失败；不会将自动关掉警告当成功。旧staging源/旧发布包负例
与新包结果必须分列，查看CURRENT/history中的本次实际结果，不能靠测试注册宣称通过。

## 调度与快速入口

`tools/Build-Core.ps1 -Profile Quick`只构建quick-tests目标；Full保持所有注册测试，
不是减少覆盖。详见[构建入口](../docs/etc/operations/BUILD.md#快速开发与持久缓存)。
`-Targets`使用逗号分隔目标，`-BuildJobs`默认8，`-TestJobs`默认4。重UI每例占2
slots，其他例可并行；私有clipboard使用资源锁并继续如实skip。测试断言/轮数/
超时不变。各CTest现在有唯一工作/捕获目录，不再共享架构根目录；环境变量
WIN16STYLE_CAPTURE_DIR必须指向本任务已登记scratch。checks/test-runner测试实际
子进程看到的cwd/env、并行目录隔离、空参数、失败码42和skip77透传。

CTest与包测试分别报告阶段/每例及总耗时，公开证据仅提炼计数/摘要，不提交真实
PID/路径转录。不要并发运行会改同一构建树的两个build；干净树与稳定树独立，
只共享有界编译缓存。用`-Jobs 1`或`-TestJobs 1`做受控串行诊断，不重跑无关完整包。

launch/legacy-rejection使用合成MZ/NE，经直接启动器和真实cmd两条路径捕获首次
stderr的193/退出4；仅观察自有Job窗口、拒绝额外Shell对话框，等待内核Job归零通知，
再核对严格活动计数0和预期总进程数。旧Shell重试版本必须失败；无后端依赖。
