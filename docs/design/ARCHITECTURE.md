# System Architecture

## 产品目标与当前契约

构建运行时 Windows 外观框架：统一接入和应用机制，装配不同年代的风格，适配不同应用框架。当前实现程度、失败与验证范围以 [CURRENT](../states/CURRENT.md) 为准；下述目标结构按 T26 逐步迁移，不表示已经实现。

机器可读产物权威为 [product-contract.json](../etc/product-contract.json)：五个产品组件、八个文件，保持现有产物和 ABI；源码组件不要求独立 DLL。

| 产品组件 | 产物 | 目标职责 |
| --- | --- | --- |
| run16s-exe | run16s.exe，x64 | CLI、托盘业务、风格选择、调用接入、最终错误报告 |
| ntwshk-dll | ntwshk32.dll、ntwshk64.dll | 共享运行时装配、进程内接入与公共状态导出 |
| ntw16s-dll | ntw16s32.dll、ntw16s64.dll | Win3.x 风格装配、兼容导出及必要适配 |
| ntw2ks-dll | ntw2ks32.dll、ntw2ks64.dll | Win2000 风格装配及导出 |
| demo-exe | demo.exe，x64 | 原生示例与手动操作入口；直接启动保持系统原生行为 |

S2已完成库源码/测试的新根归位；S3–S13已按有限清单交付生命周期、三种接入、窗口、菜单、控件、Win32运行时、创建/对话框、现代宿主与图形资源/renderer。S14负责全T归属和实际消费者终审，不能把文件归位、局部测试或已有耦合直接当作完成证明。具体状态和未通过项由CURRENT统一管理。

## 三层架构

所有样式能力统一进入 `src/lib-win32-style`。无样式含义的 Windows 能力继续位于 `src/lib-win32`。

| 层 | 组件 | 责任 |
| --- | --- | --- |
| 接入 access | startup、runtime、global | 启动接入、运行中接入及全局发现协调，复用底层进程库 |
| 样式应用 application | lifecycle、window、menu、control、dialog、graphics、adapters | 附着、移除、消息、输入、重绘、恢复、资源与框架适配 |
| 风格呈现 appearance | win3x、win2000；未来 luna、aero | 度量、布局、绘制、配色、字体选择、资源及年代行为策略 |

产品装配并选择风格，将其能力交给 application；access 调用统一附着入口。application 定义契约，appearance 实现契约；application 和 access 不包含具体风格私有头。风格之间不互相依赖。框架适配器属于 application，接受风格提供的几何和呈现，不复制每种风格的宿主生命周期。

```text
产品装配：选择风格、框架适配与接入方式
    access/{startup,runtime,global} → application
    appearance/{win3x,win2000,...}  → application 定义的契约
    access 与 application          → lib-win32 基础能力
```

## 组件边界与接口

- lifecycle：附着身份、重入引用、安装/移除事务及切换协调；公共状态和资源由公共实现拥有。
- window/menu：非客户区与菜单生命周期、原生消息路由和通用输入；布局、绘制和行为差异由风格提供。
- control：原生协议、子控件关系、通知及公共输入状态；VB/VCL 识别归运行时适配，绘制支持条件归风格。
- dialog：参数协议、代理、事件转发、结果回写与资源事务；转换选择、布局和回退策略由风格或产品提供。
- graphics：DC、裁剪、缓冲、像素缩放及字体对象所有权；字体选择和年代图形资源归风格。
- adapters：Win32、UWP、WinUI 的宿主、表面、客户区边界和重绘协调；仅实现已有能力，不预建未来框架空实现。

公共附着状态（窗口、线程、阶段、重入引用、原生快照及资源）、风格私有状态（颜色、字体选择、布局）、框架宿主状态分别拥有，按上下文关联；公共层不得接收整个 Win16 私有结构再借大量回调访问字段。接口按完整操作划分，仅真实策略差异才参数化，不逐行增加包装。现有私有类型耦合本身是待拆对象，不能单凭它认定机制必须留在风格侧。

错误在最内层处理本层资源后向上传递，最终产品入口统一报告；中间层不得重复补救和吞错。同步调用可能重入、销毁或替换附着，调用方生命周期必须覆盖回调；同线程窗口资源不跨线程直接操作。

