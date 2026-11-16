# M0 T26 S5 运行时接入

## 冻结清单与实施顺序

基线ba2147fc，S4代码4517db1e；进入时git干净，未发现编译器/Ninja进程，S4所有会话已终结且临时树已删除。有限生产清单32项的原始摘要暂存build/t26-s5/baseline.json；此scratch用于本批开发/交付，证据提炼后清理，不重启旧构建。

| 已核对来源/入口 | 本批处置 | 原因/实际调用与验证 |
| --- | --- | --- |
| access/runtime的companion.{c,h} | 已公共，保留 | 包内同伴身份、导出与pin；全局window和UWP dispatch实际使用，peer测试 |
| window_dispatch.{c,h}、window_hook.h | 扩展现有完整操作，不复制 | 统一已加载hook事务、请求/结果协议、通知的重入与退场；两个产品hook及UWP适配复用 |
| uwp_dispatch.{c,h} | 已公共，复核组合 | 宿主资格/位宽/模块验证后调用同一window dispatcher；真实宿主尚未覆盖不冒称通过 |
| run16s/window_target.{c,h} | 模块身份/字节校验迁runtime/module，临时hook及请求调用公共操作 | 产品保留DLL/导出/消息名及路径选择；console_host、explorer依赖的旧接口保持 |
| injection/late_process.{c,h} | 锁、88项批次、context发布/失败回滚归runtime/process | 产品只提供风格模块、注册API集合/传播discard；继续唯一process-hook后端，不新发明卸载 |
| attachment.c/window_thread.h的单窗口hook入口 | 重入、匹配/结果协议用公共handler | 产品保留动作值、窗口资格、模块pin及实际attach/remove，控件呈现留S9/S10 |
| uwp/hook.{c,h}及application/adapters/uwp/hook.{c,h} | 适配目标验证与风格动作分开，共用runtime通知操作 | CoreWindow/frame关系仍属UWP适配；菜单/表面/呈现S12，不移动到access |
| global/hook.c、late.c、thread.c、window.c | S5接入新的runtime操作；全局协议/会话协调S6 | 当前session cookie与owner/phase/desktop通知策略归S6冻结范围；不在S5复制协议 |
| injection/create_process.c/window_titles.c | 已有启动注册装配/风格API集合保留，runtime回调调用它们 | 传播活跃策略与具体渲染不移入公共process引擎；S4公共传播继续唯一实现 |
| ntw2ks controls/window/dllmain | 现有窗口操作继续被公共组合调用；不新增晚期函数挂钩能力 | 其已有外框/控件API归application/appearance后续S7–S10；不虚构Win2000独立晚期初始化 |
| ntwshk dllmain/def | 共享载体/ABI保留 | 不在DllMain加入等待、GUI或新增接入；八文件与所有ordinal保持 |
| run16s explorer/console_host/uwp | 调用方接入公共module/dispatch，程序选择仍产品 | UI/CLI、可复用实例定位等已有业务不升为通用接入策略；同类目标验证不扩大范围 |

先迁完整runtime module/request/hook操作及对应测试；再抽取晚期process协调状态和产品策略适配；同步全部CMake直接编译消费者/来源回执；两风格既有行为、无样式传播、重复/失败/回滚与线程/窗口身份双位宽增量验证后冻结同快照一次干净交付。先发布再测试，不按文件构建/提交/发布。

公共process协调拥有锁、batch/patch数组和安装状态，借用已公共的startup context与配置；产品提供一次完整注册回调，不把整个Win16私有结构传入库。挂钩注册API没有上下文参数，公共层仅在本线程当前初始化期间绑定目标batch，退出恢复；成功及sticky失败仍保留原process-lifetime存储。不将停用/配置discard称物理卸载。

## 验证与退出

独立module身份/枚举/映射/短读与错误传播、request结果/超时/清理，线程hook动作拒绝/重入/嵌套/窗口销毁后退场；late初始化覆盖无样式、既有owner复用、严格模式不匹配、失败回滚/重试、88项边界与进程/线程资源。复用既有实际后端测试，不以stub替代生产挂钩证明。双位宽现有普通启动/四父子位宽/窗口与控件及运行时组合、产品ABI/来源/依赖门禁，已知DWM与真实宿主限制继续如实记录。

效率检查：初审同时发现EXE尚有完整模块身份和临时hook协调，已纳入本批，避免只搬late文件而留下同类副本。process-global的mapped_identity具有不同PE/远端导出解析契约，不以新窗口hook的代码偏移64字节契约替换它，也不在本S扩大为底层模块算法重写。下一批按上述三项完整操作合并实施。

## 实施与开发复核

