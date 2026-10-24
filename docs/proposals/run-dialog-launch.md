# Win+R与cmd启动目标差异

用户报告Win+R的run16s sol失败、cmd内正常。准入M0 T14 S1，先用原生探针
区分实际映像、工作目录/环境、解析/PE/创建/安装/窗口状态，再按证据最小修复。
不能假定同名sol总是同一文件，不能将NE自动转换为原生PE或注入既有进程。

有限覆盖：读取Explorer的PATH/cwd（不输出其他环境秘密）；自有实例重放其启动
环境，比较cmd启动条件，记录退出码/原始诊断/绝对映像及文件类型。新原生回归
覆盖实际故障路径及阴性条件，复用当前CLI/双宽注入/实际包回归。若需产品修改，
三组件四产物/Console/样式和资源保持，版本0.14；干净双宽构建、实际包测试、
assets根与system32双发布、摘要核对、提交推送及实际远端复核后收口。

不修改Explorer/系统环境、不全局hook、不结束现有应用来复现；权限/不支持文件
保留错误证据。若根因仅为两个不同的同名程序，明确报告并避免静默改选目标。

## 用户明确的系统体验要求与已验证方案

用户要求原来的整个用户体验、run16s尽量透明；不要自制MessageBox，MZ/NE要
真正系统错误，UAC要系统UAC。原型已在隔离桌面证明ShellExecuteEx创建Windows
Shell_Dialog，标题This app cant run on your PC；调用栈包含shell32的
SHExecuteErrorMessageBox。关闭后Shell返回1223，不应覆盖原校验193/退出4。
先前原型误只匹配#32770，漏掉实际系统模态窗口；失败不记通过，已核实并回收自有实例。

本S最小改动仅为完整MZ头但非PE的旧映像拒绝分支调用ShellExecuteEx（STA、NOASYNC、
不带FLAG_NO_UI），传绝对目标及原参数尾；普通PE/错误位宽/包错误不裸运行。
系统若安装了执行代理，Shell行为由系统负责，不把它宣称有样式支持。现有740提权自身
再注入路径保持；没有第五产物/公开ABI变更。新纯C原生回归逐个比较系统直接Shell与
实际CLI的MZ/NE对话框类/标题、返回码及零子进程，旧包必须检出；复用既有UAC/等待/
双宽传播与23包测。无承诺所有Shell文件关联/URI/App Paths或现代应用均透明支持。

依据：[ShellExecuteEx](https://learn.microsoft.com/en-us/windows/win32/api/shellapi/nf-shellapi-shellexecuteexw)
与[SHELLEXECUTEINFO](https://learn.microsoft.com/en-us/windows/win32/api/shellapi/ns-shellapi-shellexecuteinfow)。
