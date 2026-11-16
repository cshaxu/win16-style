# T26 样式公共库重组任务设计

用户已确认三层顶层设计并授权按本规划继续；M0 固定，活动包和编号由 [CURRENT](../states/CURRENT.md) 管理。原 T25 S9 去重并入 T26。架构、状态和依赖以 [System Architecture](../design/ARCHITECTURE.md) 为权威；本文件只定义迁移与执行计划。

## 目标与范围

统一根目录 `src/lib-win32-style`，形成 access（startup/runtime/global）、application（生命周期、窗口、菜单、控件、对话框、图形与框架适配）、appearance（win3x/win2000 及未来风格）。通用进程、句柄、托盘能力保留 `lib-win32`，具体托盘菜单属于产品。现有五组件八产物契约保持。

逐文件审计必须判断是否能拆开私有状态与公共机制，不能以当前依赖 Win16 私有类型为保留理由。按能力实施，共同状态和生命周期实际下沉，两风格已有相同能力必须真正使用；删除旧路径和复制实现。保留原图形资源、来源声明及现有用户行为。T26 不新增全套 Win2000 控件、Luna/Aero 或未支持的框架能力；已知缺陷的迁移回归不得冒充通过。

## 已确认 S 序列

共 14 个 S；S1 沿用已有编号及有效成果，S2–S14 为用户确认的有序计划，逐一满足准入与退出，不预称已开始。每个 S 完成覆盖、实施、验证、交付和单人双角色复核后自动进入下一 S，用户指定验收节点除外。

| S | 工作范围 | 收口条件 |
| --- | --- | --- |
| S1 | 完整审计与架构冻结 | 全部文件、生产入口及测试有迁移归属；冻结接口、状态所有者、依赖、基线与未解决项 |
| S2 | 目录与构建归位 | 全部样式源码与测试映射到新根；构建、来源、旧路径清理及原行为验证 |
| S3 | 公共生命周期 | 附着身份、重入引用、资源、安装移除事务公共化；已有两风格路径接入并删除重复 |
| S4 | 启动接入 | 普通启动、初始化和后代传播统一装配；位宽、参数和挂起语义保持 |
| S5 | 运行时接入 | 已运行进程与既有/后续窗口使用相同附着机制，明确失败和停用边界 |
| S6 | 全局接入与切换 | 全局发现、会话和托盘业务使用统一协调；Original/Win3.x/Win2000 六种转换验证 |
| S7 | 窗口与标题栏 | 非客户区、命中与恢复公共化；各代度量、布局、按钮和绘制独立 |
| S8 | 菜单 | 菜单栏、系统及弹出菜单输入和生命周期共用，保留风格差异 |
| S9 | 基础控件 | 按钮、静态、编辑、列表、组合和滚动条的协议、状态、输入与呈现解耦 |
| S10 | 扩展控件与运行时适配 | 公共控件及 VB/VCL 身份和协议集中，具体呈现归风格，产品侧精简 |
| S11 | 对话框 | 参数、代理、转换、事件、结果及资源公共化；布局与转换策略独立 |
| S12 | 现代框架适配 | UWP/WinUI 现有宿主和外框机制风格无关，删除 Win16 适配副本 |
| S13 | 呈现与图形边界终整 | 清理过渡接口、统一风格注册和图形资源契约；无反向或跨风格依赖 |
| S14 | 全范围终审与交付 | 重查归属、真实调用、旧路径、组合及恢复，完成发布测试远端复核和规定验收 |

S3–S12 随机制迁移同步拆分风格呈现，公共图形遇到实际消费者时迁移；S13 只做最终整理。各 S 的详细改动和适用验证须以 S1 证据细化，不通过压缩范围满足收口。

## 组件迁移映射（S1 逐项细化）

