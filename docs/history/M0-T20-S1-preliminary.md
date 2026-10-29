# M0 T20 S1 — 初步接口与经典参考调查（进行中）

用户准入文件夹选择和公共对话框任务；本记录是早期证据，不是适配完成或最终接口矩阵。

本机 `D:/programs/misc/DoubleKiller.exe` SHA-256 `cdd83ab909db749992d11ace2dff671493c0568d7a14697f8bef4b06d8647ca8` 静态导入 `SHBrowseForFolderA`、`SHGetPathFromIDListA`、`GetOpenFileNameA`、`GetSaveFileNameA` 和 `CoCreateInstance`。`D:/programs/misc/md5summer.exe` SHA-256 `4646ce4f638e7d7ec820e7540b39bb185234e1f151d780c734ca557d6dba7ed9` 静态导入 `GetOpenFileNameA`、`GetSaveFileNameA` 和 `CoCreateInstance`。静态导入不证明截图的实际入口或任一功能调用；截图所属应用和真实参数尚未确认。

纯C自有探针 `SHBrowseForFolderW` 使用自有 `build/t20-s1-dialog/selected` 作为根与选择，分别对 x86/x64、带/不带 run16s、旧式 `BIF_RETURNONLYFSDIRS=0x1` 与新式 `BIF_RETURNONLYFSDIRS|BIF_USENEWUI=0x51` 共八次观察。八次均返回目标PIDL、`SHGetPathFromIDListW` 解析结果等于输入路径，初始化回调一次；旧式选择变化一次，新式三次。资源按API使用 `CoTaskMemFree` 释放。为自有测试发送选择与确认，并未覆盖真实用户输入、取消、虚拟节点、网络、重入或完整回调。探针进程没有超时，正常返回；其余测试不推定。

控件枚举显示旧式 `#32770` 包含 `SysTreeView32`，新式含 `SHBrowseForFolder ShellNameSpace Control`、嵌套 `SysTreeView32`、编辑框、新建文件夹及确认/取消。后续可见窗口 `PrintWindow` 的 x86 样式截图显示两者均仍用现代目录图标/树，旧式并非 Win3.1 的文件打开布局；新式的输入与新建功能不等于旧式。截图仅登记 scratch `build/t20-s1-dialog/old-styled.bmp` 和 `new-styled.bmp`。

用户自有 `O:/w31/SYSTEM/COMMDLG.DLL` 是 Win16 NE，SHA-256 `853574d46a8e0103028354883cc597e762bafd034b60c15aedc8cdc097629ec8`。只读资源表发现8个对话框、11个位图、6个图标；资源1536/1537的 Open 模板包含 File Name、Directories、List Files of Type、Drives、Cancel 等字段，原始资源记录的对齐长度分别352字节，SHA-256 `1580e866d7bb25dc74e59aa78dd2ef25342b7c4adca3472ef24832746acb79cf` / `1faeb6fb78598b5eb594ae00916a40f72aa25c63507eb791f5024ab399a97458`。对齐填充包含在摘要内；没有导入或分发原始资源，许可及目录图形身份尚未审查。最初一次资源表读取因偏移算错产生越界结果，已丢弃并在修正后核对资源全在文件内。

微软接口文档说明 SHBrowseForFolder 返回 PIDL，应用需释放，且新旧风格能力不同；历史 Windows 3.1 old-style GetOpenFileName/GetSaveFileName 和 Shell browse 是不同接口。不得为了外观擅自清除标志或替换语义。参考：https://learn.microsoft.com/en-us/windows/win32/api/shlobj_core/nf-shlobj_core-shbrowseforfolderw 和 https://devblogs.microsoft.com/oldnewthing/20180917-00/?p=99745/。

后续仍需真实应用动态调用参数及回调、真实目录/盘符资源和许可、经典参考截图/行为、路径/PIDL/错误/取消/网络/Unicode等回退矩阵，并先验证可保持语义的设计。当前未修改或发布产品。
