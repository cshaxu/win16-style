# M0 T20 S1 — 初步接口与经典参考调查（进行中）

用户准入文件夹选择和公共对话框任务；本记录是早期证据，不是适配完成或最终接口矩阵。

本机 `D:/programs/misc/DoubleKiller.exe` SHA-256 `cdd83ab909db749992d11ace2dff671493c0568d7a14697f8bef4b06d8647ca8` 静态导入 `SHBrowseForFolderA`、`SHGetPathFromIDListA`、`GetOpenFileNameA`、`GetSaveFileNameA` 和 `CoCreateInstance`。`D:/programs/misc/md5summer.exe` SHA-256 `4646ce4f638e7d7ec820e7540b39bb185234e1f151d780c734ca557d6dba7ed9` 静态导入 `GetOpenFileNameA`、`GetSaveFileNameA` 和 `CoCreateInstance`。静态导入不证明截图的实际入口或任一功能调用；截图所属应用和真实参数尚未确认。

纯C自有探针 `SHBrowseForFolderW` 使用自有 `build/t20-s1-dialog/selected` 作为根与选择，分别对 x86/x64、带/不带 run16s、旧式 `BIF_RETURNONLYFSDIRS=0x1` 与新式 `BIF_RETURNONLYFSDIRS|BIF_USENEWUI=0x51` 共八次观察。八次均返回目标PIDL、`SHGetPathFromIDListW` 解析结果等于输入路径，初始化回调一次；旧式选择变化一次，新式三次。资源按API使用 `CoTaskMemFree` 释放。为自有测试发送选择与确认，并未覆盖真实用户输入、取消、虚拟节点、网络、重入或完整回调。探针进程没有超时，正常返回；其余测试不推定。

控件枚举显示旧式 `#32770` 包含 `SysTreeView32`，新式含 `SHBrowseForFolder ShellNameSpace Control`、嵌套 `SysTreeView32`、编辑框、新建文件夹及确认/取消。后续可见窗口 `PrintWindow` 的 x86 样式截图显示两者均仍用现代目录图标/树，旧式并非 Win3.1 的文件打开布局；新式的输入与新建功能不等于旧式。截图仅登记 scratch `build/t20-s1-dialog/old-styled.bmp` 和 `new-styled.bmp`。

用户自有 `O:/w31/SYSTEM/COMMDLG.DLL` 是 Win16 NE，SHA-256 `853574d46a8e0103028354883cc597e762bafd034b60c15aedc8cdc097629ec8`。只读资源表发现8个对话框、11个位图、6个图标；资源1536/1537的 Open 模板包含 File Name、Directories、List Files of Type、Drives、Cancel 等字段，原始资源记录的对齐长度分别352字节，SHA-256 `1580e866d7bb25dc74e59aa78dd2ef25342b7c4adca3472ef24832746acb79cf` / `1faeb6fb78598b5eb594ae00916a40f72aa25c63507eb791f5024ab399a97458`。对齐填充包含在摘要内；没有导入或分发原始资源，许可及目录图形身份尚未审查。最初一次资源表读取因偏移算错产生越界结果，已丢弃并在修正后核对资源全在文件内。

微软接口文档说明 SHBrowseForFolder 返回 PIDL，应用需释放，且新旧风格能力不同；历史 Windows 3.1 old-style GetOpenFileName/GetSaveFileName 和 Shell browse 是不同接口。不得为了外观擅自清除标志或替换语义。参考：https://learn.microsoft.com/en-us/windows/win32/api/shlobj_core/nf-shlobj_core-shbrowseforfolderw 和 https://devblogs.microsoft.com/oldnewthing/20180917-00/?p=99745/。

后续仍需真实应用动态调用参数及回调、真实目录/盘符资源和许可、经典参考截图/行为、路径/PIDL/错误/取消/网络/Unicode等回退矩阵，并先验证可保持语义的设计。当前未修改或发布产品。

## 真实应用入口和旧文件框桥接原型（未交付）

