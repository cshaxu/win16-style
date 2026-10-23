# M0 T6 S5：独立产品及跨任务完成审计

原请求为独立出原生启动、DLL后代传播和Win16外观，三组件四产物，永久M0；
后续明确纯C/GCC/Ninja/ccache、原图形资源相同、Console自然等待、每S交付闭环、
不用computer-use且自己用探针，以及binary change双发布assets根目录和
O:/winnt/system32。最新用户要求优先，不再以整套用户人工操作作为关闭前提。

本S在9825332关闭S4后准入。覆盖全集在以下两表冻结，允许处置只有实际通过、
修复后复验、已明确环境限制且不虚报通过；没有未处置产品失败才可关闭。
S5预期生产/测试/构建源码零变更，124输入必须与S4逐项一致；任何漂移先处理。
源码/ABI/资源/依赖门禁、新干净双宽构建、实际包10项及双发布仍须由本S执行。

| 任务与实际关闭提交 | 保留生产责任及当前证据 | 完成谓词/处置 |
| --- | --- | --- |
| T1 80eccb2 | 固定来源commit/blob、124处置/294 includes、独立设计与本地门禁；当前121回执和MinHook14清单 | 来源冻结已兑现；历史MSVC探针不当作当前证据，当前C/GCC由T6 S1/S4/S5接收 |
| T2 e29b3e4 | ntw16s-dll唯一renderer及GUI线程所有者；当前双宽core/ABI/资源生命周期及原资源门禁 | 旧自绘资源已被用户否决，S2 P4 4bf65c2恢复六文件，用户确认一致；S4修复方形滚动条，不能冒用旧自绘外观结果 |
| T3 032d9f6 | run16s main首次挂起创建、私有staging/installer与有界OS helper、失败回滚 | 当前双宽首次自动窗口、载荷/PE/异常清理、stdio/长路径及S3/S4六场景Console等待通过；无自带helper产品 |
| T4 7f0bff7 | create_process A/W trampoline、线程重入保护及新建未交付进程所有权 | S4同源完整矩阵四位宽边/三代、64创建字段、四回滚通过；调用者悬挂/LastError及句柄继承保留 |
| T5 60bb624 | 纯原生demo/gallery，无样式import/API或业务测试开关；外部观察器负责验证 | 同一demo裸启动原生、注入才加载匹配DLL及样式；实际模块路径、控件数据/通知、截图差异和原生交互探针通过 |
| T6 本S待最终复核 | 纯C/GCC私有MinHook/HDE、独立快照、四产物及双发布、跨任务证据 | S1独立包，S2原资源恢复且余项由S4按用户新方式接收，S3等待，S4方形修复/完整矩阵/探针/双发布；S5新包及实际复核后关闭 |

| 八类验收面 | 当前生产路径/状态所有者 | 可复核证据及边界 |
| --- | --- | --- |
| 构建/包 | CMake、Build-Core/Build-Package，四固定PE；工具链由本机配置 | S4干净树双宽全构建，dependency trace仅快照与选定工具链；四文件PE/ABI/系统imports/声明/原资源18表通过；S5新包复核 |
| 首次注入 | x64 Console main及installer；创建者拥有未交付进程/线程句柄 | 实际x86/x64目标自动附着、错包/错误PE/启动异常/回滚、长路径及退出码；SDK staging与有界系统rundll32，不依赖源服务 |
| 后代 | DLL私有A/W CreateProcess拦截及包目录载荷 | 四位宽边、三代、GUI线程与实际窗口附着；S2系统双宽cmd中转8项为同保留传播路径补充证据，S4当前完整传播及实际包通过 |
| 创建语义 | 原参数透传、TLS重入、原trampoline及统一finish | S4完整64字段及四回滚，caller suspended不提前恢复；父/兄弟进程仍存活，失败不发布半安装目标 |
| 生命周期 | DLL的GUI线程scope/窗口状态、字体/DC/位图/子类引用；创建句柄由创建者 | S4双宽完整core，重入/线程所有者、销毁、GDI/对象基线及绘制失败负例；新方形修复只改变无状态几何 |
| 外观/输入 | 唯一renderer及保留的五图形头；原生控件数据仍由USER拥有 | 用户确认原资源外观一致；横纵多厚度方形端钮、焦点Tab、控件数据、菜单/About、下拉、捕获和窗口状态探针；宿主DPI144/150%，不宣称任意DPI或物理输入 |
| demo对照 | demo不消费样式；观察器仅在tests | 实际同一image、匹配DLL路径、数据通知及绘制差异，原生posted消息经过真实应用循环；测试fixture不进入四文件包 |
| 独立性/交付 | 包目录定位，静态GCC支持库；Publish-Package只更新四项目文件 | 无sibling/broker/RPC/常驻服务/第五产品，未替换系统自带DLL或全局主题；assets根目录、O:/winnt/system32、Git远端摘要绑定 |

S4 418be5e是最后一次产品/测试源码变更，其完整矩阵注册与执行集合相同：
x86 24项中23通过，x64 36项中35通过；0失败，两个core/edit-clipboard返回77。
原始JUnit和注册JSON已在S4远端复核中检查；不把跳过计为58项通过之一。
S5源码相同则保留此完整矩阵的来源身份，另做新构建同包验证，不虚称重跑全矩阵。