S3已在 `application/lifecycle/instance` 实施GUI根的身份、子类安装/解除、重入引用和最后引用退场；Win3.x frame/control/observer、现代host与Win2000 full/pending路径接入，具体交付状态见CURRENT。实例只拥有共同状态，资源dispose由实际风格/适配对象提供；菜单多窗口事务、消息协议和呈现仍按后续S拆分，不能据此称三层迁移全部完成。

S4已将启动接入迁入公共组件：`access/startup`拥有载荷解析、包内模块定位、安装/辅助进程校验及初始化/传播装配，消费者提供模块身份、依赖前缀、会话资格和API准备操作。`lib-win32/process-hook`仍是唯一挂钩后端；静态公共装配不自行引入第二份进程内后端。产品共享DLL的旧导出通过别名和必要ABI适配保留，CLI与自提权策略仍由产品提供。通用创建后激活/等待归`lib-win32/process-launch/lifecycle`；交付状态见CURRENT。

S5将同位宽运行时模块校验、临时线程hook事务、请求/结果与接收重入协调归`access/runtime`；晚期初始化拥有批次、锁、回滚和保留状态，使用公开startup context和调用方注册策略，继续调用唯一process-hook后端。UWP的接入消息处理也归runtime，框架窗口关系判断调用application适配器。产品仅保留模块/协议/动作与实际风格操作；S11已将CBT、首次show、延迟WM_INITDIALOG结算与pending/hook资源归application/lifecycle/creation，产品提供完整风格操作及兼容ABI；API调用scope的一次初始化、嵌套创建和错误结算现由application/lifecycle/creation_call拥有；产品保留原生ABI参数适配及年代策略，不能以局部完成声称全T迁移完成。

S7在`application/window`实际实施native_frame、caption_button、messages、install：共同原生状态、GUI实例发布/失败回滚、消息执行及捕获生命周期由公共实现拥有，两风格直接调用。`application/graphics/font_pair`拥有字体对与DPI、失败保留/提交/释放；年代字体及租约释放由renderer提供。普通和现代外框共用字体对象，现代宿主流程仍由S12终整；菜单S8、renderer过渡view和最终注册S13，不把这些交界提前称为完成。当前交付/验证状态由CURRENT管理。

S8菜单能力已公共化，`application/menu/popup`和`tracking`的线程观察/跟踪状态由既有共享运行时统一承载，GUI函数延迟解析；`owner`拥有嵌套loop、临时item与消息协议，`bar_state`拥有键盘/焦点/选择。两风格和现代菜单已有真实消费，公共input在GUI调用方执行，不使控制台runtime静态导入GUI。`bar_snapshot`拥有复制项/文本、owner-draw身份/原生回调及临时几何；`bar_controller`拥有栏/系统输入、原生跟踪迭代、焦点和NEXTMENU引用交接。Win3x与Win2000均直接消费，profile提供实际度量/屏幕几何/绘制及已有navigation差异；Win3x旧迭代文件与Win2000私有输入循环已删除。最终交付状态见CURRENT。

S9基础控件机制已交付：`application/control/control_state`持有字体/source、protocol、重绘/native-paint深度及tabstop资源，字体setter/租约结算和set-time tab事务已由Win3x实际使用。原`window/install`的通用实例发布事务归`application/lifecycle/install`，窗口与控件复用；完整setup失败时由provider完成其已有回滚/脱离策略，公共层不强制解除仍有资源所有权的实例。`application/control/retire`执行幂等/引用/脱离失败保留/最终刷新，`combo`拥有子窗口身份与字体池，`basic`、`list`、`edit`及`combo`执行原生消息、DC隔离与重绘，`scroll_resources`拥有native角色、caret资源与timer/borrowed-capture状态；实际呈现、字体与hover资格由profile提供。`scroll_geometry`提供layout/part数据契约，`scroll_track`接收命中/延时策略，`scroll_controller`实际拥有拖拽/通知/epoch/report/key及捕获/计时器状态；`scroll_managed`执行原生信息查询/设置和role启用，setter后重查绑定并pin绘制回调，Win3x只保留其几何与palette。`scroll_message`统一frame/control/sizebox原生消息，`list_snapshot`拥有原生行/临时文本与DC事务，`scroll_surface`统一原生条状态/DC坐标，`control_state_prepare`拥有初始protocol/tab副本。`button_snapshot`/`combo_snapshot`持有原生状态与复制文本，图标/字体/图片列表保持借用；`combo_setup`执行普通组合框子列表安装、身份发布、编辑绑定与失败回滚，Win3x实际消费。有限逐项归属和正式交付见CURRENT；扩展状态/通知/运行时在S10继续，创建/对话框S11、现代宿主S12、最终图形与renderer契约S13，不能称全部控件层已经完成。

