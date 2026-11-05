# M0 T21 S37：handle 来源

用户授权从隔壁固定提交 `d524de6ad7d5edb83263a4cb0fdd2cb88713bf9d` 导入；源码和测试均无文件内许可证声明，不将本项目许可当作来源许可证。三文件按用户要求统一 LF。

| 来源 | 目标 | Git blob | 来源 SHA-256 | 本地 SHA-256 |
| --- | --- | --- | --- | --- |
| `src/lib-win32/handle/duplicate.c` | `src/lib-win32/handle/duplicate.c` | `95a87120c173e7fd00e0afee83ea76960ee64548` | `7036b95282ffa11d76305094c8efc076cf0a6a864ea3fd508c993e3ce3090596` | `7036b95282ffa11d76305094c8efc076cf0a6a864ea3fd508c993e3ce3090596` |
| `src/lib-win32/handle/duplicate.h` | `src/lib-win32/handle/duplicate.h` | `d7a2e90a161e125be71cd00cda4bf8c0b0732ce4` | `f39b055b6678a24d8c69e97496adaf46bc6b16322859e7a88f899a84aefbfd26` | `f39b055b6678a24d8c69e97496adaf46bc6b16322859e7a88f899a84aefbfd26` |
| `tests/app/win32_handle_test.c` | `tests/lib-win32/handle/handle.c` | `9f74389a1385dad4c918d1eb41b1301702052291` | `4b5032a349fcec5ff51546ddd0b66616e857bc35dab086e4b5313f378193cbbb` | `4b5032a349fcec5ff51546ddd0b66616e857bc35dab086e4b5313f378193cbbb` |

测试源码在来源基础上仅拆分两处同一行的 if 语句，消除 GCC -Wmisleading-indentation，断言和行为不变。初始 clean1 测试源语法检查出现这两项告警后，主动停止本任务构建，改用 clean2。

clean2 的 x86 构建通过，x64 因上游原样 FARPROC 转换触发 GCC -Wcast-function-type 且 Werror 停止；在 win32-handle 构建目标上仅禁用该项告警，保留上游源码和其他 Werror，再用 clean3 重建。

## 交付和验证

两个 handle 产品源码与固定提交逐字节一致。调用点搜索 `rg "DuplicateHandle|CompareObjectHandles" src/ntw16s-dll src/run16s-exe -g *.c`：global_load 与 global_start 的同权限本地句柄复制使用 win32_handle_duplicate，global_policy 的动态比较函数查找使用 win32_handle_compare_provider；调用方仍持有原句柄关闭、错误和不可用时拒绝策略。测试代码中指定不同权限的 DuplicateHandle 保留；远端关闭接口无产品调用，不改变既有行为。隔壁只读，未修改。

s37-clean3 使用 GCC 16.1.0、Ninja、ccache 4.13.6 双宽干净构建，ABI、来源和原资源检查通过；core/handle 双宽各通过，覆盖本地复制、同异对象比较、自有子进程远端句柄复制和关闭。s37-package 同批实际包聚焦 19/19、无跳过，覆盖双宽 global policy、load、start、attach、manager 及原有传播。真实 UAC、完整 UI 和全部系统/权限变体未测。

s37-publish 发布至 assets 根及 O:/winnt 并核对：

- run16s.exe `0fef5ee703ace68f46ab9a94e0088dfb215108868e969aa2811a3d00be251581`
- ntw16s32.dll `6c5daa4abc893e5e5150a8eb419559215cb209227ed6d77a27c0ae37930eec5f`
- ntw16s64.dll `f476fea3c65a723a4fcacc5e5e04b3e72cd3c595181742aa9b8a007fae1cd6b1`
- demo.exe `3179337fd8b48e02b75074b1aa60688e3118fba13c2bdce98d8e9cd7c386e6c8`

P1 `881e6982e5b506f3b21d4e27c74996a6aa415dc5` 已推送，实际远端树复核公共组件、三个调用点和四产物；两处发布摘要再次核对。S37 关闭，T21 保持开放。
