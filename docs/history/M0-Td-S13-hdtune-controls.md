# M0 Td S13 HD Tune Pro控件调查

本轮已完成有限诊断，待P1提交推送及实际远端复核；没有产品修复或二进制发布。
M0/T15和既有S/债务保持，不分配新T，不修改兄弟仓库。

## 真实目标与权限

D:/programs/misc/HDTunePro.exe，4.60.0.0、718848字节、PE x86，SHA256
c6c4e51fbc93778f18732a2aec10f54188f247adfb452e7f87c2cb2bcdd6d9ea。
清单requireAdministrator，裸CreateProcess实际740。最初普通权限run16s请求提权，
私有桌面30秒未观察到自有可见目标，不能把它算为产品失败/成功或Job隔离的提权目标。
后续用户明确批准一次hdtune-style-probe.exe UAC，控制器实际TokenIsElevated=1、PID42440。
原生C高令牌控制器分别在两个私有桌面/新Job启动裸目标38816及启动器15260→目标40220，
正常环境未携带RunAsInvoker；实际高权限运行及正常显示的主窗/控件/PrintWindow均取得证据。
DLL路径为assets/ntw16s32.dll、SHA789bf8ed116c4c89aca5fc83c26806127ebf5124822e291fc78468348279474c。

普通权限诊断还临时使用仅进程树RunAsInvoker，不改清单/全局设置/权限；应用自身再次检查管理员，
只显示错误框。它背后的284个已创建隐藏子窗口用于先行分类，绝不冒充正常界面验收。
最终高权限观察得到同样284个子窗口类型，另有真实可见性和正常显示截图。

## 已证实的断点

主窗为#32770，DPI96，加载DLL但root_attached=0。裸/注入截图均340×487，顶部30行逐像素
相同，仍是系统标题；客户区有4895像素变化、下拉等标准子控件已生效。
所以并非未加载DLL，也不是整窗附着成功；主窗漏附着的具体原因尚未定位。
本轮没有从模块存在推出成功，也没有猜测WM_INITDIALOG遗漏或DWM根因。
截图本身已有窄窗/裁切，裸/注入几何相同，不归因于本项目样式导致。

| 控件 | 总数 | 注入版附着 | 判断 |
| --- | --- | --- | --- |
| ComboBox | 5 | 5 | 标准真实类，已有路径 |
| Button | 73 | 66 | 6个BS_ICON和1个BS_PUSHLIKE由既有supported拒绝 |
| Static | 161 | 161 | 附着记录不证明任意应用自绘图表已换肤 |
| EDIT | 6 | 0 | 原生精确EDIT带WS_EX_CLIENTEDGE，由现策略排除 |
| 子对话框#32770 | 12 | 0 | 容器/子页不当作12个顶层标题缺陷 |
| msctls_updown32 | 6 | 0 | 缺少微调绘制支持 |
| SysListView32 / SysHeader32 | 各6 | 0 | 缺少列表/表头支持 |
| msctls_progress32 | 6 | 0 | 缺少进度支持 |
| msctls_trackbar32 | 2 | 0 | 缺少滑块支持 |
| SysTabControl32 | 1 | 0 | 缺少分页支持 |

这是标准Win32协议及支持策略缺口，与MD5summer自定义真实类识别不同；不由类名推断编译器。
基准页可见未附着包括6按钮、编辑/微调/分页各1；其他页的隐藏控件没有被宣称实际绘制通过。
原生probe检查类/真实类A/W、样式、字体、COMBOBOXINFO和公开附着状态，未解引用opaque过程地址。
生产对照位置为control_class.h、controls.c的supported、attachment.c及injection/window_titles.c。

## 验证与失败边界

纯C17/GCC16.1/-Wall -Wextra -Werror，复用既有自有Job/private-desktop/线程观察及PrintWindow机制。
不使用computer-use、不执行Benchmark、写入、擦除、错误扫描，不点击业务按钮，不切业务页。
普通权限最终两例正常关闭exit0/Job空仅证明错误框流程，不能替代真实高权限主界面生命周期。
高权限两例在正常显示后完成截图/状态读取，但driver最终bare=1、styled=1：正常关闭检查未通过，
由KILL_ON_JOB_CLOSE回收；随后控制器/两个目标/启动器相关PID均不存在。
最后GetLastError=5是非定位的残值，不能认定具体关闭API返回5；本轮未细分失败分支，不计测试通过。
早期普通权限4个失败尝试及后续观察失败均保留原始日志，未伪装成通过。

[精选证据](../etc/evidence/hdtune-controls.json)保存计数、身份、SHA、像素对照和局限。
外部visualizations/win16style-hdtune保留HDTunePro-native.png及HDTunePro-styled.png，
不提交原始日志/截图/临时探针。未覆盖其他DPI、物理输入、正常关闭成功、业务页实际绘制或产品修复。

## 最小后续方案

1. 对#32770首次初始化记录scope/WM_INITDIALOG及原始style/ex-style、安装返回值，先定位主窗首个拒绝节点。
2. 单独扩展ListView/Header/Tab/Progress/Trackbar/UpDown，保留应用通知、数据、键盘和滚动。
3. 审计图标/PushLike按钮及原生client-edge编辑框，保留原图资源/焦点/编辑语义后再调整拒绝策略。
4. 定位自有实例正常关闭失败，再声称生命周期回归通过；不使用名称批量结束或操作用户实例。

这只是调查S，167产品输入及assets/system32八SHA保持。没有构建新包或发布产品；
后续产品实现仍须按执行规范完成双宽干净构建/同包实际测试/双发布/推送复核。
登记build/td-s13-hdtune，提炼证据并核实无存活自有进程/无reparse后再回收。