## 应用、恢复与挂钩生命周期

S6将全局协调迁入`access/global`：namespace/session拥有会话身份与查询，control拥有desired/current、worker与停止后再启动的协调，start拥有GUI线程核验、ready事件和既有worker事务；worker拥有目标端会话核验/句柄/线程hook等待，late拥有后台初始化与通知协调；attach拥有分阶段加载/启动/control事务，manager拥有发现/目标/停止回收的组合；receiver拥有目标侧接收、重入和停止竞态，owner/host拥有会话、模块、双宽host及停止资源协调，trace拥有名称与监控恢复装配。产品提供包选择、模块/导出/控制协议、目标资格、完整风格操作、UWP能力及最终报告，公共层不引用风格私有头。旧产品manager/attach/remote/start/thread/late/process实现与六个空转发已删除；实际调用和验证边界见CURRENT链接的S6清单。此完成范围不包含仍待S7–S12拆分的应用与呈现机制，也不代表所有第三方宿主均已验证。

普通启动、运行时和全局接入最终调用相同的窗口/控件应用机制。切换统一为移除当前风格并尽力恢复，再安装新风格；聚合阶段和逐目标错误，不为六种转换分别维护实现。用户托盘选择 Original、Win3.x、Win2000 调用同一协调器。

传播停用、窗口恢复、钩子解除与 DLL 物理卸载是不同能力，按现有实现报告。窗口原生快照和恢复策略有明确所有者；对不受支持目标保持原生。全局模式的权限、目标、会话和 UX 保持已确认边界，不修改注册表、目标文件或系统安全配置。当前不保证无窗口新进程、所有框架或所有宿主都支持；实际覆盖以验证为准。

## 验证与迁移完成条件

T26 [任务设计与 S 规划](../proposals/style-library-reorganization.md) 是迁移计划，[顺序台账](../history/M0-T26-S1-ordered-audit.md) 是覆盖证据。审计按文件顺序，实施按能力合批。目标目录未落地前保留当前路径和迁移映射，不能靠移动目录宣称共用。

测试对应公共机制、框架适配、风格实现和产品组合；每个源文件可追溯测试映射，一个集成测试可覆盖多个文件，不复制测试凑一一对应。验证无产品/风格反向依赖、两风格已有相同能力的真实共用、旧实现删除、资源与 ABI 保留、双位宽及发布包行为；公开未通过和未覆盖项。Luna/Aero 或主题文件加载器是未来实现，不能用目录占位宣称支持。

S10已将扩展控件与VB/VCL的共同协议迁入以下完整公共对象；具体函数、消费者、删除路径及验证由[逐项台账](../etc/evidence/t26-s10-controls-audit.json)和[S10实施记录](../history/M0-T26-S10-extended-controls.md)承载，不在架构文档复制实施流水。

| 公共所有者 | 完整职责 | 风格或产品提供 |
| --- | --- | --- |
| application/control | 原生状态、项目/文本快照、通知、消息、绑定及绘制资源事务 | 支持资格、字体选择、度量与实际绘制操作 |
| application/adapters/win32 | 运行时身份与见证、关联引用、原生调用观察、附着与消息协调 | 既有框架支持策略及风格安装/移除操作 |
| application/lifecycle | 实例身份、借用与最终释放、安装结算、正常完成及既有异常展开 | 本对象拥有的资源清理及完整呈现操作 |

公共根直接拥有状态，profile通过组合嵌入并消费同一实现；同步配置仅在操作期间借用，不保存调用栈对象。普通移除、销毁和未发布失败有不同所有权契约，保持既有顺序与原生错误；共享机制不能强行统一成错误的清理路径。无新的异常恢复、权限或DLL物理卸载能力。

S10已完成有限范围归属审计、实际迁移及整组交付；初轮失败、定向验证和历史限制由CURRENT链接的证据原样保留，不称全T或所有应用均通过。创建/对话框由S11接收，现代宿主由S12接收，最终图形资源与renderer注册由S13接收；跨S项必须在台账精确列出函数与接收者，不能泛称“业务逻辑”而留在产品侧。

