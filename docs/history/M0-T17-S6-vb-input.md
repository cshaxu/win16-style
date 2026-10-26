# M0 T17 S6 intrinsic queued input and notification validation

S6实现了真实VB5/6输入队列与通知观察，干净包/发布/实际远端验证仍待完成，
本记录不预先宣称关闭。[证据](../etc/evidence/vb-input-controls.json)拥有有限范围。

直接发送WM_KEYDOWN绕过VB消息循环前处理：裸/样式滚动条均不变，不是样式
回归的证据。焦点核实后投递End，二者都到32766，Home回0；无MSAA值变化事件，
不把等待缺失事件600ms作为完成证明。最小处理是测试侧精确线程见证器，沿用
项目已有自有线程测试hook机制，不修改产品或注入用户既有进程。

纯C测试DLL仅对本次Job内目标的准确GUI线程使用WH_GETMESSAGE/WH_CALLWNDPROC，
观察通知、不改消息/返回。私有映射和事件连接驱动器；队列标记按HWND/代际
确认此前按键已过VB循环，错误代际不确认。没有全局hook、常驻程序或第五产品。
双宽自包含原生正负例验证通知和FIFO标记；拆掉见证hook后不能伪造通过。
样例可选参数开启真实输入；原作者二进制、VB安装或兄弟仓库均非核心依赖。

真实两版本普通Combo1->2、List1->8、横纵scroll End/Home及其parent通知
已观察一致。测试保留源HFONT/矩形、原glyph，实例active0再结束。真实VB是x86；
x64为C模型。物理键鼠、IME、更多变体/OCX未认证，完整148单位交后续S和最终
独立审计，不能将本S输入检查替代全任务完成。

最终干净双宽all编译分别8.97/11.42秒（Ninja8/ccache），15聚焦实际包14.12秒
全通过：11自包含（含双宽错误代际及无钩子40ms超时负例）和4真实原作者VB5/6。
按既定四文件发布到assets根和O:/winnt/system32，八SHA一致，发布RM占用0；
安装双宽ABI及真实VB5/6输入/通知/glyph/退出再验通过。原src路径全部逐字节
保持S5，测试DLL仅是构建/测试辅助，不进产品包。重建二进制SHA变更仍执行
同批发布，不将无产品代码修复说成没有新包。204输入/包SHA由release拥有。
四外部PNG无损导出，标题top26完整RGB对S5差异0；其他变体继续交后续S。
