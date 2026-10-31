字体任务已按用户指令关闭；独立生命周期待核实债务列于下方。字体交付见[记录](../history/M0-T8-S2-system-font.md)。

# Deferred Debt

demo 旧式文件选择的跨进程观察器已用 WM_SETTEXT/WM_GETTEXT 修正，原生及 P235 单文件选择通过；覆盖边界见 [T20 S1 阶段记录](../history/M0-T20-S1-preliminary.md)。

- [ ] TODO(Medium): Shell 文件夹带应用筛选回调的旧树适配；Owner：未来经准入的公共对话框工作。Evidence：[S3 Shell](../history/M0-T20-S3-shell-filter.md)：为保留真实筛选接口，当前回退系统新树。Admission：能可靠保留真实 Shell 接口、筛选、PIDL、回调和生命周期的旧树方案；不得移除筛选或伪造结果。

- [ ] TODO(Medium): 普通 Win32 Explorer 多选旧布局兼容；Owner：后续公共对话框工作。Evidence：[S7](../history/M0-T20-S7-win32-multi.md)、[S8原型](../history/M0-T20-S8-win32-multi-prototype.md)和[S9](../history/M0-T20-S9-file-navigation.md)：O: 无短名长文件在系统旧列表不可见；[S10](../history/M0-T20-S10-file-selection.md)合并了自有导航选择原型，[S13](../history/M0-T20-S13-shadow-coverage.md)确认错误交接及 ANSI/Unicode 差异仍未闭环，目前保留 Explorer。Admission：可靠枚举长名并保持多选、返回缓冲与跨目录语义的方案，不改系统短名设置。

- [ ] TODO(Medium): COM 多选旧布局及非多选并发兼容；Owner：未来经准入的公共对话框工作。Evidence：[S3 COM](../history/M0-T20-S3-com-safety.md)：多选回退系统新框，已测双多选并发恢复；旧框串目录根因及非多选并发未确认。Admission：原生对照、真实长名与独立目录、返回对象及资源生命周期均可验证的实现。

- [ ] TODO(Medium): Shell 旧树初始化选择系统临时目录超时；Owner：后续公共对话框测试和性能调查。Evidence：[S6](../history/M0-T20-S6-folder-probe.md)：旧Shell无注入及注入均超时，现代COM无注入可取消；改用Windows目录的正负例不能证明原问题解决。Admission：可控目录、双宽同入口对照及准确阻塞证据，不扩大超时或掩盖失败。

- [ ] TODO(Low): 公共对话框聚焦回归中的间歇超时；Owner：未来复现时的原生探针调查。Evidence：[S14](../history/M0-T20-S14-completion-audit.md)首次观察发布批次 x86 COM 多选并发及新包 x86 ANSI 文件打开超时，各自独立复测通过，根因均未确定；Admission：保留失败现场并获取可复现阻塞证据，不把重试通过当修复。

- [ ] TODO(High): 普通 Win32 文件打开默认快捷方式解析的其余边界；Owner：未来经准入的公共对话框工作。Evidence：[S16](../history/M0-T20-S16-file-link-resolve.md)仅处理带筛选、绝对盘符路径或单一文件名、现存普通文件的快捷方式；目录、带子目录的相对路径、UNC、断链、虚拟对象和长路径仍未解决。Admission：原生新旧框对照及缓冲区、路径、错误码、导航和取消契约。

- [ ] TODO(Low): Explorer 文件框自定义筛选取消后的 nFilterIndex 差异；Owner：未来有真实应用需求时。Evidence：[S1 阶段](../history/M0-T20-S1-preliminary.md)：有限系统新旧布局取消对照数值不一致，明确取消契约与普遍性未建立。Admission：真实应用影响及系统正负对照；不能凭少量样本强改返回值。

- [ ] TODO(Low): 系统代建或跨进程公共对话框与特殊参数的样式边界；Owner：后续按真实需求准入。Evidence：[S2 矩阵](../history/M0-T20-S2-dialog-audit.md)、[S4](../history/M0-T20-S4-common.md)。Admission：确认窗口所属、API及可验证的受控接入；不注入共享宿主或修改全局设置。

