# 菜单取消与模式清理

用户报告鼠标菜单打开后无法取消，后续悬停持续打开。本次准入由CURRENT确定，
在一个Corrective任务的S1中完成缺陷复现、修复、回归、双发布及任务级覆盖审计。

有限覆盖：鼠标首次打开、横向切换后Esc、客户区/同菜单二次点击、WM_CANCELMODE及
窗口失活；退出后悬停保持闲置；Alt/F10键盘选择、Esc层级、左右切换及命令执行。
原生探针观察菜单loop enter/exit、bar模式/selection、焦点、capture、scope及原item
状态恢复；旧实现必须被负例拒绝。双宽core和当前四文件实际包回归，原资源不变。

状态所有者仍为窗口GUI线程，保持栈式tracking_menu重入及s->calls生命周期；
不引入全局鼠标hook、轮询或常驻状态。产物版本随任务号更新，发布到assets根目录
和O:/winnt/system32，摘要绑定实际测试包；推送后实际读取提交/远端字节再关闭。
