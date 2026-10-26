# M0 T17 S2 native demo gallery

## Scope and result

User指定S2；当前verified并双发布，待P1远端/恢复/清理，不能提前称closed。
原64位demo主窗/Apply/Reset/About保留；Controls新增八页，纯C原生Windows
对应项，禁止冒称真实VB/32位OCX。148审计单位对应页、应用内容/无视觉/缺失
对照逐项见[证据](../etc/evidence/vb-demo-gallery.json)，真实支持由后续S接收。

新catalog.c/catalog.rc位于demo组件；COMCTL32/COMDLG32/GDI32均OS依赖，
RichEdit只从SYSTEM32固定加载Msftedit.dll。没有样式接口、测试计数或第五产品。
字体/图像列表/计时器/窗口/模块租约有应用所有者，属性页在消息分派后及时回收。
文件页只读最多128条，不改process cwd；文件对话框只选择，不读写目标文件。
数据导航只在内存，OLE/图像/图表/多媒体/通信/数据绑定未伪造完整实现。

## Verification and boundaries

干净同源双宽all（Ninja8、ccache），x86/x64编译35.22/41.47秒，ABI各一次；
六项实际包测试8.64秒：ABI32/64、demo系统边界、裸/挂钩八页及原主窗交互、
双宽首次注入、cmd原生四宽度边嵌套传播。demo对照3.97秒，属性页两Tab/关闭、
三态/radio独立组/多选/只读/范围/目录/本地记录/计时器及MDI有限状态通过。
144DPI，18份正常显示窗口PrintWindow截图，原生消息/API不是物理键鼠验收。
一个按钮页warm reopen GDI稳定；首次系统模块缓存增量保留日志，未宣称全页零增长。
开发期观察到空闲等待与属性页关闭事件的竞态，最终改分派后回收并通过正式包。
旧0.15四文件被新观察器在缺少Controls菜单处0.33秒检出，退出1；不称VB负例。

93个renderer/launcher文件（不计版本RC）与4386bad一致，原五头/声明逐字节和
每DLL18栅格门禁通过；标题/字体/菜单/滚动算法不变。日历/日期/RichEdit/工具条/
ComboBoxEx等仍有原生外观，图像应用所有；后续S按实际类协议接收，不伪称已换肤。
HD Tune管理员测试默认skip；无真实VB5、OCX或完整91/全DPI验证。

四文件0.17.0.0同包实测后发布assets根和O:/winnt/system32，RM精确8资源owners0，
双发布八SHA一致、安装ABI32/64通过。文件摘要、源码199输入身份/工具链和结果由
[release](../etc/release.json)接收；截图外部登记，未加入产品或改原资源。

## Delivery review

P1提交/实际远端尚待执行。S2结束前提炼证据并只清理自有精确scratch；
S1研究资料/官方VB5 DLL交接后续T17，稳定缓存/他任务源码及外部PNG保留。
T17/M0开放；下一S按用户顺序处理选择按钮和VB5运行库基础接入，不提前关闭T。
