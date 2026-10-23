# Core verification

在 Windows 安装 MSVC x86/x64 Build Tools、Windows SDK、CMake、Ninja、Python，
从仓库根运行；不需要来源仓库、NTVDM 或已安装的样式 DLL。

```powershell
pwsh -NoProfile -File tools/Build-Core.ps1 -RunName core-full -TestPattern '^core/.*$' -TimeoutSeconds 360
```

入口立即返回后台 PID、日志和 result.json 位置；必须读取最终 result.json，
启动成功不等于验证通过。RunName 必须使用未存在的名字。Release /MT /W4 /WX
两架构稳定树为 build/.ninja/x86、x64，真实 DLL 和 fixture 均在各自树中。
每架构 13 项；单项 100 秒（ABI 20 秒）、隔离子进程 90 秒、总预算 360 秒。
可用 TestPattern 选择 focused 测试，匹配零测试会失败。

实际 DLL 验证包括 ABI、组合窗口、frame、字体、tabs、重入/销毁、默认按钮、
卸载与 sizebox。core/geometry 直接编译生产几何函数，属于数学单元检查；
core/runtime-native 与 core/sizebox-native 是原生对照，不声称覆盖样式行为。
窗口在私有桌面测试，不修改输入桌面或系统主题；资源检查对已知 HWND、最后
字体 lease 及同负载预热后的逐轮 GDI/USER 计数分别断言。原生对照记录计数，
不把其冷启动缓存增长当作 DLL 泄漏。

抓图经 WM_PRINT 写入本轮 scratch，不能证明物理鼠标、键盘、焦点或任意 DPI
交互验收。关闭任务时先将结论提炼到 history，再按活动包回收本轮日志/图片与
CTest 自动生成的 Testing 日志目录；保留稳定构建树的对象与配置缓存。
