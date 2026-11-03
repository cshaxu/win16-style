# Win16 控件与对话框内部库设计候选

## 后续 Win2000 样式的接口准备

用户要求后续加入独立 Win2000 样式库。接入协调应面向样式能力，而不直接依赖 Win16 的调色板、布局结构或字体：不同样式可以有不同标题按钮、边框度量、控件绘制和资源。对话框布局转换及应用行为应与选用的绘制实现分开；窗口生命周期、GUI 线程、重入及资源所有权必须只有一个明确所有者。

候选内部接口需覆盖：样式实例创建与释放；按窗口 DPI 和状态计算几何及命中区域；标题、菜单、滚动条和标准控件绘制；字体/颜色/资源查询；窗口及控件状态变化与失效。绘制与命中应共享同一份几何，输入/消息语义不得由主题资源隐式改变。调用方提供目标 HWND、HDC、剪裁和已获取的状态；资源的借用和释放规则应明示。Win16 与 Win2000 通过各自实现接入，不把 Win16 私有结构直接改名成通用接口，也不要求两者使用同一套像素资源。

这是待验证的内部契约草案，不是已实现的多主题接口或公开 ABI。须先从当前 Win16 的真实生命周期和 Win2000 的目标控件/标题差异提炼最小接口，并用两种实际实现验证后才能定型；不得为未来主题复制现有接入或维护两份窗口状态。现阶段继续迁移可独立的策略，保持现有行为及三组件四产物。

## 范围

M0 T21 S21 为设计和依赖盘点；不修改产品行为、原图形资源、公开 ABI、全局管理或进程接入。两个库均作为内部 C17 静态构建单元，不增加运行时产品文件，维持三个产品组件、四个交付文件。产品消费和导出仍由 ntw16s DLL 管理。

## `src/lib/win16-control`

S22 资源路径记录：原 `src/ntw16s-dll/menu_glyphs.h`、`frame_buttons.h`、`control_glyphs.h`、`combo_glyph.h` 和 `scroll_glyphs.h`
已逐字节迁至 `src/lib/win16-control/`；原 `resources.md` 作为历史来源声明
保持逐字节不变，其中重现命令的旧 `-VerifyHeader` 路径对应新的位置。
资源身份校验器和来源清单跟踪新路径，图形和来源声明的摘要均未改变。

候选职责为 Win16 标题、边框、菜单、标准控件的绘制、布局、度量、资源与进程内窗口附着。以明确的窗口上下文管理字体、DPI、画刷、资源所有权、重入和解除；不得假设所有函数无状态或纯绘制。现有 `layout.c`、`scroll_layout.c`、`*_paint.c`、菜单绘制和字形资源适合作为初始依赖盘点对象；`controls.c`、`win16_style.c`、`attachment.c` 混合窗口消息、状态和资源，先画出函数及依赖关系，再按职责拆分，不能按文件名整体移动。资源及来源声明逐字节保留。

窗口附着可由进程内调用者显式调用；何时、怎样把代码带入目标进程，仍是调用方现有职责，不属于此库。标准控件、标题、菜单、滚动条和已有应用兼容行为均以既有证据为基准；不声称全部应用自绘控件均受支持。

## `src/lib/win16-dialog`

候选职责为进程内公共对话框的参数检查、可转换性判定、旧式布局调用、结果与取消语义转换以及资源释放。初始盘点对象包括 `file_dialogs.c`、`folder_dialogs.c`、`com_dialogs.c`、`print_extended.c`、`print_ranges.c`；它们目前同时包含适配与接入相关职责，须先区分并建立调用契约。`modern_dialogs.c` 包含调用者显式请求现代布局的既有公开入口，应保持其行为和 ABI。

旧式文件对话框、Shell 文件夹树及打印等类型不能假设可互换。保留已有 A/W、错误、缓冲区、回调、线程和生命周期语义；多选、应用定制或其他无法可靠映射的情况沿用已有回退，不扩大兼容承诺。库通过明确接口接收调用方提供的操作和上下文，禁止依赖 `ntw16s-dll` 私有全局状态；可由项目现有调用路径使用，不自行实现全局接入。

