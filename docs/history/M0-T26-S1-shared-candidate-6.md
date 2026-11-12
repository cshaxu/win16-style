# T26 S1 第六批共享主题候选

同一快照清单 SHA-256：`2fb851d3a4661ded9e8522aeb68c0cb253b044c4663f503c12976336b7c196f2`。双位宽干净构建、ABI、公共库二进制边界、输入路径和八文件包检查通过。按用户要求先发布后测试；`assets` 与 `O:\winnt` 的八文件摘要由发布脚本逐一核对一致。

| 文件 | SHA-256 |
| --- | --- |
| demo.exe | 114b299e5a8ae94030f39b2785ee0ec57f1b2feb9ff73f910fc3fa73e3d860b7 |
| ntw16s32.dll | a9abc0ce538b2ddcbac274c67e5f5be8cd90f6b215f7889e08d60115c254fb41 |
| ntw16s64.dll | 0e1fbcb8c430b8bd8a35cb20625e95e4baa402c0fc4fb668642af0d58d0c20a6 |
| ntw2ks32.dll | 46e9ac61bda234ea041e8e3d54cff936283fe51dbf9d1ea6f19bd5d95f172670 |
| ntw2ks64.dll | 5eddbaf9ebf0194bcf6f208e2754b60cf137156198ab355b82c9519b057f6990 |
| ntwshk32.dll | a9ef33ebca6123b0afa586b54ae092616d958bc75a9251f1c1781d583f24ca5a |
| ntwshk64.dll | 5b1910a919b818462171fa76f7073cc2ff905c53a626c77ed10d27288cbb48a2 |
| run16s.exe | fd05328fa8c0d3fd5aad7d3cf5c35cc923e871b028cf77d79995e603d0742f97 |

发布后从上述发布 DLL 的逐字节副本运行双位宽自建探针：控制台懒加载、共享 DLL 异常、COMCTL32 v6、启动期加晚期主题钩子链、控件所有权与线程负例均通过；使用 `O:\winnt\run16s.exe --wait` 的 Win16 启动后 Win2000 控件附着/移除通过；Win2000 父子进程启动、传播及控件和外框探针通过；自建窗口的两种风格互切与子窗口枚举通过。

**失败与未覆盖**：使用发布 DLL 副本的 Win2000 外框直接生命周期测试在 32/64 位均返回 3，消息泵后 DWM 非客户区原状态未恢复。以同快照测试夹具执行的 UWP 分发测试在 32/64 位均返回 6，线程钩子返回 126。这些均不计通过。真实全局会话、实际 UWP/WinUI 应用、完整包回归和用户验收未覆盖。未使用 computer-use，未创建或切换私有桌面。此包为已发布候选，不能据此收口 S1 或 T26。
