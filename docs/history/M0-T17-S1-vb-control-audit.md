# M0 T17 S1：VB5/VB6控件全貌审计

用户准入原队列第3项，并指定S1审计、S2 demo、S3及多个S实施、最后单独收口S。
基线202c4ee，准入前工作区干净；分配T17 S1，M0固定，不改其他候选/债务状态。

## 覆盖全集与审计结论

[审计说明](../etc/evidence/vb-controls-audit.md)和[148单位矩阵](../etc/evidence/vb-controls-audit.json)
冻结版本/控件/关键变体及接收者。20工具箱对象、Menu和五宿主的双版本52行；
35个盘点扩展模块族、66控件族按宿主展开96行；第三方随盘、IDE工具、IE平台和
非可视helper分别处置，不把所有组件均称微软原生。依据微软原作Q173352/Q194784
历史转存目录，与官方运行库支持定义交叉核对；原官方Q194784的404不再是目录阻断。
RTM/SP/edition版本是独立证据要求，不按同名推导兼容，不把148个目录项称148项通过。

源码检索MSVBVM/Thunder及native角色的全部生产命中：control_class.h只有VB6
CommandButton/TextBox/Frame精确owner映射；vb_text.c只有有限容器/编辑绘制作用域；
window_titles.c只放行MSVBVM60文字调用者；control_protocol.c不一般接受OCX owner。
原C fake-runtime注册三类，不等于真实VB6。Actor真实CheckBox为已知分类断点。

纯C32位探针通过LoadTypeLibEx(REGKIND_NONE)读取真实VB6和官方提取VB5的两份
TYPELIB资源；29/96与26/67类型不是完整内建控件coclass集合，不能据加载成功
声称类注册/初始化/绘制通过。VB5官方包仅下载/只提取CAB，未运行安装器/全局注册，
完整包及DLL SHA均在矩阵，微软二进制不进Git、不成为产品或核心构建依赖。

限定32文件本地扫描只发现Actor/MadShow/ChipGenius的VB6字符串候选，未运行硬件
探测，未把字符串当导入/运行证明。真实VB5应用、OCX对象、图形/ownerdraw/无窗口
变体、事件/所有权缺口转交后续实施S，最终必测缺口未解决不能关闭T17。

## 验证与交付边界

S1仅文档/原生只读审计，197产品输入和八个现部署SHA保持，不改版本/renderer/
资源/ABI，不虚构新发布。按产品S规范独立同197输入干净双宽all重建，四文件包的
PE/版本/导入导出/原五头18表和来源门禁通过；这是基线验证，不替换现有发布文件。
该实际重建包只做ABI32、ABI64、demo裸/样式对照三项，共2.02897秒，无失败/skip；
没有重复长矩阵，没有物理输入或真实VB全控件验证声明。治理13、来源23+9、资源6
自测与diff门禁通过。新增生产/测试代码0行，既有路径删除/移动0，保留资源原字节。

当前verified，实际推送/远端后S1关闭并按用户顺序续S2。S1 scratch为build/t17-audit
和build/t15-package/t17-s1-audit-baseline；前者纯C观测源码/官方VB5研究DLL将按SHA
交接T17后续S，仍在活动T任务资产登记中；不把需要继续使用的研究资产提前删除。
无用基线构建包在证据提炼和精确检查后清理，稳定缓存及其他任务源码/截图保留。
