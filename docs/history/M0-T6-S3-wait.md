# M0 T6 S3：Console启动器的自然等待

用户先要求GUI方案并插入新S，随后明确否决GUI让交互cmd不自动等待的行为。
本S保留Console；GUI试验只在忽略的开发构建中编译，未发布、未提交产品，已撤销。
不修改原图形资源；S2原资源新版“跟ntvdm64的一致”的视觉反馈已接收。
未完真实输入/DPI/剪贴板转S4，最后全任务审计为S5，CURRENT为唯一活动包。

有限范围：Console目标默认等待根进程并返回实际退出码，GUI目标默认恢复后返回0，
--wait对两者均等待。子孙是否等待仍由目标应用控制。保持原控制台及stdin/stdout/
stderr；GUI目标不创建额外console。PE子系统从既有受检PE头返回，不增加第二套解析。
私有validate_image_ex增加可选out，旧内部接口包装保持；公开DLL ABI不变。

新等待契约使用系统真实cmd启动当前CLI，目标是原生双宽CUI与x64 GUI可执行文件。
目标在事件上保持存活，报告DLL已注入、PID/父PID、console HWND及成员；父cmd在
目标释放前不得返回，释放后检查实际42退出码。GUI默认异步须在目标仍存活时返回0，
显式wait须阻塞至42。全部6场景开发复验通过；cmd、launcher及CUI目标确在同console。

首轮等待用例失败是开发树x64目录未含32位DLL，目标尚未进入；未修改等待断言。
改为显式传入同批双DLL并在测试私有目录准备实际CLI包，保存失败stdout/stderr，
focused-fixed worker34600重跑6/6通过。原生stdio/坏包/退出码开发用例也已通过，
两种wait选项的双宽console都返回37；这只是开发证据，尚须干净新四文件闭环。

计划验证：新快照双宽全构建/ABI/PE/原资源，双宽原生异常/补丁/载荷/staging/失败、
CLI和等待契约；实际同包14项含创建字段/传播/回滚/对话框/消息框。发布及推送复核后
关闭本S并自动准入S4，不把S4的物理项目升级为通过。

开发修正后的干净中间批次已完成双宽全构建和15项相关回归（x86 6、x64 9），
无失败/跳过。实际包控制器用CREATE_NO_WINDOW启动Python时，控制台HWND可能为0，
但GetConsoleProcessList仍报告共享成员；首轮同包在此错误的非零HWND断言失败。
修正为先证明父cmd阻塞，再严格检查target/launcher/父cmd三者同属控制台成员，
HWND作为观测值；GUI目标依旧必须无console。headless worker35592六场景复验通过。
没有修改生产等待逻辑或削弱父cmd存活/退出码/GUI异步断言。旧发布包已被负例拒绝，
其launcher提前退出导致成员缺失；最终快照将用修正后的测试重新构建，不发布中间包。

最终s3-wait-final worker35788干净双宽全构建/ABI/PE/原资源18表及完整声明通过。
Console PE未改变。worker37840在同一最终构建中执行相关回归：x86 6/6、x64 9/9，
无失败/跳过；没有重链接或改换候选。旧包负例worker35640使用新版测试，准确报
“parent cmd returned before target release”，证明新契约会拒绝旧的默认异步console。
最终同包worker31436完成14/14，含等待6场景、64创建字段、四回滚、八对话框、
六消息框及四位宽边三代传播。测试前后四文件SHA一致；相同文件已复制到assets/binary，
再次核对一致，release.json绑定124输入及最终快照。当前完整矩阵没有重跑，S2的
57通过/2剪贴板权限跳过只作历史证据，完整新版验收由S4接收。

生产/测试/构建统计范围为src、tests、tools及CMakeLists.txt，共9路径，git diff
--numstat为+132/-8，净+124行。原PE验证器仍拥有验证，私有输出只返回已验证子系统；
既有main拥有进程/线程句柄及清理路径，未增加驻留状态。原图形资源六文件没有改动。
相似问题检查覆盖所有wait选择、PE子系统读取、CLI退出码期望及GUI试验文档；
旧默认异步console退出断言已更新，未引入父shell hook、额外产物或GUI发布物。

| 文件 | SHA-256 |
| --- | --- |
| run16s.exe | abc31195e11a5bf2a69866038c4608a5c889b483f794d778c8b0f727449f37f1 |
| ntw16s32.dll | 974be256a1ca532553f2d4870ffc1257b258443ef2b82c0c71fb363ef2af51b9 |
| ntw16s64.dll | 87bb0e7a422f22d21f78acbfd0f52df0600c19e0445e9db70f8674183f1def4b |
| demo.exe | 04bf545e2d019a8888f607caff8d3b00bb0c0eae2be3aa67ce95e7ff99851d2d |

本地verified，提交推送后读取实际提交和远端字节复核再关闭S3；S4未完物理验收
不能由本S自动用例代替。用户已接受原资源外观，不重复要求外观确认。

P1已推送为4b83994eb1ed71de671a1dc830d4d08049fa44a5。随后以协调复核角色
fetch origin/main，读取实际commit/stat/diff及原始请求；HEAD与远端一致，124输入
按Git换行约定逐项复核，四个远端文件与最终测试包SHA一致，六个原资源blob不变。
原始cases.json为14个exit0，JUnit确为6/9通过且无跳过，等待日志及PID成员身份
证实父cmd阻塞、CUI同console及GUI异步，stdio四场景返回37。governance13、
source23、MinHook9、资源6负例及diff检查通过。没有将旧完整矩阵冒充本S证据。
S3闭环完成并closed，自动准入S4接收剩余真实输入/DPI/剪贴板及新版全量验收；
T6和M0未完成。P2仅记录上述实际复核、关闭与准入，不改变任何产品构建输入。
