# T26 S1 对话框当前接入复核

本记录补充先前冻结清单；它不表示交互式对话框、其他风格或全部调用方已验证。

| 接入位置 | 当前公共机制及产品侧责任 | 待核实与验证 |
| --- | --- | --- |
| `injection/file_dialogs.c` | `classic_file_contract`、`classic_file_hook` 和 `message_observer` 提供参数转换、旧模板初始化及观察；产品仍负责原始 API、会话策略、线程局部作用域和异常展开。 | 当前增量双位宽自建目标验证了带应用钩子的 ANSI/Wide 旧式打开与保存对话框确认和取消；其余回退组合及发布包未据此证明通过。作用域协调是否可再共享需连同字体、颜色路径核对，不可把产品的活动会话或回退策略硬编码进公共库。 |
| `injection/com_dialogs.c` | 通过 `classic_dialog_proxy`、`classic_dialog_events`、`dialog_v2` 等公共接口处理 COM 代理、订阅和分派；具体接口接入、选择旧式或原生路径和钩子注册仍在产品侧。 | 该文件仍包含路径、事件和对话框生命周期的混合协调，需逐函数检查能否减少适配；独立公共测试和链接测试不等同真实 COM 对话框交互。 |
| `injection/folder_dialogs.c` | `classic_browse` 提供 Shell 文件夹调用契约，`classic_dialog_controls` 接收调用方提供的控件安装/移除函数；API 注册与 Win16 控件服务由产品提供。 | 不代表 Win2000 已接入相同控件服务，也不代表各文件夹路径都已做视觉及交互验证。 |
| `injection/modern_dialogs.c` | `modern_file_dialog` 实现公共调用；产品提供作用域及线程局部重入计数。 | 目前参数测试不能证明实际现代对话框或其他风格的呈现。 |
| `injection/window_titles.c` | 打印契约与 `PrintDlgEx` 桥接调用 `classic_print_contract`、`print_extended`；字体和颜色对话框共享 `message_observer`。 | 实际 API 包装和样式作用域仍混合；`win16_dialog_color_initialized` / `win16_dialog_color_swatch_edge` 依赖颜色网格及 Win16 绘制决策，不能直接声称其他风格可用。需检查可否抽出不含风格决策的作用域协调，并对真实打印和取消恢复分别验证。 |
| `win16-dialog/chrome.c` | 识别系统颜色对话框的控件并配合 Win16 移除表面及边缘绘制。 | 其控件 ID 和条件与调用方对具体视觉处理绑定；当前保留在 Win16，不能因包含标准 API 就整文件上移。 |

公共目录已有的文件和测试可通过 `rg --files src/lib-style/common tests/lib-style`
及相应 CMake 目标核对；上述接入事实不构成公共库无反向依赖、双风格均调用或
完整对话框覆盖的证明。后续须结合产品接入测试和真实绘制结果逐项关闭缺口。

## 两个风格的入口重复

`ntw16s-dll/injection/window_titles.c` 与 `ntw2ks-dll/controls.c` 都包装
`CreateWindowExA/W`、八个普通对话框入口及多个公共对话框入口。两者现在共同
使用 `message_observer`，Win16 创建作用域也复用该观察器；但这只合并了返回消息
观察的安装和释放，并未合并 API 包装或创建生命周期。

不能把两组包装直接替换为同一宏：Win16 在调用原函数前启动嵌套创建作用域，
失败时可能销毁新窗口并返回作用域错误；Win2000 当前在调用后附着，并在窗口
创建时为选定子控件关闭系统主题。对话框路径还分别处理同步/非模态结果、
异常展开、原调用的 `GetLastError` 和 Win16 专用文件/打印转换。单纯共享钩子
注册表或把某一风格的包装复制到公共库会改变这些行为。

后续提取应先固定创建前后回调、作用域失败与回滚、非模态结果及原始错误值的
契约，再让两种产品分别提供策略并共用真正的包装实现；需要两种风格实际启动、
嵌套对话框、应用钩子、异常退出和失败回滚的双位宽测试。当前尚未完成这一
合并，Win2000 对话框的实际视觉及异常回归也不能由 Win16 测试替代。

## 观察器边界的补充验证