- [ ] TODO(Low): COM 文件对话框布局桥接长尾，包括事件回调、自定义控件和隐藏目录等尚不完整的场景；Owner：未来经用户明确准入的后续工作；Evidence：[T20 S1 阶段记录](../history/M0-T20-S1-preliminary.md)，正式包存在现代布局回退，隐藏文件和事件代码仅实验；Admission：用户后续明确要求恢复。按用户 2026-10-29 二八优先级指示，本轮停止扩展 COM 布局，不将实验视为已支持。

- [ ] TODO(Medium): 展开TColorBox时偶发屏幕采样与应用绘图DC不一致；Owner：T18
  后续真实应用验收。Evidence：[S40调查](../history/M0-T18-S40-color-screen.md)：
  双宽旧/新包均复现，应用前后DRAWITEM DC相同而重画前屏幕不同，具体呈现或
  采样机制未确定。Admission：可观察显示环境和能区分应用blit、窗口合成、
  遮挡及DC采样的证据；不凭少量DwmFlush结果修改系统设置或声称因果。

- [ ] TODO(Medium): S34模块TEdit重复创建的进程级GDI预热差异；Owner：T18资源与
  最终验收。Evidence：双宽候选20次中前期从约12增至52、后续稳定并回落至50；
  x64未注入对照从9增至13。已验证精确控件重入移除和进程退出，但尚未区分
  延迟缓存与残留对象；Admission：按对象来源和生命周期做对照，不以短暂稳定
  或进程退出冒称全过程零泄漏。

- [ ] TODO(Medium): VB组合回归出现间歇性30秒子进程超时；Owner：T18后续测试及
  最终验收。Evidence：[S31](../history/M0-T18-S31-static-text.md)中新旧包均在
  同环境复现、随后分别通过；Admission：定位实际阻塞与退出条件，不把重试通过
  当作根因已解决，也不放宽原语义或超时断言。

- [ ] TODO(Medium): 延期的 VB5/VB6 DriveListBox 与外部 OCX 外观适配（23 个冻结单位：
  VB5/VB6 DriveListBox、DBGrid、MSWLess 的 9 个控件、grid32 Grid、msoutl32 Outline、
  mschart MSChart、vbsql Vbsql、MCIWnd、MedView、SchGrid）；Owner：未来经用户明确准入的
  后续任务；Evidence：[S28 最终审计台账](../etc/evidence/vb-final-audit-ledger.json)、
  [DriveListBox 审计](../etc/evidence/vb6-drivelistbox-audit.json)、
  [OCX wrapper 合约](../etc/evidence/vb-ocx-wrapper-contracts.json)。当前明确不支持这些
  单位，保留原生外观、绘制、数据与许可语义，且不把类名或文件名当作公开协议；Admission：
  合法、版本正确的 x86 运行时/样本以及公开 child-HWND、COM owner 或供应商 host 绘制合约，
  先以免注册、自有宿主探针验证，不猜测、复制、注册或绕过许可证。

- [ ] TODO(Medium): 两个裸VB6真实样本的临时IAT观察器首轮安装失败，S12自包含x86/x64及一次并发对照未复现；外部样本在当前非输入桌面于CreateProcessW前报623，未产生IAT失败节点；Owner：T17后续原生观察器审计；Evidence：[S11](../etc/evidence/vb-picture-variants.json)、[S12](../etc/evidence/vb-observer-lifetime.json)，严格实际/预期地址、模块、保护及恢复诊断保持，不放宽断言；Admission：现有T17自动后续S授权，需交互宿主真实失败日志定位，复测通过不视为修复。

- [ ] TODO(Low): 核实frame卸载的DWM策略恢复；Owner：frame生命周期；Evidence：[win16_style.c](../../src/ntw16s-dll/win16_style.c)读取SDK标记set-only的DWMWA_NCRENDERING_POLICY，调查见[S3](../history/M0-T8-S3-popup-shadow.md)；Admission：后续显式生命周期修订，本轮保留已确认标题代码。

