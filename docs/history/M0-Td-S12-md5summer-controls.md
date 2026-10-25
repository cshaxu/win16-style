# M0 Td S12 MD5summer控件调查

P1 56f9e4f已推送，实际取回六文档和四产物逐字节匹配、远端main一致；M0/T15保持开放，既有S和债务状态不变。
本轮仅用户要求的只读调查，无产品代码、测试、ABI、资源或构建输入变化。

## 真实断点

目标D:/programs/misc/md5summer.exe，1.2.0.11、860160字节、PE x86，SHA256
4646ce4f638e7d7ec820e7540b39bb185234e1f151d780c734ca557d6dba7ed9。
基线a13aa6e/0.15.0.0，加载assets/ntw16s32.dll，摘要与现发布包/系统安装一致。
主窗TMain、DPI96、root_attached=1，正常显示后的原生打印截图确证标题已应用。
框架窗口呈VCL类族，但本轮不断言具体编译器/运行库版本。

| 控件 | 数量 | 当前结果 | 证据与边界 |
| --- | --- | --- | --- |
| TButton | 3 | 未附着 | A/W真实类仍TButton；WM_GETDLGCODE为2020/2010，不能单凭它证明内部原生身份 |
| TComboBox | 1 | 外壳未附着 | A/W真实类仍TComboBox；GetComboBoxInfo成功、2选项/当前0 |
| 内部Edit | 1 | 已附着 | 原生Edit；字体由MS Sans Serif变私有Win16 System变体 |
| TShellTreeView | 1 | 未附着 | A/W真实类仍框架名；56节点，根/选择非空，TreeView协议有效但尚无renderer |
| TPanel | 1 | 未附着 | 面板/无窗口文字由应用负责，未猜测私有对象 |

上次DoubleKiller能由RealGetWindowClass识别Button/ComboBox，本应用却返回框架注册名。
因此存在识别缺口，同时确有缺少TreeView绘制支持；自定义类名也不等于完全自绘。
不将所有T前缀加入标准类，不读取opaque WNDPROC，不覆盖应用布局/图标。

## 原生验证

现有DoubleKiller纯C driver/observer的scratch派生探针，移除固定四页切换及Run卸载，
只对新建Job内的目标GUI线程设观察hook；记录实际映像、模块、类/真实类、消息、字体和附着。
GCC16.1/C17、-Wall -Wextra -Werror；未修改tracked测试或产品输入。
最终两组裸/注入共4个实例通过：主窗分类/打印、移动/最小化恢复、正常WM_CLOSE，
目标和启动器exit0、Job活动数0；裸Job总1，注入Job总4（含已有跨宽安装机制）。
[精选证据](../etc/evidence/md5summer-controls.json)包含PID/映像路径/模块SHA及各控件状态。

前6个观察实例失败：选择隐藏窗口或启动Information提示，随后提示销毁有竞态。
修正为仅可见主窗、排除已确认的Information后成功；失败均关闭自有KILL_ON_JOB_CLOSE Job。
旧GetLastError=5/1400为探针残值，不作为产品错误或通过证据。程序自身启动时尝试设置文件
关联并报告失败；仅确认自有提示OK，未改权限/注册表、未提权。不点击Create/Verify或生成文件。
未覆盖其他DPI、物理键鼠、业务对话框、x64应用变体或完整产品回归，不虚构新包/发布。
assets/system32八文件摘要逐一一致；产品输入未改，不需要产品重新构建。

PNG验收图保留在外部visualizations的win16style-md5summer目录：
MD5summer-native.png、MD5summer-styled.png；不提交原始探针/日志/BMP。
登记scratch：build/td-s12-md-summer，证据提炼后验证绝对路径/无reparse/无存活实例再回收。

## 后续最小路线

先证明安全的协议/来源识别，补同名伪类、ownerdraw拒绝、初始化和生命周期正负例；
再复用按钮/下拉renderer。下拉已有GetComboBoxInfo强证据，按钮身份还须深入验证。
目录树增加独立TreeView适配并保留节点图标、展开/选择、通知、键盘和滚动；
面板/无窗口文字另列边界。标题、菜单、现有字体策略及原图形资源保持。
本轮不扩大为产品实现，不新开数值T，保留T15等待用户外观验收。

## 收口复核

协调角色重新读取实际commit/diff、原始请求与控件证据；仅六调查文档，P1增加393行/删除4行，
产品167输入逐一保持，八部署摘要不变。治理自测13负例、diff检查通过。
登记scratch核实无存活自有实例、无reparse后回收31文件/2689580字节；验收PNG和稳定cache保留。
S12关闭，M0/T15开放；本轮未改产品、未构建新包或发布二进制。
