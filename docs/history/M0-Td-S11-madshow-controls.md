# M0 Td S11：MadShow控件外观调查

状态：P1实际远端复核通过，Td S11在P2收口，M0开放；后续产品候选未准入。
用户“准入新S调研”，因此仅准入Td S11，不消费产品T，不重开T14；M0固定。
基线cad4242，工作树干净；只改研究/治理文档，未改代码、测试、版本或产品。

## 已证实断点

截图命令工作目录为O:/winnt，本机madshow.exe为x86 PE，200704字节，SHA256
9d93484c7b230b584440b14e2d2dbb353f058be26e3385677349cff42e5db6b6。
实际运行导入/加载MSVBVM60.DLL，控件GCLP_HMODULE注册者也均为该VB6运行库。
x86加载路径C:/Windows/System32/MSVBVM60.DLL经原生文件句柄解析实际是
C:/Windows/SysWOW64/msvbvm60.dll，已核对物理文件SHA，避免跨宽路径误认。

实际assets/run16s.exe启动后，ntw16s32.dll路径和SHA等于当前发布包。
CreateWindowExA/W、CreateProcessA/W四个入口E9目标均指向该DLL，证明入口钩子
已安装；Ntw16sWindowAttached在目标GUI线程查询主窗返回true，原生截图标题
确为本项目样式。因此不是“DLL没注入”。传播仅证明入口初始化，本应用未新建
业务子进程，不把这项扩张为实际后代传播成功。

| 实际类名 | 数量 | 主动附着 | 说明 |
| --- | --- | --- | --- |
| ThunderRT6FormDC | 1 | 是 | 实际模拟器主窗，标题成功 |
| ThunderRT6CommandButton | 4 | 否 | Start/Pause/Clear/Grid |
| ThunderRT6TextBox | 13 | 否 | Settings 7、Result 6；均有WS_EX_CLIENTEDGE |
| ThunderRT6Frame | 2 | 否 | Settings/Result分组 |
| ThunderRT6PictureBoxDC | 1 | 是 | 既有边框/frame路径；不代表自绘客户区已换肤 |
| ThunderRT6Timer | 1 | 否 | 隐藏零尺寸计时器，不是需要换肤的可视控件 |

每次实际主窗共22 HWND，RealGetWindowClass仍是这些VB6类名，不返回BUTTON/EDIT。
attachment.c install_created只接受标准类名，未知普通子控件走unsupported；
control_scope.c也有同一名单；controls.c的is_*/supported再次严格检查标准类名。
在真正GUI线程调用当前发布DLL的公开安装入口：20个子窗control请求返回50，
PictureBox因已有frame scrollbar tracker返回183；13个edit请求全部返回50。
所有尝试均未新装控件，原HFONT保持；不可把它们当作已实现修复原型。

4个按钮、13个文本框、2个分组共19个可视控件在裸/样式两组的尺寸、Tahoma字体
及tmHeight14均不变，所以正文外观/字体保持原程序。顶层623x481变615x471，
PictureBox外框428x428变420x420，属于既有frame路径；不能据这些外框尺寸
声称客户区几何完全不变，本S不修改这项或擅自认定为本次控件根因。

第二道独立门槛：13个TextBox都带WS_EX_CLIENTEDGE（ex=0x204），controls.c
原EDIT策略明确拒绝它；只加VB类名别名仍不足。现有edit_border只覆盖原平面
边框，不能直接当作原生client-edge接管方案。简单清除style还可能改变客户区、
光标及布局，候选实施需单独验证，不能用全局主题设置掩盖。

Step/Speed等可见标签、版权文字没有独立HWND，也没有STATIC子窗；通用MSAA只
枚举主窗4、Result 6、Settings 11个窗口子项，未单独暴露这些标签。它们在
父容器绘图路径上，现有逐HWND STATIC/WM_SETFONT方案覆盖不到。具体VB控件对象
及字体属性桥接未追踪，不宣称已掌握通用无窗口标签方案。直接把VB Frame当
标准GROUPBOX消费WM_PAINT，可能抹掉其无窗口标签，因此不能只改名单就交付。

## 原生证据和有限可复用性

