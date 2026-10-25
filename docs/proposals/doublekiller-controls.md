# DoubleKiller控件适配

本次请求准入M0 T15 S2。先用真实自有实例区分注入、附着与绘制；按实际框架收敛有限范围。
不假设DoubleKiller属于VB6，不猜测私有对象布局；不执行扫描/删除等业务操作。
已有T12启动修复、T15 VB6行为和原资源保持，ABI预期不变。

自包含C原生测试覆盖可验证的框架协议、同名拒绝、消息及释放，真实应用仅可选本机兼容。
每S干净双宽、四文件实际包、assets根及O:/winnt/system32发布、提交推送和实际远端复核。
登记scratch为build/t15-doublekiller、build/t15-package；证据提炼后安全清理。
T15待验收不自动关闭；M0保持开放。最初S先定位及最小闭环，必要续S须记录范围与下一编号。


## 已验证断点与最小路线

实际x86目标SHA cdd83ab909db749992d11ace2dff671493c0568d7a14697f8bef4b06d8647ca8（摘要以证据机器记录为准）。
主窗附着正常，内部VCL注册名未被标准类名单识别。RealGetWindowClass明确返回Button/Edit/ComboBox；
采用系统原生实例类型识别，不靠T前缀、应用名、私有对象布局或猜测WNDPROC地址。
有限fallback只对应已有BUTTON/EDIT/STATIC/COMBOBOX/LISTBOX/SCROLLBAR协议；现有owner-draw等拒绝保持。

框架InitWndProc在WM_NCCREATE中调用原生过程之前，实例类型尚未建立。初始原型漏掉列表早期主题记录，
列表创建5023、真实Options无法完成；随后“提前对子窗口装主题子类”原型虽通过33项包测试，
实际按钮填充仍是原生f0f0f0，诊断显示控件绘制回调未执行：VCL覆盖了过早安装的子类，附着记录成为假阳性。
因此该原型不发布、不计作完整修复。强化的自包含初始化器同样重设WNDPROC，并断言c0c0c0，能检出它。

最终方案把未知子窗口的早期主题请求保存在同GUI线程的私有暂存表，不改变InitWndProc；WM_CREATE结束后
提交到正常子类生命周期，再安装renderer。原生已知LISTBOX/ComboLBox等早期路径保持；取消、失败创建、
终结和scope退出回收暂存记录。独立生产theme.c单元证明实际所有分配配对归零、失败保留旧主题、
提交/取消/终结及最新应用主题恢复。无新hook/导出/DllMain动作、无WNDPROC猜测或共享类修改。
原生超类client-edge编辑框保持布局/原始编辑过程，只覆盖现有边框；VB及标准类策略保持。

本机四页按钮/复选/编辑/下拉附着，主窗与既有TListView框架保持；TPageControl、TTabSheet、
owner-draw THintCheckListBox、TSpinButton、进度/动画及无窗口标签保留应用绘制，不宣称这些已换肤。
自包含纯C测试用自定义初始化过程再调用系统原过程，覆盖7类、同名假TButton拒绝、ownerdraw拒绝、
按钮通知/空格、勾选、编辑undo、下拉展开取消、列表选择、像素变化、几何/借用字体恢复及Job空。
双宽、裸/注入、96/144 DPI共8新建目标；当前旧包双宽均被新测试检出，真实旧包observer也失败。
真实兼容driver只切页/读状态/移动/最小化恢复/关闭，绝不点击Run、删除、移动或设置注册表。

参考微软[RealGetWindowClass说明](https://devblogs.microsoft.com/oldnewthing/20101231-00/?p=11863)及
[API契约](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-realgetwindowclassw)，无外部实现代码复制。
