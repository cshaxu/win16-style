# Product verification

当前构建使用独立x86/x64 MinGW-w64 GCC、Ninja、ccache、CMake、Python及PowerShell 7。
不需要C++、MSVC、来源仓库、NTVDM或系统样式DLL安装。配置与独立包步骤见
[构建说明](../docs/etc/operations/BUILD.md)；当前验证状态见
[CURRENT](../docs/states/CURRENT.md)，不能以注册数量或旧测试结果声称通过。

```powershell
pwsh -NoProfile -File tools/Build-Core.ps1 -TaskRoot t6-package -RunName full-new -TestPattern '^(core|launch|propagate|demo)/.*$' -TimeoutSeconds 600
```

入口返回隐藏worker PID及结果位置；必须读取最终result.json、逐架构JUnit、
run.log/run.err。RunName必须全新。每架构独立600秒预算，依次构建并测试，
总上限1200秒；超时为未定结论。稳定树为build/.ninja/x86、x64，ccache为
build/.ccache；TestPattern可选择focused测试，匹配零项失败。

T6 S1当前注册x86 24项、x64 35项。包含生产C异常清理与原生补丁事务、ABI、
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