菜单取消缺陷已修复关闭，见[修复审计](../history/M0-T7-menu-cancellation.md)。T6历史完成见
[完成审计](../history/M0-T6-completion.md)。隔离clipboard权限错误5及物理输入未覆盖
是已明示验证边界，不计通过、不自动创建任务；更换宿主后可使用既有探针复测。

新增条目格式：`- [ ] TODO(High|Medium|Low): 问题；Owner；Evidence；Admission`。
不得把待做实现藏在TODO中；可执行候选进入QUEUE，由新用户请求准入。

- [ ] TODO(Medium): 复现并定位两次并行回滚Job活动计数断言失败；Owner：原生启动/测试观测；Evidence：[T11原生诊断](../history/M0-T11-S1-uac.md)，原断言旧/新各8轮64组合未复现；[T16离屏证据](../etc/evidence/outline-validation.md)再次观察active3而成员2，旧/新各3轮串行通过，原因仍未定；Admission：可复现原生日志或用户明确准入测试观测修订，不放宽原断言。


Actor发现的VB选择控件缺口已按用户请求纳入[VB5/VB6专项候选](../proposals/vb5-vb6-controls.md)，不再作为零散实现债务。

- [ ] TODO(Medium): demo八页reopen即时GDI相等断言曾偶发失败，S7串行复测通过；S8新增warm/final诊断为103->103及91->91，未复现且未放宽断言；Owner：T17后续测试观察/独立最终审计；Evidence：[S7](../etc/evidence/vb-file-controls.json)及[S8](../etc/evidence/vb6-file-controls.json)；Admission：用户T17完整测试/后续S授权范围，需实测定位，不凭复测通过宣称根因已修。

- [ ] TODO(Low): S9容器的首轮总GDI跳变在S12两轮x64 VB5/VB6原生对照中均于第二轮平台化；Owner：T17最终独立审计；Evidence：[S9容器](../etc/evidence/vb-container-controls.json)、[S12](../etc/evidence/vb-observer-lifetime.json)，精确两份私有字体移除后GetObject为0，第二轮增量0；Admission：只在最终148单位审计需要把该有限模型结论扩展到真实应用时，逐对象另行证明，不能称全进程零增长。

- [ ] TODO(Medium): S15 x86菜单取消测试case 0一次超时并反复绘制，单独同包复测通过；Owner：菜单取消测试及renderer；Evidence：[S15观察](../history/M0-T18-S15-modern-vcl-observation.md)；Admission：复现时采集计时/绘制和窗口状态定位，不因复测通过认定根因已修，不放宽断言。

- [ ] TODO(Medium): S15 x64 v6树控件包测试一次在18变体末尾超过10秒子进程期限，单独同包复测通过；Owner：公共树控件测试与renderer；Evidence：[S15观察](../history/M0-T18-S15-modern-vcl-observation.md)；Admission：采集可复现的耗时及绘制/退出阶段信息，查明原因，不将复测通过视为修复或放宽语义断言。


S15包测试进度文件共享冲突已由[S17记录](../history/M0-T18-S17-test-efficiency.md)修复并验证；菜单和树控件超时仍未定位。

- [ ] TODO(Medium): 现代 VCL `TToggleSwitch` 的经典外观尚未实现；Owner：未来经证据准入的 VCL 控件适配。Evidence：[S20](../history/M0-T18-S20-desktop-evidence.md)。Admission：真实版本可复核的公开状态或内容中性绘制契约、开关/禁用/焦点/DPI及输入语义；不得依据像素、固定图集位置或私有字段猜测状态。

- [ ] TODO(Medium): VCL复选列表的应用裁剪外缘与显式DFCS_INACTIVE图形尚未适配；Owner：T18后续控件审计。Evidence：[S21](../history/M0-T18-S21-vcl-checklist.md)。Admission：保留应用裁剪、回调和状态的可验证绘制契约，覆盖真实多DPI及禁用/焦点；不能以81个可绘制内部像素冒充完整图形。

