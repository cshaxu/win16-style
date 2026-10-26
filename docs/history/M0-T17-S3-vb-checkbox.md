# M0 T17 S3 VB6 checkbox and VB5 registration boundary

## Finite result

S3已完成P1实际远端/恢复/清理并关闭；T17/M0开放。
Actor两框在准确ThunderRT6CheckBox分类处遗漏；真实MSVBVM60 owner、style及状态
已核实。旧包两控件BMP与裸启动逐字节相同，新包各182源glyph像素匹配。
只改control_class.h和controls.c：准确runtime owner/name、现有绘制/字号策略，
不接管VB Value/Click；未验证图形/push-like/icon/bitmap/ownerdraw保留原生，
live转换退役painter。切回普通样式需新窗口或显式重新附着，不称自动恢复。

实际Actor为32位，style AUTO3STATE但0/1循环；每框4次BM_CLICK保持原初值，
原字体属性/style保留，Job完成端口active-zero及accounting0、正常退出0。
核心C fixture独立注册无原生BUTTON superclass的类，确保缺alias会失败；覆盖
裸/挂钩legacy96/aware144、三态/Space键/4通知/字体/卸载/owner伪类拒绝、
原生图形初始与live回退。C状态模型不是实际VB业务实现证据。

## Tests and publication

同源干净双宽all/ABI通过，gcc16.1.0+Ninja8+ccache；11项实际包17.08秒，
含ABI32/64、双宽C控件与字体所有权、原demo八页、双宽首次及cmd四边传播，
实际Actor裸/挂钩分别0.63/0.89秒。旧包被新核心/真实glyph负例分别0.999/0.597秒检出，
退出1，分别断在准确checkbox附着和实际像素；未放宽断言。

两处四文件实际SHA一致，RM精确8资源owners0；安装双宽ABI及真实Actor再次通过，
实际DLL绝对路径为O:/winnt/system32/ntw16s32.dll。200输入/四摘要/工具链由
[release](../etc/release.json)拥有；状态/API/模块物理路径SHA及完整有限日志、
截图、源码/标题保留证据见[选择控件证据](../etc/evidence/vb-selection-controls.json)。
S2与S3同862x724 demo的完整top36 RGB差异0；原五头/声明和两DLL各18图形表不变。
无标题/字体/菜单/滚动算法改动，无新增组件或第五产品。

## Not complete

VB5官方runtime可纯C加载，ThunRTMain存在，DllGetClassObject不存在；加载后
六个候选准确类查询均未注册，1411。不能盲调ThunRTMain或靠字符串推导支持，
缺真实VB5项目/样本的注册和控件证据交后续S/最终审计。OptionButton/实际图形
VB/OCX同样未通过，不据此关闭T17。未覆盖真实Actor物理键鼠、键盘/捕获/IME/
无障碍、全部DPI或91全集；HD Tune管理员UI仍默认skip。

观察器曾误把AUTO3STATE当三态，已用裸/挂钩实际2态纠正；Job即刻accounting
竞态改为有界完成端口后仍查0；原C superclass可掩盖alias缺失，已改独立C类。
上述首次失败留证，不追写为通过。控件角色/外观/实际业务证据严格分层。

## Delivery review

P1 2e250c2afc8dcb0cbff45bc0e80318970c097dee，实际HTTPS取回212文件，
200产品输入/四产物逐字节等于Git对象，远端main一致。恢复204产品对象、16份
before备份（8来自S2 Git，8来自保留中间包）、8部署SHA、18PNG均核对。
源码/测试CMake/src/tests/tools增309删8净301，统计范围及全仓numstat见证据。
原请求/实际diff协调复核通过，无产品路径删除/移动，无其他所有者改动覆盖。

自审在最终包中先原消息返回再退役painter；callback销毁用例旧中间/最终均通过，
不冒称已复现旧崩溃。追加修订触发最终干净双宽35.80/40.94秒及11项17.08秒
实际包重验、再次双发布/安装实际Actor；中间包+3源覆盖/200输入身份交接
build/t17-audit/s3-interim-release，未来回收前以最终P1 Git加覆盖还原并校验。

340映像无占用、两精确根reparse0，guard正例exe/loaded DLL拒绝c0000043，
unloaded通过后清理build/t17-package和build/t17-controls。S1研究/官方VB5 DLL、
中间包恢复资料、T15源码、稳定缓存和S2/S3 PNG保持。S3关闭并自动顺序准入S4，
后者继续真实内建样本/注册/协议盘点与实施；VB5/OptionButton等必测缺口由S4/
后续S和最后独立审计接收，绝不以本S局部支持关闭T17。
