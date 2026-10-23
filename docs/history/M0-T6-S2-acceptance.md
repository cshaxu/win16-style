# M0 T6 S2：最终快照验收（进行中）

S1 1bc0440四产物已发布并推送复核，edcdde8记录关闭及S2自动准入。
S2不复用S1旧包；从当前独立源码快照重新构建。scratch仅限CURRENT登记的
build/t6-package/s2-final、s2-package-test及前者复制树内部矩阵目录。

## 有限覆盖全集

以下八单位冻结自导入总览，不以增加无关测试扩张任务：

| 单位 | 本S实际验证与完成谓词 |
| --- | --- |
| 构建/包 | 新干净副本双宽全构建；恰四文件，PE/ABI/版本/哈希/完整声明；测试前后身份相同 |
| 首次注入 | actual candidate自动附着及CLI失败集；目标双宽真实模块与窗口 |
| 后代 | 四位宽边、A/W、第三代；同包propagation |
| 创建语义 | 64原生/注入字段对照，调用者挂起及四回滚场景；长路径双宽 |
| 窗口生命周期 | 双宽完整core矩阵，原生/样式资源回收、异常、线程、DLL及失败边界 |
| 外观/输入 | 双宽core及实际包dialogs/messages；剪贴板跳过单独处置；物理输入/DPI待实际操作 |
| demo对照 | 同一候选demo直接/注入模块、外观、业务及静态边界；真实操作另记 |
| 独立性 | snapshot/Ninja依赖仅复制树和工具链；OS imports，无sibling、服务或额外产品 |

## 执行与未完成项

Build-Package -RunName s2-final worker7748：已通过干净双宽全编译、ABI及
来源/几何/治理门禁和四文件PE/声明/输入依赖核验。此时不等于完整矩阵通过。
复制树Build-Core -RunName s2-full -TaskRoot t6-package
-TestPattern '^(core|launch|propagate|demo)/.*$' -TimeoutSeconds 600，worker20908，
两架构顺序执行，终态及逐例JUnit待核对。若重新链接，候选须再次从这同一快照的
最终构建输出取四文件并重新核验身份，再执行同包测试，不能混用先前候选。

必要真实验收由package-acceptance要求，当前未完成。完成自动检查后向用户提供
最终精确候选，实测同一demo两种启动：输入文字并Apply显示Applied，Tab/Shift+Tab
与快捷键焦点移动，勾选和单选状态，列表及下拉选择、滚动拖动/捕获，File/Help菜单
及About对话框，标题拖动、缩放、最小化/最大化/恢复；记录实际显示缩放/DPI及字体。
私有桌面、合成消息与PrintWindow不能算作上述物理验收。
共享剪贴板测试有显式opt-in参数且会覆盖用户剪贴板，不自动启用；权限不足的77
始终是跳过，不能写成通过。最终发布、提交推送及复核尚未进行，S2保持active。

## 完整自动矩阵终态

worker20908已完成，result为passed-with-skips而非无条件通过。
x86注册24：23通过、0失败、1跳过，115.08秒；x64注册35：34通过、0失败、
1跳过，381.19秒。两项均为core/edit-clipboard，原生CreateWindowStation拒绝访问5，
返回77，明确未碰共享剪贴板。59个注册项均有JUnit处置，57通过/2跳过，无隐藏失败。
此前GCC首轮的painter导入、demo边界、长路径及x86资源用例问题在此最终快照
完整矩阵中实际通过，不再仅依赖focused旧证据。

全矩阵入口更换toolchain文件后发生重建/链接，故先前package.json为中间批次。
全矩阵完成后，从同一干净快照最终构建输出重新取齐四文件，check_package再次
通过并保存package-final.json；实际同包测试worker4640使用该最终批次，未使用
中间候选。13项终态仍待核对。

## S2 P1自动验收批次

worker4640已完成13/13，所有退出码及必要标记通过；包括64字段、四回滚、
八对话框、六消息框；前后四文件SHA一致，和最终构建输出一致。
原样复制发布到assets/binary后再次校验相等；release.json绑定本S交付、
121输入及快照SHA、工具链、完整矩阵和同包结果，真实交互与两项77明确pending。
本P交付已完成的自动验收批次；S2整体仍active，必要物理验收完成前不关闭。

| 文件 | SHA-256 |
| --- | --- |
| run16s.exe | 8e62dca4a1179512b74d3b0851339e4c6e9e913e476c47b5d8ffc5ef75f86d62 |
| ntw16s32.dll | 04dd38bf413f7d8594d328be57694419671f43ce82e1a6584db4b0b137d1a720 |
| ntw16s64.dll | 54322d928ec581875506fbc51070ddc40cd1e6e7d5577957b9e75d0fd231db59 |
| demo.exe | e5b4b377ccd2d070e270a205bd83a597b4e91a82744c5e93cb18c94e2f03538e |

本S产品源码无修改，121输入与S1一致；新目录与工具链文件配置触发重建，
因此二进制重新生成并使用本S最终批次，不复用S1哈希。代码统计src/tests/tools/
CMake为0路径、+0/-0；交付仅四二进制及验收元数据/记录，无规则或sibling修改。
构建、矩阵、同包和治理均有终态；发布不将两项剪贴板跳过或物理输入升级为通过。
