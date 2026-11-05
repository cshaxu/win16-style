# M0 T21 S36：process-launch 来源

固定来源 ntvdm64 `ac8bb9864c28fd5cbc58e4a888fb407685d69a0e`；逐项 Git blob 与 SHA-256，目标路径相同。

| 来源路径 | Git blob | SHA-256 |
| --- | --- | --- |
| `src/lib-win32/process-launch/attributes.c` | `745605017fdaef0f8b620da5310c684da559f035` | `5bb012796b56d581a93b95b012d5a5fc60b86f1ee2a30e97bf911f36ae562cc6` |
| `src/lib-win32/process-launch/attributes.h` | `c09e38aabefa06def087090c74d557fb59ca822c` | `dc527f1c3fafd49dfe3aac5c9c5f98418f8423bbb742f69e6adb9a2affc1ced8` |
| `src/lib-win32/process-launch/command_line.c` | `158bc62d59e55562d91a4accddd365d330fd7445` | `2e091f08db0ffaba49f6cb426d4358d13fe74b9765cb549154eadf4e3a35c7cb` |
| `src/lib-win32/process-launch/command_line.h` | `86edd4c5a3166946ad7a841b30f653b1c5d7a1be` | `f210c0f3478c6f56c9b51878a9dd3bce9d25937ded30cdd564e0f0ce24a4b7a9` |
| `src/lib-win32/process-launch/process.c` | `6179728a8e18ae6e0f15ddf3663d65f274734cb4` | `78310b75c2ad31f69085aedd232a71a334d3244edef3169ebf6c7267158398e3` |
| `src/lib-win32/process-launch/process.h` | `29977116855ce41efa7a4606b32bc91523d79dc6` | `d9ae0310e11633f073d336388a061a8fb9f8b11fcaef4a733174b588bd69305f` |

测试源 `tests/app/win32_process_attributes_test.c` blob `53981df1c8d6efa8d5a82e919ca1fd9383a0834f` 原样存入 `tests/lib-win32/process-launch/attributes.c`。

用户明确要求全部 LF：上表记录来源 blob/SHA；process.c 与 process.h 仅将来源 CRLF 转为 LF，其余内容不变。其他四个来源文件保持原字节。原 LF 门禁及 Git 属性保持，无例外。

## 交付和验证

四个来源文件逐字节一致，process.c/h 只做 CRLF 转 LF。启动事务搬至 run16s-exe，创建调用通过公共 win32_process_create；原有准备回调、仅自有未交付子进程回滚、句柄生命周期和等待策略保持。公共组件新增 attributes，并未改写隔壁实现；原本的命令行接口不变。兄弟仓库仅作固定来源，不是构建或运行依赖。

s36-clean2 GCC 16.1.0 / Ninja / ccache 4.13.6 双宽干净构建、ABI、原资源和来源门禁通过；组件的属性生命周期、进程生命周期及命令行双宽共 6/6 通过。s36-package2 实际同批四文件聚焦 15/15、无跳过，包含双宽模拟 UAC、CLI 失败、嵌套 cmd 和传播基线；真实 UAC、完整 UI 和全部集成矩阵未测。之前的 s36-clean1 和 s36-package 是用户改为 LF 前的证据，不作为最终包验收。

s36-publish 已发布至 assets 根目录和 O:/winnt，摘要与测试包相同：

- run16s.exe `40fdab265a2ce9c4e5dcad4dd048e42a869fe68afd7845d1d275eba96ec44f5b`
- ntw16s32.dll `cdb1f891e264c9e3ed938cf0ced6660ed6ec8ae9bf4b4e60361e583c5f23315f`
- ntw16s64.dll `99254853e145383b19e219f174cc06e40a769739dc4c4ebc47600402b508b3f8`
- demo.exe `440437e7d4890459109846902e41c7860ab82af7de0af496f2be55122712c591`

P1 `62ac03336197b40765b6a9e7f437fc72ce5d4402` 已推送，远端树实际复核六个公共库文件、启动器事务和四个产物，两处发布摘要再次核对。S36 关闭，T21 保持开放。