`style_message_observer_begin/end` 每次成功安装一个当前线程观察器，嵌套调用
各自拥有句柄；结束内层不结束外层。新增纯 C 测试在实际窗口消息上验证嵌套
观察、逐层移除和空句柄结束，MinGW x86/x64 均原生运行通过。观察器不负责
窗口附着、作用域失败回滚、异常展开或保存调用方错误值；这些仍由调用方负责。
此测试不代表上述两种产品入口的生命周期已统一，也不证明真实对话框行为。

第七批发布后，使用本地增量构建的公共组件测试，x86/x64 各通过现代文件
接口、经典代理、消息观察器、扩展打印、经典打印、文件辅助、对话框适配和
经典文件契约，共 16 次测试。它们验证对应公共接口，不是对真实系统对话框
交互、打印设备或两种风格产品入口的完整验证。

## 普通对话框包装共用

八个 `CreateDialog*` / `DialogBox*` ANSI/Wide 入口现在使用公共
`style/common/dialog_hook.h` 包装骨架。Win16 提供原有创建作用域、失败时
销毁非模态窗口与错误回传；Win2000 提供原有线程消息观察、调用前错误值
恢复和非模态窗口附着。公共骨架保证调用后的错误值不被清理覆盖，并在原生
异常展开时调用作用域清理；原始 API 的参数、返回值和具体策略仍属于产品。

MinGW x86/x64 增量构建通过；公共测试覆盖成功、开始失败、结束失败、
调用前后错误值及原生异常展开清理。使用自建目标和增量候选 DLL，Win16
实际执行八个 API 并观察附着；Win2000 的八个 API 同样通过模态、非模态、
具名、间接模板及空回调测试。两种位宽均通过。

`dialog-exception-recovery` 的真实窗口回调抛异常探针在新候选和第七批
发布版的 x86/x64 均以 `0xc000041d` 退出，因此真实窗口异常恢复未通过；
公共骨架的合成异常通过不能替代它。此次更改尚未干净构建或发布，其他
公共对话框入口和窗口创建作用域尚未统一。

## 单参数对话框包装共用

查找、替换、页面设置及各产品已有的其他单参数入口现使用同一调用和异常
清理骨架；Win16 仍提供非模态失败回滚，Win2000 仍在非模态返回后附着。
公共测试在 x86/x64 分别验证调用结果、错误值、开始/结束失败及异常展开。
自建真实目标在两种位宽、两种风格下通过 ANSI/Wide 查找和替换的附着、
通知及关闭；Win16 页面设置 ANSI/Wide 自动取消及外框检查在两种位宽通过。

Win2000 的打印/页面设置夹具在当前候选及第七批发布基线的两个位宽均
返回 4，没有获得页面设置结果；不计为通过，也不推测具体原因。其余单参数
入口及真实异常恢复仍需验证；本次改动尚未干净构建或发布。

普通与单参数入口原先的两个公共包装模板随后合并为一个：调用签名和参数
由调用方给出，作用域、异常清理与错误值交接只保留一份。双位宽重新通过
公共异常/回滚测试及两风格的真实普通、查找和替换入口回归。此前的 Win2000
打印夹具和真实窗口回调异常失败仍未解决，不因本次回归改变状态。

两风格的六个 `MessageBox*` 入口也使用该骨架，仍分别保留原有创建作用域
和观察器。自建目标在 x86/x64 上对两种风格分别验证了六个入口的附着、
自动关闭和返回值；该增量候选尚未发布。

## 现代对话框重入状态

原由产品 `modern_dialogs.c` 持有的线程局部深度现归公共 `modern_file_dialog`；COM 代理通过 `style_dialog_modern_active` 查询同一状态，调用方仅提供原有样式作用域，删除 enter/leave 两个转发回调及产品私有查询入口。深度的起止仍位于原 COM 初始化之后、释放之前，未改变对话框选择或错误返回策略。

双位宽增量构建通过。公共测试模拟 COM 创建失败和嵌套调用，检查线程隔离、内外层状态及初始化失败后的清理；自建启动器加载增量 DLL，实际现代打开、另存和文件夹对话框在两个位宽均自动显示并取消，观察到现代内容、外框附着、空路径和取消返回。该结果不覆盖用户选择文件、所有选项或其他风格；本次尚未发布。

## 新三层架构下的产品入口复核（P106）

本批完整阅读 injection 的10个生产源/头文件、ntw2ks 的controls/dllmain/window及所列公共接口。源码锚定见下表；只完成审计和迁移设计，未修改产品或运行原生测试。历史段落中的“尚未发布”须结合其后发布记录理解，不是本批产生新候选。

