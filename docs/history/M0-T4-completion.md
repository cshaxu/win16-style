# M0 T4 — descendant propagation and creation semantics

T3关闭于032d9f6，用户授权自动按队列推进。S1 bd619ae、S2 17720ae均已
推送并读取实际提交。S3冻结以下六单位；每单位需本项目真实路径证据，不能
用来源项目已实现、零退出或一次smoke替代。结果补齐后再判断T级完成。

## Frozen coverage

| 单位 | 实际路径和证据 | 完成谓词 |
| --- | --- | --- |
| A/W创建事务 | S1 create_process.cpp，原生trampoline、TLS guard/__finally；完整字段原样转发，仅增加一层挂起 | 无broker/RPC/VDM或旧进程扫描，原生失败错误保存，所有返回路径guard恢复 |
| 四边和第三代 | propagate/basic四种root/child位宽各A/W，第三代切换API及位宽；24个实际应用 | 每代记录父子链、正确模块、真实窗口/控件附着/字体及业务状态 |
| 原生创建语义 | propagate/contract四边×裸/注入×八向量64次；子端读取OS状态 | 引用/空参数/Unicode、环境/cwd、继承/白名单、SI/EX/安全属性、优先级、原挂起与错误一致 |
| 失败及所有权 | propagate/rollback两宽父×A/W；先健康兄弟，再删除对侧DLL，最后新健康子 | 新失败目标job NEW/EXIT匹配且无入口，PI清零/错误2；父兄弟及后续子均保持附着 |
| 首次启动/核心回归 | 两宽全部core/launch/propagate注册；安装器/helper/载荷/资源测试不变 | 真实CLI首次启动、双宽GUI/对话框/消息框、超时、stdio和13项核心均通过 |
| 独立性/来源/卫生 | 87回执、124固定处置/294 includes；实际Ninja依赖、PE/版本/哈希及scratch | 无sibling输入、额外产物或renderer，23/13负例及几何素材一致，日志先提炼再回收 |

细节见[S1](M0-T4-S1-propagation.md)、[S2](M0-T4-S2-contract.md)。
既有renderer继续拥有外观及GUI线程资源；Windows和应用拥有原生字段、数据及
消息循环。传播层只拥有自己新建尚未交付的进程/句柄；安装器借用目标，不恢复
或关闭调用者句柄。跨宽helper只使用捕获的W trampoline及既有10秒事务。

## Finite limits and receivers

本T契约为普通原生CreateProcessA/W。调试创建、其他EX属性（除HANDLE_LIST）、
受保护/提权目标、CreateProcessAsUser/WithToken/WithLogon、ShellExecute及UWP
没有独立矩阵，不声称均已支持；不增加绕过路径或全局设置。命令行包目录仍需
ACP往返或有效短路径，明确失败优于best-fit；任意Unicode包路径不在本轮承诺。
调用者原要求挂起时只证明已暂存，不能通过提前恢复去等待DLL初始化。

嵌套同包三层run16s探针分别启动32/64 CMD，均输出NESTED_OK、返回37；
没有因推测而增加launcher身份分类分支。普通原生失败及安装失败后的同线程
后续创建均重新附着，覆盖guard退出。未改旧源项目或引入第二产品helper。

demo候选接收纯原生x64产品示例及同文件双行为；包候选接收四文件同批发布、
干净独立重建、真实桌面输入、选定DPI/字体及跨任务审计。私有桌面创建和
WM_PRINT证据不升级成物理输入或任意应用兼容性结论。

## Full regression and delivery

s3-full owned PID26632最终exit0：x86 16/16（103.13秒）、x64 24/24（377.05秒）。
两宽各13项core全部执行；x64重新执行三代24应用、64字段向量及四故障回滚，
首次启动八对话框/六消息框、安装失败/10秒helper、CLI错误包/stdio全部通过。
最后只扩展测试，生产二进制没有改变；s3-target-exit PID12708两宽launch/failures通过，x64 10.37秒：
目标在helper回调内先退出37，实际W helper返回ERROR_PROCESS_ABORTED(1067)，
目标确已退出、owned helper被回收、总句柄101/101。原10秒timeout仍10000ms。
本轮生产二进制哈希不变，无须为纯测试扩展重复未受影响的完整GUI矩阵。

两宽Ninja deps各45项，解析相对路径后为28本地头/包含文件及17 MSVC/SDK输入，
没有sibling路径。PE精确31名字/ordinal2..32、NONAME helper1、无forward及额外
private导出；直接imports仍六个系统库。三产物FileVersion M0 T4、数值0.4.0.0。
SHA256：

- run16s.exe (AMD64/PE32+)：d79b72b6beb173f3e51f8d805abcd59dc470d7c0ae5a408dca56f165d21bc2ea
- ntw16s32.dll (I386/PE32)：902de93d3dc909e0371f5ff7863e075c8aa587679575acb0887c1f63792160d0
- ntw16s64.dll (AMD64/PE32+)：2c87a3a32be1b0aa70a6a7c0a1ddbb991fb6ed3aac40a27d4a0d10194ad6ee5e

全T生产/测试/CMake统计命令git diff 032d9f6 --numstat -- src tests CMakeLists.txt；
生产src +73/-10（含版本/说明），基础和合同测试+435/-0，CMake+35/-2；
S3另补目标先退出分支的测试差异，不新增生产路径。唯一状态所有者和旧路径处置
见上方六单位，服务/VDM没有新旧并存实现。最终S3统计、卫生及交付复核另记。

S3生产零变更；测试failures.cpp +14/-3，tests/README +13/-0。来源正向/23负例、
治理13负例、五几何头一致和diff --check通过。全T程序/测试/CMake合计+557/-15、
净542；其中测试代码+449/-3（不含测试说明）。没有新增产品构建或移动旧路径。

全部owned worker/fixture已结束；第一次清理检查把当前命令自身误识别为worker，
未执行删除，排除当前PID后重核无owned进程。祖先/子项无reparse且目标均为仓库
build内精确路径，回收t4-propagate 702文件27286092字节，x86/x64 Testing
各2文件1363/1746字节；稳定.ninja保留。没有删除用户或兄弟项目文件。
