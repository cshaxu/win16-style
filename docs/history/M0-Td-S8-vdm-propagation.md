# M0 Td S8: native propagation diagnosis

## Scope and owner feedback

基线944ef26；准入时无活动包/未提交改动。只读兄弟仓库与本机安装，不将其变成
本项目构建/验收依赖。M0固定，T8保持开放。所有复现新进程位于自有job/private
桌面及独立AuthenticationId，未关闭/重启/注入用户已有NTSRV/VDM。只读用户进程
快照另列，不等同自有复现。原始诊断摘要与精选状态见[清单](../etc/evidence/vdm-propagation-diagnosis.json)。

用户后续明确接受“复用没有run16s钩子的NTSRV不传播”；它是接受的边界，不为此
修改服务协议。用户先报告正常冷启动仍无效果，后报告PE winmine经NTVWM成功，
最后报告本次NE启动也成功。**原NE间歇失败未稳定复现，未确定根因，不宣称已修复。**

## Isolation and versions

LogonUser NEW_CREDENTIALS及CreateProcessWithLogon NETCREDENTIALS_ONLY建立新的
本地登录身份（不创建账户，不执行网络登录），RPC端点使用session+AuthenticationId，
与用户端点不同。端点初始1715/not-listening或1722/unavailable才开始。先用不具备
指派令牌权限的CreateProcessWithToken失败1314，随后改用WithLogon；受限环境
LogonUser失败5，取得执行权限后成功。job kill-on-close仅拥有本次树，70秒外部预算。

最终release矩阵实际使用O:/winnt/system32四文件，SHA与assets及本项目release.json一致。
兄弟仓库当时HEAD e787f1a，但有其他任务未提交修改；未修改任何兄弟文件。
探索前run16为313344字节/c482f54b…，另一任务20:14:03更新为313856字节/
e7ac4e89e9d1e88d57d7cea1a1ad239deead1537a58c8537820bb504c1739121。
初步cold5早于此更新，不混作固定输入矩阵。最终release/history对照在更新之后，
均使用同一后端；完整SHA在清单。不冒称工作树源码与发布二进制逐项匹配。

## Final finite matrix

| 场景 | 实际创建/状态及结果 |
| --- | --- |
| release-cold | run16s -> run16 -> NTSRV -> ntvdm；64->32 rundll32 helper安装成功。run16/NTSRV/ntvdm均flags3、committed1、25 hooks；ntvdm style_ready1。 |
| release-service | 自有裸NTSRV；包装后的run16已注入，但复用RPC服务，没有创建NTSRV子进程。旧服务创建的新ntvdm无ntw DLL；按用户接受边界记录。 |
| release-reuse-vdm | 先裸启动自有VDM/Minesweeper HWND，再包装命令；run16注入成功，无新VDM，继续同一个HWND且无ntw模块。不能用注入NE文件解决。 |
| history-cold/service | 固定本仓库5b5ee58四文件（T6 S5）在相同当前后端下：冷态附着成功、裸服务复用无样式；未发现最近菜单/字体版本导致此边界改变。不是过去后端/用户会话的重演。 |
| release-cold-observe | 不设置任何软件断点的控制仍flags3/committed1/25 hooks、HWND双回调登记；仍属于debug进程，不能冒称无调试器。 |
| normal-cold | **无DEBUG_PROCESS**，具名job严格限定所有权；恢复本次新建的最小化窗口并等4秒，ntvdm/目标HWND正常237x240原生PrintWindow明确显示Win16标题/菜单；25跳转全部匹配。 |
| user-readonly | 用户当时NTSRV、ntvdm同用户原登录身份；两DLL路径SHA为当前system32，均flags3/committed1/25 hooks，ntvdm ready1；目标HWND双回调登记。未操作该窗口。随后用户报告成功。 |

每层创建者/PID/位宽/实际映像、匹配DLL绝对路径及SHA、API断点与状态已在本地
原始诊断核实；公开清单仅保留脱敏结果和SHA，不公开用户进程/登录身份或运行路径。
helper DLL被动加载flags0/committed0是正确helper状态，不误判传播初始化成功。
目标线程的CC32SubclassInfo里精确callback/id/data对与当前DLL符号对照，区别于仅
模块存在；这不是跨进程直接调用线程亲和的Ntw16sWindowAttached。初始窗口有些
是iconic，故初始登记不能证明完整绘制；最终补无调试器恢复后截图，视觉检查成功。

冷态实际命中run16/NTSRV的propagate_W、WOW32调用的style_window_A，随后进入
USER32 -> win32u NtUserCreateWindowEx；所有25个跳转匹配本DLL已记录replacement，
该NE树没有加载nthook DLL，未发现后装覆盖。WOW32确有其他私有counted创建路径，
本次Minesweeper证据不支持把它当首个断点；也不宣称所有WOW创建API都已覆盖。
初次尝试的早断点复制进trampoline、debugger内同步RPC/PrintWindow阻塞及iconic
等待等问题均修正/丢弃，清单逐项标明，不归因于产品或记作成功。

## Independent dual-hook defect and minimal fix

用户询问nthook/ntw16s竞争，另做自有纯C GUI fixture：经NTSRV/NTVWM进入native
父进程，两DLL均加载、NthookContextFlags=1、窗口Ntw16sWindowAttached=TRUE；
但A/W子创建在64->64及64->32均返回193，父进程回收未交付子进程，未影响用户树。
它是**独立的双钩子兼容缺陷，不是原NE间歇失败已证实的根因**。

stage-proof3读取实际挂起子进程：PE有效（MZ/PE，
machine8664，optional sizef0/magic20b），SizeOfImage=0x1c000，import_rva=0x20000。
导入表处MEM_COMMIT/MEM_PRIVATE/PAGE_READWRITE，描述符可读，随后本项目
ntw16s_stage_process实际返回193。nthook先安装Detours重定位表；本项目staging.c
仍限定描述符必须落在SizeOfImage之内，拒绝有效外部表。两层调用链并非互相
抹除：父创建进入本项目hook并已由nthook准备导入，错误发生在随后staging读取。

最小方案仅修本项目描述符读取：允许映像外、可读、地址不溢出的现有RVA表，保留
1024项/终止符/远程权限/格式检查；不禁用nthook，不替换CreateProcess trampoline，
不改服务协议、全局hook、现有进程或UI。须独立准入新产品任务，双宽自包含负例/
真实初始化与包测试，再做可选只读后端兼容实验；不能把本诊断当修复发布。

## Verification and handoff

当前126产品输入/四文件/原资源未改，无新产品构建或部署。治理SelfTest及diff检查
通过。P1 ce20f1519ae1d0cb1da7024642eb1d6e55ca7385已推送并fetch核对，实际读取远端
diff/脱敏清单/请求复核通过，126产品输入未变。P2关闭诊断S，准入T9 S1最小兼容
修复；精确scratch build/td-s8-vdm及清理责任由其接收，先提炼后安全回收，不清理用户目录。
本次原NE失败仍有未覆盖：用户失败瞬间的完整创建链、过去后端二进制及当时状态；
可选完整192DPI/物理输入不在本诊断范围，不虚构通过。

公开证据经脱敏：自动审批曾拒绝含用户进程与登录身份/路径的初始提交推送命令，
该命令未执行。完整诊断留在登记scratch，公开只含技术结论、非身份字段和SHA，
不通过其他通道绕过拒绝。