明确限制：本宿主CreateWindowStation对四种权限mask均返回5，因此无法自动创建
隔离剪贴板，复制/粘贴/剪切/撤销的隔离端到端用例未验证；不接触共享剪贴板，
不再将整套人工测试转给用户。代码审查确认EDIT消息仍交给DefSubclassProc，
这个审查不代替端到端通过。此为宿主隔离能力限制，无已知产品失败，不承诺
所有宿主/DPI/字体/创建API；支持范围继续由BUILD说明约束。

没有为绕过限制启用computer-use、修改权限、使用共享剪贴板或伪造物理输入。
S2原待物理项按用户后续指令由S4原生探针接收，原视觉确认保留；旧工具链和旧
自绘交付留在历史中，不能成为当前产品通过依据。本S产品/测试源码、资源和兄弟仓库均未改。

本S长任务为s5-audit新干净快照，单架构600秒；s5-audit-test实际包预算1800秒。
部署备份s5-deploy，前序raw证据提炼后只回收本任务已登记scratch，稳定.ninja/
ccache保留。最终提交、远端字节复核、清理及关闭结果随后补记；当前不宣称M0已关闭。


本S新s5-audit干净双宽全构建通过，ABI各1/1、PE/导入/声明及两DLL18原资源表通过。
124个产品/测试输入与S4逐字节相同，source_inputs_sha256不变；当前S5同包10/10，
含新Tab/捕获/About/窗口状态及等待六场景，全部实际候选路径和DLL身份正确。
S5未重跑完整60项；保留S4 418be5e的58通过/2权限跳过来源绑定，并保存原始注册/
JUnit SHA。沒有借用旧自绘或旧MSVC结果。

s5-deploy已双发布，先备份两处四文件，before均为S4已提交包，after与本S被测包
一致。发布及测试四SHA、快照、工具链、124输入与原始证据摘要均入release.json。
30个实际历史交付提交的标题/路径、ancestor关系和git show --check已检查，无断链。

本S程序/测试/构建源码统计0/0，状态所有者及旧路径不变。全T6统计命令
`git diff 60bb624 --numstat -- src tests tools CMakeLists.txt`：107路径，+5627/-13865，
净-8238；其中CMake +160/-57，src +3578/-13368，tests +1285/-196，tools +604/-244。
删除主体为旧Detours/C++/MSVC路径；当前只有C17私有MinHook/HDE及一个renderer。

| 文件 | S5 SHA-256 |
| --- | --- |
| run16s.exe | 435dfc22827582ea0a2bbec53954bbb1579e0c2a58cc25a10d038ec884470f02 |
| ntw16s32.dll | 828798f6dd938fb9d2f713599b29f163702e7a316f5022ddaa4a63c579d2cba4 |
| ntw16s64.dll | fcd9d17983fd85f8cdcc0dde119e1963676edaf0e8b26f1ee4aed344f6ec2bf1 |
| demo.exe | 0104cb2b20a35daae7e45034dc76cfde5299736f9ed09fa43e7972569bdbcf6c |

本地审计verified，完成推送后实际字节/提交复核及owned scratch回收再记录关闭。


## P1实际复核与T6/M0关闭（P2记录）

5b5ee58376d3ff9a98e5852cb2c3c5247e202e02已推送并fetch origin/main复核。
协调角色读取实际commit/stat/diff、原请求和30个交付提交、124个远端输入blob，
按LICENSE显式检出换行约定全部匹配；源码输入摘要与S4相同。远端assets四文件、
本S被测包、本地assets及O:/winnt/system32逐项SHA一致，六份原资源blob与固定来源
一致，两个DLL各18个原图形表及完整声明。实际包10例原始exit0、S4注册/执行
24/36及JUnit的58通过/2权限跳过已复核，没有把本S包测试冒称全矩阵重跑。
治理13、source23、MinHook9、原资源6负例及diff检查通过，无未处置产品失败。

全部owned构建/测试已结束。核实精确绝对目标O:/repos.hobby/win16style/build/t6-package
在本仓库内、祖先/子项无reparse且无活动owned进程后，原生PowerShell回收40个顶层
scratch目录、13003文件、521103134字节；目标已不存在。保留稳定build/.ninja与
build/.ccache、源码、发布四文件及兄弟仓库。部署before文件为已提交S4 418be5e，
更早发布也可由各Git交付复取；备份在部署期间已建立并核验，原始scratch现已回收，
身份、before/after及证据摘要已提炼于release.json和本记录。

六个T及T6五个S的责任全部对应实际交付；S2原交接已由S4按用户最新探针要求接收。
原始三组件四产物、独立启动/传播/外观/demo、C/GCC、原资源和双发布目标完成。
因此关闭S5、T6及M0本次有界产品验收，保持milestone仍为0，不自动创建新任务。
隔离剪贴板的两项权限跳过和物理输入未覆盖继续明示，不能读成本宿主端到端
clipboard或任意DPI/应用的通过承诺；这些限制不是新发现的产品失败或待用户整套
手工操作。CURRENT移除活动包，QUEUE无剩余候选，后续工作由新的用户请求准入。
P2只记录实际复核、清理和关闭，未改编译输入或二进制，双发布摘要保持P1相同。

收口文档首次将含数值任务号的历史链接放入QUEUE，被编号预分配门禁拒绝；
改为指向CURRENT中的完成审计，保持QUEUE无数值任务号，未削弱检查器。