自有 `dk-owned.exe` 副本的进程内 IAT 探针实际观察到用户截图的 Options 文件夹加号调用 `SHBrowseForFolderA`，flags `0x71`、根 PIDL 为 NULL，应用传入回调。该回调使用实际 Shell 窗口和控件，发送 `BFFM_ENABLEOK` 并接触 Shell 的目录树及编辑控件。因此直接把旧文件框 HWND 作为 Shell 窗口传给应用并不兼容。原目标的 import descriptor 缺少 OriginalFirstThunk；仅按导入名称扫描不能找到此调用。

自有探针把系统旧式 `GetOpenFileNameA` 用作文件夹选择界面，同时保留实际 Shell 对话框及其回调、PIDL 生成；其中一次选择原有目录 `C:\Users\neko`，原程序返回非空 PIDL，解析路径为该目录，选项列表由 0 项变为 1 项。探针亦观察过取消。旧文件框截图和临时探针在登记 scratch `build/t20-s1-dialog`，均不是产品代码或完整的验收测试。

该原型不是可安全发布的实现：选择不同目录需要等待 Shell 异步处理 `BFFM_SETSELECTIONA` 及确认回调；原型还未证明跨目录选择、取消及失败的完整语义，也未覆盖文件新建、Unicode、网络、虚拟目录、重入或 `IFileDialog`。Shell 界面隐藏/显示的闪现、窗口生命周期及其他应用的回调兼容性也需解决。2026-10-29 尝试重新用自有受作业对象清理的实例验证不同目录时，注入安装成功，但操作没有打开目标对话框（`DIALOG_TIMEOUT`），因此该次没有产生目录选择结果；不可记作通过。

后续需先用可靠的自有原生驱动完成跨目录、失败与取消测试，再定义可替换条件及回退；不允许伪造 PIDL 或以测试观察过一个应用推定所有公共对话框均兼容。本 S 仍活动，当前没有产品修改或二进制发布。

## 主工作树桥接开发（尚未交付）

用户进一步要求直接执行并覆盖相关现代接口。产品代码试作普通单文件的 GetOpenFileNameA/W、GetSaveFileNameA/W 及一类 SHBrowseForFolderA/W。旧式文件框必须设置旧式回调，而不仅仅清除 OFN_EXPLORER；微软文档说明默认界面仍为 Explorer：https://learn.microsoft.com/en-us/windows/win32/dlgbox/open-and-save-as-dialog-boxes 。对带应用 hook、模板、多选等未映射契约的文件调用保持原接口。浏览文件夹时保留实际 Shell 对话框、应用回调及 Shell 所有的真实 PIDL；新建文件夹入口返回实际 Shell 界面。仅线程局部捕获本次回调发向自身窗口的初始选择请求，不使用全局 hook。

早期双宽 ANSI/Unicode 原生探针得到八个打开/保存选择成功，四个跨目录 PIDL 成功、四个取消成功；一次真实 DoubleKiller 自有实例显示旧式界面。随后增加的应用初始化目录测试最初失败：Shell 异步选择导致旧界面显示旧目录；捕获初始化回调中的选择请求、处理尾部分隔符后，x86 ANSI当前目录测试通过。以上早期结果不是最终候选包验收；最近更改后仍须重新进行实际包矩阵及真实应用验证。新建文件夹在一次 x86 ANSI 探针中回到 Shell 并取消，尚未测试实际新建。早期没有应用 hook 的对照因判别规则错误而失败；已查明 Explorer 的同 ID 控件类为 SHELLDLL_DefView，旧式对应 ListBox，测试已修正。

本阶段未支持 IFileDialog、定制文件模板与 hook、多选，文件夹桥接仅限根为空且标志恰为 0x71 的调用；其他调用原样交给系统。长路径、网络、重入、DPI和资源释放仍需验证。T20保持开放，尚未发布。

## 用户补充的全对话框双维标准（开发中）

