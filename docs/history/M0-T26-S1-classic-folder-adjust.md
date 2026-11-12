# T26 S1 旧式文件对话框路径调整

将 `win16_dialog_classic_folder_adjust` 从 Win16 事件模块迁入已有
`lib-style/common/classic_file_helpers`，改名为
`style_classic_file_folder_adjust`。产品 COM 对话框适配和原链接测试已
切换到公共入口；原 Win16 实现和声明已删除。该接口仅处理调用方提供的
`OPENFILENAMEW` 和缓冲区，不依赖风格或绘制。

双位宽 MinGW 增量构建通过，公共辅助测试覆盖普通路径、盘符根目录、
UNC 路径、缓冲区不足和参数复制，Win16 对话框链接测试也双位宽通过。
未验证交互式文件选择，也未声称其他风格已经接入。测试临时目录已由测试
清理；来源清单与 `git diff --check` 通过。

尚未完成干净打包、发布及提交；既有 UWP 分发和 Win2000 DWM 恢复失败
仍待解决。
