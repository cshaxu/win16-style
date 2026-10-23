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

## P1推送后实际复核

b044345770c4ddabbb6f4f288fef1e7ad92807e6已推送，HEAD与origin/main一致。
切换协调复核角色读取实际commit/stat及release/CURRENT差异：7路径、+137/-34，
净+103；四二进制大小不变、哈希为本S新批次，无生产源码或规则变化。
从origin/main实际取回四文件并计算SHA，与发布目录、原始13项被测包完全一致；
逐一取回121输入，按声明的LICENSE换行约定全部匹配。再次读取双宽原始JUnit，
24/35注册全集、57通过/2剪贴板跳过/0失败与发布清单一致；13个同包退出码均为0。
git show --check通过，交付后工作区干净。P1自动验收交付复核通过，S2仍active。

已向用户提供assets/binary精确发布目录及两种启动方式，请其反馈真实鼠标/键盘、
剪贴板、焦点/捕获、菜单、窗口操作与显示缩放/DPI结果。依据package-acceptance
的“必要物理验收完成”条件等待反馈，不提前关闭S2或准入S3；所有自动worker已完成。

## S2 P3：系统cmd中转传播补验

用户询问GUI启动器经cmd继续传播的可行性。检查当前CreateProcessA/W拦截确认
传播状态由目标DLL所有，与启动器PE子系统分离；当前构建及包检查仍固定Console。
本轮不擅自把可行性讨论改成GUI/标准流行为实现，先对当前真实发布包补验cmd路径。

有限集：系统SysWOW64/cmd.exe与System32/cmd.exe各自启动32/64位propagate-target，
再创建相反位宽、再回初始位宽；每链初始A/W各一例并切换调用者挂起。总8例、
24个后代目标窗口。使用s2-final干净树已构建的现有propagate_target.c，未改fixture。
每目标原有断言检查对应DLL在内存、Ntw16sWindowAttached、Win16 System20字体、
复选框数据语义、销毁及不存在目标的LastError。外层读取三代标记，核对entry、
window-attached和complete各一次及两条child PID准确连接；不只检查退出码。

复现模板（替换为实际绝对路径；R与L为初始位宽fixture，N为相反位宽fixture）：

```text
run16s.exe --wait <system-cmd.exe> /d /s /c ""<R>" 2 "<N>" "<L>" <A/W:0|1> <suspend:0|1> <case-id>"
```

工作目录与WIN16STYLE_CAPTURE_DIR设为各例独立目录；根使用CREATE_NO_WINDOW，
隐藏worker7656总预算300秒、单例60秒，仅超时时终止其自有树。确切命令、子进程ID、
标记及stdout/stderr位于已登记s2-cmd-chain scratch，完成后result为passed、8/8。

| cmd位宽 | 目标三代位宽 | A/W与挂起组合 | 结果 |
| --- | --- | --- | --- |
| 32 | 32 → 64 → 32 | 0/0与1/1 | 两例通过 |
| 32 | 64 → 32 → 64 | 0/0与1/1 | 两例通过 |
| 64 | 32 → 64 → 32 | 0/0与1/1 | 两例通过 |
| 64 | 64 → 32 → 64 | 0/0与1/1 | 两例通过 |

测试前后发布四文件SHA一致，完全等于S2 P1清单；没有重编译、替换产品或新S关闭，
本P仅补充同一S现有发布批次的实际证据。src/tests/tools/CMake变化0路径、+0/-0；
三份验收文档/清单更新，规则和sibling无改动。GUI版启动器仍未实现/验证，
交互式cmd、ShellExecute/提权转交和conhost/Terminal外观不在本次通过声明内。
物理输入/DPI及剪贴板验收仍待用户，S2保持active，不能以本补验替代。

## 用户外观验收否决与原资源恢复（P4进行中）