用户明确：Windows API 提供旧式布局时桥接，所有对话框接管样式；两个维度分开报告。`O:/windows/notepad.exe` 是 x86 PE，SHA-256 `cf2d882d6ec84bb4f9e6475d3b3674e4a480153ff3d34caab617f1a799a45ad0`，静态导入 `FindTextA`。启动自有实例并向其发送 F3 后，候选 ntw16s32.dll 实际加载，原生截图 `build/t20-s1-dialog/notepad-find-product.bmp` 显示查找对话框采用 Win16 标题及控件。原生查找/替换模型覆盖双宽 A/W、窗口与编辑框附着、查找、替换、全部替换、关闭消息。一次因测试用 BM_CLICK 的焦点依赖造成找不到通知，改用发向该自有对话框的 WM_COMMAND 后八组通过；不虚构物理点击验收。

已增加传统 ChooseFont、ChooseColor、PrintDlg、PageSetupDlg 的创建作用域，双宽 A/W 探针验证前两者确定/取消与调用者 hook，后两者验证取消而不打印。PropertySheet A/W 与 v6 TaskDialogIndirect 使用自有窗口测试，双宽已验证窗口显示后的附着和关闭。PrintDlgEx UI 探针在本机多次等待超时、作业按时回收；未证明此对话框已接管，先保留失败，不计为通过。待实际包重测，仍未发布。

文件夹初始化回调的选择变动经原生探针发现延迟，已在同一UI线程临时观察发送给该自有 Shell 窗口的初始化选择消息，保留应用回调和真实 PIDL；该观察不跨线程或跨进程。新建文件夹转回原生 Shell，无法完成映射时应回退。候选截图 `build/t20-s1-dialog/doublekiller-product.bmp` 真实显示旧式文件框。IFileDialog/IFileOpenDialog/IFileSaveDialog 及自定义、多选的旧布局桥接、PrintDlgEx 的实际显示、系统凭据等其他对话框仍未完成；不能把当前有限入口称为“所有对话框”。

2026-10-29 同批候选 b 干净双宽构建及资源检查通过，但实际包测试 `dialog-folder-x86-same` 失败：真实 Shell 选择已变化，但隐藏 Shell 的 `BM_CLICK` 未可靠确认，最终返回取消；该包未发布。改为向同一自有 Shell 窗口发送带真实按钮 HWND 的 `WM_COMMAND/BN_CLICKED` 后，开发包的 x86/x64 A/W 当前目录四组和双宽新建入口回退两组通过；仍须重新构建并测试最终包。PrintDlgEx 的带 owner 显示测试，注入版和不注入对照都在25秒作业时限内未完成，不能把超时归因于产品或计作通过。

独立 C ABI 只读对象探针：本机 x86/x64 `IFileOpenDialog` 与 `IFileDialog` 的接口指针不同，两者和 `IFileDialogCustomize` 查询到的 IUnknown 是相同身份；默认选项 `0x1808`。这是下一步 COM 桥接的生命周期与 QueryInterface 设计约束，不是桥接成功的证据。

候选 c 干净构建通过，但包测试 `dialog-folder-x86-same` 在四路并发时再次返回取消；单独重跑该候选通过，不能仅根据上次判断断言已解决。并发独立探针又发现临时目录采用“创建临时文件、删除、再创建同名目录”的竞争窗口，一组中一例退出码4；修正为基于进程ID和时间的独占目录。观察器原10秒限时也可能在繁忙时提前发WM_QUIT，调整为20秒、外层作业35秒；仅为等待上限，正常完成立即退出。之后对同一候选四路并发当前目录选择全部通过。原包测试失败保留，因测试源码变化须重新干净构建和完整实际包复测，不能拿修正前的失败包充数。

用户明确三条演示路径：旧式 Windows API、经 run16s 默认桥接的现代 Windows COM API、主动调用 ntw16s 公开 API 保留现代对话框。第三条不是 demo 专属：任何程序均可显式使用此 API，普通应用默认仍应自动桥接。开发中的双宽 DLL 增加 ordinal 33 `Ntw16sShowModernFileDialog`；demo 在 DLL 已加载时动态查找该入口，不加载额外产物。当前仅编译及 PE 检查通过，COM 默认桥接、运行和包验证尚未完成，不代表已支持或发布。