| 当前来源 | 目标归属 | 迁移判断 |
| --- | --- | --- |
| lib-win32/style 的启动、创建待处理、窗口钩子 | access/startup、runtime 与 application/lifecycle | 分离进程加载、窗口观察和具体风格应用 |
| DLL 的 global 与 UWP 会话协调 | access/global、runtime | 复用 process-global 和 process-hook，具体呈现不进入接入层 |
| lib-style/common 的附着和主题状态 | application/lifecycle | 状态身份、重入和事务公共化，策略留在风格 |
| lib-style/common 的 frame、menu 与 Win16 对应实现 | application/window、menu 与 appearance | 按机制/策略拆分文件，不整目录归类 |
| lib-style/common 的 control、Win16 控件实现 | application/control 与 appearance/win3x/control | 拆开运行时协议、输入、资源状态与呈现支持条件 |
| 公共对话框及产品 injection 对话框入口 | application/dialog、access 与 appearance | 参数/事件共用；挂钩装配、转换选择和布局分别归位 |
| lib-style/common 像素、裁剪、缓冲、字体资源 | application/graphics | 资源所有权与算法公共化，具体年代资源归风格 |
| lib-style/uwp 与 win16-window-uwp | application/adapters/uwp、winui 与 appearance/win3x/window | 按实际宿主及框架分类，几何/绘制回到风格 |
| win16-common、win16-window、win2k-window | appearance/win3x、win2000 与 application | 不能以现有类型或目录决定归属，审查是否仍藏有公共机制 |
| run16s-exe、ntwshk-dll、ntw16s-dll、ntw2ks-dll | 产品装配及上述公共组件 | 产品保留业务选择、导出和兼容适配；通用管理及生命周期迁出 |

此表定义组件归属与核查方向，不代表每个文件已完成迁移；151 文件台账必须补充对应公共入口和状态所有者，产品入口需另查。Win2000 缺少对应渲染功能不阻止通用机制下沉，也不要求在 T26 追加该功能。

## S1 执行与在途工作

当前 151 文件顺序清单保留，之后补查产品对话框、接入和测试调用者；不能只审库文件。第 41 项裁剪 DC 去重及测试的未提交修改保留为在途能力批次，尚未发布/提交。`build/t26-package/ordered-41-frame` 是此前暂停中断的快照；本轮核实无完成结果、无活动编译进程和 reparse point 后已清理（12,861,797 字节），保留源码与 .ninja/.ccache。后续合批时新建快照，不能称中断构建已通过。

后续连续审计，每项登记旧位置、目标组件、状态所有者、公共入口、调用方、旧路径处置、对应验证及缺口。审计完成不自动等于全部机制已迁移；已有“已过关”记录须在最终归属矩阵中复核。相关发现合批到能力单元，不对每个文件单独完整交付。

开发中增量构建、针对性双位宽测试；本 S 的源码与测试合并后进行必要的同快照完整交付。二进制变化按先构建打包、发布 assets 与 O:/winnt、实际包测试、摘要核对、提交推送及远端复核执行。文档/测试变化不虚构发布。完成后清理本轮临时副本，保留增量缓存。

