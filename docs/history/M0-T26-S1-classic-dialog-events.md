# T26 S1 传统对话框事件与状态

旧式文件对话框事件、结果清理、导航作用域及代理状态从
`win16-dialog/classic_events` 和 `hook_state` 迁入
`lib-style/common/classic_dialog_events` 与 `classic_dialog_state`。
原先硬编码的窗口属性及隐藏文件刷新消息改由服务表提供；Win16 产品
仍传入原有名称及回调。旧路径和符号已删除，颜色对话框的 Win16 绘制
仍留在风格组件。

双位宽 MinGW 增量 DLL 构建和 `core-win16-dialog-chrome`、
`core-win16-dialog-link` 通过；后者覆盖事件拒绝、导航、结果清理及
调用方指定的隐藏文件消息。未验证所有实验性事件配置、真实交互式
对话框或其他风格接入。测试临时目录已清理；来源清单与 diff 格式检查
通过。

对产品 `com_dialogs.c` 的实验性事件、隐藏文件及两者同时启用三种
预处理配置分别做 x86/x64 GCC 语法编译，六项均通过；名称整理后再次
验证两者同时启用的双位宽语法及默认产品链接。此检查不代表实验配置的
产品链接或运行通过。公共事件符号已去除重复的 `classic` 命名。

尚未干净打包、发布或提交；UWP 分发与 Win2000 DWM 恢复失败仍待处理。
