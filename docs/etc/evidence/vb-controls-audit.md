# T17 S1：VB5/VB6 控件全貌审计

当前为目录/源码审计，未新增产品适配。权威矩阵为[vb-controls-audit.json](vb-controls-audit.json)：
148个版本/控件单位，其中52个内建/宿主行、96个扩展宿主行；每行列关键变体、
模块/已知SHA、已有覆盖、真实样本、四层初始化/绘制状态及后续接收者。
缺样本不能计通过，目录包含不等于当前支持，未逐个实例化所有148项。

## 目录依据与边界

分别核对Microsoft原作Q173352（VB5）和Q194784（VB6）的历史档案转存：
[VB5目录](https://jeffpar.github.io/kbarchive/kb/173/Q173352/)、
[VB6目录](https://jeffpar.github.io/kbarchive/kb/194/Q194784/)。原作者是微软，当前
传输副本由KB档案维护；不把它们称现行微软托管页面，也不复制全文。官方VB6
资源中心仍引用Q194784，但当前官方URL404；档案补足该目录缺口。

两篇分别提供edition/SP目录，不能从VB6反推VB5。微软把Menu列在21项内建控件内；
本矩阵为20个工具箱对象加Menu及Form/MDIForm/UserControl/UserDocument/PropertyPage
五宿主，共26类，VB5/VB6分列52行。扩展按35个被盘点模块族、66个控件族列明，
分宿主后96行；包括MSWLess九种真正无窗口控件及旧Grid/Outline/Chart。
35模块族是盘点分类，不等于所有模块厂商/协议已验证；MCIWnd/MedView/SchGrid等
仍须用实际二进制确认所有者。厂商随盘控件、IDE wizard/repository及IE平台组件
逐项单列处置，不混成已支持微软原生控件，也不静默遗漏。任意第三方、MSForms/
VBA、VB.NET、VB4/16bit及应用图像/曲线内容替换不在本T默认实现承诺。

## 本机与原生证据

当前源码精确认识MSVBVM60注册的CommandButton/TextBox/Frame。VB5类所有者和文字
调用者均没有接入。Actor真实CheckBox被分类拒绝；native superclass/公共控件成功
不能直接推导OCX包装层成功。tests/ntw16s-dll/vb_runtime.c是仿真运行库，只注册
三个类，不能冒充真实VB运行库。当前32位VB包装层与x64 native fixture明确区分。

本机VB6 6.00.9848的完整SHA/路径见矩阵。VB5官方redist只下载到任务scratch，
未运行安装器；从内嵌CAB只提取MSVBVM50.DLL，版本05.02.8244，x86，SHA记录。
纯C32位LoadTypeLibEx(REGKIND_NONE)：VB6资源1/3为29/96类型，VB5资源1/2为26/67。
这些主要为公共运行库接口和枚举，不是完整内建控件的COM目录；LoadTypeLib成功
不证明类注册、接入初始化或实际样式绘制。没有注册OCX、安装IDE、读私有VB对象
布局、改全局设置，也没有把微软二进制加入Git、产品或核心测试依赖。

限定D:/programs/misc与O:/winnt顶层EXE及misc内OCX共32个文件，只发现Actor/
MadShow/ChipGenius的VB6字符串候选，不能据此宣称导入或运行。Actor/MadShow已有
真实原生记录单独引用，不冒称本S新运行。每个扩展模块的限定查找结果与缺口列明，
不把检查SysWOW64/misc未找到称为全机不存在。没有运行ChipGenius硬件检测。

## 分S交接与完成谓词

S1冻结目录和源码断点；S2扩充64位纯C native demo对应项、状态与交互，不载入
32位VB/OCX，不自建renderer或伪装真实VB。S3先CheckBox/OptionButton及VB5接入，
然后按编辑列表、文件滚动、容器/无窗口、微软扩展等连续分多个S，不预占新编号。
图像/线形/绘图和嵌入对象的应用内容保持；Timer等非视觉对象不虚构换肤效果。
最后独立收口S逐行核对实际通过/有依据原生保留/阻断，未完成必测不能称T完成。

真实VB5应用、各OCX对象、图形/owner-draw变体、实际class owner/事件/绘制与生命周期
仍缺验证；S1已识别并交给后续实施S，不算通过，也不因native demo存在就消除。
官方common-controls更新包有EULA资格条件，未确认前不执行或当任意可分发素材。

本S不改变产品197输入、公开ABI、原五头/声明、既有标题字体菜单滚动条或发布四文件。
按用户要求短测试，审计只读探针和基线构建/短包结果单列，不跑旧长矩阵。
