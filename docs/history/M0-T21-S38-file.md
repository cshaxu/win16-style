# M0 T21 S38：共享文件原语来源

用户授权来源固定为 ntvdm64 `50668d7ab02e8bd463a17e0c54194f5c123f59bb`，本地为 LF；源码无文件内许可证声明，不假称覆盖第三方许可。

| 来源/目标路径 | Git blob | 来源 SHA-256 | 本地 SHA-256 |
| --- | --- | --- | --- |
| `src/lib-win32/file/identity.c` | `0ca44d96bbedf74afd3ae22b6bead81178bb3acd` | `69f6cc49da69919409b098a832ccbd9454457f44b35e238773b02af97c727dcc` | `69f6cc49da69919409b098a832ccbd9454457f44b35e238773b02af97c727dcc` |
| `src/lib-win32/file/identity.h` | `c7435199c388e685a752744596ba17f09375ac98` | `2ef85eb205659294495f3f2d38e4217cc5cef56923c00d8e11505a0996b5e750` | `2ef85eb205659294495f3f2d38e4217cc5cef56923c00d8e11505a0996b5e750` |
| `src/lib-win32/file/native_path.c` | `bf96c5cd00899bc34012dfe9b9d0ee00cf640a90` | `ee0abc65fb60b94bc6753a7c47e43a06c7fc72983ecad2bc279e85b98baa477c` | `ee0abc65fb60b94bc6753a7c47e43a06c7fc72983ecad2bc279e85b98baa477c` |
| `src/lib-win32/file/native_path.h` | `264119adcd2ec9062690c356dcc143091d4a3c9b` | `6146edd97ddeb7194009cca3dddb2b87ba4f4a7f18061f339f518294bafbb701` | `6146edd97ddeb7194009cca3dddb2b87ba4f4a7f18061f339f518294bafbb701` |

隔壁 `tests/observation/run16_image_classification_test.c` 中 application_path 含身份、缓冲和句柄数量检查，但整份测试依赖隔壁产品 classifier；本项目按公开 API 编写独立探针，不复制隔壁构建依赖，覆盖身份、路径和失败清理。

上游测试原文保存为 `tests/lib-win32/file/upstream_run16_image_classification.c`，来源 Git blob `13d7a613a8d45eaae44e2c67c1e24ae85948ae1f`，来源 SHA-256 `55dead41e02e2458f0ba3f5dc8f5dbf76e3349752f92556a8f31bf4b4d12d71f`，本地 LF SHA-256 `55dead41e02e2458f0ba3f5dc8f5dbf76e3349752f92556a8f31bf4b4d12d71f`；它依赖隔壁 classifier，作为来源参考而不纳入本项目必需构建。clean1 在加入该参考前由本任务主动停止，最终构建使用 clean2。

## 范围和验证

四个文件均与来源提交逐字节一致。process-hook、process-image、process-launch 和 handle 此前已导入；pipe、rpc、thread、crt-compat 等未发现本项目等价需求，不为导入而引入。`rg GetFileInformationByHandle src/ntw16s-dll src/run16s-exe -g *.c`：global_load 检查文件属性，global_session 验证文件属性并逐字节比较；隔壁同一文件身份判断及最终路径解析不等价，因而没有生产调用替换，win32-file 目前为独立静态库和测试入口，不宣称产品已消费。

s38-clean2 使用 GCC 16.1.0、Ninja、ccache 4.13.6 双宽干净构建、来源/资源和 ABI 检查通过；本地原生探针 core/file 在 x86 和 x64 通过，覆盖相同/不同文件、路径解析、无效输入、缓冲不足和句柄数。上游测试整份依赖隔壁分类器，仅以原文留存，不计作本项目运行通过。s38-package 实际同批包 13/13 聚焦通过、无跳过，含 CLI、嵌套 cmd 和传播基线。真实 UAC、全量 UI、网络路径、硬链接及重解析点未覆盖。

s38-publish 将同批四文件发布至 assets 根和 O:/winnt，摘要与测试包相同：

- run16s.exe `e3aa1fbeb0a6ec55f5104e52f7d0bf60ad44ffa93b951e3ae46e708ce82bd24e`
- ntw16s32.dll `bb468befe0b742f7679f8595ffa69aa197255915b8277e2d1287d2c98620b307`
- ntw16s64.dll `b6cdf28e3acbf958be053c74d2d67776653f66ba2152ad374cb4757917b8ccac`
- demo.exe `cff97866c448879f5bfa2db10038ef21f44c4540b43dce91cf0247ea9ee283d9`
