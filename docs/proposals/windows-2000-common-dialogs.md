# Windows 2000 公共对话框

M0 T28 S3 对齐 [Win16 已交付的调用范围](../history/M0-T20-closure-audit.md)。
以真实 Windows 2000 样本和原版源码确认布局，Win16 记录用于确定入口和参数行为。
同一入口分别核对窗口外观、调用契约和系统回退。

| 执行顺序 | 入口 | 当前 Win2000 基线与验收 |
| --- | --- | --- |
| 1 | `GetOpenFileNameA/W`、`GetSaveFileNameA/W` | 已观察原生弹窗；核对普通单选打开与保存的 Win2000 布局、过滤器、初始目录、结果缓冲、取消和扩展错误。对模板、回调及多选逐项对照现有 Win16 的系统回退。 |
| 2 | `SHBrowseForFolderA/W`、`IFileOpenDialog`／`IFileSaveDialog` | Win2000 装配缺对应的旧布局转换；核对原版目录树、选择/取消、PIDL 与 COM 结果。在受支持的 COM 请求中复用公共参数与回写机制；自定义控件或受限回调走已验证的原生路径。 |
| 3 | `FindTextA/W`、`ReplaceTextA/W`、`ChooseFontA/W`、`ChooseColorA/W` | 查找替换和颜色已观察，字体缺专用入口。验证模型无关通知、重复操作、字体选择返回值、基本／自定义颜色及取消行为。 |
| 4 | `PrintDlgA/W`、`PrintDlgExA/W`、`PageSetupDlgA/W` | 页面设置已观察，打印入口需核对。验证页范围、打印机选择、句柄所有权、打印与应用结果；设备、驱动或回调受限用例记录实际回退或跳过。 |
| 5 | `MessageBox`、`MessageBoxEx`、`MessageBoxIndirect` A/W | 已有观察。核对普通图标、按钮组、默认按钮及返回值。应用私有模板与系统代建框按已证实的窗口归属处理。 |

各批先确定来源、当前 Win16 与 Win2000 的真实行为和适用参数，再实现并增量测试。
共用的观察、参数、导航和回写复用 application/dialog；年代模板、配色、度量和
选择策略留在各年代。以本地 Win2000 源码树和真实 Windows 2000 界面取证。
源码树缺少的资源模板单独登记，并以具有可核对来源的实际样本补证。原图形资源及
现有声明保留；实现不依赖源码包或兄弟仓库。

首批源码核对已确认 `private/shell/comdlg32/comdlg32.rc` 引用了
`fileopen.dlg`、`prnsetup.dlg`、`findtext.dlg`、`color.dlg`、`font.dlg`
和 `printnew.dlg`。当前目录仅查到最后一项；两个已读文件的摘要与
[T25 取证记录](windows-2000-classic.md)一致。已从原版 Windows 2000 SP2
安装镜像只读核对资源，普通 Win32 打开／保存的 Explorer 布局有 1547 和
1552 两种模板，默认由原版代码按版本、模板／回调标志及 Places Bar 策略选择。
当前 Windows 11 的 1552 资源客户区多 40 个对话框单位。运行时由 DirectUI
重排，保存对话框有时会折叠文件列表。兼容的 COM 请求复用公共控制器，
文件模式保留 Explorer 导航，文件夹模式复用经典目录树；模板、回调和
多选按各入口保留原生路径。[来源与原生基线](../etc/evidence/t28-s3-dialog-reference.json)

用自有普通桌面探针验证 x86/x64 的普通启动路径，逐组覆盖成功、取消、返回值、
焦点、原生回退和资源清理，补充相关全局入口及 Win16 回归。验收记录区分已实现、
系统回退、有条件跳过和未验证。最终按执行规则用同快照八件先发布两处、再测试，
提交推送后逐件核对远端。XAML 客户区、系统主题、未知私有模板和其他控件留在各自范围。