纯C17 x86调查驱动及诊断DLL经MinGW GCC16.1、Ninja、项目共享ccache构建，均只在
登记scratch。每次裸/样式新建自有进程，私有桌面、kill-on-close Job、90秒总
截止和各阶段有界等待。诊断WH_CALLWNDPROC只限定到新目标的确切GUI线程，通过
WM_NULL调度，在DllMain之外查公开API/类/字体/主题/原生消息，并在正常关闭前解除。
不安装全局hook，不碰用户现有实例，不改系统设置。调查辅助DLL不是第五产品。

3个有效对照批次共6个实际应用实例正常退出0，严格Job活动数均0；最终裸Job总1、
样式Job总4。首次误选ThunderRT6Main零尺寸运行库辅助窗，虽清理成功但不计作
真实UI证据。后续限定模拟器标题、非WS_CHILD、宽高>200的实际主窗后才计入。
PrintWindow初次按观察器物理尺寸分配出现白色余区；改为只在诊断线程借用目标
DPI context并恢复，得到623x481/615x471逻辑截图，目标应用未改DPI。目标逻辑96
DPI，非物理输入/桌面抓屏、多DPI矩阵。没有PIL，直接检查原生BMP，没有加依赖。

4按钮的WM_GETDLGCODE为0x20a0，BM_SETSTATE/BM_GETSTATE均0->4->0，按下状态立即
恢复，无点击业务事件。13文本框WM_GETDLGCODE为0x89、EM_GETSEL为0/0、行数1。
这些是复用标准消息语义的有限正面证据，不等于已验证换肤后的点击、焦点、键盘、
IME、无障碍、剪贴板或应用全部交互。VB窗口过程返回opaque USER token，未误当
可解引用代码地址；类注册者用GCLP_HMODULE取得，不能用空的模块解析谎称钩子丢失。

证据见[精选记录](../etc/evidence/madshow-controls.json)。原日志、截图、探针源码/
DLL均只留scratch，证据提炼后按登记安全回收。154产品/测试/工具输入及assets根/
O:/winnt/system32八文件SHA前后均未变，没有产品编译打包或二进制发布声明。

## 后续候选（未准入实现）

沿用三个组件四产物，在DLL内增加有明确VB6运行库注册模块身份及具体类名的私有
控件分类，统一attachment/control_scope/controls，不能按MadShow文件名或任意
Thunder前缀强行接管。按钮可优先复用现有绘制器，但需真交互/销毁回归；Frame
必须保留原容器标签绘制，再局部适配边框/标题；TextBox需独立解决client-edge的
NC绘制、布局和原编辑语义。PictureBox保留应用客户区绘图。无窗口标签字体/绘制
桥接单列验证范围，不承诺只用WM_SETFONT或MSAA就能完成。后续需真实VB6用例、
经典标准控件双宽回归、原资源保留及完整产品交付；本S未授权这些产品修改。

背景API核对仅用微软原始文档：
[GetWindowLongPtr](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getwindowlongptrw)
明确过程值可能是代表过程的句柄，因此不解引用opaque token；
[RealGetWindowClass](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-realgetwindowclassw)
提供窗口类型查询，本文的VB类返回值来自本机实测。外部检索不是构建/验收依赖，
未复制第三方代码。先前外部博客/PDF抓取失败未当作已读证据。

## 交付和收口

P1 bb59d0a0722dd98f6ce68def51f3774909d2e1bb已提交推送，协调角色实际复查Git
原始diff、请求边界及证据，从GitHub精确提交读取六份变更文档，逐字节匹配Git
blob；154产品/测试/工具输入、两处八个发布文件SHA前后不变，产品/rules差异为空。
没有把这次调研称为产品修复或再次发布。P1原始回执SHA已提炼到精选证据。

build/td-s11-madshow在回收前检查精确工作区解析路径、祖先/子项无reparse、
提升CIM进程核查无存活探针；6个自有实际实例已正常退出且Job严格0。提炼后回收
该唯一登记根，稳定.ninja/.ccache、正式四文件和用户既有实例均保留。
治理自测/差异检查通过，Td S11关闭；M0开放，T14等任务及两项原TODO保持。
QUEUE新增的VB6有窗口控件兼容是未编号、未获实施准入的候选，无窗口标签桥接
不默认列作已支持。P2只记录审查与收口，产品输入及二进制仍不变。