## 依赖方向与实施顺序

按用户确认的方向，`global_*` 源码留在 `ntw16s-dll`，由现有全局模式调用公共样式及对话框适配能力；公共库不反向依赖全局模式。普通 `run16s` 启动路径也必须调用同一套业务实现，不能只连接全局模式或复制第二套。当前普通启动及后期接入分别有既有入口，具体依赖和共享状态仍须在逐函数审计后确定；本段是目标边界，不宣称现有实现已完成接线或每个 `global_*` 文件都直接调用两个库。

1. 先产出逐文件及函数级依赖、状态所有者、资源和既有测试清单；区分视觉算法、窗口附着、对话框适配与现有接入机制。
2. 为两个库分别建立内部接口和静态目标，在保持现有入口、文件与 ABI 的情况下逐组迁移；构建依赖单向，避免两个库互相引用。对话框需要样式时，通过调用方提供进程内窗口适配接口处理。
3. 逐组运行针对性行为对照、双位宽构建、已有视觉和对话框回归、完整包检查；保留用户验收和未覆盖项，不把探针等同真实交互。

当前 `docs/design/CODING.md` 对通用产品层的表述须在实施前澄清：本设计仅增加内部静态库，不增加产品组件或文件；如需更新源码布局设计，应在对应实施任务同步修订，不改写治理规则。新增内部接口不等于公开 DLL 导出；若实施要求变更公开 ABI，先单列审查。

## 可协助范围和限制

可以审查和设计纯绘制、布局、控件资源管理、进程内窗口行为及对话框参数和结果适配。涉及跨进程批量接入、注入和自动挂钩传播的代码不能代为重构或提供实现；相关既有机制保持原位，视觉库与进程内适配库只定义明确边界。本设计不承诺所有现有入口均可在不修改接入层的情况下直接迁移，需在依赖盘点后逐项确认。

## S21 依赖审计：是否能不留尾巴

“所有样式和对话框业务代码都在两个库内”与“调用方完全没有入口、状态或接口代码”不是同一要求。现有实现需要 DLL 对外导出和既有接入层；前者可以作为目标，后者与保持现有调用及 ABI 不相容。仅按本次静态审计，不能保证所有业务代码都能无残留搬迁，更不能保证迁移后行为等价。

| 审计对象 | 实际耦合与处置 |
| --- | --- |
| `layout.c`、`scroll_layout.c`、菜单及控件 `*_paint.c` | 视觉计算和绘制是优先候选；仍须逐项确认调用方、资源与编译依赖。 |
| `controls.c`、`win16_style.c`、`attachment.c` | 同时包含绘制、窗口身份与线程状态、生命周期和异常清理；不能当作独立纯绘制文件整体搬运。窗口附着需要调用者传入目标和上下文。 |
| `injection/window_titles.c` | 同一文件混有对话框调用范围、窗口创建、字体、菜单和控件相关路径；若要求把所有样式业务移出，必须对混合职责做函数级拆分，不能仅移动 `*_paint.c` 后宣称完成。现有跨进程接入与自动传播实现不在可协助重构范围。 |
| `file_dialogs.c`、`folder_dialogs.c`、`com_dialogs.c` | 适配逻辑与现有接入、线程局部状态、错误和清理混合，且彼此调用。仅移动文件或改变 include 不能证明脱耦；必须明确调用契约与状态所有者。 |
| `print_ranges.c`、`print_extended.c` | 范围转换与打印参数适配可作为较独立的优先候选；仍须保持页面范围、句柄所有权、取消与错误语义。 |
| `modern_dialogs.c` | 拥有显式现代布局入口及线程局部状态；x86/x64 的公开导出序号 33 必须保留，调用层与现有 COM 适配的共享状态不能重复。 |
| `injection/unwind.*` | 现有样式和对话框都依赖异常清理机制；迁移时必须保持同一状态所有者和清理语义，不能复制独立线程状态。 |
| 原字形资源及 `resources.md` | 资源校验器和来源清单固定记录当前路径及摘要。若要求资源也移动，必须先设计可审计的路径迁移并保持图形字节与来源声明原样；直接移动会使现有门禁失败。 |
| CMake 与测试 | 产品和多个测试直接列出原源文件；迁移必须同步调整构建及测试依赖，禁止为通过测试保留第二套实现。仅静态搜索无法证明完整运行时覆盖。 |

