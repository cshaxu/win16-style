# M0 T3 — launcher and initial automatic attachment audit

用户授权自动准入并完成剩余队列。T2 e29b3e4 已关闭；T3 仅负责首次挂起创建/
安装/恢复及真实自动窗口附着，不把子孙传播、产品demo和完整包提前判通过。
S1 2cc5d4d、S2 fc12e3c 已推送并进行实际提交复核。S3 冻结下方六单位；
本任务内失败必须修复，既定后续能力才交回其候选，不能以转移掩盖缺项。

## Frozen task coverage

| 单位 | 当前事实及接受证据 | 完成谓词 |
| --- | --- | --- |
| x64 CLI/参数/返回 | S1原始尾串、SearchPathW、W创建、明确阶段错误；S2中文文件名/空格包目录/Unicode/引号/空参数；S3真实32/64 CMD stdin/stdout/stderr、--wait返回37、非等待返回0 | 实际CLI运行且目标端保持参数、退出/流；无服务/DOS路由 |
| 唯一安装事务/位宽 | IsWow64Process2选择真实machine，SDK PE验证、32字节包、固定UTF16目录；同宽直接导入，异宽真实OS rundll32 | 不借用源仓库、broker或另发helper；原目标挂起/参数所有者清楚 |
| DLL三加载模式/ABI | 无载荷被动；helper回调ordinal1 NONAME；有载荷最小Detours事务，GUI入口延后私有原函数绑定；31public/ABI3不变 | 无loader-lock GUI/等待/LoadLibrary；坏载荷使loader拒绝，应用入口不执行 |
| 实际自动GUI | S2两宽×原生/CLI×初始/新GUI线程；A/W root/七动态控件、八Dialog APIs及六MessageBox APIs；入口/线程/末尾witness | 不链接或安装样式；只读诊断附着及实际字体、原生数据/标题/通知返回；用例数/观察标记完整 |
| 失败/回滚/资源 | 六种错误DLL的CLI诊断/无入口；受限句柄ERROR_ACCESS_DENIED；非法载荷0xc0000142/无入口；真实W helper创建失败及10秒超时 | 安装器不恢复/终止借用目标；目标原挂起仍1，创建者回滚；helper退出、句柄总量不增长 |
| 来源/完整回归/卫生 | 79回执、124固定来源决策/294 includes、23/13负例；双宽完整注册集、PE/版本/依赖、代码统计和scratch回收 | 全部目标有本地证据或既定后续接收者；推送后实际复核再关闭 |

细节：[S1安装器](M0-T3-S1-installer.md)、[S2引导及矩阵](M0-T3-S2-bootstrap.md)。
源代码编译为MSVC C17及必要C++17、Release /MT /W4 /WX。Detours八文件保留
Microsoft版权/MIT，七份byte-exact；W helper仅适配精确目标宽路径、32768命令
缓冲及统一10秒deadline。五替代图形仍由项目几何生成器产生，未导入原像素。

## S3 failures and convergence

s3-failures 首轮32位运行通过，64位夹具因C4389有符号比较被/WX拒绝；改为
DWORD预期错误值，未放松生产编译。s3-failures-verified PID13668两宽通过：
受限句柄拒绝且目标保持原挂起；故意绕过正常编码器写入version99载荷，使真实
DLL初始化拒绝，退出0xc0000142且入口文件不存在。测试故障只在夹具，没有
产品环境后门。x64实际W helper故障callback覆盖创建拒绝和无响应actor，
deadline实测10000毫秒；helper已退出，借用目标没有恢复。handle count
32位172/172、64位141/141；创建者作用域负责中止和关闭其未交付目标。
六种CLI包错误（两宽×缺失/错宽/截断）均返回6及Windows错误2/193，目标不入场。

首次全回归 s3-full PID25028：x86 16/16（99.37秒），x64 21/21（302.23秒）；
核心各13项、启动器/自动GUI/失败矩阵全部实际运行。之后只扩展CLI测试，
s3-cli-io PID3788聚焦通过：32/64×等待/非等待四例输入输出与返回契约均通过。
两个正向stdio探针也确认Win32及C CRT输出，无需修改生产句柄策略。