## 创建事务与生命周期

启动器和 DLL 都选择实际目标进程的位宽，不能使用父进程位宽代替。
DLL 从可信包目录按绝对路径定位。A/W CreateProcess 参数、环境、工作目录、
继承句柄、启动信息及原有 CREATE_SUSPENDED 语义须保留。安装成功后只恢复
本层为安装增加的挂起；调用者要求挂起时仍返回挂起进程。
失败时不发布半初始化进程；创建者仅可终止本事务新建且尚未交付的子进程，
并关闭自己拥有的句柄。禁止影响既有进程。

外观、字体、绘制、命中测试和资源只有 DLL 一个实现。窗口资源属于 GUI
线程；跨线程调用不得直接更换 HWND 状态。进程内拦截保留原函数 trampoline，
避免递归，窗口销毁释放恰好一次。DllMain 只做经过明确审计的有界引导，
不得加载样式库、枚举窗口、创建线程、等待、RPC 或创建 GUI/GDI 对象。
最终初始化及 hook 安装点必须先研究源实现再准入，不能把源项目行为直接视为合格。

## 历史来源与 ABI 演进（非目标目录契约）

源项目 renderer 与 nthook 是两个 DLL 家族；当前产品已拆分共享 ntwshk 与具体风格模块，产物以 product-contract.json 为准。原始 run16 有服务依赖，
W16Style 会主动加载样式，均不是可直接复制的最终应用。
ABI 的保留/删除及导出 ordinal 在固定来源对照后逐项决定；不承诺二进制兼容
ntvdm64，也不修改它的消费者。具体文件与证据见[导入候选](../proposals/native-style-import.md)。

T1 已选独立化设计：[引导与注入事务](../etc/import/INDEPENDENCE.md)。
本项目核心 ABI 定为 3，原 scroll/theme binding 改为 DLL 私有，helper 使用
ordinal 1；普通无载荷加载为被动核心，带载荷的新目标才启用自动拦截。
T6 S1当前实现用私有MinHook/HDE纯C后端替换旧Detours，初始加载事务无等待/
线程冻结；SDK导入表staging与头恢复仍与renderer延后初始化分离。保留静态
运行库/TLS清理所需的线程通知；新工具链独立整包通过状态以CURRENT为准。无 broker/句柄身份继承；载荷仅含固定宽头和包目录。
同宽直接更新 imports，跨宽使用有界 OS rundll32 安装事务，所有等待共享
10 秒截止时间。staged import 成功与目标实际初始化成功分别验证，调用者
原要求挂起时不能为等待 ready 提前恢复。
T3 已验证首次注入与同 DLL 自动窗口附着；T4 已实现原生A/W子孙传播并通过
四位宽边/三代、原生创建字段对照和回滚聚焦矩阵，完整回归与任务关闭以CURRENT为准。
当前核心 DLL 的显式 API 与自动加载边界见
[核心说明](../../src/ntw16s-dll/README.md)。详细限制和后续证明责任由设计记录承载。

S11已交付的对话框路径已直接消费公共完整proxy，不再构造并行字段指针view；Show的结果/原生回退和既有实验引用展开归dialog。子控件scope的链表/hook/计数和原生过滤、主creation的严格嵌套/屏障/pending/首次show归现有公共根，产品只提供真实风格操作和必要ABI。主事务通过不可变完整操作表借用prepare/install/attached/cancel/unsupported，到scope及其回调结束；普通结束不移除已安装的窗口。Win2000现有post-create observer合同尚未因此被改写为Win3x预创建协议。实际交付、原激活失败和现代/图形接收范围以CURRENT/S11证据为准，不表示T26全部完成。

S11的`application/dialog/classic_com_controller`现拥有既有COM激活/工厂、线程创建depth、代理/事件资源、消息和显示协调；由产品装配固定会话判断、样式scope、协议字符串和原实验配置。配置值复制到公共根，借用的字符串与callback代码覆盖hook/工厂/proxy的完整存活期；底层patch事务仍由调用方提供，控制器不另建hook后端或物理卸载机制。消息入口直接消费同一公共proxy和message configuration，只为完整导航保留操作回调，删除逐字段/逐消息转发。默认实验开关仍关，完整UI/交付状态见CURRENT，不将原生对象单元测试扩大为所有应用支持。

