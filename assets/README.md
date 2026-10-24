# Product package

当前批次以[发布清单](../docs/etc/release.json)为准：M0 T8 S1 P1 / 0.8.0.0。
三组件四产物：run16s.exe(x64)、ntw16s32.dll(x86)、ntw16s64.dll(x64)、demo.exe(x64)。
README不是第五产品。将四文件放在同一目录，启动器从自身目录定位DLL。

本次修复字体与原生布局不匹配：标准控件保留应用原字体、字号/字重和后续
WM_SETFONT；标题10pt粗体、菜单8pt常规MS Sans Serif，按窗口DPI一次换算。
不强制统一20像素粗体、不重排应用控件，不干预程序自绘Score/Time等文字。
公开font_acquire/release兼容API仍保留，其20px显式字体不是renderer默认字体。

原资源五头及声明逐字节保留，两DLL各18图形表；菜单取消及17逻辑像素滚动条
修复保留，端钮/固定滑块仍按横截面保持正方形。启动器保持Console：控制台目标
默认等待并返回实际退出码，GUI默认异步，--wait均等待，样式和后代传播默认启用。

本批构建/原生探针/实际包/双发布及远端复核身份见发布清单及
[字体修复记录](../docs/history/M0-T8-S1-font-layout.md)。原生探针不等同物理输入，
本次用户实际sol.exe外观验收待反馈。每S有binary change按
[执行规范](../docs/rules/EXECUTION.md)发布同批被测四文件至assets根目录和
O:/winnt/system32，先备份后逐项核对SHA；入口见[BUILD](../docs/etc/operations/BUILD.md)。