在不动现有接入机制的条件下，两个内部库可以集中可分离的视觉和对话框业务，但 DLL 的导出、调用入口、接入及必要桥接仍要保留。若“不留尾巴”要求包括把现有批量跨进程接入与自动传播也迁出，则不在可协助实施范围；如果要求 DLL 不保留任何公开导出或业务桥接，则与保留现有 ABI 和使用方式冲突。若只要求“业务实现不重复，剩余薄层只作必要适配”，这是合理的目标，但必须先经过逐项迁移和双位宽实测才能确定是否完全达到，当前不能承诺零尾项。

本审计只读检查了当前源码、导出、构建引用和资源门禁；资源身份及来源校验自测通过。未建立完整函数调用图，未迁移或编译候选库，未验证所有窗口、对话框组合和全局模式，因此不以此宣称拆分已完成。

## 对话框后续迁移准备（静态盘点）

S22 已完成控件样式库交付；本节的对话框候选已准入 S23，仍须逐项实现和验证。

| 现有实现 | 可迁移业务与必要边界 | 验证重点 |
| --- | --- | --- |
| `print_ranges.c` | 页码解析/格式化只依赖 Windows 类型和 C 运行库，可首先作为对话框库内部模块；`print_extended.c` 和单元测试需引用同一实现。 | 数字溢出、区间与容量边界、失败时不改输出。 |
| `print_extended.c` | 参数和资源快照、范围编辑及结果转换属于对话框适配；其 A/W 入口已通过回调接收原始打印函数。调用方仍负责现有接入。 | 打印/取消、A/W、句柄所有权、驱动副本、异常和无法映射的回退。 |
| `file_dialogs.c` | 参数资格检查和结果映射可拆分；线程局部观察状态、异常清理、原始函数和入口安装目前与调用层交织，须明确单一所有者。 | 两种结构大小、A/W、标志回退、快捷方式、缓冲区和嵌套取消。 |
| `folder_dialogs.c` | 文件夹路径读取、UI 和结果转换应与观察及原始入口分离；`active` 是线程局部状态，不能复制成两份。当前有旧式辅助路径，迁移不得改变用户要求的 Shell 文件夹布局策略。 | PIDL/路径、回调、取消、嵌套、Shell 树与实际现有回退。 |
| `com_dialogs.c` | COM 代理和结果转换有内部状态、引用计数及与文件夹辅助函数的调用；`native_creation_depth` 与显式现代对话框状态必须保持单一且按原次序清理。 | COM 引用与异常、多选和定制回退、文件夹、事件及现代入口。 |
| `modern_dialogs.c` | 对外导出和调用方可见 ABI 留在 DLL；显式现代布局范围的线程状态必须仍被 COM 适配共享，不能用独立副本替代。 | x86/x64 序号及签名、显式现代选项、取消、嵌套和资源释放。 |

`ntw16s_dialog_style_scope`、异常清理和接入层仍是现有调用边界；静态库应通过明确回调使用调用方的能力，不能把接入状态复制进库。以上顺序和测试是实施候选，不代表实际迁移、链接或运行已经验证。

## S24 剩余职责盘点（P58 时点）

这是后续迁移的工作清单，不代表函数级审计已结束，也不把薄包装的数量当成剩余工作量。与上面的 S21、S23 历史盘点冲突时，以本节和 CURRENT 的活动包为准。

