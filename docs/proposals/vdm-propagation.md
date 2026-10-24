# VDM propagation diagnosis

用户授权追踪run16s -> run16 -> NTSRV -> ntvdm内NE窗口，先证据诊断和最小方案。
只读兄弟仓库及本机安装文件，不改协议或扫描注入既有进程；私有桌面不代表RPC隔离。
M0 Td S8仅原生诊断，产品修复须在确认根因后另准入。T8保持开放。
有限覆盖：初始注入/策略、真实创建者、64->32 helper、各层初始化、后装hook覆盖、
WOW窗口API与附着；冷启动/复用服务/复用VDM分列，历史仅固定相关提交/二进制。
预检存在服务时不假造冷态；如不能隔离，仅记录未覆盖和具体原因。
临时纯C探针和原始日志存build/td-s8-vdm，180秒自有进程预算，最终提炼并安全回收。

用户后续接受裸NTSRV复用无传播边界，原NE冷态已成功但历史间歇失败未证实根因。
另确认双钩子外部导入表被staging错误193拒绝，最小本地修复独立交付；见
[诊断记录](../history/M0-Td-S8-vdm-propagation.md)。

Td S8诊断已收口；T9 S1获原条件授权准入最小staging修复。只接受可读且地址不溢出
的外部表，保留1024条目及失败回滚；自包含双宽测试无需nthook/NTVDM，兼容实验
只操作自有实例。版本推进0.9.0.0，标题/字体/菜单代码及六资源原样。


## Real nested cmd test design (T9 S1 extension)

用户追加明确要求：纯C驱动真实cmd标准输入/输出，不只跑单层/c；当前活动包T9 S1
承接，不分配冲突编号。复用launch/staging的自包含外部表正负例及真实初始化，
复用现有private desktop/job/有界子进程的所有权模式。

新集成driver使用管道驱动外层cmd交互启动内层cmd（/k初始化独有环境标识），
由目标内层cmd echo唯一标记，父驱动器不直接代写；观察内层真实PE映像/PID/
创建者及DLL/初始化与实际传播。exit /b 37后验证外层恢复及errorlevel37，再外层
exit /b 23并核对原生退出码。每case独有标记，x64/32根与子四边（32到64用Sysnative）。

三场景分列：run16s cmd核心自包含；run16 cmd与run16s run16 cmd仅可选本机兼容，
独立登录AuthenticationId隔离RPC端点，权限/后端缺失明确skip，兄弟不作必需依赖。
保留原生创建错误和cmd errorlevel/警告不同证据层次，193需真实复现才归因为
staging外部表；不能凭“不兼容16位”文字断言实际执行NE。无用户进程干预。

本轮T9 S1已按有限范围交付、实际远端复核与清理收口；真实cmd及外部导入表
结果见[T9 S1记录](../history/M0-T9-S1-native-cmd-staging.md)。此前NE间歇问题仍不作
根因/修复声明；T8开放，当前活动状态以CURRENT为准。
