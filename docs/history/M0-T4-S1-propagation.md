# M0 T4 S1 — native descendant propagation

用户已授权自动完成全部候选；T3关闭记录032d9f6已推送，准入T4 S1。
固定来源create_process.cpp适配为原生A/W事务，删除RPC/broker观察、legacy分类/
VDM重定向、launcher身份和frontend/execution字段；不转码或重构原生A参数。
source packet/installer保持唯一，默认策略改STYLE|PROPAGATE，版本M0 T4。

两个原函数trampoline只有一个所有者。局部TLS guard与__finally处理原生A->W
重复进入/异常退出；helper显式使用W trampoline。DLL有载荷且PROPAGATE才在
原有事务内挂接进程入口，无载荷被动模式、helper模式和31 public ABI不变。
创建先把原flags加CREATE_SUSPENDED并原样转发全部其他字段；安装器借用目标。
调用者本来要求挂起时不恢复，否则只抵消一层；失败由创建层终止未交付子进程、
有界回收/关闭并清零输出，最后还原原生或安装错误，不影响父/兄弟进程。

## Finite S1 verification

s1-basic owned PID18768 exit0：x86 payload1/1；x64 payload+propagate/basic2/2。
八条真实CLI根路径：四种root->child位宽边各A/W两例；第二边切换A/W，
grandchild取与root相反宽度，第二边也覆盖四边，共24个应用进程/三代。
每代验证实际同宽模块、真实root和BUTTON附着、私有20px字体、原生checkbox。
记录各代PID/child PID并在控制器核对父子链及入口/窗口/完成marker，避免零退出
或PID统计代替附着。半数路径保留调用者挂起，SuspendThread1、配对恢复2，
随后调用者主动Resume1。每代还验证缺失W目标的原生ERROR_FILE_NOT_FOUND。

窗口均在自有私有桌面，无全局设置/既有进程扫描；控制器用KILL_ON_JOB_CLOSE
只管理自己新建树以回收测试失败，产品不创建job或终止已交付后代。完整八路径
12.67秒通过。两宽实际PE仍ABI3、31名字+NONAME helper1、六个系统imports。
没有额外产品helper或第二renderer。

完整环境/cwd/继承/STARTUPINFO/EX、空application、Unicode、更多回滚/错误
组合由S2接收；首次启动及完整核心回归由S3接收，不能用本轮代替整个T4。
82回执、来源23负例、治理13负例和diff --check通过。实际staged numstat：
src +73/-10（含版本/说明）、tests +112/-0、CMake +13/-2，净+186。
唯一状态所有者/renderer不变；旧服务/redirect路径未建立，不留第二实现。
本S交付推送后实际复核，再关闭并进入S2。
