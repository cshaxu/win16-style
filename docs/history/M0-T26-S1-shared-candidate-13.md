# T26 S1 第十三批共享候选

双位宽 MinGW、Ninja 和 ccache 干净构建通过，源码快照清单 SHA-256 为 `cf5fa509c547241b1c10e128634136dc183ca852ae7b1ebbaa1298550420f176`。源码清单、资源原样性、治理及双位宽公共归档边界检查通过。随后先发布到 `assets` 与 `O:\winnt`；以下八文件与同批包摘要一致。

本批将原生与 VB5/VB6 控件类身份识别从 Win16 控件层抽入公共样式层，Win16 产品路径已接入；Win2000 未接入该识别策略。发布后使用同快照公共测试及自建 VB5/VB6 文件控件模型验证身份，x86/x64 均通过；Win16 控件独立链接通过。使用正式 `O:\winnt\run16s.exe` 的自建 VB6 模型，在两种位宽及 96/144 DPI 下验证普通和套样式控件、像素、输入、移除和资源回收，均通过。测试未创建或切换隔离桌面。

VB 文件控件完整产品夹具在该变更的增量构建及第十二批发布基线均因线程钩子返回 126 而失败，见[身份记录](M0-T26-S1-control-class-identity.md)；本批未重复，不能算通过。此前真实 UWP/WinUI 宿主、跨进程分发、Win2000 DWM 恢复及打印夹具的失败或未覆盖情况不变。S1 完整审计、提交、推送与远端复核尚未完成。

| 文件 | SHA-256 |
| --- | --- |
| run16s.exe | 01b8b67966c5b1d5f75e5f3d843793f953f29167bf17eb355105735d830de069 |
| ntw16s32.dll | 9018a864dbb8e952b6530daf85a063a1c174e61e78e968ae9e8031d1acc7ff1d |
| ntw16s64.dll | 2c71870bc8fb0a7e171852166a3718f7d91e2fed1335c5070ce8dcabf7e92b1e |
| ntw2ks32.dll | 78e65a1bbd0fc791514c10d21831de0c8360638017b6fd5628010b1672ec8577 |
| ntw2ks64.dll | 349e25bd1c8b2b875a056d2063c3e50e8d60483c36355dbee5169c623a8dc030 |
| ntwshk32.dll | 10d081b99796bb0dcd787a33e618f70270555dc57d85517665e5ec082651bafe |
| ntwshk64.dll | 4434e7ba5c35282c88a5a73d9ae0bb6f46dcdcf5842a407723a879844ebc1cce |
| demo.exe | 1029afc6e682f8bcd748a40cc3280c8646458d7256038c82dc5b4a8253e99d53 |