| 来源 | 现有调用与应迁移机制 | 保留在产品/风格的边界 | 目标与验证 |
| --- | --- | --- | --- |
| entry.c | context_read → stage_restore → win32_style_startup → prepare_hooks；事务已共享，不新增另一套 | 产品module校验、当前hook集合和兼容helper导出 | access/startup装配；保持无payload被动加载、开始失败/准备回滚/提交结果的既有契约 |
| create_process.c | process_hook_propagation_attach/as_user，active/accept/install回调；共享传播不重写 | run16s自提权重定向、打印宿主排除、当前session/style选择为产品策略 | access/startup及runtime；传播可单独启用，仍复用process-hook |
| late_process.c/h | init_lock、batch/sticky_error、context发布和既有传播复用属于运行时接入协调；88槽不变 | 产品cookie/module/flags映射与特定hook列表由装配提供 | access/runtime；普通/晚期两入口共用hook集合，无物理卸载承诺；复用late-batch与late-process测试 |
| intercept.h | 混合context、5组hook注册、scope与dialog chrome服务，需拆职责声明 | 原有导出ABI不扩展，删除已失效的Detours注释随迁移合批处理 | access内部注册与application/dialog scope分开，不让公共库包含该产品头 |
| file_dialogs.c | 有界A/W契约已有公共实现；old-dialog TLS栈、observer配对、resolve-links及异常退出仍由产品协调 | 当前准入/回退和原始API注册为策略；旧式布局选择属风格 | application/dialog file操作拥有scope，调用chrome服务；传参地址、结果回写、GetLastError及应用hook原样保留 |
| folder_dialogs.c | classic_browse/controls已共享；owner标记、控件服务组装可由统一dialog实例持有 | Win3.x grid/combo/scroll服务和选旧式Shell树的支持策略 | application/dialog browse；shell拥有PIDL，公共机制不删除返回给应用的结果 |
| modern_dialogs.c | 9行公开ABI转发已经调用style_dialog_show_modern；重入深度归公共库 | Ntw16sShowModernFileDialog导出和选择本风格scope | 保留薄导出，删除包装本身并不能提高解耦；公共现代深度/取消测试 |
| com_dialogs.c | proxy/events/dispatch/show已有公共实现，但native_creation_depth、CoCreate*/class-factory包装、MULTI_QI替换和factory引用仍混在产品 | CLSID/IID及原生回退条件为既有转换协议，风格提供启用/布局/chrome，产品提供API注册和标识 | application/dialog COM activation完整操作，复用proxy/events，不再让调用者组装几十个字段指针；工厂/聚合/接口身份和部分失败回退验证 |
| window_titles.c | 同时含创建scope、dialog包装、theme open/close、绘图和scroll API观察、临时popup owner；这些机制不能作为一个产品文件整体保留 | Win3.x字体、字符资格、各控件绘制选择；兼容导出与产品选项 | access/runtime注册，application/lifecycle创建，dialog调用，graphics/theme/control/menu观察分别归位；原始trampoline和LastError契约保留 |
| ntw2ks controls.c | 原生创建后附着、线程dialog observer、theme ownership以及26个dialog相关注册共同装配；已用STYLE_DIALOG_HOOK | 目前只对五类控件关闭原生主题，不是自绘Win2000控件；保持该范围 | application实例统一observer/ownership，Win2000提供控件支持和资源策略；与Win16不同的失败策略保留 |
| ntw2ks dllmain.c/window.c | 相同startup事务和process-hook传播已复用；窗口export路由已有frame/control真实调用 | module、global-session拒绝条件、helper及三个导出是产品契约 | 入口装配保留薄层，不把产品文件名/导出硬编码到公共库 |

COM 当前默认不启用EXPERIMENTAL_CLASSIC_EVENTS、EXPERIMENTAL_CLASSIC_HIDDEN和EXPERIMENTAL_CREATION_SUBCLASS，主CMake也未定义这些开关；源码保留分支不等于产品支持。style_classic_com_can_bridge明确拒绝FOS_ALLOWMULTISELECT，即便下游有multi-result代码也不能宣称多选已支持。原生GetOpen/SaveFileName的兼容flags同样不接管OFN_ALLOWMULTISELECT及应用自定义hook/template；T26保持回退，不借迁移追加功能或激活实验分支。