公开新式 API 的自有进程原生探针覆盖 x86/x64 的打开、保存和文件夹共六组：观察到真实现代 `DirectUIHWND`、在目标窗口所属 UI 线程确认标题附着、关闭后 HRESULT 为 `0x800704c7` 且输出为空；三个非法参数也检查。此前从观察线程查询附着得到 0 是探针线程错误，已修正；一次重复重置定时器造成超时亦已修正。此为开发目录验证，不证明现代内容所有控件已换肤、选择结果正确或默认 COM 桥接完成。六组已登记到包测试，尚未运行最终同批包。

COM 桥接开发：在本次注入进程内拦截 CoCreateInstance 并包装普通文件选择接口；对基本单文件打开和保存转到旧式系统窗口，再以 Shell 项返回选择。x86/x64 的打开/保存各验证选中和取消共八组，选中路径一致，取消 HRESULT `0x800704c7`。保存不存在的目标曾真实失败 `0x80070002`，增加系统 `IFileSystemBindData` 绑定信息后通过，测试未创建目标文件。公开新式 API 六组对照依然显示现代 DirectUI。此为开发构建测试，非同批包验收。FOS_PICKFOLDERS、多选、筛选器、事件及定制接口尚未桥接；代理向不支持接口返回原生对象时 COM identity 一致性尚未验证，不可发布或称为普遍兼容。

用户要求实际滚动旧式对话框右侧目录列表。自有进程在开发候选包的系统旧式模板内创建 30 个目录和文件，右侧列表有 41 项；原生 WM_VSCROLL 向下后 `LB_GETTOPINDEX 0→1`，再发送滚动条下箭头的非客户区按下/抬起后 `1→2`。`right-scroll-before.bmp` 与 `right-scroll-after.bmp` 为实际窗口截图，滚动后仍显示方形 Win16 箭头和灰色滑道；列表由 comdlg32 自绘，通用 Ntw16sWindowAttached 返回 0 不代表私有滚动条适配未安装。候选 DLL 与 O:/winnt 已安装 DLL 的 SHA 不同，此结果未证明已发布版本表现。

用户进一步要求系统审计所有水平和垂直滚动条滚动后变样。审计发现 `controls.c` 的 `frame_scrollbars` 分支仅在非客户区绘制消息时重绘，原生处理滚动消息可能自行更新滚动条；已增加共享路径的滚动、按键及位置消息后重绘候选，尚未证明有效。右侧目录列表测试中 `LB_GETTOPINDEX 0→1→2`，但屏幕 DC 读取的箭头区域滚动后 289 个像素全为零，与 PrintWindow 截图不一致；不能把该测试算作通过，也不能凭 PrintWindow 重绘后的截图声称实际屏幕未变。需改用可靠的实时屏幕捕获并覆盖水平/垂直、滚轮、键盘、拖动、程序设置、控件销毁及反例。

COM 文件夹桥接现复用系统旧式文件模板，选中真实目录后创建 Shell 项；x86/x64 选择和取消四组开发测试通过。筛选器及默认扩展名在 x86/x64 打开/保存接受/取消八组观察到正确筛选器，选中路径匹配。打开接口可查询相同的 IUnknown 身份，单选 GetResults 返回系统 Shell 数组；x86 普通文件和 x86/x64 文件夹各有一次真实结果证据。复杂接口身份、回调、多选、实际包仍未覆盖。
滚动条捕获复核：DwmFlush 后屏幕像素可稳定，但 WindowFromPoint 的进程不是探针进程，窗口被其他窗口遮挡；因此像素一致不是有效的视觉证据。测试遇遮挡会明确输出 FOLDER_RIGHT_VISUAL_SKIP，不将它当作视觉通过；列表位置及输入行为仍独立检查。

