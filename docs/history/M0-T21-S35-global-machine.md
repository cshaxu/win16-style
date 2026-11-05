# M0 T21 S35：全局策略复用进程架构查询

用户要求 global_policy 删除自主 IsWow64Process2 查询。现调用 process-image 的 win32_process_image_machine，保留针对期望 I386/AMD64 的错误架构 193 检查，原进程、线程及身份状态所有权不变。搜索 `rg IsWow64Process2 src`：生产代码唯一命中公共 image.c；global_policy 的独立测试目标显式链接 process-image。公共库和隔壁未改。

正常 x86/x64 及查询失败语义保留；AMD64 主机上的其他进程机器类型由旧 193 改为公共函数 ERROR_NOT_SUPPORTED（50），不扩大支持。非 AMD64 主机仍返回 ERROR_NOT_SUPPORTED；其他架构未实测。

GCC 16.1.0、Ninja、ccache 4.13.6 的 s35-clean1 双宽干净构建、ABI、来源和资源门禁通过；s35-package 同批实际包 15/15、无跳过，包括双宽 global policy、attach、manager、cmd-nested 及传播基线。真实 UAC、全量 UI 和非 x86/x64 未覆盖。s35-publish 发布四文件至 assets 根和 O:/winnt 并核对摘要：

- run16s.exe `2808934477476dad2da88c98e824e9bcc4fb7a7cbe82a6b448dd7cb71d2974b9`
- ntw16s32.dll `7a7bc65d95492d9cefcaaa38ef8a8b563dd22ccad1e49dd103fa2292f65e8c2b`
- ntw16s64.dll `25862762838ef213fad8f46dfcc5c4854bd6b0280f0c6a2f792e2d653e5858f3`
- demo.exe `7a5f5a35e506adc540e7c14c6b36325f5b91323b33c9334500e4b4e079049ee6`