EXE window_target已从完整模块枚举/映像比较/请求/临时hook实现缩减为产品路径与协议选择，真实算法归runtime/module和window_dispatch。late_process的锁、88项批次与失败回滚由runtime/process拥有，产品保留约20行API集合配置/兼容入口。UWP接入消息处理及对应测试从application/adapters/uwp/hook迁至access/runtime/uwp_hook，目标关系仍调用application适配器；运行时负code/busy的提前返回语义保持。

完整临时hook操作拒绝零PID/TID后才安装，避免无效目标退化成桌面hook；安装失败且OS未给错误时沿用产品的ERROR_PROC_NOT_FOUND，防止旧通用分派误返成功。原发送错误优先于unhook清理错误；注册失败、失配与请求均最终解除自有hook。UWP现在也在安装后复核窗口PID/TID，不扩大目标范围。独立故障测试覆盖这些分支，无新增异常吞错/后台重试。

16项双位宽机制/后端测试通过，其中独立子进程在消息前后均用实际映射文件和64字节入口验证公共module，验证x86/x64真实加载/链接/运行，而非只编译。实际Ntw16sWindowThreadHook双位宽安装hook/直接调用4项通过，保留LastError、拒绝错误窗口并实际应用/移除。晚期后端复用/失败重试与四位宽边/A-W/仅传播/停用后新目标/恢复共24项通过；无样式模式style_registration=0、窗口attached=0。Explorer/控制台产品3项通过，含已映射文件改名/替换拒绝。详见[原输出](../etc/evidence/t26-s5-runtime.json)。

Explorer旧测试自建私有桌面，未执行；夹具改用当前桌面的自有窗口，按本测试映像/PID过滤，子进程挂入kill-on-close作业对象，失败退出亦有界清理。已删除该测试的CreateDesktop/SetThreadDesktop路径，不操作用户Explorer或切换桌面。早期自有late包清理3,637,500字节，Explorer测试临时目录内容已由用例清空并回收。

效率检查：本批把模块身份、通知事务和晚期协调一起实施，独立API与生产调用同时接入；通过all增量发现两个直接编译消费者的缺失链接并统一补齐。首轮编译发现WinSock accept名称冲突，已重命名内部谓词；新增故障夹具的成功mode原被旧timeout条件捕获，已只修夹具，不改产品去适配测试。所有相关修正合并后才进行一次完整交付构建。S6全局会话、S7–S12实际样式/框架边界仍按原规划，不把该批局部通过当作全面支持。

## 收口前职责复核与最终交付

再次核对attachment的CBT/返回观察、Ntw16sCreationBegin/End及window_titles的begin/end_titles：这组是API创建期间的application创建链，负责FALSE内层屏蔽、主题stage/commit、首show提前settle、延迟WM_INITDIALOG不重装已移除窗口、EndDialog失败终止及pending资源；不是对既有进程/选定窗口的运行时接入。按原规划S11的title creation与对话框装配一起抽离到application/lifecycle/dialog，原[产品入口审计](M0-T26-S1-product-adapter-review.md)已指定该目标，[T26方案](../proposals/style-library-reorganization.md)的产品对话框段也明确其待迁移。当前已调用公共creation_pending/message_observer，但整链尚未共享；不把这组代码说成纯产品业务或声称T26五大干净已完成。

S5最终唯一公共入口为runtime/module的module_compatible/module_open；window_dispatch的request/dispatch_loaded/dispatch；window_hook的handle；runtime/process的process_init；uwp_hook的hook_handle。产品window_target只提供文件/导出/协议名与原兼容调用，late_process只提供API注册集合/模式，attachment与uwp/hook只提供选定动作、目标资格、PIN及风格操作。全局session/phase协议留S6，创建链留S11，实际控件/窗口/现代表面留S7–S12，均有明确接收者。

单快照双位宽干净构建通过：x86 151.48秒、805 cache hits/0 misses；x64 179.95秒、905 hits/0 misses。双宽各17个样式公共归档和process-global依赖门禁通过，1005个源码/构建/测试/工具输入与当前文件摘要一致。原图形资源与ABI/ordinal/导入集合保留。先发布同批八文件至assets根及O:/winnt，再运行118项去重实际包回归全部通过；另2项DWM恢复旧失败与S4一致、不计通过。证据见[最终交付](../etc/evidence/t26-s5-delivery.json)。

源码按no-renames统计15个路径、增加278/删除184/净+94行，其中ntw16s产品侧净-44、run16s产品侧净-61；测试10个路径、增加293/删除144/净+149行。S5此P为完整运行时能力合批，不是每文件交付；已完成本地验证，仍须实际提交/推送/远端复核后关闭。

所有自有测试/构建会话已返回，提炼证据后清理开发scratch、完整快照及发布备份277,522,232字节，另早期late包3,637,500字节。已验证绝对归属及reparse，保留.ninja/.ccache。下一批效率调整：测试改动先构建其聚焦target，再统一all增量，减少中间接口改动触发的重复链接；仍只按完整能力做干净交付。
