# M0 T26 S3 公共生命周期

## 冻结实施契约

以S2 P1的381项迁移结果为起点，先核对Win3.x窗口、控件、产品observer及现代host的GUI线程引用和退场路径，并检查Win2000外框的属性/子类安装事务。非GUI的COM引用、字体租约、主题缓存、菜单tracking及异步接入会话各有独立资源契约；不把它们误合并为同一个引用计数。

本S把既有GUI实例的身份（HWND/子类过程/creation owner）、回调保活、脱离标记和最后引用释放集中到application/lifecycle/instance。每个渲染私有对象以该实例为首成员；应用层拥有共同状态和原生Set/Get/RemoveWindowSubclass操作，具体资源清理由完整dispose操作提供，实例调用它而不读取风格字段。调用者在重入操作期间持有引用；正常移除与terminal保持现有不同的恢复动作，移除失败不标为detached。creation owner和取消指针从公共状态提供，不另造注册表或每字段回调。

Win3.x frame/control/产品observer和现代host改用同一公共实例；Win2000以现有外框实例接入，保留active/menu/DWM等待S7/S8拆分的呈现与窗口协议。公开产品ABI、原图形资源、目标范围和六种切换行为不变，不新增卸载能力。窗体和控件私有状态其余部分由S7–S10继续拆，不把本S称为整个T完成。

实施顺序：公共实例及独立测试；Win3.x frame/control/observer和现代host合批接入；Win2000安装/销毁/移除接入；搜索所有旧计数/脱离写入并逐项核对；双位宽全目标增量；合批干净构建、先发布后实际包验证、提交推送及远端复核。禁止每个文件单独完整交付。

预期验证：公共实例独立的嵌套调用、移除失败、重新附着和最后引用清理；已有frame、control link、combo、reentrant removal、modern host/start/hidden frame和Win2000 frame lifecycle；产品observer实际launcher夹具。既有失败保留证据，不借迁移改断言或系统设置。禁止computer-use与隔离桌面。

## 本轮效率检查

上一轮为有效进展：S2完成381项归位及36项实际包回归，远端51c68a06已逐blob核对，工作区干净。恢复时没有本轮已启动构建可重启；旧构建会话均已终结，系统进程枚举因凭据错误不可用，不冒称读到全系统无构建。将有限搜索和审计暂存build/t26-s3，供同一批实施和复核，提炼交付后清理；保留.ninja/.ccache。下一步以同一个公共实例实际替换三种手写计数，不先做新的模板或兼容包装层。

## 开发验证与效率检查

公共实例新原生测试x86通过：嵌套回调中移除并安装后继、最后引用清理、外部更换/移走子类后的身份拒绝及重试。新增独立链接首次暴露Comctl32 v5的名字导入不可用（0xc0000139），改为复用既有ordinal导入库；没有新建Windows兼容分支。x86产品及全目标增量通过；现代host/start、hidden frame、普通frames和control-link通过。首次增量还发现scroll_message和control-link的旧私有字段消费者，已同步新状态，未改断言。

Win2000 frame_lifecycle的DWM恢复断言仍失败（restored=0），S2已发布DLL在同一测试exe、同一环境逐项得到相同输出（geometry/repeated/reattached/pending均1）；这是既有未通过项，不算S3通过。临时baseline只复制自有测试exe及两份已发布DLL，结果提炼后清理。另增加真实生产frame代码的分配/释放见证测试，验证新实例引用和安装/移除/terminal资源，而不改DWM恢复标准。

计数搜索中的COM对象、字体缓存、菜单scope/popup以及主题记录具有独立协议，不将这些结构整体改为GUI根实例。popup/scope与多窗口菜单事务由S8接入同一生命周期；主题/控件协议由S9–S11各自所有者负责。S3必须完成普通frame/control/产品observer、现代host及Win2000 full/pending根的实际实例接入，不用未拆的借用绘制view充当公共状态。

## 公共入口与实际消费

`application/lifecycle/instance.{c,h}`提供init、retain、release、lookup、attach、detach六个操作，持有GUI根的HWND、子类过程、创建owner和退场标记；不包含产品名、风格头、字体/像素或全局进程策略。exact identity核验防止旧节点解除后继；attach重复与detach失败直接返回，未增加重试、容错队列或新卸载流程。

Win3.x frame_state/control_state以实例为首成员，并由existing dispose释放字体、菜单或数组；frame_lifecycle/control_install/control_remove真正调用公共native安装/解除，dispatch及现有异常unwind调用公共release。产品attachment的owner引用与最终释放已接入公共计数；其资源选择/清理仍是原有VB/VCL策略，后续S10下沉，不复制引用机制。modern host删除独立calls/detached/context/dispose字段与旧retain/release接口，使用同一实例；保留多窗口surface/title的适配动作供S12继续拆分。Win2000 full/pending根也由公共实例管理，原active/menu/DWM属性及绘制留S7/S8。

53处初始引用/取消检查全集及处置见[生命周期审计](../etc/evidence/t26-s3-lifecycle.json)。只读calls用于保留原重绘节奏；scroll gesture的removing属于尚未发送结束通知时的控件协议，留control_remove而不是让所有实例带无用字段。菜单popup/scope未冒称已经迁移；明确由S8接收。