COM 接口一致性进展：`IFileDialogCustomize` 与 `IFileDialog2` 现有代理，分别从自身和 IFileDialog 查询 IUnknown 返回同一身份；无定制操作的接口查询仍桥接旧布局。直接以两种 IID 创建对象，双位宽身份测试通过；经 IFileDialog2 打开旧式文件或选择目录并读取结果在 x64/x86 分别通过。实际添加定制按钮时目前保留系统现代布局，并以创建作用域尝试附着样式；x86/x64 实际取消对照通过，此行为仍未满足用户要求，不能视为完整桥接。其它原生接口、事件及多选仍待处理，尚未发布。

用户再次实际确认滚动期间回退现代样式。共用钩子审计发现，旧式系统目录列表是内部 frame-scrollbar 子类而非 attachment 表项；原 `eligible_scroll` 仅检查后者，导致对该列表的 SetScrollInfo/SetScrollPos 等调用未进入样式路径。候选增加同线程内部子类识别及共用重绘。原生探针改用屏幕 BitBlt、DwmFlush、所属进程检查和 DPI 逻辑到物理坐标转换；比较滚动/点击及 SetScrollInfo 后的真实屏幕像素与同状态强制框架重绘，避免正常的禁用→启用差异。旧代码对照出现 26 个不同像素并失败，候选 x64 和 x86 该目录列表各零差异通过；仅证明该路径，水平、拖动和其他宿主待测。此前未做 DPI 转换的“零差异”结论无效。
COM 多选仍未准入运行：自有探针选择两项后旧式对话框返回空缓冲，实际未通过；多选选项仍回退原生界面。开发中发现未指定筛选器时旧式文件列表为空，候选已使用系统全文件筛选默认值。

滚动条共用路径扩大到已接管的列表与多行编辑框：SetScrollInfo 抑制系统重绘并使用相同 renderer，拖动期间 GetScrollInfo 可取得追踪位置。自有窗口探针在 150% 缩放下比较实际屏幕完整水平/垂直滑道与同状态重绘，覆盖 SetScrollInfo、SetScrollPos、SetScrollRange、按下/释放、滚轮、拖动开始/移动/结束，并确认捕获和位置变化。x86/x64 的独立框架、列表、编辑框各两轴通过；旧实现负例在列表及编辑框两轴的 SetScrollInfo，以及框架位置设置中出现大量像素差异且测试失败。截图归属校验失败时明确 skip，不作为视觉通过。探针以消息驱动，非物理鼠标；其他宿主与系统版本仍待验证。测试加入实际包队列，尚未运行最终包、发布或验收。

COM 多选开发证据：旧探针仅设置列表选中位而未发送 `LBN_SELCHANGE`，因此初始空缓冲不代表真实用户选择；补全通知后发现旧格式目录尾已有反斜杠，拼接两次使 Shell 解析返回 `0x80070057`。修正后，本机 NTFS 具备 8.3 名称的卷上，x86/x64 普通名和带空格长名双文件选中、取消共八组通过；GetResults 返回两个匹配真实长路径的 Shell 项，GetResult 在多选时失败。多选模式只选一个带空格文件的双宽测试返回一个数组项及可用的 GetResult。缺少 8.3 别名、非 NTFS、超过 512 项和多选文件夹仍未验证；同批包未运行。原未启用桥接的负例没有旧式对话框且探针失败，不能计作通过。

COM 事件调查：实际旧式 GetOpenFileName 的 FILEOKSTRING 在 x86/x64 均能提供正确路径，回调拒绝后窗口保持打开，第二次接受成功；直接系统对照同样通过。真实 COM 事件测试发现现行现代回退的回调取得底层对象、IUnknown 身份与应用代理不同（失败证据 identity=0）。现增加事件转发代理并避免所有者强引用循环；双宽真实对话框验证选择路径和代理身份、第一次拒绝后第二次接受、正常 Unadvise、未显式 Unadvise 的销毁释放，以及回调内部自 Unadvise，引用数均恢复。取消订阅后旧式桥接恢复。事件订阅期间仍暂用现代布局，因为目录/选择/覆盖及拒绝语义尚未完整映射，未满足全桥接要求；实际包尚未验证或发布。

