# M0 T21 S43：公共组件极简质量修正

S43 已收口：修复、验证、两处发布及提交远端复核完成；M0 固定，T21 保持开放。

基线 `359f96e6`。本 S 仅处理终审确认的四个代码问题及公共接口前提，不新增公共接口、产品导出、状态机、重试机制或第五个产品文件。兄弟仓库没有修改，亦不是构建测试依赖。

## 实现

- process-global/targets：回收入口排除 QUEUED/WORKING，公共组件维护自身队列与执行生命周期；保留产品原有到期判断。
- process-global/transaction：每轮 pending 推进前检查 deadline，超时直接向调用者返回 1460，不依赖等待句柄恰好无信号。
- process-hook/staging：相同载荷的重复 DLL 请求额外比较当前 staging 的 DLL 名称；不同名称执行原有替换流程，纯载荷幂等保持。
- process-hook/helper：奇数字节载荷入口返回 87，统一接收端原有 WCHAR 对齐要求；不添加新传输协议。
- 头文件说明 worker ready 事件所有权、远程载荷复制与释放先后；file_handle 要求零偏移同步文件；propagate 回调通过返回值报告失败，不让异常逃逸。展开只恢复重入状态，不声称完整子进程异常回滚或物理卸载。

## 验证

增量 GCC 双宽各 3 项聚焦用例通过。首次沙箱后台没有结果；随后误判沙箱外进程可见性而重复启动，产生日志/静态库竞争，独立监控调用失败。原任务最终完成；混杂日志不作为唯一交付依据，后续同快照干净树重新验证。

用当前测试链接 HEAD 的旧实现进行 x64 负例：targets/contracts 退出 1（队列节点错误回收）；transaction 退出 13（已到期但持续有信号）；payload/dll-contract 退出 3（DLL 名未改变）；payload/helper-contract 退出 1，旧实现返回 5 而不是入口 87。四项均检出；所有子进程自有、有限等待并回收。旧版目标队列负例在检测回收后直接退出，不执行悬空队列节点。

临时证据登记于 CURRENT 的 s43-review；干净包、实际包测试、部署及远端复核结果在交付完成后补齐。原资源和来源声明保留，来源回执仅更新被修改公共源码与头的摘要及说明。

MSVC /std:c17 本次 staging/helper 及更新的 payload 原生测试在 x86/x64 均编译链接运行通过。首次命令输出目录尾部反斜杠引号错误造成 D8003，修正命令后通过；没有修改产品工具链。

`s43-clean1` 双位宽干净构建、ABI、原资源、来源与公共源码/静态库依赖检查通过；生产 C 实现净增 17 行，无新状态或接口。干净树两位宽各 5 项聚焦回归通过（targets、transaction、payload、manager、stop-race）。

`s43-package1` 并行实际包在 global-attach-x86 退出14：advance/remove=1460，attached=3、clicks=1，自有子进程退出0且票据已关闭。该路径不经过本次更改的 transaction_run，未认定新回归或根因；保留失败，随后同候选串行 `s43-package2` 复测，不改产品超时或增加重试机制。

干净构建用时：x86 384.8秒；x64 408.7秒。Ninja 8 并发，ccache 共用原缓存；x86 479命中、116未命中。

`s43-package2` 同候选串行44项实际包测试通过，无顶层 skip；global-policy 双宽其他 window station 子项仍为 skip（start=1/exit=12），不计通过。管理器停止后绘制摘要仍为2875890938，与旧版一致而不等于原始343534586；不扩大本 S 范围深挖。管理员 ETW、真实托盘端到端和全矩阵未在本 S 重跑。

`s43-publish1` 已将同批四文件发布到 assets 根目录与 O:/winnt，八个部署文件摘要均与测试包一致；本次未关闭用户进程。ABI、图形资源与来源声明保持；没有修改兄弟仓库。

四文件 SHA-256：

- `demo.exe`：`bca307a81a13a593defcd79434c8e9c31b7b3ee5351ccd1538e58c097a251e26`
- `ntw16s32.dll`：`ca1b8e340095bd2dbcd4cc5c7cdb2e537c39a8d6e1c3ee7b12e6a7622ae084f2`
- `ntw16s64.dll`：`55c409404f98a2f945d5042b64a023be273de24521dfb33d5c90ff6ccbd2c661`
- `run16s.exe`：`d297b90ee526a249e31d1986500f5759c94eeb32a980db710b5d6599f543f245`

P1 `7fd72964` 已提交推送；实际读取 GitHub main、完整1303个文件的 blob 树及四个 assets 二进制内容，与本地提交及 O:/winnt 摘要一致。交付18个文件，文本新增195行、删除30行；其中生产 C 净增17行，主要增量为必要回归和契约证据。无新状态机、新接口或容错框架。治理13项负例和 diff 空白检查通过。

最终范围：四项终审修正均完成；MSVC仅验证本次变动的 staging/helper 及 payload 测试，不扩大为 process-global 全库 MSVC 已验证。首次并行1460失败及环境跳过如上记录，不将串行通过称为并行根因已解决。T21开放，不准入下一S。

Scratch 为 build/t21-package/s43-review、s43-incremental、s43-clean1、s43-package1、s43-package2、s43-publish1；保留构建/测试/部署证据和已测试包，临时旧实现源码与独立测试产物在提炼后清理。