新Win2000夹具在x64发现GWL_STYLE的Get返回零扩展而Set返回符号扩展（84000000/ffffffff84000000）；按该字段的DWORD宽度比较，保留两值日志，不使用指针宽度比较32位style。两位宽的新实例及现代host/hidden用例现已通过；完整发布验证尚未进行。

## 合批交付前检查

双位宽global_style_bridge原生测试通过：Original/Win3.x/Win2000转换、已有/新建子控件、标题/客户区/底部刷新均保持；普通frames两侧通过。x64实际build launcher加载新的ntw16s后，VCL bitbtn的嵌套绘制、SEH unwind、retire和cleanup通过；不是仅测试counter。后续正式发布八文件后重跑双位宽实际包路径。

本批约100处生产调用使用公共实例；没有扩大权限、扫描范围或修改原始资源。增量发现均是字段/fixture/链接消费者同步问题，已使用相同批次修正；Win2000恢复旧失败保持原样，不无限修补。现在冻结源码，整批只做一次最终干净交付构建；暂存build/t26-package/s3-lifecycle及build/t26-package/s3-lifecycle-publish分别供测试和发布回滚，提炼证据后清理。

## 兼容性复核修正

首个干净快照构建过程中发现pending重复接入的旧语义遗漏：旧SetWindowSubclass同id重复设置返回TRUE，新公共attach拒绝替换实例。直接在自有隐藏窗口调用S2已发布ntw2ks64.dll确认两次Attach及Remove均TRUE。产品pending路径改为查询并复用既有公共实例，不再新建或覆盖；测试同时断言重复调用成功且无额外分配。完整frame的原重复拒绝语义保持，公共身份核验保持。删除接入后不再使用的STYLE_ID宏。

为避免发布错误快照，核对自有x64构建cmd的PID、映像和启动时间后终止该进程树；首个非提权进程查询看不到已知PID，没有因此重启。提权只用于读取/终止已确认的自有构建，未弹用户UAC或操作系统设置；父会话随后退出并记录failed-or-inconclusive。该次构建不计通过，x86已完成缓存继续复用，残留快照提炼后清理；最终采用新目录s3-lifecycle-final，合并修正后交付。

最终冻结前已按公共契约将frame/control terminal的引用取得移至解除之前，失败按原语义释放并上抛。只变更共同状态/所有权，不增加恢复重试。首次已废弃快照清理104,985,036字节；最终快照路径为build/t26-package/s3-lifecycle-final。

## 正式交付验证

最终同快照八文件已先发布至assets根和O:/winnt，摘要一致。发布后48项双位宽聚焦测试全部通过：公共instance、Win2000分配/释放见证、modern host/start/hidden、普通frame、两风格桥接、对话框/控件链接及正式launcher的VCL edit/bitbtn和Win2000 frame-child路径；2项Win2000 DWM恢复断言仍失败，与S2同条件各自基线字段完全一致，明确不计通过。详细原生命令/输出见[交付证据](../etc/evidence/t26-s3-delivery.json)。未运行物理输入、真实UWP/WinUI全矩阵或管理员全局扫描，不宣称这些通过。

干净构建指标：{'x86': {'seconds': 159.092687700002, 'hits': 756, 'misses': 42}, 'x64': {'seconds': 194.58635110000614, 'hits': 858, 'misses': 42}}。两侧15个公共样式归档和4个通用进程归档无反向依赖；原18资源表/渲染DLL和产品导出检查通过。967项编译/工具输入与快照逐项一致，尚待提交推送及远端复核。

48项聚焦测试及2项对照均已退出，证据提炼后核对绝对路径/无reparse point，清理审计scratch、最终构建/源码副本和发布备份271,040,367字节；首次废弃快照另清理104,985,036字节。保留正式assets/O:/winnt与.ninja/.ccache。

## 最终代码复核

共37个生产文件接入，新增公共实现33行、头30行，六个完整操作；没有新包装DLL或异常重试引擎。生产diff {'added': 525, 'deleted': 446, 'net': 79}；tests {'added': 182, 'deleted': 34, 'net': 148}（含两个原生生命周期测试及既有测试源接口同步）。其余主要为来源回执、逐项审计和交付证据。已实际读取核心diff、state声明、公共实现及CMake接线，核对正常/terminal、失败留存与最后引用dispose；两风格根实例实际接入，剩余菜单/呈现/运行时协议有既定S接收，不把它们隐藏为本S已完成。

## 远端复核与收口

P1提交8e21860a0406b8a4db333436f277f94189083d62已推送并实际fetch，逐blob读取59个变更文件与本地一致，八文件在assets/O:/winnt与测试摘要一致。S3 GUI根生命周期退出条件满足，收口；2项DWM旧失败及菜单/呈现/会话后续归属保持明确，不称全T完成。按既定规划自动准入S4启动接入，T26开放。P2只记录复核、修订过时债务表述和准入，不改产品输入或二进制，不重复构建发布。