COM 标签映射：`SetOkButtonLabel` 与 `SetFileNameLabel` 先交系统对象校验并复制，再由旧式对话框初始化时设置原生控件文字；打开、保存及文件夹三个类型均适用。自有探针检查实际标题控件文字、旧布局及真实选择返回，x86/x64 共六组通过；带空格文件名的多选双宽回归通过。极长标签适配及文件夹“新建”回退到 Shell 时的标签未覆盖，事件订阅和其他定制仍可能回退现代布局；实际包尚未测试或发布。

COM `IOleWindow` 身份调查：真实 OnFileOk 中，系统直接运行的 IFileDialog 对其 IOleWindow 与原对象查询到的 IUnknown 相同；此前代理回退能得到 HWND 但身份不同（x64 失败 `ole=2 ole_identity=0`）。现为 IOleWindow 增加同身份代理，现代回退的双宽确认回调能得到真实存活 HWND 和同身份；自回调 Unadvise 也通过。普通旧式打开/保存/文件夹路径在查询 IOleWindow 后仍保持旧布局及返回值，双宽各三组通过；旧文件框在创建和销毁期间记录自己的 HWND。事件订阅仍回退现代布局，旧式事件时的 HWND 使用及跨线程接口封送尚未测试。

COM 入口扩展：除 CoCreateInstance 外，CoCreateInstanceEx 的同批多个接口被原子替换为同一代理，保留失败项及 `CO_S_NOTALLINTERFACES`；CoGetClassObject/IClassFactory::CreateInstance 的常用接口也接管。通过线程局部创建深度及原生异常清理避免内部嵌套重复包装。自有双宽探针对打开、保存、目录各验证 Ex 和类工厂共十二组，旧布局与实际结果通过；Ex 同时请求 IFileDialog、具体接口、IOleWindow 和不支持 IID，身份一致且部分成功码为 `0x00080012`。无关系统 COM 类及本机类工厂的 IMarshal/IClassFactory2 直接对照一致；显式新式 API 双宽仍保持现代布局。远程服务器、聚合、超64接口与未知成功接口按系统原样处理，其他工厂接口身份和跨线程封送尚未覆盖。事件订阅及部分定制仍回退现代，实际包尚未测试发布。

2026-10-29 用户再次观察到点击和滚动时现代外观。复核发现 O:/winnt 与 assets 的 ntw16s32.dll 仍为旧版（SHA-256 前缀 9e4768faa615e0af），不同于开发目录候选（前缀 87db077d87520187）；因此用户观察不能证明候选已生效，候选仍须实际包验证并发布。隐藏项桥接探索发现旧式目录列表不显示隐藏子目录：自有 COM 探针根目录隐藏文件及切换过滤器成功，但选择隐藏子目录失败（passed=2/3）。因此 FOS_FORCESHOWHIDDEN 暂回退系统对话框，避免遗漏目录；该实验不构成完整桥接。

## P218 滚动条与公共对话框阶段包

同批候选 f 以 MinGW GCC/Ninja/ccache 双位宽干净构建；源清单、原始图形资源逐字节校验和 ABI 检查通过。实际包聚焦 17 项全部通过、零跳过：demo 九页、首次注入、双位宽四传播边、列表/编辑框/窗口框架两轴的设置、按下/释放、滚轮、滑块拖动，以及双宽文件夹对话框目录列表滚动和颜色对话框展开。屏幕像素在窗口归属和 DPI 坐标校验后与相同状态的经典重绘一致；消息驱动不等于物理鼠标验收。旧代码负例曾被这些测试检出。未声明所有控件或所有 DPI 已覆盖。

