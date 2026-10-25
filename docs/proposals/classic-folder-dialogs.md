# Win3.x 文件夹选择与公共对话框适配

## 请求与状态

用户反馈：Select Folder窗口的标题、文字和按钮虽已换肤，目录树、彩色Shell图标、
展开箭头及整体导航结构仍为现代外观；希望恢复Win3.x风格的目录选择体验。
依据用户“加入到队列第2位作为一个单独proposal”，登记为M0未编号T候选。
顺序由[QUEUE](../states/QUEUE.md)拥有；仅排队，不准入实施、不分配实现T/S/P，
不重开已关闭的T15，不改变[CURRENT](../states/CURRENT.md)技术基线。

用户截图codex-clipboard-ca7c5e7c-1006-4e65-be58-a69d510a98d5.png为问题线索：
包含Desktop/Gallery/云盘/This PC/Network等节点、现代图标及Make New Folder。
截图不能独自证明调用接口或所属应用；正式准入后用原生探针确认实际来源。
T15树适配明确保留原生图像、展开器、连接线和布局，见
[树适配记录](../history/M0-T15-S8-tree-controls.md)及[UI边界](../design/UI.md)。
本候选补足整个选择界面的适配，不把换字体或更换树图标等同恢复完整体验。

## 目标与架构边界

优先恢复已验证文件夹选择入口的经典布局、目录/磁盘图标、导航和选择反馈，
同时保留应用所需的选择结果与交互语义。文件打开/保存对话框作为相关接口分别
调查、冻结支持矩阵；不把任意自绘文件浏览器或应用内嵌树一并强制替换。

保持三组件四产物：在ntw16s32.dll/ntw16s64.dll内部增加受控对话框兼容模块，
复用现有唯一renderer；不默认导入、分发或替换旧版commdlg.dll/comdlg32.dll，
不新增产品DLL、helper或服务，不将兄弟仓库/原系统二进制变成运行或测试依赖。
纯C17、MinGW GCC、Ninja、ccache；标题、字体、菜单、滚动条及其他控件保持。
原五图形头和来源声明逐字节保留。若需额外经典目录/磁盘图标，先核实原始资源、
版本和许可并按[来源策略](../etc/operations/source-policy.md)准入；不自行发明近似
图标，不盲目替换应用自带图像或修改系统共享图像列表。

## 候选研究与实施顺序

以下为阶段划分，不预分配S编号；正式准入时按实际编号和证据拆分。

1. 原生定位：区分SHBrowseForFolderA/W、GetOpenFileName/GetSaveFileNameA/W、
   IFileDialog/IFileOpenDialog及框架自建界面，记录调用者、位宽、参数、选项、
   回调、目标HWND/类和实际显示；不根据窗口标题、截图或是否有树先判定接口。
2. 原始参考：冻结Win3.x目标版本，以可审查的原始资源/模板和实际显示证据核对
   目录导航、图标、文字、布局、键盘及DPI映射。系统现存old-style公共对话框仅
   作候选机制研究，不能先认定可直接替代现代文件夹选择接口。
3. 最小兼容原型：先为一个真实应用、一类已证实入口形成纯C闭环。验证调用约定、
   同线程模态owner、消息泵/重入、受控初始化及窗口生命周期；不在DllMain中操作
   GUI，不重跑启动bootstrap，不全局hook或扫描注入既有进程。
4. 结果与回调适配：逐项保留初始位置、根目录约束、筛选/选择限制、确认启用状态、
   路径/PIDL或IShellItem结果、内存分配所有者、取消/错误/HRESULT及必要回调。
   文件打开/保存另核对过滤器、多选缓冲区、自定义模板/hook与覆盖确认；不得直接
   清除OFN_EXPLORER/BIF等标志或吞回调来强行换成旧界面。
5. 支持面扩展与回退：明确虚拟Shell节点、云盘、网络/UNC、长路径、Unicode、
   新建文件夹、应用定制及多UI线程的可映射范围。无法忠实保持语义时保留原对话框，
   记录未覆盖原因；不返回伪造路径，不静默缩减应用能力。先说明最小方案再实现。

参考入口（仅说明接口边界，不代表本机原型已通过）：
[Shell文件夹选择](https://learn.microsoft.com/en-us/windows/win32/api/shlobj_core/nf-shlobj_core-shbrowseforfolderw)、
[公共打开/保存对话框](https://learn.microsoft.com/en-us/windows/win32/dlgbox/open-and-save-as-dialog-boxes)、
[old-style与Win3.1的关系](https://devblogs.microsoft.com/oldnewthing/20180917-00/?p=99745)。

## 验证与交付条件

使用纯C原生探针，不使用computer-use，不将整套测试交给用户；仅运行自有隔离
实例，有超时、取消和完整回收。覆盖x86/x64、正常显示后的实际布局/图标/字形、
DPI、键盘导航/焦点、确认/取消、回调顺序、结果身份和内存/窗口/图像资源释放。
双对照同时核实应用收到的真实路径/PIDL/对象及错误状态，不能只检查模块或附着。
新建目录只在自有临时树中测试；不修改用户目录、系统设置或共享Shell宿主。
需要提权的HD Tune真实探针继续按用户要求默认skip，不据此宣称兼容。

提供经典参考与真实应用原生截图、旧包负例、支持/回退/未覆盖矩阵；回归既有
标题/字体/菜单/滚动条和双宽注入传播。每个实施S按[执行规范](../rules/EXECUTION.md)
完成双宽干净构建、同批四文件实际包测试；有binary change时双发布到assets根及
O:/winnt/system32，核对摘要、提交推送并实际远端复核。任务级另审计接口语义和
有限覆盖，不能把某个应用成功扩大为全部公共对话框兼容。