S11的`application/dialog/classic_file_controller`拥有A/W打开和保存的参数转换、原生返回与LastError结算、嵌套线程状态及观察hook/creation-observer恢复；产品只提供active/scope/chrome及既有observer模式。原应用hook/template、多选及扩展参数沿用既有原生路径，观察hook失败不新增阻断或重试。`application/lifecycle/creation_call`持有固定prepare/active/creation操作上下文、一次初始化和调用结束结算；这些是实际状态所有者，不是新的产品转发层。实际测试与交付状态由CURRENT及S11证据记录。

S11的`application/dialog/observation`统一持有创建调用、当前线程观察hook和可选嵌套目标身份。Win3x font/color组合创建事务与观察；Win2000用NULL创建上下文明确只观察，不改成预创建附着。结束按既有顺序恢复身份、释放本层hook和创建作用域，应用既有呈现不会因结束被移除。classic_file_live操作直接借用同一完整proxy，原生调用可重入并改变active_window，之后读取当前字段；没有另建字段指针视图或并行状态。实际交付状态仍由CURRENT管理。

S12交付的`application/adapters/frame`嵌入既有host/instance并拥有bitmap、刷新/输入/激活状态、menu owner和action事务；profile在同一次分配中嵌入公共根及action尾存储，只保留年代布局、命中、命令/菜单绘制和字体策略。input/surface直接使用完整根，不再接收字段指针view。内部移除/表面查询由产品直接调用公共入口，DLL协议及导出保持。有限验证、六项原生接入/实际宿主未通过观察及交付状态由CURRENT/S12证据确定，不能据此声称所有现代应用已验证；原frame_view字段视图已由S13替换为完整frame_renderer操作，普通与现代路径共用；S13仍未正式交付。

S13当前图形所有权契约：`application/graphics/font_cache`直接拥有来源字体缓存、字形映射、快照借还、已发布内存字体引用和variant池；年代提供实际字形/FNT生成及配方key，不复制计数和释放链。`graphics/present`同步借用完整实例与绘制操作，拥有缓冲/DC和既有打印顺序；`control/native`通过现有control state字体操作拥有临时租约和存活检查后的原生回退。具体tab/header/progress呈现由appearance直接选择，产品无需转发它们。上述是已实施的局部契约，外框renderer已增量接入，整组审计及交付状态见CURRENT，不能由此称S13完成。

外框renderer同步借用完整instance和font_pair；年代操作提供布局、配色、滚动条及整份呈现状态。快照不拥有资源，应用菜单测量和标题文本回调后重读，保留原生重入对font/active/selection的更新。普通/modern使用同一年代renderer，公共graphics负责有界buffer提交和失败直接绘制；两种宿主已有生命周期根保持独立，不为消除字段view复制状态。实际交付状态仍以CURRENT为准。

文字资源调用中，公共`graphics/font_scope`拥有临时font/saved-DC恢复与字体lease归还，完整thread gate协调已有嵌套；年代只提供选字体、文本支持条件及实际lease返回策略。公共`adapters/win32/runtime_text_imports`承载既有ANSI API绑定和原import事务，调用者资格与字体provider仍由产品/年代选择。普通menu-only分派复用已有公共dispatch异常完成机制；未给原正常返回的ANSI/字形叶子新增异常处理，也未增加目标或DLL卸载能力。

已有theme与popup临时注册作用域分别由`application/lifecycle/theme_track`和`application/menu/menu_owner`拥有：前者结算现有open/close取消，后者持有稳定owner身份、移除前清除installed状态并核验窗口存活。产品提供原生API调用、资格与具体样式操作。沿用既有异常展开调用这些完整根，不新增catch/retry或物理卸载保证。

完整文字图形操作也归graphics：`text_metrics`持有既有缩小文字的临时DC/bitmap与成功发布，年代提供字体/颜色/源尺寸上限；`font_cache`的glyph-text根持有解码/单字节编码两块缓冲。产品ExtTextOutW仅装配原ABI、支持规则和A/W路由，保持字体恢复后再释放文本、最后退出gate的顺序。普通外框测量借用完整instance/font_pair，在native菜单测量回调后读取当前DPI和退役标志，不保存字段地址视图。原正常返回路径没有新增异常处理合同；交付及失败边界见CURRENT。
