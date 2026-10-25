# Actor VB6复选框：原生只读诊断

用户截图指出左下两个复选框保持现代外观。本次只定位，不在T16轮廓移动候选中
夹带控件修复，不分配新T或覆盖CURRENT活动包。没有computer-use或可见桌面操作。

## 身份与原生证据

- O:/winnt/actor.exe：32位PE，98304字节，SHA256 `df4887ba302322cd3fd8c3d542015f492e4de2619ee62532d66eb465bfb51500`。
- 实际运行库 C:/Windows/SysWOW64/MSVBVM60.DLL，SHA256 `898288bd3b21d0e7d5f406df2e0b69a5bbfa4f241baf29a2cdf8a3cf4d4619f2`。
- 挂钩观察使用现部署 assets/ntw16s32.dll，SHA256 `f4611cd1a429eb7ce74aa9106f80198d93ce9aac28e6e14b7845fdb573078ae7`；模块路径由实际目标进程模块表取得。模块存在本身不作为绘制成功证据。
- 最终裸实例PID22640；挂钩实例PID25224，创建者run16s PID26412；主窗均ThunderRT6FormDC。
- 每组14个窗口：3个CommandButton、3个TextBox、4个Timer、1个Frame、2个CheckBox和主窗。
- 两控件GetClassName/RealGetWindowClass均为ThunderRT6CheckBox，GCLP_HMODULE匹配目标MSVBVM60模块；不是BUTTON原生superclass，也不是任意应用自绘类。
- 两控件style均 `0x54012006`，低四位6=BS_AUTO3STATE；WM_GETDLGCODE `0x2080`，BM_GETCHECK/BM_GETSTATE分别1/1和0/0。仅GET消息，无BM_SETCHECK/CLICK/输入操作。

纯C17/GCC观察器在自有私有桌面创建实例、等正常Form出现、枚举自有Job中的
实际目标窗口和模块，再WM_CLOSE并有界回收Job。两组最终标记均
`ACTOR_OBSERVE result=0 windows=14 checkboxes=2 active=0`。没有操作用户既有
实例。初次观察误选运行库ThunderRT6Main辅助窗，checks=0；观察器改为精确
Form后重跑，初次两组也active=0。此失败不作为产品缺陷或通过证据。

## 第一个断点与有限修复方向

src/ntw16s-dll/control_class.h中BUTTON的VB6映射只包含
ThunderRT6CommandButton/ThunderRT6Frame；RealGetWindowClass回退也不匹配。
control_protocol.c的角色识别不接纳ThunderRT6CheckBox。因此该真实类在分类
阶段就被排除，无法进入现有checkbox绘制。现有controls.c已支持BS_AUTO3STATE，
原control_glyphs资源包含相应状态；无需另造复选框资源。

后续控件S应限定MSVBVM60注册的准确类，复用现有绘制；先验证VB属性管理的
Check/三态循环、键盘/焦点/通知、disabled/pressed、字体借用及卸载恢复，不能
仅添加名称别名即宣称兼容。相似OptionButton类须单独取得实例和协议证据，不
凭名称顺带纳入。截图乱码不在此次复选框诊断范围，不改变已确认字体策略。

本次没有产品修复或二进制发布。原始C观察器及日志暂存已登记build/t16-outline，
由当前任务最终证据/清理步骤接管；核心回归不依赖Actor或其安装路径。
