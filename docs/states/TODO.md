字体任务已按用户指令关闭；独立生命周期待核实债务列于下方。字体交付见[记录](../history/M0-T8-S2-system-font.md)。

# Deferred Debt

- [ ] TODO(Low): 核实frame卸载的DWM策略恢复；Owner：frame生命周期；Evidence：[win16_style.c](../../src/ntw16s-dll/win16_style.c)读取SDK标记set-only的DWMWA_NCRENDERING_POLICY，调查见[S3](../history/M0-T8-S3-popup-shadow.md)；Admission：后续显式生命周期修订，本轮保留已确认标题代码。

菜单取消缺陷已修复关闭，见[修复审计](../history/M0-T7-menu-cancellation.md)。T6历史完成见
[完成审计](../history/M0-T6-completion.md)。隔离clipboard权限错误5及物理输入未覆盖
是已明示验证边界，不计通过、不自动创建任务；更换宿主后可使用既有探针复测。

新增条目格式：`- [ ] TODO(High|Medium|Low): 问题；Owner；Evidence；Admission`。
不得把待做实现藏在TODO中；可执行候选进入QUEUE，由新用户请求准入。

- [ ] TODO(Medium): 复现并定位两次并行回滚Job活动计数断言失败；Owner：原生启动/测试观测；Evidence：[T11原生诊断](../history/M0-T11-S1-uac.md)，原断言旧/新各8轮64组合未复现；[T16离屏证据](../etc/evidence/outline-validation.md)再次观察active3而成员2，旧/新各3轮串行通过，原因仍未定；Admission：可复现原生日志或用户明确准入测试观测修订，不放宽原断言。


Actor发现的VB选择控件缺口已按用户请求纳入[VB5/VB6专项候选](../proposals/vb5-vb6-controls.md)，不再作为零散实现债务。

- [ ] TODO(Medium): demo八页reopen即时GDI相等断言曾偶发失败，S7串行复测通过；S8新增warm/final诊断为103->103及91->91，未复现且未放宽断言；Owner：T17后续测试观察/独立最终审计；Evidence：[S7](../etc/evidence/vb-file-controls.json)及[S8](../etc/evidence/vb6-file-controls.json)；Admission：用户T17完整测试/后续S授权范围，需实测定位，不凭复测通过宣称根因已修。

- [ ] TODO(Medium): S9容器测试重复完整GUI创建/销毁时，裸启动和样式启动总GDI计数均出现跳变，尚未区分系统字体/GUI缓存与未定位所有权；Owner：T17后续资源观察/最终独立审计；Evidence：[S9容器](../etc/evidence/vb-container-controls.json)，精确两份私有字体移除后GetObject为0，六次重复绘制计数上界通过，但不宣称全进程零增长；Admission：T17既有自动后续S授权，需原生创建/销毁逐对象证据，不能凭扩大warm次数消除断言。
