# M0 T4 S2 — creation fields and owned rollback

S1 bd619ae 已推送并实际复核。本S不改生产路径，在四种父子位宽边建立相同
原生/注入对照；每边两种父模式、八字段向量，共64次真实应用创建。
子exe与包DLL分目录，W目标另有中文/空格文件名；每个应用读取实际OS状态、
实际包内模块路径与窗口附着，不能由复制的预期文本或PID计数替代。

## Frozen eight vectors

| Vector | API/application | Startup/environment/flags |
| --- | --- | --- |
| 0 | W/explicit Unicode path | standard SI，Unicode environment/cwd，inherit false |
| 1 | A/explicit | standard SI，ANSI environment，inherit false，caller suspended |
| 2 | W/null application | standard SI，Unicode environment/cwd，inherit true，suspended |
| 3 | A/null application | standard SI，Unicode environment，inherit true |
| 4 | W/explicit | SIEX HANDLE_LIST，Unicode environment/cwd，suspended，BELOW_NORMAL |
| 5 | A/explicit | SIEX HANDLE_LIST，ANSI environment，inherit true |
| 6 | W/null application | standard SI，Unicode environment/cwd，inherit false，suspended，NEW_PROCESS_GROUP |
| 7 | A/null application | SIEX HANDLE_LIST，Unicode environment，inherit true，suspended |

每例均验证空格/引号/尾反斜杠/空字符串、W中文参数；自定义环境的确切值、
排除父sentinel和测试capture变量；子cwd、desktop、startup title/show flag、
实际priority与编译位宽。policy不依赖环境变量。继承事件在高编号句柄槽，
正向真实信号/负向无句柄；EX仅允许指定event和标准I/O，另一个可继承event
被排除。返回process/thread的security属性继承位及父event属性保持不变。
子stdin来自文件，stdout/stderr回到指定文件；caller suspended时入口文件
尚不存在、计数1/配对恢复2，调用者再Resume1。父cwd整个序列不变。

父还先做A缺失目标和W互斥flags的原生失败，明确错误2/87，再执行全部正向
创建，验证错误保存及TLS guard恢复。原生对照使用同一个exe和oracle；不会
为注入结果改变正确预期。未做的debug/其他EX属性等变体在T级审计单列。

## Rollback corpus

两宽父各A/W一例，真实CLI注入。先启动同宽健康兄弟并等到实际附着/ready，
只删除测试包中未加载的对侧DLL，再创建合法对侧exe。要求FALSE/错误2、
全部PI输出清零；自己job的NEW/EXIT通知匹配同一失败PID，active数量回到
原值，失败目标没有入口文件。父和已运行兄弟继续创建/销毁已附着窗口。
失败后同线程再创建一个健康同宽子进程，也实际附着，证明递归guard没有
停留在禁用状态。测试控制器的job只清理自己新建树，产品不增加job机制。

## Actual results

s2-contract PID2836：64向量通过，20.06秒；之后增加原生错误前置检查，
s2-contract-verified PID38464最终64向量通过19.24秒，exit0。s2-rollback PID13784四例通过8.71秒；扩展失败后
新健康子进程后，s2-rollback-verified PID4636四例通过20.13秒，exit0。
job事件证明目标已回收，不能仅用不存在入口或父零退出作替代。

本S只新增测试/构建注册，未修改已通过的生产实现。来源/治理、回执、
实际统计与提交复核在本S交付时记录；整个T4仍待S3完整回归及任务审计。

补充嵌套探针：同一包run16s --wait连续三层，最终分别启动SysWOW64及System32的cmd，
两例stdout为NESTED_OK、退出37、stderr空；未发现重复暂存导致失败，不为推测增加生产分支。
87项来源回执已逐项核对，本S无生产修改。
交付统计（本段加入前）：10文件 +465/-19、净446；其中新测试5文件323行，CMake +22，生产源码零变更。原字段由Windows拥有、句柄由各创建者清理，旧路径无移动。

P1 17720ae已推送，实际提交10文件+466/-19；上段为末段加入前统计。协调复核读取实际测试、
CMake注册及87回执，原生对照、挂起计数和job退出通知均由OS实测，不以摘要替代。关闭S2，进入S3。
