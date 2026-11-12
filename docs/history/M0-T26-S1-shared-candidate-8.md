# T26 S1 第八批共享候选

快照清单 SHA-256：`03af7d59776f4950943cc4a09ebd0f401c77ee7596f9a5117b2d7f70c107458c`。MinGW GCC、Ninja、ccache 双位宽干净构建、ABI、公共依赖边界和资源检查通过；先发布后测试。assets 与 O:\winnt 的八个文件摘要逐项一致。

| 文件 | SHA-256 |
| --- | --- |
| demo.exe | e7f6c8991794e0ae3265ec762e6469805316017d9db0b723d04469c1d8bdd718 |
| ntw16s32.dll | 7312f3c3224c6d8363db06321ab43b63f609ab09ac0c49a6a32d5dc844694252 |
| ntw16s64.dll | 15cc1becf7a894d47b0dab3d26a23bcae3e34841d00d85faafdac1ad71fc8493 |
| ntw2ks32.dll | 1ca034d1a123a5ac97a7ea295eadc735a6e169ade14383f2d7caa62b5349348a |
| ntw2ks64.dll | 58ad0c4830ef1a960a707c541ae2488992b5f70a93acc999e32292ad3a0f97b3 |
| ntwshk32.dll | 8eda6ee10ef7e9955b588773bbf1f3206892a0555326ad404cb5381ddb1c2f78 |
| ntwshk64.dll | 4c5fd7950e7f656da28a2bb554ce926ffcfc19f34cd02a60af344092f915025f |
| run16s.exe | e4ce0b7d3db6dfafb9272ac8d44408f83a6bea476fe03bd390a6fbfbe96a11a5 |

发布后使用 O:\winnt\run16s.exe、自建目标和与发布 DLL 摘要一致的构建 DLL：x86/x64 的 Win16 与 Win2000 普通对话框、查找替换、六个消息框入口通过；公共对话框错误处理与异常清理、同线程/跨线程子窗口过滤、两风格子窗口互切、受控表面提交和主题钩子链通过。

未通过或未覆盖：双位宽自建真实分层表面提交报错 5 并标记 SKIP；真实跨进程窗口分发错误 126，UWP 全局自建目标准入错误 5，Win2000 DWM 状态恢复退出 3。此前 Win2000 打印夹具及真实窗口回调异常未通过；真实 UWP/WinUI、正式全局会话、完整包测试及用户视觉验收未完成。没有创建或切换隔离桌面，也未使用 computer-use。此候选不构成 T26 收口。
