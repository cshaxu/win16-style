# M0 T8 S3：弹出菜单右下厚黑边

P1已验证并双发布，待提交推送和实际远端复核；S3尚未关闭，T8保持开放。基线622e8d9。
用户提供Win3.1/sol对照，指出右/下厚黑线。标题、S2 System字体和六原资源保持。

## 根因与实现

实际原生探针确认两个独立问题：

1. USER另建SysShadow辅助窗口：144 DPI的popup为168x30，辅助窗口同原点173x35，
   向右/下各伸出5像素；GW_OWNER为空，但在同GUI线程z序紧邻该popup之后。
   仅在当前样式scope内，按线程、邻接z序、相同原点及有界扩展几何关联，局部
   阻止该辅助窗口显示；保留USER窗口，不改共享#32768类或全局SPI。缓存重用/
   scope结束/销毁解除适配，回调计数保护对象；隐藏明确SWP_NOACTIVATE。
2. 旧WM_PRINT转发清掉PRF_NONCLIENT，破坏原生打印坐标处理。106x83菜单样例在
   客户区右下留下3像素未绘制条，黑色初始位图显示成粗黑线；洋红哨兵底色证明
   该区域实际未绘制（旧包角点x108/y16为ff00ff）。保留完整原生打印标志，再
   覆盖细框，正常WM_PAINT后也保持细框。实际菜单窗口/客户区/项几何不重排。

[WM_PRINT官方契约](https://learn.microsoft.com/en-us/windows/win32/gdi/wm-print)定义
这些标志控制绘制内容；具体坐标问题由旧/新实际DLL哨兵和PNG对照确认，不声称
已审计所有Windows内部实现。探针NC全边框和客户区角落分开校验，不能只查NC。

## 有限覆盖与失败处置

有限全集：popup关联/外部投影、NC细框及打印客户区完整、重入/销毁/缓存重用、
双宽96/144 DPI各15菜单场景（每宽30）、原生popup前后路径、嵌套/系统菜单、
取消/捕获/焦点/命令、菜单类/SPI不变、标题/字体源身份、原资源及ABI、实际四
文件包/双发布/远端复核。不能关联的辅助窗口不改，不扩大到任意第三方菜单。

搜索：`rg -n 'shadow|border|NCPAINT|32768|Dwm|popup' src/ntw16s-dll/menu_scope.c`，
及tests/ntw16s-dll的menu/frames/terminal命中。生产改动仅menu_scope，
font.c/font.h/controls.c/win16_style.c与S2逐字节相同。原有frame DWM策略保存
读取set-only属性的疑点按边界转TODO，不悄悄修改已确认标题路径。

失败保留：

- 初始DWM思路读取set-only策略报87，且旧popup的NC rendering已false，不能解释
  独立SysShadow，已弃用；最终menu_scope无新增DWM调用。SDK与
  [官方属性定义](https://learn.microsoft.com/en-us/windows/win32/api/dwmapi/ne-dwmapi-dwmwindowattribute)一致。
- 首次ShowWindow隐藏导致x64/DPI96取消超时，旧S2同探针--observe正常；明确
  NOACTIVATE后双DPI完整焦点/取消通过。没有用放松语义断言掩盖。
- NC-only探针和探索包曾14/14通过，但原生PNG仍显示内侧黑条。扩大覆盖后定位
  WM_PRINT标志；尝试按GetMenuBarInfo项范围裁剪的方案未解决且回归，已弃用。
  最终保留原生完整打印标志，不通过缩小客户区、改字体或覆盖内容隐藏缺陷。
- 探索x64 control-present在8轮预热后仍出现USER延迟缓存39/45。最终固定64轮完整
  负载预热；每帧owned-object归零、20次严格无增长及错误路径断言不变，不条件重试。
- s3-final/s3-release只作探索证据，均不发布；最终从s3-flat独立干净源码同批重建。

探针仅在拥有的私有桌面执行，不用computer-use/物理键鼠。动画期间GetWindowDC
可能CLR_INVALID，不把它当黑边证据。NC与客户区使用WM_PRINT；窗口外使用实际
SysShadow可见性/几何证明，不冒称真实桌面合成截图。原生非样式popup在本宿主
可能走DWM且没有SysShadow，如实记录数量0；核对原生前后路径和类/SPI未变。

原生PNG是实际候选DLL的六项菜单样例，非XP sol.exe。旧/新视觉先查看，源图形
资源不替换。正式交付结果、源快照/工具链/四文件SHA、原始证据SHA及统计待最终
测试、发布和实际远端复核后记录。

## 最终验证及双发布

最终s3-flat独立干净双宽全构建及ABI各1/1，126编译输入与当前源码相同。纯C17
GCC16.1.0/Ninja/ccache4.13.6，来源124/294/123，负例23/9/6/13通过。
相关注册全集26/29，剩余CTest24/27与实际包font/menu组合覆盖，最终53通过/2clipboard
权限跳过；实际包14/14。每宽双DPI各15菜单场景/共30，细框和客户区哨兵、SysShadow
隐藏、焦点/取消/嵌套/系统菜单及类/SPI保持均通过。旧S2每宽两个独立负例均exit1：
full模式角点ff00ff未绘制，outline-only模式可见SysShadow。

额外x64非动画原生Window DC在96/144 DPI各5有效样本，角点白色或选择背景均无黑条。
实际包字体保持84控件场景/12组合及GDI43/43/45/45。完整传播contract/dialog/message
矩阵本S未重跑，不借历史结果；两clipboard为宿主CreateWindowStation错误5/77跳过。
临时菜单预览只作六项原生样例，已明确PREVIEW ONLY，不冒充完整矩阵或sol.exe验收。

同批四文件已备份后发布assets根和O:/winnt/system32，8摘要与实际包相同；8份before
与Git622e8d9 assets一致可恢复。版本/工具链、快照、SHA/PE/imports和原始证据SHA
见release.json。统计范围src/tests/tools/CMakeLists：5路径，增加129、删除15、净114。
所有权为GUI线程popup适配节点和借用SysShadow窗口，USER负责native HWND；没有旧路径
移动、资源替换、主标题/字体改动。P1待实际远端复核后再提炼清理及S3收口。
