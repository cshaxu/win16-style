# HD Tune Pro控件调查

只读核实本机HDTunePro.exe的注入、实际控件和当前外观；对照MD5summer及DoubleKiller识别边界。
不执行任何磁盘测试/扫描/写入/擦除，不改产品，不分配新T。状态由CURRENT管理。
登记scratch为build/td-s13-hdtune；原生证据提炼后安全回收，外部PNG验收图保留。

## 已核实的有限结论

实际为x86/4.60，清单requireAdministrator，普通CreateProcess返回740；程序自身也检查管理员。
仅诊断实例临时RunAsInvoker可读取错误框后尚未显示的284个子控件；不能视作正常主界面验收。
与MD5summer不同，这里是标准Win32类。列表/表头、分页、进度、滑块、微调缺少renderer；
带图标/PushLike按钮和原生client-edge编辑框由既有策略排除。普通按钮/复选/下拉/Static已有附着。
用户已批准一次正常UAC；高令牌裸/注入真实主窗截图完成。主窗root_attached=0、
标题顶部30行逐像素相同，客户区4895像素不同，部分子控件有效；主窗漏附着的具体拒绝点尚未定位。
正常关闭检查失败、由自有Job销毁兜底，已核对相关PID消失；不计生命周期通过。
本轮只交付有限诊断与最小路线，不修改产品；详见[调查记录](../history/M0-Td-S13-hdtune-controls.md)。