公共COM激活操作必须在原生调用成功且接口满足各入口既有条件后才代理：CoCreate路径对聚合outer、远程server、递归激活和modern主动调用走原生；factory_create保留自身现有检查，不能凭统一接口额外增加或取消条件；CoCreateInstanceEx先构建全部replacement，任一失败释放新引用并保留原MULTI_QI，完整成功才提交；factory/native与proxy各拥有自身引用。类工厂包装目前仍在com_dialogs.c，是可迁的协议/生命周期，不是必须保留的产品业务。现有标识字符串通过caller配置维持，公共组件不得硬编码Win16/T20名称。

window_titles注册集合与绘制来源不应以每API一个可配置回调膨胀公共接口。按创建、对话框、主题、文本/图形、滚动协议的完整观察操作分组，每组拥有对应native leaves和scope；风格提供实际支持与renderer。begin_titles的创建错误可覆盖返回并销毁新建modeless HWND，Win2000 observer路径不采用该策略，两者都必须由明确的finish契约保留，不能强制最复杂策略覆盖所有风格。

最内层释放本层资源，原始失败向产品入口传播；本次不额外设计通用容错或无限重试。现有异常展开层复用process-hook/unwind，不将普通VB_TEXT_HOOK没有异常cleanup的现状写成已具备；迁移时保持明确边界，并用既有真实回调异常未通过记录约束结论。

测试对应：公共dialog_hook、classic_file/bridge、classic_dialog_proxy、file_events、modern_file_depth、print_extended/contract/resources/ranges和chrome；产品dialog-entry/common/find/file/folder/com/task/print/sheet/modern目标。公共参数/代理测试只证明接口，实际目标的自动取消不证明选择、打印或全部参数，Win2000打印及真实回调异常失败继续开放。源文件重组后逐一迁移对应CMake目标，不能用原生回退像素当作已完成风格接入。

### 本批源身份

| 文件 | SHA-256 |
| --- | --- |
| `src/ntw16s-dll/injection/com_dialogs.c` | `ac1c3e5546d3b8f47272b58148c5c39fd2e209a76726f312cf5b1818c95bc2e4` |
| `src/ntw16s-dll/injection/create_process.c` | `2d6c9fbba7d4dc9b8a1e9bcbaeff3e377948bc1cd0528c0bf010201a8b1a8a23` |
| `src/ntw16s-dll/injection/entry.c` | `7782c72233bd1045a64e4af5831613c186980270275b8d61dea498e6099737f4` |
| `src/ntw16s-dll/injection/file_dialogs.c` | `1d8e9e4b3aedb0bdd7e4509bfe02d78143c8bda74d55800d9cb96628a1f34138` |
| `src/ntw16s-dll/injection/folder_dialogs.c` | `9db8eb732997c7983d65c1014985e1c78dcc560a37c1b3529403dcb7d0d8aea6` |
| `src/ntw16s-dll/injection/intercept.h` | `a9cd91b4ae61179932e21354e65a870ae65da389bf3753a16e0a0a748af736ab` |
| `src/ntw16s-dll/injection/late_process.c` | `4f7c5e97d572657699010546b67dff264d9dff0e4e5157b1198414042a174159` |
| `src/ntw16s-dll/injection/late_process.h` | `923fc11663961293cbf41e86f66338e61deaade0d3ff8ba033351927fb2d6a1d` |
| `src/ntw16s-dll/injection/modern_dialogs.c` | `8e8931a417dac91191c554c5e25879eaf87558166a1eefc3b70f6b7dc6fed8c5` |
| `src/ntw16s-dll/injection/window_titles.c` | `e83a10dc23a2560c06d3db72753c943946ddd3cacf63543a08b89eb3b2a69d86` |
| `src/ntw2ks-dll/controls.c` | `6b0ca3680fcd85c4cdf1c64156d93739ecc129f2e81264ad6d24c8af2c9f26b7` |
| `src/ntw2ks-dll/dllmain.c` | `3a20c40d13f003be7c3fc8a7a3c5bb9428180d625697b0c4ed0983a11e363543` |
| `src/ntw2ks-dll/window.c` | `c6cb610ac330a245b65cb1655eccd18523ecefa5ef0958357924722baa72f27b` |

单人复核与效率检查：对照源码注册表与CMake实际输入，分清已有公共机制、产品可迁事务和未启用实验分支；修正“有multi-result函数就已支持多选”的推论风险。没有新增二进制改动、测试或scratch，保留第41项在途改动。下一批继续global、UWP和启动器的接入组合核对；未完成该范围前不关闭S1。
