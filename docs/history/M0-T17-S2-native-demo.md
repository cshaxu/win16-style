# M0 T17 S2 native demo gallery

## Scope and result

User指定S2；S2已完成P1实际远端/恢复/清理并关闭，当前包0.17.0.0。
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

P1 eab7201438bf039cfd19d06dbf8f8b5dfb05ba14，实际GitHub210文件/199产品输入
及四文件逐字节通过；恢复203产品对象、8旧备份可从4386bad取回、8发布SHA和18PNG核对。
实际Git diff/原请求协调复核通过；CMake/src/tests/tools增加352删除38净314行，
P1统计范围和逐文件numstat见证据（文档统计记录前快照，区别源码与全仓）。无路径移动/删除。
192映像均无占用、reparse0，正例exe/loadedDLL拒绝c0000043、unloaded通过后，
仅清理精确build/t17-package。T17 S1研究/官方VB5 DLL、T15源码、稳定缓存、原外部图
及本次18PNG保留。S2关闭并按用户顺序准入S3；T17/M0开放，不称全部VB已适配。