候选 e 的完整并发测试在颜色对话框展开失败（COLOR_GRID colorful=0），同一包单独重跑成功；原因与窗口并发遮挡相关，但并发失败仍有效。非提权环境的 demo、注入和传播测试因私有桌面错误 5 跳过；桌面可访问环境下候选 f 聚焦重跑全部通过。新增 demo 第九页后旧观察器仍断言八页，修正并双模式核验九页。完整包矩阵尚未通过；COM 事件、定制和隐藏项目等存在原生回退，PrintDlgEx 及其他系统对话框未完成，T20 S1 保持开放。

发布至 assets 根目录及 O:/winnt 的 SHA-256：demo.exe 969cc2d8f24b5fb0d376aeb6a8a4ef7f062d6943445e6a717399189c77d072df；ntw16s32.dll b04eb342b6d456ead100edb297e8c7b92315a1e0c2ebe89e34b914c86eaa5a47；ntw16s64.dll 63ae5404dbf249b44c4b5df3444f6de09b0ad340926931676096d3d6e7da4b74；run16s.exe 7e0a215c37cceecdce8ed88bef92fe796dff946f2c21599aa060b156ee241f55。首次发布因 splwow64.exe PID 32364 占用 DLL 中断，核对映像路径和启动时间后按发布授权关闭该确切进程，重试后两处四文件摘要与测试包一致；首次备份及记录保留于登记 scratch。

## 自定义取消文字的旧布局桥接（开发验证）

IFileDialog2::SetCancelButtonLabel 原先无条件令对话框回退现代布局。候选保留系统对输入的校验，复制成功的最新标签，在旧式打开、保存和文件夹框设置真实 IDCANCEL 文字；分配失败则保留系统对话框。双宽各三类、接受与取消共十二个开发用例验证实际控件文字、旧布局和返回语义；首次 x64 保存接受曾出现返回路径不匹配，随即单独重跑通过，原因尚未定位，该失败不可删除。P218 旧 DLL 对新增 x64 取消标签用例显示 old=0 并失败，证明测试能检出旧行为。候选仍需同批干净包、实际包测试和发布复核；COM 事件、复杂定制与隐藏目录回退保持不变。

P219 同批候选 g：双宽干净构建及原资源身份检查通过；实际包聚焦 20 项通过、零跳过，包括 12 项取消文字及接受/取消、demo 九页和传播矩阵。双宽旧式目录导航探针额外确认目录切换前可拒绝、切换后有通知，但不证明全部 COM 事件语义。已发布至 assets 根目录及 O:/winnt，摘要一致：demo.exe 0b322191ac441e06c9dc65f661c06a27f93655377959afd1ae812cbd14636fab；ntw16s32.dll c2cf93ee95e6722051d4ebc5f546e52997cd261dd70c514661293eee2cf09b39；ntw16s64.dll b6f6e31b4bc0b904f37c2dd1bd5c7977fafdd5268593195fbb8c551b02341171；run16s.exe 4ffa2bb21300b541bded9bd0283cb5a76b60b63dd3c4d6fcbddeff86d4e2c431。完整测试矩阵、此前 x64 保存路径偶发不匹配、长标签与新建文件夹回退时的标签仍未覆盖，T20 S1 不关闭。

工作树治理脚本先前在排除 build 之前递归遍历缓存，工作树自检长期无结果；优化为遍历时跳过既定排除目录及重解析目录，工作树治理自检 13 个负例通过。此工具不参与产品二进制编译；候选 g 构建快照内原治理脚本也已通过相同自检。

## COM 事件及对话框探针隔离调查

P219 曾出现 x64 保存路径不符；进一步重现时旧式输入框初始为预期路径，提交前已被改成 `waim`，结果是工作目录下同名文件。此证据表明输入框被修改，但未查明输入来源，不能断言产品或用户输入是唯一原因。独立私有桌面重复十次保存均保持初始输入并返回正确路径。对话框驱动增加可选私有桌面且作业对象负责退出清理；实际包中的非屏幕像素对话框用例默认采用该隔离，滚动条和文件夹像素用例仍使用可见桌面并验证遮挡。私有桌面访问失败会明确跳过，不能计作通过。

