本轮外观反馈由CURRENT的开放任务接收；独立生命周期待核实债务列于下方。字体交付见[记录](../history/M0-T8-S2-system-font.md)。

# Deferred Debt

- [ ] TODO(Low): 核实frame卸载的DWM策略恢复；Owner：frame生命周期；Evidence：[win16_style.c](../../src/ntw16s-dll/win16_style.c)读取SDK标记set-only的DWMWA_NCRENDERING_POLICY，调查见[S3](../history/M0-T8-S3-popup-shadow.md)；Admission：后续显式生命周期修订，本轮保留已确认标题代码。

菜单取消缺陷已修复关闭，见[修复审计](../history/M0-T7-menu-cancellation.md)。T6历史完成见
[完成审计](../history/M0-T6-completion.md)。隔离clipboard权限错误5及物理输入未覆盖
是已明示验证边界，不计通过、不自动创建任务；更换宿主后可使用既有探针复测。

新增条目格式：`- [ ] TODO(High|Medium|Low): 问题；Owner；Evidence；Admission`。
不得把待做实现藏在TODO中；可执行候选进入QUEUE，由新用户请求准入。