用户指出左右标题栏按钮与ntvdm64不同，并明确“不得替换图形资源，必须使用相同的”。
这不是验收通过：此前自动测试证明注入/生命周期，却未证明资源符合原始拆分目标。
根因是T2曾把五头换为generate_glyphs.py自行画的近似图，原始标题按钮阴影/箭头/
系统框不同；绘制主体win16_style.c/controls.c仍来自来源，layout.c与固定源无差异。
恢复必须覆盖五头全部18表、共8688源像素及原resources.md，不只修被指出的标题按钮。

本次明确用户要求覆盖历史资源替代方案；保留原声明，不把资源重新许可或标成原创。
从5b0b65f66c281050aa4ce1454c6cd31441a6d484读取固定Git对象，逐文件核对原blob/size/SHA，
五头及原说明原样写入本仓库，未读取或修改ntvdm64脏工作区。清单retain、119回执；
删除自绘生成器，独立本地固定摘要检查拒绝6种字节替换；来源自测23及第三方9通过。
两DLL内嵌原说明RCDATA5，包门禁核对18个实际编译表；新批次构建及运行尚待验证。
有限验证为原字节/消费者差异审计、两DLL实际表、双宽完整矩阵、新四文件13项同包，
以及标题正常/按下/系统框与demo视觉对照。任一失败先修复，不复用旧包通过结论。

### P4原资源新包实际验证及发布

s2-original-art worker17348干净双宽构建、ABI/PE/声明/依赖及来源门禁通过；
18个原图形表逐一在每个DLL中找到，完整原说明RCDATA5保留。人工查看同包demo
实际PrintWindow：系统框、右上箭头、勾选/单选/下拉/滚动均已使用原资源；同文件
原生/注入模块与业务对照通过。截图是私有桌面观察，不宣称用户视觉或物理验收。
二进制负例分别改x86 frame_buttons和x64 system_menu_bitmap一个单元，两例均被
新包门禁明确拒绝，原始候选不改动。六源文件字节变更负例及23/9门禁同时通过。

Test-Package -RunName s2-original-art-test -FullIntegration worker16760最终13/13通过。
完整矩阵worker13288顺序运行原构建树CTest，没有重新编译/链接；x86注册24，
23通过/1剪贴板权限跳过，114.56秒；x64注册35，34通过/1相同跳过，355.08秒。
57通过/2跳过/0失败，原始JUnit与每例标记已核对，跳过的原生错误5/返回77不计通过。
复现对每架构运行ctest --test-dir <clean>/build/.ninja/<arch>
-R '^(core|launch|propagate|demo)/.*$' --verbose --output-on-failure --no-tests=error
--output-junit <evidence>/results.xml，WIN16STYLE_CAPTURE_DIR设为各架构独立证据目录。
最后cmd链worker30468使用新包及新构建fixture，8/8、24个后代目标窗口/字体/控件/
父子PID匹配。13项前后、完整矩阵后、cmd链后、发布复制后的四文件SHA全部一致。
122个src/tests/CMake/LICENSE输入与干净快照和工作区匹配，LICENSE检出换行约定保持。

| 新发布文件 | SHA-256 |
| --- | --- |
| run16s.exe | f7b77cda8fa9528a425a869155d4a704588f01214941a1161117013727d75d67 |
| ntw16s32.dll | 592de00c333530f5d14b4d9e13efac206015a5cc861c6f5b398c7d3953706a0b |
| ntw16s64.dll | 8b5c8c8d86992c99f938492d7db0dae02d515f6a1fcbd54933515730dc129e22 |
| demo.exe | ba8ab0120b4f3d904715c8df8e80707960c02583d6cef1baa4f4bc575b6b4222 |

代码/构建统计范围src/tests/tools/CMake：14路径、+819/-573，净+246；
主要为原样资源/原说明恢复、生成器退休和身份门禁；窗口状态/创建事务所有者不变。
规则及sibling未改动。发布四文件已更新；推送及实际提交复核后才称本P交付完成。
用户旧版视觉否决保留，新版仍待视觉/物理反馈，S2及M0不关闭。
