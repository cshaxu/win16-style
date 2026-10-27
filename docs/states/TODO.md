字体任务已按用户指令关闭；独立生命周期待核实债务列于下方。字体交付见[记录](../history/M0-T8-S2-system-font.md)。

# Deferred Debt

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

- [ ] TODO(Medium): VCL `TPanel` 自绘背景和边框尚未适配；Owner：T18后续控件适配及最终审计。Evidence：[S28](../history/M0-T18-S28-panel.md)。Admission：可验证的公开绘制及颜色/继承契约，区分父窗背景与应用自定义色和bevel，保留子控件、剪裁、布局及交互；不得依据类名或画刷颜色猜测。

- [ ] TODO(Medium): Delphi 无窗口 `TLabel` 尚未取得可安全区分应用画布的绘制身份；Owner：T18后续控件适配及最终审计。Evidence：[S29](../history/M0-T18-S29-label.md)。Admission：公开可验证的标签所有权及布局契约，保留字体/字号、透明、对齐、加速键及Unicode；不得按文字、坐标或私有对象字段猜测。


- [ ] TODO(Medium): ComCtl32 v5 `TProgressBar` 客户区填充仍为原生；Owner：后续进度条适配。Evidence：[S23](../history/M0-T18-S23-progress-border.md)、[S24](../history/M0-T18-S24-progress-colors.md)。Admission：不触发可观察的探测性颜色设置或猜测绘制实现，能可靠获取或保留原生颜色及状态并保留应用语义；现仅适配经验证的非客户区边框。

- [ ] TODO(Medium): Delphi `TStatusBar` 其余版本、简单/凸起/无边框/RTL变体和真实第二分区外观仍未认证；Owner：T18后续控件及最终集成审计。Evidence：[S44](../history/M0-T18-S44-vcl-status.md)只证实一个真实runtime首分区边框、双宽C模型及应用内容保留；S43主题调用观察点未能安装。Admission：真实样本、公开绘制入口与所有权证据，保留文字、图像、自绘回调、裁剪及生命周期。
