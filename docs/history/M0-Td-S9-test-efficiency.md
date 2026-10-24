# M0 Td S9: build and test efficiency

用户在只读审计后明确准入效率工具修复。3245d15为基线，下一未用Td S9；
M0及T8保持开放，T9产品任务仍关闭。治理工具归Td，产品C/版本/ABI/原资源不改。

## Root causes and changes

旧toolchain只set普通CMAKE_C_COMPILER变量；脚本却只查CMakeCache，缺项即--fresh，
因此每次重做配置并丢弃对象目录。热运行实测再次编译51个quick步骤已证实。
现写入FILEPATH缓存并兼容读取旧CMakeCCompiler.cmake，只有实际编译器变更才fresh。
Toolchain移至持久.ninja/<arch>，内容相同时不更新时间，不依赖任务scratch。

旧干净源码副本的CCACHE_DIR跟随新repo根创建临时缓存；直接Ninja又依赖外部环境。
现将DIR/TEMPDIR/MAXSIZE5G/BASEDIR嵌入命令，干净副本显式共享原项目持久缓存。
不清空全局缓存、不启用sloppiness，Ninja依赖检查继续拒绝外部源码。

新增Quick聚合目标及按目标入口，完整集合保留。CTest命令经工具分配唯一工作/
捕获目录，原生命令参数、断言、轮数、超时和77均保持；默认4slots、重UI占2。
包测试保留原每例目录、标记和前后四SHA，在相同预算下有界并行；失败finally只
清理本次已启动进程。编译8jobs可调整，所有阶段及cache前后计数均有记录。

## Verification and delivery

修前同集合串行基线：x86 ABI/staging/font/menu四例150.9225秒；x64加真实cmd五例
127.1251秒，全部通过。不是历史CTest平均值。修后热Quick双宽6+8全通过，Ninja
两宽均no work，build阶段0.3806/0.3420秒，test阶段0.6978/2.9778秒。
实际launcher自测4例证明并行目录隔离、空参数、skip77/失败42以及缺少scratch时
在启动目标前拒绝。原生silent fixture exit0但无ABI标记的包负例被正确拒绝；
另一输入含README被四文件门禁先拒绝，不冒称执行到了scheduler。

同集合并行实测x86为67.6470秒、x64为67.0839秒，9项全通过；总278.0477→
134.7309秒，约减少51.54%。基线取已有稳定输出，修后同注册原生集合重建；产品/
fixture C源码未改，但不是同二进制微基准，也不承诺任意宿主负载下的固定性能。

只制作一次最终td9-final干净候选。共享原项目cache实测x86为80 direct hits/0miss，
x64为90/0，构建阶段12.7964/15.2998秒；Ninja依赖门禁通过，未借外部源码。
在该最终干净构建上跑完整集合，26+41项：64通过、双宽clipboard权限5/77及
未配置的可选后端77共3跳过。所有旧注册声明保留，只新增checks/test-runner。
完整测试阶段187.7015/243.8159秒，构建阶段再次no work，四产物与候选不变。

实际包独立19/19通过，4slots、重UI占2，用时270.8132秒；原前后SHA和必要标记
全部保留。高位DWORD退出0xc0000005也通过实际launcher透传并原值记录。
此次未显式重跑安装后端8项兼容，不能借T9结果称为本S覆盖；完整物理输入、
任意宿主负载、跨机cache及原地升级编译器未实测。没有调整原native断言/轮数/超时。

产品C/H/RC、版本0.9.0.0、ABI3、标题字体菜单、六原资源及源码声明均与3245d15
相同。因重建产物摘要变化，已先备份再发布assets根及O:/winnt/system32，八份与
被测包一致；八份before与3245d15 assets相同可恢复。源码/测试/配置输入131项及
四SHA/PE/imports、77份原始摘要见[release](../etc/release.json)和
[效率证据](../etc/evidence/test-efficiency.json)。公开记录无真实PID/路径/转录。
P1实际远端复核及安全清理已完成，收口证据如下。

变更统计范围CMakeLists/src/tools/tests相对3245d15：8路径，增加309/删除76/
净增233行；产品src零变更。没有迁移产品实现；持久元数据归构建树，每例scratch
归相应测试，GUI原有所有权不变。稳定树更新后不依赖旧task toolchain。

## Remote review and closure

协调复核读取实际fetch后的P1提交425b7df095d21899501102ef1cb1eb7693e5cea3及diff，
逐项核对131输入的Git文本规范化/实际复制字节、远端四产物、八发布副本、77份
原始摘要及完整JUnit/19包标记。产品src与rules对基线无diff，旧测试注册全部保留，
公开证据无运行身份。复核结果及其原始SHA已提炼入release.audit。

确认无活动进程、无reparse且绝对路径在本仓库build下后，仅回收本S两个登记根：
build/td-s9-efficiency的311文件/27,258,499字节及build/t9-package的1394文件/
88,830,314字节；本轮双宽LastTest.log亦在记录摘要后删除。稳定.ninja对象/元数据、
CTestCostData、.ccache及发布文件保留。清理后实际执行两宽cmake --build --parallel8，
均Ninja no work，0.508716/0.3501539秒，证明toolchain不再依赖已删除任务目录。

有限审计清单全部满足：错误fresh分支、持久toolchain及共享cache、目标/Quick入口、
隔离并行与资源预算、原完整集合、失败标记/退出码负例、最终干净同包回归、双发布、
实际远端输入和产物复核、安全清理及清理后增量。上文跳过及未覆盖面保持，不扩张
性能结论。P2仅记录复核及关闭，不改131输入或产物，不虚构重复构建/发布。Td S9
收口；M0、T8开放，无活动S，T9保持关闭。
