# M0 T17 S8 authentic VB6 file and directory lists

S8有限实现已通过同批双宽干净构建、包测试、双发布与安装复测，P1实际远端
复核待完成，尚未关闭。T17/M0开放。[证据](../etc/evidence/vb6-file-controls.json)拥有范围。

固定Code Architects树内28带EXE表单只有Photo Album包含文件组，实际仍是VB5。
原vbAccelerator固定归档214个VB6 ZIP的表单/控件静态检查无失败，仅Compress-Z-It
包含FileListBox/DirListBox。作者Benjamin Dowse，原readme明确免费修改/使用；
原EXE/DLL/源码/声明逐字节保留，可选本地运行，不进入产品/必需测试。

原示例Form_Load构造CompressZIt，类初始化只赋两字段。根据公开类型库GUID在
样本目录内提供私有免注册COM清单，实际加载原CompressZItLib6.dll；两处32位
CLSID注册键均不存在，无安装/注册/系统设置更改。未调用压缩、文件写入、导航
或zlib路径。正常显示并核实真正MSVBVM60的两控件/原生superclass及DRAWITEM协议。

S7对真实VB6 FileListBox已附着但无System文字，DirListBox被owner-draw保护拒绝。
最小产品修改仅为准确runtime60的这两个角色，复用S7 renderer/精确DRAWITEM DC
作用域，原图像/数据/布局/返回保持；未启用未认证VB6 DriveListBox。原标题、菜单、
字体算法、五原glyph/resources.md、三组件四产物与公开ABI全部保持。

双宽C模型分开提供VB5三类/VB6两类，实际文字、432源色块像素、错误runtime owner、
16次VB6绘制GDI delta0、按键/通知、重入移除/目录重挂和资源清理通过。真实VB6
End/Home及父通知、源字体/几何/选择归还/退出active0通过；x64模型不冒充64位VB。
一次Toolhelp模块快照错误24保留，观察器仅对ERROR_BAD_LENGTH作最多8次立即重试，
记录次数且最终失败仍拒绝，不用Sleep或忽略错误。私有COM路径与最终SysWOW64
运行库路径核实，避免用32位System32别名误哈希64位文件。

干净all双宽编译11.78/14.44秒（Ninja8/ccache），23项首轮22通过/23.48秒；VB5
Photo Album测试目录漏放其*.bmp过滤器所需BMP、列表为空，补齐自有BMP后仅复测
两对照通过，无产品改动。双宽模型及真实VB6三旧包负例均在实际字体谓词失败。
demo保留即时相等谓词，新增warm/final为裸103->103、样式91->91；S7偶发原因
仍未定，列TODO交后续观察/最后审计，不将串行通过称为已修复。

四文件同批发布assets根和O:/winnt/system32，八SHA一致，安装双宽ABI/双版本
模型/真实VB5及VB6共8项通过。原生正常显示PNG保留，标题top26对S7差异0。
真实VB6 DriveListBox、过滤/多选/属性/路径变化变体与其余148全集仍交后续S；
源/测试/CMake增65删25净40，无移动删除，应用状态所有者不变。T17不能由本S关闭。