| 位置 | 当前可见职责及下一步 |
| --- | --- |
| `attachment.c` | 首次绘制验证已迁入库；仍混有控件身份判断、见证和绘制作用域编排、附件生命周期及创建观察。逐组分离仍在 DLL 内的样式判断；窗口观察、线程局部状态及接入留在调用层。 |
| `controls.c` | 大量绘制和消息算法已调用库，但还含控件过程、状态视图、原生回调及异常清理。审计每种消息处理，将仍属于样式的处理迁至库；子类入口与调用方作用域保持原状。 |
| `win16_style.c` | 通过库实现框架绘制、菜单和生命周期，仍有窗口过程、菜单跟踪、状态视图与回调。继续辨别可迁的主题业务与必须保留的线程/子类入口。 |
| `theme.c`、`vb_text.c` 和 DLL 内相关头文件 | 存在向库转发及调用方适配；逐个核实是否还有真实样式算法、反向依赖或可消除的冗余，不以文件存在就认定未抽离。 |
| `injection/window_titles.c` | 混有窗口与 API 接入、对话框范围及绘制相关回调。入口、原函数和异常边界保留；其余字体、菜单、滚动条及对话框策略需继续逐函数审计。 |
| `injection/file_dialogs.c`、`folder_dialogs.c`、`com_dialogs.c` | 已有库参数转换、代理及布局逻辑；现存观察、线程局部状态、异常处理、COM 事件及导航须区分入口和仍可迁移的业务。实验事件路径只做过语法检查，完整运行未验证。 |
| `injection/modern_dialogs.c` | 公共导出及共享的线程局部现代对话框范围仍在 DLL；保留 ABI，复核其与库的职责及生命周期。 |
| `global_*`、进程注入和传播 | 用户明确要求留在 DLL；不把这些文件迁出，不更改其接入流程。 |

验收仍需逐项证明业务实现没有留在调用层或重复存在、库独立链接且两种启动路径使用同一实现；仅凭以上清单或聚焦回归不能证明全部完成。日历、滚动条视觉和 144 DPI 微调控件既有差异按 CURRENT 记录，不能记为通过。Win2000 的公共主题接口仍是后续设计，不能以现有 Win16 私有接口宣称已实现。

## S24 final responsibility audit

The completed split is by responsibility, not by an unsafe physical move of
whole mixed DLL files. `win16-control` owns all Win16 layout, palette, font,
glyph, hit-test, paint, message-policy, control-state, and attachment-policy
algorithms. `win16-dialog` owns all dialog eligibility, parameter/result
conversion, classic-layout coordination, dialog chrome, COM-proxy policy and
dialog state helpers. Neither library includes a `ntw16s-dll` header or links
to the product DLL; both are independently whole-archive linked by the native
library tests.

The remaining code in the DLL is deliberately limited to the process-local
adapter boundary:

| DLL area | Retained responsibility | Why it is not library style logic |
| --- | --- | --- |
| `attachment.c` | HWND subclass installation, per-thread scopes, native callback entry and exact unwind order | It supplies handles and callbacks to library views; it has no direct Win16 drawing call. |
| `controls.c` | `SUBCLASSPROC` entry, native control callbacks and TLS/unwind ownership | It assembles service tables; control drawing, state transitions and policy are library implementations. |
| `win16_style.c` | frame subclass entry, per-thread menu hook and DWM/USER callback binding | Frame geometry, painting, input policy and lifecycle are delegated to `win16-control`. |
| `window_titles.c` | imported API trampolines, creation scopes and original-error preservation | Hook leaves only call the renderer/adapters; they must remain beside the original trampolines. |
| `file_dialogs.c`, `folder_dialogs.c`, `com_dialogs.c` | original Win32/COM hook leaves, TLS/session scope, COM reference and unwind ownership | Dialog conversion, result/state logic and classic chrome live in `win16-dialog`; the remaining code binds it to original APIs. |

The final static sweep found zero direct Win16 drawing calls in those seven DLL
adapter files. API names such as `DrawThemeBackground`, `BitBlt`, and
`DrawText` remain only as intercepted original trampoline declarations and
calls. Moving those leaves into either library would make the library depend
backwards on DLL-local original pointers, loader scope, TLS and unwind code,
breaking the intended one-way dependency. `global_*` remains untouched in the
DLL as required.
