# M0 T20 S11：公共对话框扩展错误契约

## 原生观察

[微软 CommDlgExtendedError 文档](https://learn.microsoft.com/en-us/windows/win32/api/commdlg/nf-commdlg-commdlgextendederror)
规定其返回最近公共对话框的扩展错误；取消时返回0，成功后值未定义。它并未
在该页面承诺所有实现细节。`GetLastError` 是另一接口，不能作为替代。

自有纯C探针在私有桌面上分别直接运行及由当前发布包启动；x86/x64、ANSI/Unicode
共8组在本机得到相同结果：

- 错误结构的打开文件返回FALSE，扩展错误 `CDERR_STRUCTSIZE` (1) 可多次读取；
  显式设置普通last-error为`12345678`后再读扩展错误，普通last-error观察为0。
  后者是本机观察，不把文档未承诺的副作用推广为保证。
- 错误hook的打开文件产生 `CDERR_NOHOOK` (11)，随后错误结构的ChooseColor
  将其变为1，表明覆盖跨不同公共对话框类型，而非仅跨文件API。
- 线程A保留错误1的同时线程B产生并再次读取11；两线程的观察互不覆盖。
  本机实测为线程隔离，不以此推断所有平台或所有异步边界。
- 非嵌套时随后实际显示并取消打开文件框，FALSE后连续两次扩展错误均为0。探针自有
  线程有5/7秒上限，外层自有driver有超时及作业回收；未使用物理输入。
- 嵌套负例：外层打开文件框hook内调用错误结构的ChooseColor，其扩展错误为1；外层随后取消后，本机原生和注入两路仍连续读到1，双宽A/W各8组一致。初版错误地断言取消必须为0，8组均失败后保留该证据；修订探针把非嵌套及嵌套分开，并允许其他Windows实现返回文档所述的0。不能笼统地在外层退出时清零。

## 对产品实现的影响

S10 的受控IDABORT返回FALSE且本机扩展错误65535；探针自行生成的12291
并不是 `CommDlgExtendedError()` 的实际结果。不能只使用 `SetLastError`，
也不能把线程局部伪错误永久置位：后续ChooseColor、取消和其他公共API必须
覆盖它。重复读取、嵌套对话框和调用次序需要准确归属；同一错误值可能由不同
调用产生，单纯比较当前原生错误数值也不能证明没有发生新调用。

搜索 `CommDlgExtendedError|FindText[AW]|ReplaceText[AW]|ChooseFont[AW]|ChooseColor[AW]|PrintDlg|PageSetupDlg|GetOpenFileName|GetSaveFileName`：
`file_dialogs.c`拥有打开/保存入口；`window_titles.c`还挂接查找替换、字体、
颜色、普通及扩展打印和页面设置；`com_dialogs.c`、`folder_dialogs.c`、
`print_extended.c`内部也调用公共对话框或读取扩展错误。设计必须区分内部
调用、嵌套作用域和应用调用，且考虑未被列出的其他系统入口，不可静默污染。
本S没有修改产品，普通Win32多选仍原生回退；S10正常确认路径的其他契约亦未
由错误探针解决。

首个`s11-error-final`快照在发现嵌套差异后停止，未作为验证；修订版重新
构建并保留两种调用次序。嵌套实测结果是本机与注入对照，不扩张为所有系统
必然偏离文档的声明。

## S11 同批包验证

`s11-error-final2` 使用GCC/Ninja/ccache双宽干净构建并通过资源、来源及ABI
门禁；`s11-error-focus` 实际同批包28项通过、无跳过，约40秒：16项双宽 A/W
直接/注入、普通及嵌套错误对照，4项S10小缓冲原型及8项必选基础回归。
非嵌套和嵌套的8对原生/注入 `ERROR_CANCEL` 实测逐行一致；嵌套版本测试允许
文档所述0或本机1，避免把本机异常扩展为跨版本保证。消息驱动不是物理输入。

仅增加测试及文档，无产品源码或二进制变更；assets与O:/winnt四文件SHA一致，
没有覆盖为不同字节的重建包。重建包SHA-256：run16s
`7e761369abe1145b6c06d3cbc99d42c8662440099a1a6fba7d35543a9f9859af`；
ntw16s32 `37c52804e55cc0417a8d932c5d1e3afb271b879b163d8793a5e7867f032fd99d`；
ntw16s64 `d775bc2eea74ff33f272f5ba6aeccf5dd59b1338f017f1c391fbb6b0df8094e3`；
demo `de430d6f28b3a7ccaf2e9ff82783d5660ba03641e7cfb78baca9678d4a0a6b0d`。

S11 P1 `90f9736` 已推送，GitHub 实际测试、记录及四文件对象复核一致；
测试和构建描述+89/-0（探针83、CMake3、包测试3）。S11 收口，自动准入S12。
