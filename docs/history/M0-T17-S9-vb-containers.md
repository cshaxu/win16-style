# M0 T17 S9 PictureBox and windowless labels

用户授权自动顺序S及永久提交推送；另明确将Delphi提案/QUEUE调整纳入本提交。
候选不编号、不准入，不改变T17的148单位全集。

## 实际断点与有限实现

真实原作者Alpha Icon Creator VB5/6中的PictureBox分别由MSVBVM50/60注册为
ThunderRT5PictureBox、ThunderRT6PictureBoxDC。旧包DLL与其他控件有效，但
custom child安全门拒绝这两类容器；无窗口Label由runtime直接TextOutA绘制。
精确身份后复用既有被动字体边框/滚动叶节点，paint内只适配runtime直接ANSI
标签调用；不读VB对象内存、不全局扫描。角色/RTL变更及重入移除停止适配。

源HFONT、原生DT_CALCRECT、TabbedTextOutA Print和EXE自绘仍属应用。
真实PhotoAlbum的Print实测API=3/TabbedTextOutA、alignment=0，不能按先前猜测
归因为TA_UPDATECP。整块220x208缓存图对旧包RGB差异0；真实Alpha标题上26行
及1689色logo差异0，正文标签814像素改变。原glyph/声明、公开ABI3/31命名+
ordinal1、35 detours/40 slots及三组件四产物保持。

## 验证与交付

固定来源、命令、四文件SHA和实际包/安装结果由
[机器证据](../etc/evidence/vb-container-controls.json)拥有。
双宽干净all构建使用GCC16.1.0/Ninja8jobs/ccache；33实际包测试全部首轮通过，
26.71秒，demo单独串行。6旧S8包负例全部检出缺口；安装9项3.13秒通过。
assets根及O:/winnt/system32同批四文件八SHA一致。

最终包前的首批也通过33/6/9；随后清理一个未使用的测试头及文档空行并重建。
驱动复用了旧截图目录，CREATE_NEW返回80且启动记录累计，造成一轮无效结果；
已保留记录并改用全新目录，最终包上述33项全通过，不当作产品缺陷或隐去失败。

真实VB均x86，x64是独立纯C协议模型；正常paint字体选择和IAT恢复、源属性/
几何、嵌套承载、图像144px/线16px、点击/焦点/键盘消息、晚创建、错误类所有者、
换字体、回调移除/RTL退出及自有实例active0分开验证。不是物理输入验收。
观测器只改新建自有线程的已验证IAT槽并精确恢复，不替换产品trampoline。
测试见证映射改为卸钩后显式释放，避免系统延迟卸载钩子模块导致映射仍存活。

两份私有字体精确句柄在移除后GetObject均为0，源字体仍有效，六次绘制上界通过。
完整GUI循环总GDI在裸启动与样式实例均跳变，原因未定位；没有用多次warm扩大
阈值使其通过，未宣称全进程零增长，留TODO/后续资源观察及最终审计。

正常显示原生截图保留会话visualizations/win16style-t17-s9，四张PNG已登记摘要。
真实样本/源码/许可及运行库不进入产品或必需测试，保留在审计目录；不安装IDE/
注册OCX，不执行样本文件写入。来源与应用图像不变。没有移动/删除旧src或test，
所有者仍为原runtime/应用及本DLL每节点租约；确切numstat见机器证据。

## 未完成面和关闭条件

当前仅S9有限交付。VB6 DriveListBox、文件变体、图形按钮、其余intrinsic/OCX、
复杂AutoRedraw/语言/DPI仍由后续S及最后148单位独立审计承担。HD Tune提权默认
skip。既有demo GDI偶发未宣称修复。实际远端逐字节读取、产品对象/备份恢复及
安全scratch回收通过后另P记录关闭，T17/M0持续开放。

## P2 实际协调复核和关闭

P1 `7a25f2b2705d9d462dfd603ab79e5be7a8f7b6a5` 已实际推送；读取Git diff、
原始148单位请求和证据后，实际HTTPS取回227文件，与Git cat-file逐字节比较。
208输入/四产物及远端main通过；212产品对象已另存研究恢复目录。八部署SHA、
18原样本/私有runtime及四PNG一致；8原S8备份可由8ce7678复取，8中间批备份
与保留研究包一致。源码/测试/CMake统计482增23删净459，无移动/删除旧路径。

独占映像探针正例确认已映射EXE/DLL返回c0000043、卸载DLL返回00000000；
仅两个登记根共346+22映像检查无占用、reparse0后清理。保留全部真实来源/
运行库/归档/声明/研究及恢复对象、其他任务、稳定Ninja/ccache和PNG。
Delphi提案100行及队列调整确实进入P1，未分配或启动新T。

本S9关闭，按用户自动顺序授权准入S10图形按钮/选择变体；未认证Drive6、
文件变体/复杂无窗口/语言/DPI和未定GDI继续后续S/最终独立148审计，T17/M0开放。