最终路径审计发现扩展UNC前缀比较应忽略大小写，修正_wcsnicmp并加入DOS/UNC/
扩展DOS/大写及小写扩展UNC正例。语法解析通过不冒称真实网络共享验收。
来源中两份旧Hook .def 改reference：实际新ABI由现有根.def唯一拥有，不创建
第二套导出文件或旧NthookAnchor/ContextFlags接口。固定blob/摘要不变。
新事实与其接收者明确，没有为满足导入数量复制无用代码。

最终状态 s3-final PID26632 exit0：x86 16/16（105.46秒），x64 21/21
（300.77秒）。两个架构均包含全部13项核心回归；x64运行真实CLI的双宽基本/
八对话框/六消息框矩阵、故障helper及六错误包/四stdin-out-exit组合。两宽
20轮GDI=43/43、USER=38/37；本轮helper超时10000毫秒，借用目标保持挂起。

最终SHA256：run16s.exe（AMD64/PE32+）
cccbb31c34369383fbf349be0d2dbbf8c5443e2a5e1046fca56cfbb667c2940e；ntw16s32.dll
ca870a85adbd045bc1c85ea033087bd4c931636813e6d9ce515c1d18aeb0cf80；ntw16s64.dll
ba9a7489c30f9a654682972acb860f6501154a5f487cac68ef76550a59ba9968。
CLI直接imports仅KERNEL32；DLL仍六个系统库，31名字/精确ordinal1..32及无转发/
无私有binding导出通过。三个产物数值版本0.3.0.0、FileVersion M0 T3，尚未发布。
全T已验证，仍须交付推送与实际提交复核后关闭。

## Independence, statistics and remaining owners

实际Ninja依赖每宽44项：27个本地include输入（包含被creatwth包含的uimports.cpp），
其余为MSVC/Windows SDK；无兄弟仓库路径。默认来源门禁/编译/运行不访问来源。
额外来源研究明确使用可选开关，124/124固定tree/blob/size/SHA对照通过，不能
把该研究步骤当作产品外部依赖。实际DLL直接imports仍为六个Windows系统库。

原renderer继续拥有绘制/几何/字体/清理；原生控件拥有数据、选择和通知，应用
拥有消息循环。新层只负责事务与创建作用域；没有第二renderer、旧Hook产品DLL、
broker、RPC或service。T3安装器只借用新建未交付目标，安装成功是暂存，不等于
初始化已执行；CLI --wait返回目标实际退出码。包路径需ACP往返或有效短路径，
不以best-fit截断；不能表示时显式失败，任意Unicode包目录不是当前承诺。

传播候选接收四种父子位宽边、A/W及第三代/创建语义；demo候选接收无样式API的
64位产品例；包验收接收干净环境独立重建、四文件同批、真实桌面输入、选定DPI/
字体环境及完整跨任务审计。当前私有桌面矩阵不替代上述后续目标。

## Statistics and hygiene

统计命令 git diff e29b3e4 --numstat -- src tests tools CMakeLists.txt：src
+13229/-9（22文件），tests +445/-1（10文件），CMake +68/-1，tools +6/-5
（2文件），合计+13748/-16，净+13732。多数为带声明的Detours导入，不能当作
新增算法统计。S3窄修复context +1/-1，测试 +154/-0、CMake +11/-0；净+165。
S1 src加总笔误已在其记录更正为+13020，不改变实际交付。

所有登记worker和fixture已结束；CIM按本任务路径/命令核验无活动owned进程。
绝对路径/祖先及子项无reparse验证后，回收build/t3-launch（260文件、
33899025字节）、两宽CTest日志（14842/20552字节）。build只保留.ninja，
不提交日志、截图、故障包或二进制。来源正向/23负例、固定源研究124/124、
治理13负例、五素材一致、实际PE与diff --check通过。

S3 P1交付后另读实际提交、证据和原请求进行协调复核，P2才记录T3关闭。
