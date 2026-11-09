# M0 T24 S1: XAML 控件接入基线

本记录是研究基线，不是产品实现或测试通过声明。T24 S1 首先验证真实 UWP Button；WinUI 3 的运行时及程序需分别验证，不能用 UWP 结果代替。

## 已有代码和本机证据

- `src/run16s-exe/uwp.c` 激活应用；`src/ntw16s-dll/uwp/target.c` 与 `hook.c` 负责目标窗口线程及外框。`src/lib-win32/win16-window-uwp/frame.c` 实现外框；这些路径目前没有 XAML 控件树接入。
- 在 `src` 和 `tests` 搜索 `XamlDiagnostics`、`VisualTree`、`IVisualTreeService`、`IXaml` 等没有发现现有诊断接入。现有 `ntw16s64.def` 没有 `DllGetClassObject` 导出；这不能证明不存在其他可用接入方式。
- 本机 Windows SDK 10.0.22621.0 含 `um/xamlOM.h`，其中有 C 风格 COM vtable 定义。当前 x86 和 x64 MinGW GCC 分别用 `-std=c17 -Wall -Wextra -Werror -fsyntax-only` 成功编译包含 `windows.h`、`xamlom.h` 并调用 `IVisualTreeService_AdviseVisualTreeChange` 的内存中 C 片段。该检查使用工具链默认头文件，不等于 Windows SDK 头兼容验证，也不证明链接、实际回调或运行。
- 本次普通权限 `Get-AppxPackage` 对 Calculator 和 Notepad 均返回 Access is denied；本次没有启动或操作这些应用，不能把权限错误推断为控件接入不可行。
- 自有纯 C 探针在当前普通进程中分别以 x86/x64 加载系统 `Windows.UI.Xaml.dll` 并查询 `InitializeXamlDiagnosticsEx`，两者均存在、进程退出码均为 0。仅证明入口可解析，未调用初始化、连接目标或操作控件。
- 自有纯 C 控制台进程双宽 WinRT 探针：`RoInitialize` 与 `RoGetActivationFactory(Windows.UI.Xaml.Hosting.WindowsXamlManager)` 均返回 S_OK，但 `InitializeForCurrentThread` 双宽均返回 `0x8000FFFF`（E_UNEXPECTED）。探针退出 0 仅表示完成记录，不是初始化通过；原因未确定，不能推断真实 UWP/WinUI 目标不能接入。第一次编译缺少 `winstring.h` 声明，加入该头后双宽编译链接成功。
- 按 [Microsoft XAML Islands 文档](https://learn.microsoft.com/en-us/windows/uwp/xaml-islands/host-standard-control-with-xaml-islands-cpp)在自有探针嵌入包含 `maxversiontested` 10.0.18362.0 的应用兼容性清单后，双宽工厂获取及线程初始化均返回 S_OK；两个可执行文件均含该清单标记。链接器输出 `.rsrc merge failure: multiple non-default manifests`，但双宽可执行文件均生成、运行并返回以上结果，警告的具体资源合并影响尚未查明。首次尝试使用带架构前缀的 windres 命令失败，随后使用各编译器目录下的 windres。该对照支持清单条件影响此自有探针，尚未测试实际控件或诊断会话。
- 同样清单下，自有双宽 C 进程成功初始化管理器并用 `RoActivateInstance` 创建按钮，两种位宽 `GetRuntimeClassName` 均报告 `Windows.UI.Xaml.Controls.Button`，退出码为 0；链接器同样报告清单合并警告。按钮没有附着到可见 XAML 树，没有验证绘制、诊断会话、用户输入或样式恢复，也不是 WinUI 3 实例。
- 新增自有纯 C 原生目标 `tests/integration/xaml_island_target.c`，双宽手动构建并运行：`DesktopWindowXamlSource` 附着本进程隐藏 HWND，Button 设为内容后可读取到非空内容，两种位宽均输出 `hr=00000000 child=1 content=1`、退出 0。链接器仍输出清单合并警告；窗口未显示，不证明真实可见绘制、对象身份、诊断接入、输入或恢复。此目标尚未接入自动构建，且只验证系统 UWP XAML，不能当作 WinUI 3。
- 扩展目标在自有私有桌面显示宿主、设置子窗口尺寸、有限运行消息循环，并用 `IUnknown` 身份对照实际内容与按钮；沙箱内双宽均因 `CreateDesktopW` 错误 5 返回 77，跳过后续步骤。沙箱外以同一双宽目标复测均输出 `hr=00000000 child=1 same=1`、退出 0；不证明真实屏幕像素、诊断接入、输入或恢复。退出时请求解除内容、销毁窗口并关闭桌面，尚无独立泄漏验证。
- 同一自有按钮实验用系统 XamlReader 加载最小 `Style/ControlTemplate`，对按钮调用 `get_Style`、`put_Style`、读取非空样式并调用 `put_Style(previous)`；x86/x64 调用均返回成功且进程退出0。样式在显示前即被恢复，没有检查应用前后视觉、原样式来源/绑定或恢复后的属性，因此不能称换肤或完整恢复通过；模板仅为实验，不是产品 Win16 样式。链接器的清单合并警告仍存在。
- 双宽私有桌面补充读取 `Style` 的 COM 身份对照：设定后与实验 Style 身份相同，恢复后与此前读取值的身份相同（包括空值），均返回0。仍未检验依赖属性的本地值、绑定或资源来源，故读取值身份恢复不等于完整语义恢复；也没有显示实验样式。
- 调整自有私有桌面探针：显示宿主后设置实验 Style，`IControl::ApplyTemplate` 双宽返回成功且结果为 true，保持样式时运行有限消息循环，再恢复此前 Style 并验证读取值身份。双宽进程退出0；未做屏幕像素、模板子树结构或用户输入核验，不能证明完整视觉和恢复。链接器清单合并警告仍存在。
- 双宽补充 `VisualTreeHelper` 检查：实验模板生效期间，Button 有一个直接视觉子节点，运行时类型为 `Windows.UI.Xaml.Controls.Border`；两种位宽均退出0。该检查验证模板子树中的边框，不证明其像素、完整子树、交互或普通应用接入。
- 自有进程内新增测试专用 C TAP：双宽 `InitializeXamlDiagnosticsEx` 返回 S_OK，`IObjectWithSite::SetSite` 各收到一次，视觉树回调收到 6–7 次，但在有限3秒内没有看到 Button；仅观察到 PopupRoot、FullWindowMediaRoot、RootScrollViewer、ScrollContentPresenter、Border、Grid，x64 另有 ContentPresenter。双宽探针按未找到返回2；这不是 Button 接入通过。事件与目标窗口的关联、控件树未完整出现的原因尚未确定。初次脚本 PowerShell 参数解析失败和 C vtable const 编译失败均已修正；链接器清单合并警告仍存在。TAP 仅是测试文件，尚未接入产品或自动构建。
- 监听建立后在自有目标中移除并重新挂载同一 Button，双宽 `put_Content` 均成功，但3秒内诊断仍未报告该按钮。独立 `VisualTreeHelper.GetParent` 在重挂前双宽返回一个 `Windows.UI.Xaml.Controls.Border` 父节点，说明 Button 已有视觉父节点，但未证明该节点属于诊断报告的同一根。另以新的自有进程尝试 `VisualDiagConnection2`，双宽初始化均返回 `0x80070490`（未找到），不能说明其他端点均不存在。两个测试均未通过 Button 诊断识别。
- 使用已知自有 Button 的 `IInspectable`，测试 TAP 经 `IXamlDiagnostics::GetHandleFromIInspectable` 取句柄、再经 `GetIInspectableFromHandle` 反查，并以 `IUnknown` 身份对照；双宽该往返均返回 S_OK。但同次监听仍没有报告 Button，进程按未发现返回2。该实验需要测试对象主动提供指针，不证明普通应用能通过树发现按钮，也未读取或修改其属性。
- 双宽自有 Button 的 `GetPropertyValuesChain` 均成功识别一个有效 `Style` 来源，初始 `BaseValueSourceDefault=1`。调用先前实验的模板设置与 `put_Style(previous)` 后，同一有效来源变为 `BaseValueSourceLocal=4`，虽读取值身份相同，属性来源已改变。该负例证实不能以重设读取值冒充恢复；绑定和资源等其他来源尚未验证。诊断监听仍未发现 Button，探针整体返回2。返回的 BSTR 与 COM 数组由测试代码释放，尚未进行独立泄漏验证。
- 仅在测试按钮原来源确认 Default 且实验后来源确认为 Local 的条件下，使用诊断接口清除该属性的本地覆盖：x86/x64 均返回成功，随后有效来源恢复为 Default=1。属性索引由当前属性链获取（本次为620，不作为常量）。此结果仅覆盖原默认值，不可推广为对绑定、资源或应用原有本地值的无条件清除；诊断监听未发现 Button，完整探针仍返回2。
- 视觉父链对照中，诊断回调可报告模板子节点，而其父句柄反查的运行时类是 Button。测试 TAP 以此增加父对象识别：本次 x64 在已知指针操作前通过子节点事件识别 Button；x86 初始3秒未识别，随后测试使用已知 Button 指针应用实验模板才收到子节点并识别。两种位宽最终退出0，但 x86 不证明无主动操作的发现；两者也不证明普通应用、无子节点控件或所有生命周期均可发现。该逻辑目前仅属于测试 TAP，不是产品实现。
- 新对照只对诊断事件反查并保留的 Button 对象执行实验 Style；本次 x64 在测试目标重新挂载 Button 后识别并成功执行实验，原 Default 来源经条件清除恢复；x86 三秒内无可用按钮，返回未找到(0x80070490)，整体退出2。该测试目标仍主动重新挂载内容，且自行调用测试 TAP、设置和恢复样式；它不证明普通应用无配合接入或屏幕外观。x64 退出0只是该受限路径，不推广到其他程序。
- 自有 Island 的诊断 `HitTest` 以矩形 `(0,0,320,200)` 对照时，双宽均返回 `E_INVALIDARG`，没有结果；原因尚未确定，不说明此 API 不能发现 Island。x64 本次仍经子节点事件发现 Button，x86 未发现。WinUI 3 的 [IXamlDiagnostics2](https://learn.microsoft.com/en-us/windows/windows-app-sdk/api/win32/xamlom.winui/nn-xamlom-winui-ixamldiagnostics2) 提供按 XamlRoot 选择的接口，文档说明其替代假设单窗口的接口；这提示 WinUI 3 必须单独实现并验证，不能把上述 UWP 测试直接视为 WinUI 3 证据。
- 自有被动 Island 对照：新增测试专用独立控制器，通过系统 `InitializeXamlDiagnosticsEx` 对已经就绪的自有目标建立诊断；目标未向 TAP 提供 Button 指针，也没有调用换肤代码。x86/x64 控制器均返回 S_OK，目标正常退出0，TAP 收到框架树回调，但各自在2.5秒内均未发现 Button；不计自动发现通过。目标仅通过测试就绪和停止事件配合测试生命周期，仍不是实际第三方应用；未进行样式变更。该测试也不证明更长时间或其他应用不可行。
- 沙箱外只读包清单显示本机 Notepad 11.2607.14.0 声明依赖 `Microsoft.WindowsAppRuntime.2`，最低 2.1.3.0；本机已安装 2.6.5.0 的 x86 和 x64 包。只读导出表检查发现这两个位宽的 `Microsoft.Internal.FrameworkUdk.dll` 均导出 `InitializeXamlDiagnosticsEx`，x64 的 `Microsoft.UI.Xaml.dll` 导入 `VisualDiagnosticsPort_Register/Unregister`。最高已安装版本不代表运行时一定选用该版本；本次未启动记事本、未观察其实际加载模块，也未在 WinUI 3 创建诊断会话。首次用不存在的前缀 objdump 命令失败，随后使用工具链 bin 目录的 objdump 完成检查。
- 扩展测试专用控制器，允许传入运行时初始化模块、诊断模块及端点；用 `--probe` 只加载模块不连接目标。在沙箱外自有双宽普通进程均可通过受限 DLL 搜索路径加载已安装 2.6.5.0 对应位宽的 `Microsoft.Internal.FrameworkUdk.dll`，并取得 `InitializeXamlDiagnosticsEx`（退出0）。未调用该入口，不能证明 WinUI 3 会话、版本匹配或现有记事本接入。
- 用本任务新建的唯一空文档启动自有记事本，观察到系统启动器 PID36784 将任务转交给子进程 PID40868；该子进程实际加载 WindowsAppRuntime.2 2.6.5.0 x64 的 `Microsoft.Internal.FrameworkUdk.dll`、`Microsoft.UI.Xaml.dll`、`Microsoft.UI.Xaml.Controls.dll` 与 `Microsoft.UI.Xaml.Internal.dll`，以及记事本的 `NotepadXamlUI.dll`。隐藏启动在等待期内未观测到主窗口，未执行诊断或样式测试。首次采样过早，仅启动器已加载 FrameworkUdk，强制退出启动器后留下子进程 PID38224；随后依据父 PID28868、路径及创建时间确认归属并结束。第二次自有父子进程亦分别核验并结束；未操作其他记事本进程。不能把模块加载当成控件接入。
- 实际 WinUI 只读诊断对照：以自建空文档启动隐藏记事本，核对启动器 PID30280 和其子进程 PID34420 的父子及创建时间，从该子进程实际已加载模块得到 FrameworkUdk 与 Microsoft.UI.Xaml 路径；独立 x64 控制器使用 `WinUIVisualDiagConnection1` 初始化返回 S_OK。测试 TAP 检测 `Microsoft.UI.Xaml.Controls.Button` 本身或其子节点的 Button 父对象后，以唯一命名事件通知控制器；事件被触发。未记录具体控件名称/数量或用户界面像素，未设置样式，也未证明 x86、多个窗口、正常可见交互或撤销。已核验并关闭自有父子进程，测试输出和空文档清理；目标不传递 Button 指针。
- 自有记事本 x64 的首次模板对照：第一个发现的 Button 的 Style 来源为 Local=4，受限探针按条件跳过；首次脚本在发现事件后立即采样，早于终态，补加有限等待才确认跳过。改为继续检查其他已发现按钮后，同一测试中观察到 Local=4 和 Default=1；测试 TAP 在原来源确认为 Default 的一个 Button 上使用从已安装 WinUI 元数据读取 IID 与方法顺序的 `Microsoft.UI.Xaml.Markup.XamlReader` 创建实验模板，通过 `IVisualTreeService::SetProperty` 设置后立即 `ClearProperty`，事件显示 APPLIED 和 RESTORED，后者要求属性链再次报告 Default。控制器返回0，已核验并关闭自有父子进程。实验只运行在测试 TAP 内，不是产品代码；模板并非完整 Win16 设计，未检查视觉像素、点击、绑定/资源、重复或并发撤销，也未验证 x86。跳过 Local 的事件同时存在，不能声称所有按钮已处理。
- 自有记事本 x64 延迟撤销对照：保持该实验 Style 约350ms并经过目标 UI 消息循环，然后通过 `IVisualTreeService2::GetProperty` 和 `IUnknown` 确认当前 Style 仍与实验对象相同，再清除覆盖并检查来源回到 Default；APPLIED、RESTORED 为真，FAILED、CONFLICT 为假，控制器退出0，自有父子进程已核验并关闭。未人工改变样式制造冲突负例，不能声称竞争处理已验证；未观察像素或交互、不是产品实现。
- 自有记事本 x64 冲突负例：初次通过诊断 `SetProperty` 模拟第二次赋值失败；改用本机 WinUI 元数据所列 `IFrameworkElement::put_Style` 后，模拟的新值设置成功，但撤销检查无法从缓存句柄取回被替换的旧 Style。探针在失败路径通过公开 `get_Style` 证实模拟新值仍保留；随后改为持有实验 Style 对象本身进行身份对照，负例中 CHANGED、CONFLICT、PRESERVED 均触发，未触发 RESTORED。无冲突正例首次仅报告 FAILED，原因未记录；增加阶段事件的另一次正例报告 RESTORED。不能把重试通过当作首次失败已解决，也不能推广至多控件、跨线程或任意绑定。所有自有记事本进程已核验并结束；依旧是测试代码，非产品实现。
- 自有记事本 x64 通过本机 WinUI 元数据确认 `VisualTreeHelper` 公共接口后，在实验 Style 生效期间读取测试 Button 的直接视觉子节点，观测到预期 `Microsoft.UI.Xaml.Controls.Border`；同次 APPLIED、TEMPLATE_TREE、RESTORED 均触发，已有本地样式的按钮仍跳过。测试 TAP 双宽均可编译，x86 未在 WinUI 程序中运行。该结果证明一个真实 WinUI Button 生成了实验模板结构，不证明实际屏幕像素、点击或所有状态，也不解决此前正例偶发 FAILED。
- 本次自有记事本 x64 正例与冲突负例各运行一次。旧探针两者的撤销检查均在诊断 `GetProperty` 失败；冲突时另通过公开属性读取确认应用的新值仍在。测试探针改为持有 Button 并通过其 WinUI `get_Style` 读取当前值，清除前重新获取 Button 诊断句柄；新正例触发 APPLIED、TEMPLATE_TREE、RESTORED，负例触发 APPLIED、CHANGED、CONFLICT、PRESERVED，二者控制器均退出0。此前无阶段记录的失败原因无法反证为同一个问题；该两次受控测试不证明其他竞态、x86 或一般应用均安全。自有进程和临时文件清理，逻辑仍仅在测试 TAP。
- 自有记事本 x64 实验模板加入 Button Content/ContentTemplate 的 TemplateBinding 及 Normal、PointerOver、Pressed、Disabled 视觉状态；测试调用 WinUI `VisualStateManager.GoToState`，上述四种状态及返回 Normal 均返回成功，且观察到 Border 子节点、撤销后来源恢复 Default。按钮内容实际显示、视觉状态像素、真实鼠标键盘、绑定语义和辅助功能尚未验证；这是测试模板而不是完整 Win16 风格或产品实现。已有 Local 样式 Button 被跳过，自有进程和临时文件已清理。
- 自有记事本 x64 非空 Button 内容对照：实验模板 ContentPresenter 与 Button 的内容 COM 身份不同；增加 Windows.Foundation.IPropertyValue 字符串值比较后，在另一个自有实例观察到两者文本值相等，状态调用及 Default 来源恢复均成功。两个实例不证明同一按钮或所有内容类型，且未记录具体文本、屏幕像素、动态图标、内容模板或绑定更新；非产品实现。测试进程及临时文件已清理。
- 自有记事本 x64 的实验按钮尝试使用 `IVisualTreeService2::RenderTargetBitmap` 获取内存位图，未保存截图；两次均未成功。带阶段标记的第二次失败位于 RenderTargetBitmap 调用本身，而非后续描述或字节复制；具体 HRESULT 和成因未记录，隐藏启动/控件可见性可能相关但未证实。同次模板节点、状态调用、文本值对照和 Default 来源恢复成功；位图结果不计像素或视觉通过，测试进程与临时文件已清理。
- 将自有 Island 目标、诊断 TAP 与控制器加入 CMake 的 `BUILD_TESTING`，Island 使用专用兼容清单及已有 `explicit_manifest_options`，本次双宽 Ninja 增量构建成功且未再出现临时手工构建的清单合并警告。新增 `core/xaml-island-fixture` 只验证自有私有桌面宿主及 Button 内容身份，不执行诊断或换肤；沙箱外 x64/x86 各1项通过（约0.9/1.1秒），沙箱内 x64 因创建桌面错误5按退出77标为跳过。以上不等于 WinUI 产品接入或视觉通过；保留既有 `build/.ninja` 缓存，没有新增产品包文件。
- 实验 Button 模板从测试 TAP 移至纯 C `src/lib-win32/win16-control-xaml/button_template.c`，测试 TAP 通过静态库复用，未接入产品启动流程。双宽 Ninja 增量构建通过；自有记事本 x64 使用该 TAP 的一次对照报告 Button 发现、实验样式应用、Border 子树、状态调用、文本值比较及默认来源恢复成功，诊断位图读取仍失败。该模板不是已验收的完整 Win16 外观；x86 仅编译、未在真实 WinUI 程序运行。自有进程及本次 scratch 清理。

## T25 承接验证

- 已抽出的实验 Button 应用与条件撤销组件随测试 TAP 在 MinGW x86/x64 增量编译和链接通过；自有 Island 基础夹具沙箱外双宽各 1 项通过，夹具不验证换肤。
- 自建 WinUI 记事本 x64 对照：首次正常场景只发现被跳过的按钮，有限等待内未应用；再次独立运行观测到 APPLIED、TEMPLATE_TREE、STATES、CONTENT_TEXT_MATCH 和 RESTORED，BITMAP_FAILED 仍出现。两次冲突对照均观测到应用、模拟改写、冲突识别和新样式保留。测试进程按父子身份清理；这些是有限的测试 TAP 证据，不代表首次未发现原因已解决、真实鼠标交互或产品接入已完成。

- 新增仅用于测试的跳过原因事件，区分属性查询失败、来源不唯一和非默认来源；双宽增量编译通过。自建记事本 x64 一次对照中，非默认来源按钮被跳过，同时另一个按钮应用并恢复成功；这不证明先前未应用运行是同一原因，也不代表所有非默认来源都无法安全支持。测试进程和 scratch 已清理。

- 产品接入盘点：`src/ntw16s-dll` 当前没有 XAML Diagnostics 的 `DllGetClassObject` 或视觉树监听入口；`InitializeXamlDiagnosticsEx` 控制器及 TAP 类工厂仅在 `tests/integration`。`win16-control-xaml` 当前也只由测试 TAP 链接，因此标准启动不会应用该 Button 模板。后续产品接入必须实现并验证诊断会话、UI 线程上的多控件生命周期及按实际加载运行时定位；不能把测试 DLL 当第五个产品文件。该盘点仅为源码证据，并不证明接入可行或控件可恢复。

- 新增同 UI 线程 Button 集合，按 COM 身份去重，保留移除失败项供调用方决定，安装/移除重入返回 busy；测试 TAP 改用集合。双宽增量构建通过。自建记事本 x64 首次正常对照有限期内未找到可应用按钮；第二次观察到应用、重复接入返回已有项、恢复。两次冲突对照均观察到重复接入去重以及应用改写后的新样式保留。尚未验证多个不同按钮同时存活、所有重入或正式产品；测试进程和 scratch 已清理。

- 多 Button 对照：测试 TAP 在首个按钮之外尝试通过相同集合接入另一个不同按钮，并先移除第二个。双宽 TAP 增量编译通过；自建 WinUI 记事本 x64 首次对照有限等待内未观测到应用事件，第二次报告两个不同条目均应用、第二项移除调用成功、第一项默认来源恢复。第二项未独立读取恢复来源，像素、用户交互、复杂重入及其他程序仍未验证；自有进程和 scratch 已清理。

- 加强测试 TAP：第二按钮移除后重新查询默认来源，另从自有工作线程验证集合拒绝非所属线程的 discard。双宽增量编译通过；本次自建记事本 x64 两次独立对照诊断初始化均返回0，但有限期内都未出现样式应用事件，因此第二按钮来源和跨线程断言均未执行，不计通过。实例和 scratch 已清理。

- 将 Button 集合从样式实现分至独立 C 文件，新增原生集合测试：模拟样式操作，实际运行集合和 COM 身份引用管理。x86/x64 `core/xaml-button-set` 各 1 项通过，覆盖两个对象、重复接入、安装失败回滚、安装重入拒绝、跨线程 discard 拒绝、撤销冲突保留和释放引用。此测试不启动 XAML，不证明实际绘制或产品接入。复用 build/.ninja，无临时包。

- 诊断测试 TAP 从事件父节点沿 WinUI `VisualTreeHelper.GetParent` 最多向上查找 64 层，以便识别深层子节点的 Button 祖先；方法顺序只读核对本机 WindowsAppRuntime 2.6.5 x64 元数据。双宽增量编译通过，自建 x64 记事本两次观察到两个按钮应用、两者默认来源恢复、错误线程拒绝；第二次明确观察到深层祖先命中。该结果不证明相对旧探针的可靠率提升、目标所有按钮或产品接入；未验证像素和真实输入，实例及 scratch 已清理。接口说明见 [Microsoft 文档](https://learn.microsoft.com/en-us/windows/windows-app-sdk/api/winrt/microsoft.ui.xaml.media.visualtreehelper.getparent)。

- 深层祖先遍历已从测试 TAP 提取至可复用纯 C `win16-control-xaml/visual_tree.c`，由测试 TAP 作为调用方；双宽增量编译通过。自建 WinUI 记事本 x64 一次对照观察到深层按钮、两个按钮应用及默认来源恢复、错误线程拒绝；这是测试调用，不代表正式产品接入、全部发现或真实视觉通过。测试实例和 scratch 已清理。

- 本机 WinUI 元数据确认 `IDependencyObject.ReadLocalValue` 与 `IFrameworkElement.GetBindingExpression` 可用；微软文档指出绑定也算本地值，因而仅保存有效值不能证明原绑定可恢复。诊断探针新增记录本地 Style 元数据位，双宽编译通过；自建记事本 x64 一次观察到本地来源值含 IsValueHandle，同时一个默认来源 Button 应用和恢复成功。本次未观察到的标志不能证明不存在动态资源或绑定，暂不扩大接管。新增可复用自有记事本对照脚本；自动清理自有进程与文档。

- 为 Button 集合增加逐项撤销入口：调用者须先停止发现，遇首个撤销错误立即返回并保留剩余项。模拟样式操作的双宽原生测试验证冲突时对象仍保留、再次调用可移除两项并释放引用；双宽增量构建及 `core/xaml-button-set` 均通过。尚未从真实诊断会话调用批量撤销，不能计真实应用恢复通过。

- 新增同 UI 线程实验会话，把祖先发现、默认来源 Button 应用及停止后逐项撤销组合；测试 TAP 以独立测试模式调用，非正式 DLL。双宽增量编译通过；自建 WinUI 记事本 x64 一次报告会话应用和停止均成功。停止只证明集合的撤销调用返回成功，本次未逐个核验原来源或像素，也未覆盖多线程、多窗口和产品启动。脚本自动清理自有实例与临时文档。

- 实验会话的 observe 增加可选已应用按钮引用，供调用方识别该次变更。自建 WinUI 记事本 x64 测试对会话应用的首个按钮，在停止后重新读取属性链，确认 Style 来源为 Default；双宽 TAP 增量编译通过。仅覆盖一次首按钮，不代表其他按钮、绑定或像素已恢复；会话尚未进入产品 DLL。测试脚本清理了自有进程和文档。

- 对照既有 GDI Button 边框，实验 XAML 模板加入上左高光、下右阴影、按下反转及焦点边框，调整测试读取内容的视觉树路径；双宽增量编译通过。自建记事本 x64 两次仅发现本地样式按钮，有限期内没有应用默认来源按钮，故新模板解析、视觉状态和恢复均未在真实 WinUI 对照中执行，不计通过。自有进程和临时文档由脚本清理；尚未产品接入。

- 新模板补充实测：自有 WinUI 记事本 x64 的 XamlReader 单独解析成功；后续两次自有实例观察到默认来源 Button 应用、文本值匹配和来源恢复，程序调用 Normal、PointerOver、Pressed、Disabled、Focused 和 Unfocused 状态均成功（焦点状态在第二次纳入）。双宽 TAP 编译通过；未验证屏幕像素、物理输入或所有内容类型，不能宣称视觉验收通过。测试进程和临时文档已清理。

- 将诊断初始化从测试控制器提取为 `win16-control-xaml/diagnostics_start.c`：调用者提供 PID、实际运行时模块路径、诊断 DLL、TAP 路径、端点及 CLSID；入口管理 WinRT 初始化及模块引用，不固定应用名或版本。测试控制器已使用该入口，双宽增量编译通过；自建记事本 x64 一次初始化、会话应用及首按钮停止后默认来源验证通过。目标模块路径仍由测试脚本提供，正式启动器、TAP DLL 入口及多位宽实际连接尚未接入或验证；测试实例和临时文档已清理。

- 新增按目标进程实际模块定位 WinUI 运行时的入口，测试控制器改为使用它，而不是由脚本传入模块路径；x86/x64 增量编译通过，自建记事本 x64 发现模块后会话应用和首按钮默认来源恢复通过。以当前自有 PowerShell 进程作无 WinUI 对照，x64 返回 ERROR_MOD_NOT_FOUND=126；x86 对 x64 目标返回 ERROR_PARTIAL_COPY=299。仅证明上述位宽及负例，不代表跨位宽会话接入或产品启动已实现。测试实例和临时文档清理。

- 新增自有进程的运行时定位原生测试，使用隔离输出目录的两个最小测试模块；x86/x64 各 1 项通过，覆盖两个模块均缺失、只加载一个、均加载时路径一致和两种输出缓冲不足。测试模块不是 Windows 运行时，不证明跨位宽或会话初始化；测试后清理临时测试模块，保留可复用编译缓存。

- 提取可复用纯 C 诊断 site 对象：实现 IObjectWithSite、视觉树回调以及连接/断开通知，调用方提供事件和 UI 线程生命周期处理；测试 TAP 可选用该对象。双宽增量编译通过。自建 WinUI 记事本 x64 首次有限等待没有观察到事件，第二次观察到 site 连接、按钮发现、会话应用及停止后首按钮默认来源恢复。真实断开、COM 引用竞争、多 UI 线程和正式 DLL 类工厂未覆盖；不称产品接入。测试进程和临时文档清理。

- 修正实验诊断 site 生命周期：订阅时持有自身引用，Unadvise 失败向调用方返回错误并保留回调与上下文，成功断开后释放。新增双宽原生模拟测试，覆盖订阅失败清理、事件分发、取消订阅失败保留及后续成功释放，均通过。修改后自建记事本 x64 检测到 site 连接，但有限期内未观测到应用或停止事件，不计真实运行通过；真实断开竞争及产品接入仍未验证，测试实例和临时文档已清理。

- 将自有记事本对照改为可选可见启动，验证位图读取：两次 x64 对照按钮应用、状态及来源恢复成功，但诊断 RenderTargetBitmap 仍失败；第二次记录 HRESULT=E_NOTIMPL，说明该次不是简单因隐藏窗口而成功。该结果不排除其他截图路径；未得到屏幕像素，测试实例和临时文档清理。探针双宽编译通过。

- 对自建可见记事本尝试 PrintWindow 捕获：调用阻塞超过有限观察时间；核实父子 PID、映像及专属空文档后结束自有子进程才返回，所得图像仅空白窗口，不计视觉通过。测试父进程和空文档已退出清理；撤销这次会阻塞的临时捕获代码并删除截图。后续需采用有超时隔离的捕获方式。

- 修正实验诊断 site 在订阅同步回调期间可能被重入断开的问题：连接/断开进行时拒绝再次 SetSite。模拟订阅中同步重入的原生测试双宽通过，原有订阅失败和撤销失败测试继续通过；一次自有记事本 x64 只观察到 site 连接，未见应用/恢复事件，真实结果不计通过。实例和临时文档已清理。

- 在自建 WinUI 记事本 x64 中通过模板视觉树读取四条边的 SolidColorBrush：程序切换 Normal 时上/左为白、下/右为灰，Pressed 时相反；同次文本值对照与默认来源恢复成功。WinUI 接口标识和方法顺序只读核对本机元数据；双宽 TAP 编译通过。该检查是对象属性而不是屏幕像素，不证明实际鼠标、焦点外观或完整布局。测试实例和临时文档清理。

- 实验 Button 模板的默认前景色改为样式 Setter 的黑色，内容展示器绑定控件 Foreground，以允许控件自身更高优先级的前景值；禁用状态只把内容前景设为灰色，不再降低整个边框的不透明度。自建 WinUI 记事本 x64 模板解析成功，一次有限期无事件，另一次应用和默认来源恢复成功，并通过状态检查确认禁用内容画刷灰色。仅验证对象属性，未验证自定义前景优先级、像素或实际点击；双宽 TAP 编译通过，自有实例和临时文档清理。

- 诊断时序对照：自建可见记事本等待目标主窗口句柄就绪后再连接，两次均连接成功；第一次发现 Button 并应用、停止后首按钮来源恢复，第二次有限期内没有 Button 或应用事件。主窗口就绪不能保证当前诊断发现可靠；不改变产品时序或把复测通过视作根因。两次自有进程和临时文档已清理。

- 对自有可见记事本的泛用 site 增加阶段事件：控制器初始化返回成功，site connect 回调触发；两次等待期内既没有视觉树回调，也没有观察到订阅完成或订阅失败事件（第二次启用失败事件后）。这表明“连接回调触发”不足以证明订阅已完成；尚不能确定阻塞发生在订阅调用还是其前后，不能归因于按钮类型或声明诊断失败。双宽 site 原生模拟测试通过，真实实例和临时文档清理。

- 临时阶段探针明确两次自有记事本的泛用 site 均已到达 AdviseVisualTreeChange 调用前，有限等待内没有观察到该调用返回成功或失败，也没有视觉树事件；一次为会话模式，一次为只读模板解析模式。不能据此断言永久死锁或具体根因。随后旧探针的独立对照控制器返回4，未取得有效比较。移除仅用于定位的订阅前临时接口后双宽重建及 site 模拟测试通过；自有实例和临时文档清理。

- 控制器生命周期对照：测试控制器在初始化返回后保持进程及所加载的运行时模块，直到有限观察结束才退出；一次自有可见记事本 x64 仍只观察到 site connect，未见订阅完成、失败或视觉树事件。该结果不支持“仅控制器进程或该模块过早退出导致本次未发现”的简单解释，也未排除其他生命周期问题。控制器双宽编译通过，自有进程与临时文档清理。

- 以相同可见窗口与控制器驻留条件对照旧测试 site：一次诊断初始化返回成功且连接回调触发，但有限等待内同样没有订阅完成或视觉树事件，故本次现象不能归因于新 site 抽取。只读探针读取输入桌面为 Default；保持控制器的 WinRT apartment 和运行时模块直到观察结束后，另一自有实例仍仅收到连接事件。这些均为有限对照，未定位实际等待链。测试实例和临时文档清理。

- 增加自有进程只读等待链探针，双宽编译；两次自建记事本 x64 均在有限等待未见订阅完成。第一次读取 64 个线程均有结果，观察到同进程一条 SendMessage 等待指向标记的 site 线程；第二次标记 site 线程自身为 WCT blocked、仅单节点，未解析出其等待对象；另一条 ALPC 等待指向外部 PID、不能认定与订阅有关。WCT 未给出调用栈或循环，不能据此确定根因。输出文件和自有实例已清理。

- 订阅线程对照：线程栈与窗口线程核实，原方案在自有记事本的主 UI 线程调用 AdviseVisualTreeChange 后等待；定时器延后至当前调用返回之后仍未完成。把接口封送到 MTA 工作线程的首次尝试中，服务接口封送成功而回调封送失败；测试回调聚合自由线程封送器后，自有记事本 x64 对照观察到工作线程订阅返回、视觉树事件、按钮状态/文本及默认来源恢复。一次线程标记对照中首次视觉树回调与 Notepad 窗口为同一 UI 线程，结束时该线程回到 GetMessage。仅证明本机该次行为，不保证其他线程/应用或真实外观；这是测试探针，尚未产品接入。测试实例、探针输出和临时文档清理，双宽 TAP/探针编译通过。

- 将 COM 封送与 MTA 工作线程订阅提取到纯 C `diagnostics_subscribe`，返回由调用方持有的线程句柄，约定调用方在工作线程退出前保持模块与上下文有效。测试 TAP 改用该入口；双宽增量编译通过，自建 WinUI 记事本 x64 一次订阅、树事件、按钮应用、状态和默认来源恢复成功。真实应用的断开、取消及正式 DLL 接入尚未实现；测试实例与临时文档清理。

- 通用诊断 site 新增实验性异步订阅模式，聚合自由线程封送器并通过公共 MTA 订阅入口工作；订阅未完成时拒绝重复 SetSite，模块必须由调用方保持加载。双宽编译及 site 模拟测试通过。自建 WinUI 记事本 x64 观察到 site ready、视觉树事件、按钮应用及首按钮默认来源恢复；初次对照同时报告 E_NOINTERFACE，定位为诊断事件中非 WinUI 视觉对象。将该对象的接口不匹配作为遍历结束后，再次对照观察到应用与恢复且未报告该错误。异步断开竞态、多个 UI 线程、其他应用和正式 DLL 尚未验证；测试实例和临时文档清理。

- 自有 WinUI 记事本 x64 测试先停止实验会话并核验首按钮 Default 来源，再在窗口线程执行异步 site 的 SetSite(NULL)；观察到实际断开返回成功，首次树回调线程与窗口线程一致。双宽原生模拟新增订阅工作线程暂停期间断开被拒绝、完成后断开及引用释放的对照，均通过。仅覆盖一次真实目标和模拟竞态，不证明全部 UI 线程均可恢复或产品接入；测试实例与输出清理。

- 新增可由 DLL 导出调用的纯 C COM 类工厂，创建异步诊断 site；调用方负责模块及上下文生命周期，LockServer 当前明确返回 E_NOTIMPL。双宽原生测试通过 CLSID 匹配、禁止聚合、实例创建与引用释放；自有 WinUI 记事本 x64 经测试 DLL 实际调用该工厂，观察到订阅、按钮应用和首按钮默认来源恢复。正式 ntw16s DLL 尚未导出该工厂，产品启动、模块固定及多线程清理未验证；实例和临时文档清理。

- 新增实验性单 UI 线程 XAML 适配器，组合诊断事件、会话应用、UI 线程恢复及断开后引用保留；测试 TAP 使用该适配器。自建 WinUI 记事本 x64 两次观察到异步 site 就绪、按钮应用、首按钮默认来源恢复及 site 断开；第二次另观察到一次 ERROR_NOT_FOUND，来源可能为短命对象或属性链，未定位，保留为未解决。双宽 TAP 编译及既有集合/site/工厂测试各 3 项通过；多 UI 线程、断开与回调竞态、全部按钮及正式 DLL 接入未验证。测试实例与临时文档清理。

## 候选机制和关键验证

[InitializeXamlDiagnosticsEx](https://learn.microsoft.com/en-us/windows/win32/api/xamlom/nf-xamlom-initializexamldiagnosticsex) 接受目标 PID、诊断 DLL、实现 `IObjectWithSite` 的 TAP DLL 及 CLSID；文档将其列为 Windows 10 桌面应用接口。是否适用于本机真实 UWP 与 WinUI 3、能否在不新增产品文件和不修改注册表的约束下使用，均待实测。

[IVisualTreeService](https://learn.microsoft.com/en-us/windows/win32/api/xamlom/nn-xamlom-ivisualtreeservice) 提供树变化监听及属性接口；[SetProperty](https://learn.microsoft.com/en-us/windows/win32/api/xamlom/nf-xamlom-ivisualtreeservice-setproperty) 要求先获取属性索引与目标值句柄。设置有效值不自动证明原绑定、资源引用、模板状态或移除后的来源可恢复，必须在自有目标上分别验证。

先证明同位宽真实目标的诊断初始化、UI 线程回调、按钮身份、属性来源和撤销；再分别证明模板绘制、输入语义及动态控件。不同失败分别记录，不能以进程加载、树枚举或属性设置成功代替实际换肤。
