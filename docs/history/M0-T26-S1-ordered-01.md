# T26 S1 顺序审计第 1 项交付

双位宽 MinGW、Ninja、ccache 干净构建通过，先发布 assets 和 O:\winnt，再验证正式包。

`win16-common/control_api.h` 提供现有 Win16 产品的 DLL 导入标记；产品 `ntw16s-dll/api.h` 原本重复定义同一宏，现引用该头，保留已有静态、构建及 DLL 消费者条件，不令公共库反向依赖产品。字体和配色声明继续使用该头。

正式发布包 x86/x64 ABI 测试均报告版本 3、38 个公开导出及无私有绑定导出；对应双位宽 Win16 公共库链接测试通过。该验证不代表所有运行时场景或其他风格已覆盖。

导航焦点、Win2000 DWM 恢复等既有失败及真实 UWP/WinUI、全局等未覆盖项仍开放。

| 文件 | SHA-256 |
| --- | --- |
| run16s.exe | 88c4da05789a5dc1de2588dad22520137a6a4fbc00f1983e15773bef4bb017c7 |
| ntw16s32.dll | 7d24d18617ce5a3084f68a6901a7f00297f37f27f1374e7df22998d2121dd643 |
| ntw16s64.dll | 59de1fe869d87dab3533f7d5ee883d03cb170b10615fc0a6d0614800e5f3cdbd |
| ntw2ks32.dll | 6c74b52902a9f9f0c78e67dc54b9a03851eba287bf25ce930e94c3200300d612 |
| ntw2ks64.dll | 6e6d102735ff3975a38719fe3081b4a66273b04f7614dc98fe8d7f7ba6aba028 |
| ntwshk32.dll | 9d6484579421e52ac70cc776714e9acdff387c5b2d354c840f1b0252040c989d |
| ntwshk64.dll | 121b1378da2f7fb4ac7c2b406c107eeabafedf03804809d2d221802f7161ff92 |
| demo.exe | 500db505f2d5ff2bceda7b4e68cd92af1c8308481be50013817c2382c999dabb |

源码快照清单 SHA-256：`36519ba98682ad87ad163af4b5d11ed5f66c4f2801434cd38c8ea875799bec9b`。测试后清理本批构建、候选及发布备份。

提交差异统计（不含本记录）：4 个文本文件，新增 10 行、删除 13 行、净减少 3 行，另有 8 个二进制文件。导入宏唯一归 Win16 公共头，DLL 导出表仍由产品定义；重复宏已删除。
