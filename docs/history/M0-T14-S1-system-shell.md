# M0 T14 S1：旧程序的原生Windows启动错误体验

状态：P1实际远端复核通过，S1及T14收口，M0开放；M0固定，准入97999c0工作树干净。
原请求Win+R的run16s sol失败、cmd正常；用户随后要求Windows自己的错误对话框，
保留整个系统用户体验，run16s尽量透明。此追加要求在同一T14 S1内继续实施。

## 已证实原因及范围

只读原生探针读取Explorer的cwd为C:/Windows/system32，用户现有cmd为D:/programs/misc。
PATH含O:/winnt及其system32。前者解析到O:/winnt/sol.exe（NE，180688字节），后者
先找到cwd的D:/programs/misc/sol.exe（x86 PE，56832字节）。直接在用户目录运行旧
run16s sol得到退出4、校验错误193；尚未进入创建或注入，不能称为钩子失败。
没有改搜索顺序、PATH或静默挑另一个同名程序。用户已接受此诊断；真正需修的是
旧校验只写stderr，从Win+R看不到持久错误界面。对两个映像记录SHA，不把同名当同文件。

## 最小实现及所有权

run16s-exe私有legacy_shell.c/h：仅校验193且完整MZ头、非PE的旧映像分支使用
ShellExecuteEx；STA COM、NOASYNC/NOCLOSEPROCESS，不设FLAG_NO_UI/NOZONECHECKS，
不自制MessageBox。传同一个绝对目标和原参数尾，不替换verb/cwd。Shell持有的
进程句柄返回后关闭；COM只在本层成功初始化时配对释放，文件句柄有界关闭。
关闭系统UI后仍保留原校验193/退出4，Shell取消1223不覆盖原始失败。

普通PE与错误架构/包故障不从Shell裸运行，原挂起创建/注入/恢复及740提权自身路径
保持；elevation.c/h字节不变。源有TOCTOU的一般边界未在本S扩张为可原子保证。
系统若配置了legacy执行代理，其行为由Windows负责，不宣称代理或NE有样式支持。
没有任意URI/文件关联/App Paths/现代应用透明支持的承诺；未覆盖错误仍按既有诊断。
三组件四产物、Console等待、ABI3及31命名导出不变。产品统一0.14.0.0。
所有renderer源、字体、菜单、滚动条及原资源/声明与97999c0原字节相同，DLL版本RC除外。

参考微软[ShellExecuteEx](https://learn.microsoft.com/en-us/windows/win32/api/shellapi/nf-shellapi-shellexecuteexw)
及[结构/标志](https://learn.microsoft.com/en-us/windows/win32/api/shellapi/ns-shellapi-shellexecuteinfow)。
没有复制第三方实现或引入依赖。

## 原生验证与失败处置

新launch/shell-unit双宽直接编译生产helper，8分类（MZ/NE/LE、标准/重叠PE签名、截断、非MZ、缺失），
5次Shell请求检查原始参数/标志/1223/5/零错误转31/句柄稳定；COM缓存预热后严格计数。
新launch/shell-errors仅x64，对合成MZ/NE逐个比较直接Windows Shell与实际候选CLI，
验证原生Shell_Dialog或#32770类、同一系统语言标题、原样显示、关闭及返回码。
自有私有桌面，CLI实例先挂起纳入kill-on-close Job再恢复，有限等待、全Job计数归零；
不使用用户sol/后端作为核心测试依赖，不关闭用户应用，不使用computer-use。

本机实际窗口为Shell_Dialog，This app cant run on your PC；自有原型调用栈含shell32
SHExecuteErrorMessageBox，确认来自Windows，不是本项目自制提示。原型最初误只认
#32770而漏掉系统模态窗口，出现限时未定；主STA加独立桌面观察后找到真实窗口。
残留的两个自有原型按确切PID/路径/创建时间核实后回收，串行重测通过，避免模态冲突。
旧run16s被新fixture检出：直接Shell出现系统UI，旧CLI退出4却无UI，测试退出1。
初次旧版负例传相对路径只检出测试路径错误；改绝对路径后才作为有效缺陷负例。

其他探索失败：scratch探针UNICODE重复/版本声明/头常量警告已修正；通用sol三用例
原型早期退出断言不符，不计通过；不会将其称为真实sol的完整运行复验。
Build-Core首次误给x86配置指定不存在的x64 run16s目标，改all后通过；来源门禁
检出已改main/RC及README回执陈旧，更新精确授权路径回执后通过，没有放宽门禁。
最终冻结注册x86 28/x64 44，共72，不冒充全72项运行；最终实际包和两宽新单元分列。
物理Win+R按键、任意语言/系统版本/显示器及本轮真实安全桌面UAC同意操作未计覆盖。

## 交付

最终s1-release独立冻结153输入，GCC16.1/Ninja/ccache干净双宽构建；
实际包24/24通过，无失败或跳过，4槽并行250.35秒，两宽冻结新单元各1通过。
72注册项未全运行；前轮guarded清理观测失败不计通过，重做完整最终包。
四文件0.14.0.0同批双发布，八摘要等于测试包，八备份等于97999c0资产。
RM预检/发布均0占用，本S发布没有关闭用户进程；已部署两宽DLL ABI3/31通过。
实际system32 run16s.exe又通过MZ/NE直接Shell对照及整树清理，未改图形/字体。
来源/资源/治理自测通过；当前[发布清单](../etc/release.json)及
[精选证据](../etc/evidence/system-shell-launch.json)记录摘要、统计与未覆盖范围。
scratch为build/t14-launch与build/t14-package；证据提炼后已安全回收，稳定Ninja/ccache保留。

最终审查补充e_lfanew小于DOS头的PE签名排除，避免被误判成DOS后交Shell；
新增对应单元，使用新冻结候选重做实际包，前轮包只作探索不发布。

前轮guarded包系统UI类/标题/退出码对照通过，但新测试在root退出后立即查询Job
计数断言失败。没有剩余run16s实例；测试改为在创建前关联IOCP，root退出后有限
等待JOB_OBJECT_MSG_ACTIVE_PROCESS_ZERO，再严格查询整个Job计数0，不放宽断言。
这是本新测试的回收同步修订，不据此关闭TODO中原T11的独立回滚计数债务。

代码统计范围CMakeLists/src/tests/tools，12路径+210/-15，净+195；不修改或
移动旧renderer状态所有者，无新公开ABI，用户及其他任务状态保留。

## 协调复核与任务收口

P1 91bba9f80d2687ab058ecf7290c354794d35566b已推送，并实际从GitHub精确提交
读取168文件：153输入、四产物及变更说明/规范；逐项SHA通过，八部署摘要匹配。
实际diff确认renderer除版本RC、UAC源及发布规范保持。对照原始请求及追加的
系统UI要求：没有静默换目标，没有自制提示，没有原生PE裸启动路径；旧NE本身
仍不支持注入，系统拒绝体验已恢复。没有扩张为所有Shell激活/关联兼容。

T14本轮只有S1，根因定位、最小系统UI修复、原生正负例、包/发布/远端及清理
已逐项审计。S1/T14关闭，M0开放，其他关闭/取消状态及独立TODO不变。
清理核对精确绝对路径、祖先/子项无reparse、无存活映像，提炼文件/字节及远端
下载回执SHA后删除两登记目录。P2仅更新收口文档，153输入及八发布文件未变。
