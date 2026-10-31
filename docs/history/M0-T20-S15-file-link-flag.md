# M0 T20 S15：普通打开框快捷方式保留标志

## 实际断点与方案

`GetOpenFileNameA/W` 请求 `OFN_NODEREFERENCELINKS` 时，原白名单拒绝该
标志，正式包维持现代框。使用自有 .lnk 和目标文件的原生对照，x86/x64
ANSI/Unicode 各四组均观察到：新旧框带标志时返回 .lnk；不带标志时，新框
返回目标，旧框返回 .lnk。正式包 x64 Unicode 不带标志时也返回 .lnk，
与原生新框不同；这是已有独立缺陷，不以本次修改宣称修复。

本 S 仅在打开入口允许显式 `OFN_NODEREFERENCELINKS`，保留标志交系统旧框；
保存入口、其他标志、应用 hook/模板及多选保持原判定。系统仍持有文件选择和
错误，未新造对话框或复制图形资源。微软文档对该标志的含义见
[OPENFILENAMEW](https://learn.microsoft.com/en-us/windows/win32/api/commdlg/ns-commdlg-openfilenamew)。

探针是在自有私有桌面预填绝对 .lnk 路径后发送确认或取消；不代表双击快捷方式、
目录链接、网络链接、不同语言或所有 API 参数均已覆盖。旧框默认不解析快捷方式
的缺口以高优先级进入 TODO，后续 S 需独立处理完整路径和错误契约。

## 验证

首次包测试 `s15-link-focus` 在进程创建前失败：注册项重复传入了框架已自动
加入的 `--private`，驱动返回 6；产品代码未执行。修正注册后，用同批候选包
运行 `s15-link-registration-check`，32 项通过、零跳过，包括双宽 A/W 新旧
原生对照、注入后旧框确认与取消及必选包检查。此为修正注册的前置检查，最终
同源码快照仍需重新干净构建和实际包回归。

`s15-link-final2` 完成 x86/x64 干净构建、来源、原图形和 ABI 检查；实际
同批四文件 `s15-link-focus-final` 32 项通过、零跳过。已发布到 assets 根目录
和 O:/winnt，摘要逐项与测试包一致；从 O:/winnt 直接启动 x86 ANSI 与 x64
Unicode 探针均观察到旧框、返回 .lnk、退出 0 和自有文件清理成功。
SHA-256：run16s `867c9ac910ca910b310af5a99157a7ee059dda0963337306ed2806dfbb2e0b08`；
ntw16s32 `b099e692f08c3ee6898b6a0b7a1fd2f1d92c896f0f07f430a1816facc3ea8da9`；
ntw16s64 `e85f2b2fb60540e34285d74fdd75942de9204d5efd0cd95ca3c4a48dc3b70849`；
demo `245a0a3c8a4ab2b7d3e92aff4c8cda3c2094f0310a7b467a3f8cbb93ba4c73eb`。

生产改动统计：`file_dialogs.c` +6/-6，净 0 行；新增测试源码及包注册见
提交 diff。保留系统结果、系统错误及原有状态所有者。未验证实体鼠标输入、
双击、目录链接和网络链接；默认解析缺陷保持未解决。