单人先承担实施角色，再以复核角色独立读取 diff、原请求和实际证据，发现问题返回实施；复核不是复述自己的总结。每批或约 30 分钟执行效率检查，记录新证据、批次调整、资源和下一步，规则见 [执行规范](../rules/EXECUTION.md#效率检查点)。禁止 computer-use、隔离桌面、修改兄弟仓库或把它们作为依赖。

## S1 连续审计批次：编辑、owner-draw 与表头（41–46）

本批是职责与接口设计，正式状态仍见顺序台账；不把设计完成当作迁移或运行通过。按 41–46 顺序核对源码与实际调用，集中以下能力到 S7/S9/S10/S13 实施。

| 当前文件 | 公共目标与状态所有者 | 风格呈现与调用方 |
| --- | --- | --- |
| edit_message.c | application/control 的原生编辑消息和重绘协调；裁剪 DC 由 application/graphics 管理 | application 调用风格的边框/滚动条绘制；现有 control_dispatch 为接线来源 |
| edit_message.h | 改为编辑操作的公共上下文与绘制契约；控件实例持有公共状态，回调期间有效 | 移除对完整 control_style 的接口依赖，度量和颜色由 appearance 提供 |
| fileowner_message.c | application/control 的 owner-draw 保留、原生转发和绘后通知；实例拥有有效性及父子关系 | Win3.x 仅提供 chrome 绘制及支持资格；VB runtime 保持 item/text 权威 |
| fileowner_message.h | 与基础控件共享绘制上下文；契约表达 overlay 区域及状态 | 不复制一套 owner-draw 生命周期；具体支持条件归风格 |
| header_paint.c | application/control 的表头项目/文字快照、变更代次与重入核验；graphics 的 DC/裁剪 | appearance/win3x 的颜色、边框、间距和 pressed 位移；format 支持掩码由风格提供 |
| header_paint.h | 快照与变更通知归公共契约，支持与绘制归风格契约 | 当前 guard/state/controls/native_presentation 调用点须统一迁移 |

编辑状态应拆出 suppression depth、redraw-off 与公共附着存活标志；Win16 的 edit_border、配色、字体映射不得强塞进公共接口。原生 WM_PRINT 的 DC 保存恢复、重绘抑制与销毁后禁止绘制维持现有顺序。

表头快照在任何像素写入前采集全部项目；公共采集过程在每次可能重入的原生查询后检查附着及代次，失败整体返回给调用方回退。现有文本增长上限和项目数限制按现有行为保留；支持的格式是风格能力参数，不能将 Win16 的格式限制硬编码为所有风格的限制。不要为文本长度、格式、颜色等每个字段各建回调。

在途 style_frame_update_dc 是图形资源操作，当前临时位于 common/frame.h；S2/S7 归位时迁入 application/graphics，窗口与编辑共用。取得 DC 成功后 USER 接管复制区域，调用方 ReleaseDC；失败由公共层释放副本，绘制回退由调用方决定。

本批验证计划复用公共区域所有权测试、编辑 undo/selection/IME/scroll 相关现有夹具、owner-draw 原始内容保留以及表头重入/特殊格式回退。先区分现有不稳定，再验证迁移前后；不为 S1 的纯设计重新启动完整包构建。在途二进制改动不能冒充已交付，合批后执行正式闭环。

## S1 连续审计批次：列表（47–50）

| 文件 | application 公共机制 | appearance 风格策略 | 所有者与验证入口 |
| --- | --- | --- | --- |
| list_message.c | 原生消息转发、选中/顶行快照、重绘抑制、WM_PRINT 坐标与滚动通知协调 | 无 hover 策略、绘制替换范围、何时强制更新 | 公共控件实例拥有 suppression/redraw 状态；control_dispatch 调用；control_list 和 superclass 夹具 |
| list_message.h | 公共列表消息上下文与 tabstop、scroll、paint 操作契约 | 提供对应策略和绘制实现 | 不再通过整个 Win16 control_style 传递公共状态 |
| list_paint.c | set-time 制表位换算/提交；原生项文本、选择和几何采集；通用缓冲与边框算法 | 颜色、字体选择、固定文本间距、原图案、边框度量 | 列表实例持有 tab 像素数组，快照操作拥有临时文本；list_paint 与 control_link 现有测试 |
| list_paint.h | tabstop 操作和列表数据视图分别成为公共契约 | Win3.x 列表和编辑/列表边框绘制接口 | 避免把当前混合头原封不动迁到公共库 |

制表位保持 set-time 字体测量、现有舍入和整数边界；先计算临时像素数组，再转发原生 LB_SETTABSTOPS，只有原生成功且附着仍有效才提交并替换旧数组，调用方退出时释放。原生失败或重入脱离时释放临时数组并保留原生返回值；不改变字体选择策略。

列表项目采集应与绘制分离，同时保留 USER 的多列顺序、单列 xOrigin、选择和焦点语义。公共 snapshot 必须定义回调期间存活与重入后的失效规则，不能只把旧的查询代码改名；当前直接读取与绘制交织，属于 S9 待迁移项。现有长度查询和随后取文本之间的重入行为须在迁移前核实，不能用新增无限重试掩盖状态变化。

本批仅完成源码和调用/测试入口核对，没有新实现或运行结果；未将审计文件标为已迁移。与编辑消息共用同一控件状态拆分，放在同一能力批次增量验证。

## S1 连续审计批次：月历与原生呈现（51–58）

| 文件 | 公共机制目标 | 风格与调用边界 |
| --- | --- | --- |
| month_paint.c | application/control 的 month native 几何查询、来源/主题匹配、绘制关联状态和存活核验 | 字体选择、支持 DPI/字符/part-state、黑色判断、背景与箭头绘制归 win3x |
| month_paint.h | 公共 month 几何/作用域快照，与风格字体租约分开 | 当前混合 view 不能整头迁移；attachment 提供作用域存活和窗口身份 |
| month_text.c | 标题居中区间与两个导航区间不相交的几何运算 | ASCII/U+200E 与长度上限是当前 Win3.x 字体支持策略，不能升为所有风格字符限制 |
| month_text.h | 将通用几何接口与风格文本资格声明分开 | injection/window_titles 的实际调用链须同步迁移 |
| native_combo_identity.c | application/control 的原生颜色/日期按钮 snapshot 及查询后有效性检查 | 当前 identity 已在 common；按钮 Win3.x 绘制由风格提供 |
| native_combo_identity.h | 通用原生按钮描述，不携带 Win16 painter | attachment 提供公共 identity/作用域，再选择具体风格绘制 |
| native_presentation.c | application/control 的绘制事务、存活判断与原生 fallback 协调 | 字体选择/租约由资源所有者提供，tab/header/progress 绘制由风格实现 |
| native_presentation.h | 定义通用呈现结果和上下文契约 | 不把 control_style 作为公共接口参数，不逐个控件复制相同回退骨架 |

公共月历查询必须保留每次原生调用后的存活检查和 scope 当前性检查。几何快照由当前绘制作用域拥有；选中背景与文本的关联记录有明确有效期，字体缓存与借用租约独立管理，不将 Win16 固定字体要求放入公共身份识别。

公共呈现需要区分成功、未接管和已失效；只有仍存活且未接管时才回退原生。在开始提交像素前完成所需快照/资格检查，避免一次失败导致半套自绘叠加原生。接口以一次完整绘制事务为单位，依赖操作范围内的资源有效性，而不是复制每个控件的 retain/paint/fallback/release。

已核对 attachment 的 month_view/query 入口、window_titles 的文本拦截、controls 的 tab/header/progress 回调，以及 native_combo 的颜色/日期入口。现有验证入口为 core-month-model、core-style-control-native-combo、颜色/日期模块夹具、tab/header/progress 和字体租约测试；本批仅核实入口和范围，没有新运行结果。具体重入、异常及回退证据在实施 S9/S10 前对照基线，旧的不稳定项仍开放。

## S1 连续审计批次：通知、按钮、进度与滚动资源（59–69）

| 文件 | 公共机制与目标 | 风格归属/完成要求 |
| --- | --- | --- |
| notify_message.c | application/control 自定义绘制通知、app 返回值协商、私有阶段、重入存活和错误码保留 | 现有 control_custom_draw 继续复用；trackbar 特有 pending/postpaint 匹配纳入协议，具体 supported/paint 由风格提供 |
| notify_message.h | 公共 custom-draw 作用域拥有 owner、draw、DC、item、矩形和嵌套关联 | 移除 control_style 类型依赖；不能用悬空 draw 指针跨回调保存状态 |
| other_paint.c | application/control 的原生按钮图片状态采集、借用图片所有权；graphics 的临时文字位图和比例缩放 | Win3.x 字体选择、按钮/分组/静态度量、颜色、焦点边框、拟合启用策略留 appearance |
| other_paint.h | 公共按钮内容描述与风格绘制契约分开 | 不把一整套 Win16 other painter 作为通用 API |
| progress_paint.c | application/control 的 range/position/state/color 快照、变更代次与原生协议核验 | Win3.x 支持样式、缺省配色、错误/暂停颜色、分块和边框属于风格 |
| progress_paint.h | 公共快照/变更接口与风格 paint/supported 分开 | 实际 control_state/guard/native_presentation/controls 调用同步接入 |
| scroll_assets.c | 无通用进程或窗口机制；资源访问表可原样归位 | appearance/win3x/resources，普通/pressed/disabled 的枚举映射须保留 |
| scroll_assets.h | 年代资源描述与公共像素描述分离 | Win16 原图案仅由该风格注册，通用 updown 不引用这些枚举 |
| scroll_glyphs.h | 固定 VGA.DRV 原始图案，禁止重新生成或改像素 | appearance/win3x/resources；保留来源、摘要与资源校验 |
| scroll_layout.c | range-to-pixel 等纯算术仅在保留舍入语义时复用已有公共 scroll 算法 | 固定 thumb、短条边界与箭头优先命中为 Win3.x 策略；Win2000 需独立比例滑块策略 |
| scroll_layout.h | 公共布局描述可包含条、箭头、滑块、travel；风格提供 measure/hit 策略 | 不能把 nPage 忽略规则硬编码成公共布局；不为两个风格复制原生状态采集 |

通知作用域由公共控件实例/当前调用持有，应用自绘返回值保持权威；先协商、后承诺私有阶段，绘制后移除或换父仍须正确退场。原生字体与图片只借用，不由公共快照删除应用对象。文字缩放资源是图形操作，启用缩放及限额保持现有策略，不对所有按钮强制缩放。

progress 查询须在每次可重入消息后核验代次/存活，采集阶段不画像素；绘制使用完整快照。supported 条件与 palette 不混入原生身份核验。scroll 的 Win3.x 几何有明确原版证据，提取共享算术前须与现有 scroll_state 比较，不能通过改舍入或 hit 顺序实现表面去重。

已核对 controls 的 trackbar/toolbar notify、guard/state/native_presentation 的 progress、choice_view 的 other painter、scroll_track/paint 及 special_install 的资源调用。测试入口为公共 custom_draw、vcl toolbar/trackbar 模块、control_extensions、progress v5/v6、scroll_layout 及原始资源核验。仅资源身份在本批实际运行且通过；其他测试是已定位待实施验证入口，不宣称运行通过。

## S1 连续审计批次：滚动消息、原生适配与跟踪（70–77）

| 文件 | application 公共机制 | appearance 与状态归属 |
| --- | --- | --- |
| scroll_message.c | 滚动消息与原生转发、禁用期间退场、打印 DC 保留、全宽 trackpos 回读、操作后重绘协调 | no-hover、sizebox 与 legacy progress 边框由风格策略提供 |
| scroll_message.h | 公共滚动服务契约与实例状态 | 不能通过 control_style 绑定 Win16 控件，风格仅提供动作策略和画法 |
| scroll_native.c | control/embedded 输入适配、通知派发、计时器与捕获生命周期、全宽 reporting、caret 资源及 managed API 对接 | Win3.x drag limit、固定 thumb 和 caret 外观几何独立；palette 更新由风格实例负责 |
| scroll_native.h | 统一公共滚动动作/实例接口，避免多组分散字段指针形成隐式所有权 | track、timer、epoch、capture、report 归同一实例；绘制及几何通过操作契约 |
| scroll_paint.c | 原生 SCROLLBARINFO 快照、窗口坐标适配与一次缓冲提交 | 棋盘、箭头、固定 thumb、颜色及高光阴影由 Win3.x painter 提供 |
| scroll_paint.h | 通用滚动视图和布局结果 | 脱离 Win16 track/asset 类型，不把 frame/list/edit 具体画法混入共享像素接口 |
| scroll_track.c | 公共动作容器和可复用位置换算；状态机按行为差异提供策略 | Win3.x 初始 200ms/后续 50ms、容差外吸回、取消报告最后位置等行为保留该风格 |
| scroll_track.h | 公共 notice/action、timer 指令与实例生命周期，风格 track 私有状态独立 | 不将 KEEP_TIMER 与固定两条 notice 的容器误当年代行为，按现有需求确定共享契约 |

公共执行层拥有 timer、捕获、键盘操作配对、full-width 临时报送值和附着 epoch，通知前保活并在通知返回后重新验证。样式状态机产生动作，执行层按目标协议发给独立控件父窗口、frame、edit 或 list；不能由每个风格再复制多列列表索引换算和 16-bit 消息边界。已存在的 common/scroll_state 是唯一基础，先复用再扩展，不新建平行版本。

结束手势先提交 inactive 状态，再结束计时/归还本层捕获，最后派发通知，避免重入看见仍活动的旧手势。只能释放自己拥有的捕获和 caret，不关闭其他控件状态；原始 reporting 值在嵌套通知之后恢复，不把物理输入、窗口恢复或 hook 卸载混为一谈。

源码复核发现：scroll_apply_actions 在设置 reporting/report 后，多列列表的 LB_GETITEMHEIGHT 重入或无效几何分支会直接 return，跳过下方恢复旧 reporting/report 的代码。这是明确的清理路径缺口；在公共动作执行层用统一退出清理恢复，并以通知中重入、失败及存活目标验证，不通过增加多层补救处理。尚未修复或运行复现测试，不宣称已证实用户可见故障。

Win3.x 几何及取消时的视觉和通知位置有不同契约；Win2000 比例 thumb 和自身行为必须通过策略表达，不能为了合并而改变现有原版证据。绘制中用原生快照和当前 track 几何合成，公共 present_buffer 继续承担资源与提交；buffer 失败时保留现有明确回退行为。

实际调用为 controls 的 scroll_action/input/caret 与 managed API、control_dispatch/control_state 的消息处理、scroll_track 的公共位置换算及 scroll_paint 的呈现缓冲。验证入口包括 core-style-scroll-state、core-geometry、core-scroll-native、vcl-scroll-module 和 frame target；本批核对源码/入口，没有运行整包测试或新建 scratch，所有行为结论待迁移时对照基线验证。

## S1 连续审计批次：特殊控件与状态栏（78–89）

| 文件 | application 公共机制 | appearance 风格及所有者 |
| --- | --- | --- |
| special_install.c | lifecycle/control 的按 owner 安装事务、父窗口通知子类、主题租约与失败退场 | supported、箭头资源和 frame painter 由风格提供；公共实例持有子类/租约 |
| special_install.h | 控件能力注册和安装服务契约 | 不把 Win16 palette、control_style 与所有具体控件回调打包为公共对象 |
| special_paint.c | graphics 的位图快照、背景差分与保留原图像合成、裁剪框/线/文字绘制原语；control 的绘制来源匹配 | 黑框颜色、Win3.x 箭头/按钮/sizebox、VB Data 目标布局和月历支持由风格/运行时策略提供 |
| special_paint.h | 分成图形操作、运行时识别和具体呈现接口 | 删除 other/special 大杂烩式接口；按实际调用拆，不每个小函数另建组件 |
| spin_scope.c | control 的 DC 克隆来源、选中位图身份、变更失效、scope 和 composite 事务 | VCL part/state 和支持几何属运行时适配策略，面板画法属风格 |
| spin_scope.h | 公共 composite scope 拥有缓冲和有效位图，绘制期栈与失效生命周期 | 16 个克隆与 64×64 观察边界保持，不因通用化扩大目标或无界分配 |
| status_font.c | graphics 字体签名（LOGFONT/metrics/widths）、嵌套 borrow/return、失效延迟释放 | 具体替代字体及 acquire/retain/release 由字体资源提供者决定 |
| status_font.h | 缓存签名与借用状态通用化；owner token 有明确配对契约 | Win16 字体映射不作为公共缓存的固有规则 |
| status_frame.c | control 状态栏 pane/图标快照、代次、原生调用来源与绘制阶段核验 | Win3.x 黑框和 sizegrip 最后 pane 的呈现策略由风格提供 |
| status_frame.h | 公共 pane/绘制作用域描述 | 不把黑框绘制入口当所有风格的统一身份规则 |
| status_text.c | control 主题文本与原生文本调用关联、内容/作用域匹配、借用字体 token | 当前 ASCII、长度与 flags 限制属于既有 Win3.x 文本支持策略 |
| status_text.h | 公共文字观测 scope 与风格字体请求解耦 | 513 字符缓冲和简单 pane 模式的现有契约保留，按实例/调用作用域归属 |

special_install 的父通知子类与 theme lease 应加入公共实例的安装事务，失败只回滚该 owner；安装中原生通知可移除或替换实例，不能假设安装返回后状态仍存在。共享 theme_state/control_protocol 继续复用，避免为每个扩展控件复制租约和销毁流程。

spin composite 捕获的是可验证来源的背景，再保留应用后绘制的箭头/图像，风格只能替换已识别区域。DC 删除、位图更换、未知主题调用必须使旧 snapshot 失效。不能把某种风格的 face painter 藏在通用合成算法内部；graphics 提供合成，control 提供有效性，appearance 提供面板绘制。先保留现有固定观察边界，不把失败处理扩展成任意 API 参数兜底。

status 字体缓存不能只按 HFONT 值判断身份，现有 LOGFONT/metrics/字符宽度签名用于句柄复用检测，应保持。借用期间缓存失效时延迟释放，最后 return 清理；资源提供者保证原生字体只借用，不误删除应用/stock 字体。文本观测和字体支持资格分开，公共层不统一拒绝非 ASCII；当前 Win3.x 不支持的文字仍走原生。

已查实际 controls 的 special install、attachment 的 spin/theme/status 中转、injection 的 DC/图形/文本调用，以及公共 bitmap/clip/protocol 与现有 status identity。现有验证入口包括 spin_controls/vcl_modern_spin、status_controls、font isolation/stock/alias、status pane/simple 字体夹具及自定义绘制负例。本批只检查入口和源码，没有新运行结果；实际重入、异常、字体句柄复用和像素保留仍需在对应 S 实施时验证。

## 未通过与未来候选

导航焦点、Win2000 DWM 恢复、VB 清单和表头不稳定及实际现代宿主/全局覆盖限制继续由 CURRENT 和已有证据承载；不因架构设计被关闭。T26 结束条件是现有能力的三层归位、实际复用和验证，而非所有年代所有应用都支持。

后续 T 候选依次为 Win2000 控件与对话框完善、现代应用兼容性、Luna 与主题资源导入、Aero 与合成能力；不预分配 T 编号，不在本 T 自动扩展这些产品范围。
