# M0 T5 — native demo audit

T4已关闭于7f0bff7；T5 S1 9d94cdf、S2 bd0dd65已推送并实际读取提交复核。
全任务冻结以下五单位；S3完成后再判断关闭，当前不提前宣称产品M0完成。

| 单位 | 实际路径/证据 | 完成谓词 |
| --- | --- | --- |
| 纯原生x64产品 | main/gallery适配固定源原生菜单/六类内容；独立x64 WIN32 CMake目标 | 无样式API/加载/链接、外观或测试开关；普通消息循环/通知/资源所有者清楚 |
| 同一文件两行为 | S2同一demo路径真实裸启动/CLI注入；Toolhelp模块路径与PrintWindow图像/排版 | 原生无DLL、注入同包64DLL；实际画面不同且业务初始数据相同，无偷换exe |
| UI业务保持 | 六类11窗口；Apply/复选/单选/列表/组合通知，滚动/Reset/Exit，系统最小/最大/恢复 | 实际通知更新业务label和控件数据；外观不改变返回和原生状态 |
| 边界与已有回归 | S3 demo/boundary解析实际PE及源码，core/launch/propagate/demo两宽全注册集 | 静态USER32/KERNEL32、无导出/延迟依赖；所有原矩阵重新通过，不靠旧项目结果 |
| 来源/版本/卫生 | 93回执、124处置/294 includes、23/13负例、五几何头；四文件M0 T5/哈希/PE | 无来源资源复制、sibling依赖、第五产品或renderer；实际提交/统计与scratch回收 |

S1选择源示例的标准菜单/控件业务，省略28页controller和显式着色/owner-draw/
私有字体/计数，见[S1记录](M0-T5-S1-demo.md)。产品只依赖Windows，窗口拥有
菜单与子控件，未交付菜单由创建层清理，无自建字体/GDI对象；renderer仍只有DLL。

S2首次失败在原生路径发现observer跨进程SetDlgItemText和传DC不适用；实际值仍
Native text，产品Apply正确使用该值。改为明确WM_SETTEXT/读回及PrintWindow，
保持原业务断言，未改产品。最终外部对照成功，见[S2记录](M0-T5-S2-pair.md)。
已目视实际两图，标题/菜单/六类内容完整；字符advance41/43、标题色像素0/26386。

私有桌面消息驱动的验证不替代真实鼠标/键盘、菜单展开、捕获、焦点、DPI/字体
环境验收。独立包候选明确接收这些边界、四文件同批发布、干净无sibling重建及
跨任务M0审计；不把后续未验证项藏成已通过，也不将未选28页扩张成新产品目标。

完整回归、四文件哈希、最终统计及卫生在本轮完成后补入。

## Artifact boundary

四文件FileVersion均M0 T5、数值0.5.0.0；demo/run16s/ntw16s64为AMD64/PE32+，
ntw16s32为I386/PE32。DLL仍ABI3、31公开名ordinal2..32及NONAME helper1，
六系统imports；demo只有USER32/KERNEL32，无exports或delay imports。
实际两宽Ninja deps各45项（28本地头/包含文件、17 MSVC/SDK），无sibling输入。
S3仅增加边界测试/注册/文档，未改生产代码或构建版本；当前哈希：

- run16s.exe：96574e2fbb1f6a55d68b219911311a0ca04d2c2b65fda2ca85acd47f2e90ee8c
- ntw16s32.dll：190bf5aa6264f08550af515dc62af2ad4b251b6b61fd86777ad3b492e3203beb
- ntw16s64.dll：c57c685029b252f8de2bd21826ac02a437f4064d96ade3cd3d7b7fe8abfaacc3
- demo.exe：3cd94f25800a56d4593e52cc3293a1645a70bf396590c608f32e651e42d38d34

本地源码不读取来源仓库。导入声明保留，两个demo adapt来自固定blob；不含原
Win3.1图形/字体像素。Source manifest正向/23负例、治理13负例及几何生成器
一致通过。完整回归未结束前不以这些静态结果代替行为通过。

## Final full regression

s3-full PID26864最终exit0：x86 16/16（102.15秒），x64 26/26（361.67秒）。
两宽各13项核心实际执行；x64全部CLI自动/八对话框/六消息框、三代24应用、
64创建向量、四故障回滚和错误包/stdio再通过；helper超时及目标先退出仍通过。
新增demo边界和同文件对照也通过，advance41/43、标题色0/26386与S2一致。
无跳过、无失败遗留，未修改原生产路径。各S事实与五单位对应完整；后续仅剩
独立包任务接收的最终发布/干净重建/真实输入及M0跨任务审计。

统计命令git diff 7f0bff7 --numstat -- src tests CMakeLists.txt：src（含说明/版本）
+173/-6、测试代码+167/-0、CMake+14/-1，合计+354/-7、净347（测试说明另+6/-1）。
S3程序源码零变更，boundary.py 51行，CMake仅注册+2；没有旧路径移动。
owned worker/fixture均已结束；精确绝对路径/祖先/子项无reparse后回收
build/t5-demo 309文件35306060字节，x86 Testing 2文件14831字节，x64 Testing
3文件27237字节。保留.ninja，不提交截图/原始日志或未正式验收二进制。

## Actual review and closure

S3 P1 a4e5504已推送，实际9文件+155/-24。协调角色读取实际boundary/构建差异、
五单位及S1/S2交付；git show --check通过，远端main一致。没有未处置T5范围
缺项，关闭S3及T5。P2记录本关闭并按用户授权自动准入T6独立包与M0验收；
不提前关闭其真实输入或发布目标。
