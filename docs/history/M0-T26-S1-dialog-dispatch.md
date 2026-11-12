# T26 S1 传统对话框消息分发

旧式文件对话框初始化、快捷方式解析调度及 COM 对话框消息分发从
`win16-dialog/file_hook` 和 `com_hook` 迁入 `lib-style/common`，由
`style-dialog-common` 提供。公共 COM 分发的窗口属性名由调用方提供；
Win16 产品继续提供原来的属性名、导航消息和回调，既有事件与样式策略仍
由产品及 Win16 代理持有。旧源文件已删除。

双位宽 MinGW 增量 DLL 构建以及 `core-win16-dialog-chrome`、
`core-win16-dialog-link` 测试通过；前者覆盖自定义属性名、回调、消息和
属性清理。测试未覆盖完整交互式文件对话框或其他风格实际接入。测试产生的
临时目录已清理；来源清单及 diff 格式检查通过。

尚未完成干净打包、发布和提交；已知 UWP 分发及 Win2000 DWM 恢复失败
仍未解决。
