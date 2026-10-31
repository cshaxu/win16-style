# M0 T20 S3：Shell 文件夹筛选安全回退

S2 审计记录了已发布文件夹桥接对带应用回调的 `BIF_USENEWUI` 请求清除
`BIF_NEWDIALOGSTYLE`，从而漏发系统 `BFFM_IUNKNOWN`：应用不能通过
`IFolderFilterSite` 安装筛选。P295 实际发布包的双宽 A/W 四组负例均显示
`unknown=0 installed=0 blocked=1 valid=0`；自有目录清理成功。

P1 对精确命中的两种标志组合，若存在应用回调则保留原 Shell 调用，让真实系统
提供接口及筛选；无回调的既有旧树路径不变。这是**安全回退到新式布局**，不是
声称已在旧树内实现筛选。搜索 `BROWSE_WRAPPER|BRIDGE_FLAGS|BFFM_IUNKNOWN`：
A/W 共用一个生产包装，未覆盖的标志继续原路径。状态与 PIDL 由 Shell/调用者
管理，不合成接口或项目。代码/测试变动（不含文档、清单和二进制）：
`folder_dialogs.c` +1/-0、`tree_folder_target.c` +1/-1、
`Test-Package.ps1` +11/-3，总计 +13/-4、净 +9 行。

增量候选双宽 A/W 四个真实 IFolderFilter 探针均 `unknown=1 installed=1
blocked=0 refs=1 cleanup=1 valid=1`；双宽各验证普通带回调选择/取消及无效
输入校验/恢复。x64 Unicode 额外检查筛选错误、隐藏项枚举、重入和清除，
五项通过。测试为自有私有桌面、消息驱动，不代表物理输入及所有命名空间。

`s3-shell-filter-final` 纯C、GCC、Ninja、ccache 双宽干净构建，来源、原图形、
治理及 ABI 门禁通过；实际同批四文件 `s3-shell-filter-focus` 36 项通过、
无跳过，包括双宽 A/W 筛选、带回调、无回调旧树、输入、demo、注入传播。
四文件已发布到 assets 与 O:/winnt，逐项相等且与测试包一致：
run16s `56ab9cfff97ded8f1830b55c18f9becf293296d9bee9e032cd7414e1700c731c`；
ntw16s32 `1ba050727d341faa861e021af2a89770bfc80bab49ab2cd8d3b866c530a48a18`；
ntw16s64 `440cc6adb1883a3aa4cb238f23919f326e39f589fac0c30890dde78be8dc1ed5`；
demo `d9d3e915e5125a27e775542de58e76a349591644ec4d027c39e61db50dc5d1d5`。

带回调场景尚未做到用户期望的旧树布局；其他标志及虚拟命名空间并非全部验证。
COM 多选长文件名及并发串目录已知缺陷仍需按 S3 范围评估安全止损；不声称解决。
