# M0 Td S6: English README and native screenshot gallery

## Request and scope

用户要求全英文README供GitHub读者检索与查看，允许demo和真实sol.exe截图。
Governance模式准入下一未用Td S6；M0固定，T8开放。原工作区干净，基线
fde1c67b7aa25dbc69e54dcc6215bd1596aba395，产品仍为0.8.0.0/S4 P1。
本S只改README、assets使用入口、过期BUILD指针、精选截图及来源索引和任务记录。
用户公开截图要求覆盖CONTRIBUTING/etc索引禁止提交截图的一般约束，仅这8张
公开展示PNG例外；未修改规则或门禁，原始日志/探针/游戏文件不提交。

根README全英文，覆盖四文件下载、直接/注入启动、等待/传播语义、组件、纯C17/
MinGW GCC/Ninja/ccache构建测试、限制和原声明。根README是读者入口，CURRENT/
release.json继续分别拥有活动状态/发布身份。assets旧S3菜单8pt和BUILD旧T6状态
改为当前权威指针；原资源和许可逐字节保留，没有把第三方内容泛称MIT。

## Native captures

[8图与元数据](../etc/screenshots/README.md)包含demo原生/样式/菜单/About，
XP Solitaire原生/样式/Game菜单/Options。每张均实际运行当前assets包，纯C17
临时控制器使用自有private desktop和kill-on-close job，只操作本次拥有进程；
验证注入DLL的完整assets路径/位宽，直接启动确认没有注入。没有computer-use。
跨进程PrintWindow实际画面，弹层单独PrintWindow并按真实HWND坐标合成；对话框
用原生SetWindowPos居中。BMP到PNG无缩放/修图/生成内容。不是桌面合成器截图。

真实sol来自本机既有XP PE32文件，56832字节，版本5.1.2600.0
(xpclient.010817-1148)，SHA256
`a6fc95a5b288593c9559bd177ec43bf9b30d8a98cf19e82bf5a1ba5600857f04`；
匹配cards.dll 359936字节，SHA256
`0fb81e48d8b45e9995ae1e05fd28c1631dbbcdc71180904c1356bfa2c1ead506`。
未下载，原文件未改，仅scratch临时副本运行，游戏二进制不提交。

最终capture3 exit0，8次SHOWCASE CAPTURE PASS。demo报告144 DPI、862x724；
XP sol为DPI-unaware报告96 DPI，原生601x443、样式593x432。每幅Sol有151528个
绿色棋盘像素；菜单/对话框非空黑像素分别1361/8475/1382/3769。随机牌局和动态
牌背随独立启动不同；Score/Time由应用绘制。原始日志/拍摄器SHA和全部PNG/BMP
SHA保存在[manifest](../etc/screenshots/manifest.json)。

失败及处置：首次编译警告在源码修正，不降低-Werror；capture1把空窗口枚举当
失败，改为有界EnumDesktopWindows就绪等待；capture2直接WM_PRINT没有画出弹层，
视觉检查发现后全部改用跨进程PrintWindow重拍，且居中真实对话框。只公开capture3，
未把失败图或仅主窗截图冒称菜单/对话框成功。8张最终图均逐张人工视觉检查。

## Verification and boundary

- README LINKS/ENGLISH PASS：根/assets/截图README无中文或异常控制字符，局部
  文件链接和根导航锚点均存在；英文alt文字/图注可供搜索和无障碍阅读。
- PNG LOSSLESS PASS：8张完整解压、CRC/尺寸/SHA正确；逐像素RGB与原生BMP完全相同。
- BINARY IDENTITY PASS：126个产品输入逐项SHA与release.json一致；四个assets和
  四个O:/winnt/system32副本与既有发布SHA一致；原资源包含其中，ABI不变。
- Governance PASS，SelfTest PASS（13拒绝例），git diff --check通过。

这是文档Td，不改变产品输入，不重建/混包/部署，不伪造新产品测试。截图只证明图中
实际场景和加载路径，未做完整Solitaire游戏测试、物理键鼠或新的全DPI回归；既有
S4验证边界及T8/TODO继续保留。有限覆盖无剩余README缺项；产品任务不因此关闭。

## Delivery and review

P1：本地verified，待提交推送后读取实际远端diff/PNG/文档/源输入及发布摘要复核。
登记scratch为build/td-s6-readme；原始诊断先提炼，实际复核后检查owned进程和
reparse并安全回收。精选8张PNG和来源元数据持续保留。
