# M0 T21 S2：全局模式窗口接入基础

本 S 在正式双位宽 DLL 内加入私有同 GUI 线程窗口接入层。标准顶层窗体、支持的原生控件和 EDIT 分别复用现有 renderer；已由正常 run16s 创建作用域附着的窗口可识别并在统一退出路径中移除。重复接入不会重复安装；错误线程拒绝；窗口销毁和接入重入销毁保留状态生命周期。对未知或不支持的窗口返回失败，不接管其应用绘制。

双宽原生隔离测试验证了已有窗体、按钮和编辑框、文字保留、重复调用、错误线程、销毁、安装中重入销毁及既有创建作用域窗口的卸载。测试源与 DLL 共用同一私有实现源码，链接并运行同批 DLL；该测试不是跨进程全局钩子，也不证明所有窗口类型或两种真实 run16s 启动顺序。没有新增导出或第五产物。

`s2-window-final` 同一快照 x86/x64 干净构建及 ABI、来源和原图形资源逐字节门禁通过；`s2-window-check` 同批四文件 10 项聚焦包测试通过、无跳过，包括双宽全局窗口基础、普通启动/传播及 demo 必选回归。此前先完成的 `s2-window-clean` 是加入编辑框前的中间构建，不用于发布。`s2-window-publish` 将同批四文件发布到 assets 与 O:/winnt 并核对摘要：demo 78ee05c9927c23bb5f1951b88c69ab878b7f2c38745e4afe1fd906fe32e25e2a；x86 DLL dad4b50fe0319d9526530119b6af29d5dd6da2a6afc2feae9fab4da656e70962；x64 DLL 558623ae9c070dab70159e6d30f2538417cc4ca3dc9d71480f8a230f3df7f4d6；run16s db1e9947fb8550e451dfad734401c296d0fff27a894caeca88caf3cec83aafc9。产品版本随新 T 由 0.20 更新为 0.21。

源代码新增私有实现 83 行、私有头 10 行、测试 53 行，其他生产变更为三个版本资源；CMake/包测试及来源回执同步更新。搜索 `Ntw16sWindowAttached|Ntw16sWindowRemove|win16_style_install|win16_style_control_install|win16_style_edit_install`；接入复用现有每窗口所有者，移除只调用对应现有释放入口。未安装桌面全局钩子，未实现控制器、跨进程分发或 `--global`；用户此前接受退出残影，完整恢复和多种应用仍须后续验证。
