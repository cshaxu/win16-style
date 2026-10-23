# M0 T3 S1 — CLI and suspended installation transaction

用户授权自动按顺序准入并完成全部剩余 T。T2 e29b3e4 已关闭，因此准入 T3，
当前 S1 建立 CLI/安装暂存，S2 才负责 DLL 初始化与窗口自动附着。

固定来源 5b0b65f66c281050aa4ce1454c6cd31441a6d484，导入 Detours 八文件并保留
Microsoft copyright/MIT LICENSE；其中七项 byte-exact，creatwth.cpp 为已审 adapt。
仅编译五 cpp，uimports.cpp 继续由 creatwth.cpp 的两宽实现分支包含，未独立编译。
W helper 保留精确目标宽 DLL 路径、系统 rundll32 选择，命令缓冲改为 32768；
等待及终止回收共用进入事务时建立的 10 秒 deadline，无无限回收等待。
旧 A helper 包装和创建/控制目标的 Detours 高层包装不被产品调用。

context/installer/header 适配为固定 32 字节、version1、machine/flags/包目录，
无服务/frontend/execution/broker/RPC/转移句柄。解析验证全部字段、长度、终止、
绝对 DOS/UNC/扩展目录，先复制字符以避免非对齐载荷访问。实际目标位宽来自
IsWow64Process2，DLL 文件经 PE/header/subsystem/位宽检查；包路径必须 ACP
往返或有效短路径，UTF8 ACP 单独使用合法转换参数。

安装器借用已创建且尚未交付的挂起目标，只写 imports/payload。同宽直接写入，
异宽调用有限 W helper；不终止、恢复或关闭目标。创建者 CLI 持有全部回滚责任。
CLI x64 解析应用与 --wait，保留原始参数尾串，以 W API 创建、安装后抵消一次
本层挂起；失败中止本层目标并清理句柄。阶段错误码与路径限制见组件 README。
无 wait 的成功仅指暂存/恢复，--wait 返回实际目标退出码，未承诺初始化已通过。

## Finite verification

首轮 s1-build owned PID 25012：两宽各 payload/staging 2/2。补充 CLI 诊断、
UTF8 ACP 和 ResumeThread 失败处理后，s1-verified owned PID 42196 exit0：
x86 launch/payload、launch/staging 2/2；x64 同两项加 launch/cli 3/3。
载荷夹具覆盖八 header 字段、越界、终止、相对路径/嵌入 NUL；挂起夹具使用
真实同宽子进程，安装前后 SuspendThread 返回1、配对恢复返回2，主线程仍为1；
非法 flags 拒绝，实际 imports/载荷暂存返回成功。该目标不被恢复，测试所有者
中止并关闭它，不把暂存通过当作 DLL 已初始化或窗口已附着。

CLI 实际二进制验证 help=0、缺参/仅--wait=2、目标不存在=3、损坏 PE=4，
无应用入口执行。双宽核心 DLL 仍与 T2 摘要一致，PE 门禁通过。新 installer
未链接入核心，renderer 无改动，本 S 不无理由重复 T2 的完整窗口回归。
70 回执、来源正向/23 负例、治理正向/13 负例与 diff --check 通过。

异宽 helper 的实际初始化/ordinal1、透明窗口与初始/新线程/对话框、参数尾串
的目标端观测由 S2 接收；安装失败/回滚和完整核心复验由 S3 接收。未增加产品
helper EXE、未复制原图或使用 sibling 构建/运行输入。

统计（git diff --cached --numstat）：src +13120/-0，其中 Detours 八文件
12754 行，实际含原版权/许可证；其余为适配/新编入口。tests +73/-0；根
CMake +35/-1，构建入口 +3/-2。生产 renderer/状态所有者不变，无旧本地
产品路径需要移除。全部本地生成物留在已登记 scratch/稳定树，不提交二进制。
P1 2cc5d4d 已推送，实际25文件 +13364/-13；git show --check通过。
协调复核已读取实际installer/CLI/构建和测试差异，责任/暂存证明边界明确。关闭S1，
进入S2。尚不关闭整个T3。
