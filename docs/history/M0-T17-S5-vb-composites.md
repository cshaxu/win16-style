# M0 T17 S5 VB ComboBox/ListBox/scrollbar wrappers

## Scope and evidence

S5完成准入和实现，正式包验证/双发布/实际远端复核后关闭；T17/M0保持开放。
[证据](../etc/evidence/vb-composite-controls.json)记录有限范围、正负例和限制；
[包身份](../etc/release.json)拥有四文件及实际发布SHA。

S4缺少准确runtime50/60 ComboBox、ListBox、HScrollBar/VScrollBar角色，真实
样例因此保留现代绘制；Combo还忽略父WM_SETFONT，内部原生EDIT单独绘制。
最小方案仅增加精确类别名并复用既有renderer/字号策略/原glyph；EDIT通过
COMBOBOXINFO、同线程父子关系和原生公开类元数据确认。真实manifest样例是
COMCTL32 EDIT；非manifest USER32还可能经系统分派模块，x86/x64分别实测，
不按过程地址必须落在USER32的错误假定，不读私有布局或改共享类。

本层child subclass持有父样式生存期引用；最多32字体租约，容量不足回退源字体。
回调/原生setter返回后才释放；移除/重挂归还源字体，终止销毁不重入GUI恢复。
正常WM_GETFONT保留源字体，同步paint内嵌套原生查询可能看到私有字体，明确
不将此描述为应用所有查询均透明。真正VB5/6是x86，x64仅独立C协议模型。

## Verification and delivery

新增C模型覆盖五项数据、列表双选择、下拉取消、原生子字体、移除归还、setter
回调重入移除、换父恢复、40次源字体变更及句柄上界；裸/样式、96/144 DPI。
真实原作者两版本各2Combo/1List/2Scroll公开协议、选择/取消、HFONT/矩形保持、
正常显示抓图；原Combo32黑点及每首端scroll225内区像素独立对照原头。
旧S4包的child字体与真实VB6 scroll像素两负例均退出1，确认新测试能检出缺口。

初次来源门禁发现作者README/测试回执未更新，未编译即拒绝；随后13实际包
用例检出x64公共类过程表示差异（12通过1失败），拒绝发布。修正为公开类过程
一致性校验后，最终从干净双宽all重建；这些前期失败不是成功计数。

普通单列表/下拉/下拉列表与当前样例横纵滚动只是有限覆盖。简单Combo、勾选/
owner-draw列表、RTL、真实IFont动态变化、物理键鼠/IME/辅助技术尚未认证，
继续由T17后续S和148单位独立全任务审计接收，不能以本S宣称全部VB支持完成。
标题、字体算法、菜单、scroll renderer及五原头/来源声明保持，没有第五产品、
VB IDE安装、OCX注册、兄弟仓库依赖或全局设置变化。

最终干净双宽all编译分别38.98/42.12秒（Ninja8/ccache）；13聚焦包测试15.46秒
全部通过，四安装复测通过。两处八SHA相同，真实标题top26完整RGB对S4差异0。
RM识别四旧文件占用者，逐个核对启动时间/映像后按用户发布规范关闭：cmd及ntsrv
超时结束，ntcon自行退出，run16s随目标退出；仅此发布步骤涉及既有进程，不将
它们冒称测试自有，也没有启动/重启服务制造测试条件。最终图片5PNG保存在外部
登记目录。实际远端取回和恢复/清理仍待P1/P2记录，不预先宣称关闭。