- [ ] TODO(Medium): VCL无窗口 `TSpeedButton` 尚未适配；Owner：T18后续控件审计。Evidence：[S22](../history/M0-T18-S22-speedbutton.md)。Admission：真实版本可复核的公开对象/绘制边界，须保留应用图像、透明、Flat/Down/Disabled和事件；不得以文本、坐标、颜色或静态地址猜测控件。

- [ ] TODO(Medium): VCL `TPanel` 未识别的背景、自定义色、其他边框变体及运行库版本仍待核实；Owner：T18后续控件适配及最终审计。Evidence：[S28](../history/M0-T18-S28-panel.md)、[S64 P75](../history/M0-T18-S64-P75-delphi-panel-frame.md)：已实现真实默认凸起/凹陷外圈的受限路径，未接管背景。Admission：保留子控件、颜色、裁剪、布局及交互；不得依据类名或颜色单独猜测。

- [ ] TODO(Medium): Delphi 无窗口 `TLabel` 尚未取得可安全区分应用画布的绘制身份；Owner：T18后续控件适配及最终审计。Evidence：[S29](../history/M0-T18-S29-label.md)。Admission：公开可验证的标签所有权及布局契约，保留字体/字号、透明、对齐、加速键及Unicode；不得按文字、坐标或私有对象字段猜测。


- [ ] TODO(Medium): ComCtl32 v5 `TProgressBar` 客户区填充仍为原生；Owner：后续进度条适配。Evidence：[S23](../history/M0-T18-S23-progress-border.md)、[S24](../history/M0-T18-S24-progress-colors.md)。Admission：不触发可观察的探测性颜色设置或猜测绘制实现，能可靠获取或保留原生颜色及状态并保留应用语义；现仅适配经验证的非客户区边框。

- [ ] TODO(Medium): S47首轮x86日期模块异常回归进程GDI计数27降至25导致相等断言失败；Owner：T18资源与最终验收。Evidence：[S48](../history/M0-T18-S48-date-gdi-test.md)精确区域句柄及故意泄漏负例通过、预热后重复计数不增长；初始进程总数波动原因仍未定位，不能据此证明所有GDI泄漏均不存在。Admission：定位进程级波动及其他资源的独立所有权，不把重试视作根因修复。

- [ ] TODO(Medium): Delphi `TStatusBar` 其余版本、简单/凸起/无边框/RTL及自绘末分区外观仍未认证；Owner：T18后续控件及最终集成审计。Evidence：[S47](../history/M0-T18-S47-status-icon.md)证实一个真实runtime图标末分区和双宽C模型；自绘末分区仍原生。Admission：真实样本、公开绘制入口与所有权证据，保留文字、图像、自绘回调、裁剪及生命周期。

- [ ] TODO(Medium): TRadioGroup container variants beyond the witnessed ordinary enabled frame, and additional TGroupButton lifecycle variants, remain unverified. Owner: T18 final integration and control/lifecycle work. Evidence: [S51](../history/M0-T18-S51-radio-group.md), [S52](../history/M0-T18-S52-group-lifecycle.md), [S53](../history/M0-T18-S53-radio-frame.md), and [S64 P72](../history/M0-T18-S64-P72-delphi-radio-frame.md). Admission: verify each remaining title, clip, DPI, parent and role without changing option semantics.

- [ ] TODO(Medium): VCL TStringGrid/TDrawGrid custom cells, editing, physical drag, additional DPI and runtime versions remain unverified. Owner: T18 final integration and follow-up control work. Evidence: [S54](../history/M0-T18-S54-grid.md) covers empty sample client preservation and reparent; [S55](../history/M0-T18-S55-grid-content.md) covers two string values and horizontal/vertical synthetic scroll. Admission: independent custom content and interaction evidence; retain application cell and data ownership.