事件映射的系统 COM 探针在实际现代对话框中观察到初始 OnFolderChanging -> OnFolderChange，回调分别可读取真实目的 Shell 项及当前目录；OnSelectionChange 可读取真实当前文件项；OnFileOk 拒绝一次后接受及代理身份仍通过。双宽旧式探针已证明列表双击切换前拒绝及之后通知可用，但盘符、键盘、输入路径、选择变化、回调重入、共享/覆盖通知及多个监听者仍需逐一映射，不能据此启用不完整的事件桥接。纯测试与证据修改，不改变产品 DLL；仍须同批实际包核验。

P220 同批候选 h：仅修改原生探针、测试编排和证据，产品源码未改；重新干净双宽构建后的四文件摘要与 P219 不同，因此没有混用，按规范部署整批。实际包聚焦 26 项通过、零跳过，包含双宽 COM 事件及自取消订阅、保存和文件夹标签、颜色对话框、现代对话框入口、列表滚动、首次注入和传播。事件回调用例在各自私有桌面并行，视觉用例在可见桌面执行；并行失败历史保留。发布 SHA-256：demo.exe 5693ca1620d63a6914bed73b8354163a13761c85545941f136a75a3737cf2db0；ntw16s32.dll 1bb87e70eaf500e51b683343a0641419cc9d178543b5f0ec63d58a2c09e5b737；ntw16s64.dll 12d62f2142dc85d5f10b578f113e3aa68be1082e1cea126713f8381c10313b1e；run16s.exe b31068eb2c561ebcc9d44469f11306fce45fffd6334c878babc5c9849da844ea。两处部署摘要匹配实际包。完整矩阵、真实物理输入及事件桥接仍未完成，T20 S1 保持开放。

## 系统 COM 导航与否决的隔离对照

新增自有私有桌面探针，以真实 IShellItem 发起异步 SetFolder：初始根目录通知一次，第一次目标目录在 OnFolderChanging 被拒绝后 GetFolder 仍为根；第二次允许后才收到目标目录通知，取消返回 ERROR_CANCELLED，监听器引用恢复，观察线程正常退出。该探针分别对系统对象和 P220 注入后的现代回退运行，双宽需实际包确认；它是事件桥接基线，不是旧布局事件桥接已实现。

调查早期曾观察 Close(E_ABORT) 返回 S_OK，发现该观察无效：监测线程超时发 WM_QUIT 导致 Show 退出；修正为检查监测线程退出码后，独立 Close 实验在作业时限内未结束，不能得出其返回值或认定 Windows 与文档不一致。当前导航基线改为正常 WM_CLOSE 取消，不依赖超时退出。Close 的原因和正确测试仍未解决。微软文档对 Close 与 OnFolderChanging 的语义见 https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/nf-shobjidl_core-ifiledialog-close 和 https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/nf-shobjidl_core-ifiledialogevents-onfolderchanging 。

P221 双宽干净包的 14 项聚焦回归通过、无跳过；四个导航用例覆盖 x86/x64 系统直接调用和注入后现代回退，另含双宽 COM 事件、demo 和注入传播。产品源码未变；重建文件摘要不同，已按规范整批发布并核对：demo.exe 58ba412bbe7186c5d02f8667517084497958f67a3b9aa53c00b7df1b9e844217；ntw16s32.dll 12d0e329ddca027ec29036a95ad1565e7f5e3a3b56ef548cdb743d28aabade58；ntw16s64.dll ea5b64aa1d8ebe2e4b3ad601bb335b3bdfadb8a590a82c509112db5ea99aef7f；run16s.exe 5a755765949e349928bd2ab6a5be2d50645ce53d2f142b8358910159f2d1fdbb。完整事件映射、Close 真正行为和完整回归未完成，T20 S1 保持开放。