- [ ] TODO(Medium): VCL TControlBar custom bevel widths, versions and DPI beyond its eight witnessed ordinary bevels and no-bevel remain unverified; toolbar-band content remains application-owned. Owner: T18 subsequent control audit. Evidence: [S56](../history/M0-T18-S56-controlbar.md), [S64 P135](../history/M0-T18-S64-P135-controlbar.md), [S64 P136](../history/M0-T18-S64-P136-controlbar-bevels.md). Admission: establish each distinct nonclient drawing contract and preserve child layout, images and callbacks.

- [ ] TODO(Medium): VCL TToolBar image/separator/other drawing modes, more than 64 items, additional runtime versions/DPI and physical input remain native or unverified. Owner: T18 subsequent toolbar work. Evidence: [S57](../history/M0-T18-S57-vcl-toolbar.md) covers uniquely identified text-only ordinary buttons; application skip, duplicate ID and image model fall back. [S58](../history/M0-T18-S58-toolbar-boundaries.md) validates clipped, 64 and 65-item model boundaries. Admission: independent image/callback ownership and interaction evidence; preserve command semantics.

- [ ] TODO(Medium): VCL TTabControl owner-draw, alternative orientations, image tabs, runtime versions and physical input remain native or unverified. Owner: T18 final integration. Evidence: [S60](../history/M0-T18-S60-vcl-tab.md) covers ordinary two-tab x86/x64 actual runtime and self-owned fake/reparent model. Admission: distinct native semantics and rendering evidence per variant.

- [ ] TODO(Medium): VCL TGroupBox variants beyond the ordinary enabled group remain unverified; Owner: T18 control audit. Evidence: [S61](../history/M0-T18-S61-groupbox.md), [S64 P71](../history/M0-T18-S64-P71-delphi-group-frame.md). The ordinary enabled frame is implemented; additional runtime versions, disabled states and DPI require separate title/clip and child-content evidence.

- [ ] TODO(Medium): VCL TMemo additional runtime versions, DPI, long text, IME, physical input and complex scrolling remain unverified. Owner: T18 final integration. Evidence: [S63](../history/M0-T18-S63-memo.md) covers ordinary multiline actual x86/x64 and native model selection, read-only and scrolling. Admission: verify each remaining version and semantic variant without changing application data ownership.

- [ ] TODO(Medium): MD5summer主窗口内嵌TShellTreeView保留现代文件夹图标；Owner：后续经准入的应用内嵌Shell树图标研究。Evidence：[P181真实截图](../history/M0-T18-S64-P181-common-verify.md)、[S13树状态](../history/M0-T18-S13-md5-tree-current.md)。独立[文件夹对话框候选](../proposals/classic-folder-dialogs.md)明确不自动覆盖应用内嵌树。Admission：核实实际图像列表来源、共享所有权、图标资源版本和许可及选择/展开语义；禁止替换共享系统图像列表或未验证替换应用图像。

- [ ] TODO(Medium): Delphi `TBevel` 与非默认 `TSplitter` 的无窗口共享画布外观未适配或未认证；Owner：未来有实际应用需求时准入。Evidence：[TBevel](../history/M0-T18-S64-bevel-investigation.md)、[TSplitter](../history/M0-T18-S64-splitter-investigation.md)。Admission：证明控件绘制身份、保留应用画布、布局和输入；不得根据坐标或颜色猜测。

- [ ] TODO(Medium): Delphi 其他未验证的原生或框架控件及变体（包括 `TAnimate`、旧式 `TTabSet`、`TDockTabSet`、外部容器和模型-only路径）保留原生外观或未认证；Owner：未来出现具体普通可见应用需求时准入。Evidence：[T18台账](../history/M0-T18-S64-control-inventory.md)。Admission：准确运行库、所有权、绘制与交互证据；不把模型当真实运行库认证。

- [ ] TODO(Medium): Delphi非报告列表视图协议在同步查询中改变父窗或样式的重入保护尚缺独立有效故障负例；Owner：后续控件生命周期测试。Evidence：[列表调查](../history/M0-T18-S64-runtime-lists.md)：移除末尾重检的故障版仍通过现有重入模型。Admission：建立能检出该故障且不影响应用语义的可重复原生测试；不称已证实完整重入安全。
