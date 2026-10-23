# 来源清单与本地导入回执验证

执行 `python tools/verify_source_manifest.py --self-test`，需要 Python 3.10+
标准库。默认只读取本仓库，不运行 git、不访问网络/兄弟目录，也不复制源码。
CI 执行同一命令。来源为 124 decisions / 294 includes；local imports 随已审核准的
实际导入增长，以 import-receipts.json 和校验输出为准。

该工具校验固定候选 inventory 与扩展清单的对应、处置/目的路径/接收者/声明、
源 blob/size/SHA 字段、全部 include 接收者、限制资源必须 replace、ABI 决策、
以及本地实际导入文件的回执/内容。元数据字段一致并不能证明原字节身份；
T1 的固定源对象研究复核另外执行如下可选命令：

```powershell
python tools/verify_source_manifest.py --self-test --source-repository <research-source>
```

该选项只对固定 source_commit 批量读取 Git tree 与 blob，逐项核验 mode、
大小、blob 与 SHA-256。它是来源研究工具，不是构建/产品验收的外部依赖。
没有此选项时不会猜测 sibling 路径；没有任何自动 fetch 或更新清单行为。

## 后续导入回执

实际导入任务须将 manifest 的 import_status 从 not-imported 改为 partial，
并新增同目录 import-receipts.json，完成全部目标后才改 imported。
产品 contract 的状态也须按实际源码导入状态同步，由产品任务一并交付。
每个条目有本地原字节 SHA-256；不要对 receipt 使用来源 hash 假充现有内容。
retain 必须 byte-exact，adapt 保留 source blob 并有明确 review，replace 不得
等于受限源字节且记录新素材来源。已排除/仅参考项不能通过回执变成导入。

回执结构如下，仅是 schema 示例，不能据此产生真实已导入声明：

```json
{
  "schema": 1,
  "files": [{
    "path": "src/ntw16s-dll/example.c",
    "source": "src/ntw16s-dll/example.c",
    "source_blob": "<40 lowercase hex digits from admitted manifest>",
    "sha256": "<64 lowercase hex digits of actual local bytes>",
    "disposition": "adapt",
    "review": "named reviewed delta and verification record",
    "notice": "retained source notice or original project authorship"
  }]
}
```

新编源码的 source/source_blob 为 null、disposition 为 authored，并明确自编
来源与 review；不得用 authored 绕过一个已经规划的源文件/替代资源目的路径。
导入 Detours 文件时必须同时有其原 LICENSE 回执。src/ 中未知/未登记文件
或目的路径越界/大小写别名被拒绝；所有 retain 源文件应保持 LF 原字节，导入
任务须配置相应属性或清楚记录必要 adapt，不能自动规范化后冒充原样导入。

本工具不对不可识别的复制/改名素材作视觉著作权鉴定，不替代实际来源审阅；
receipt 的 review/notice 内容仍由协调复核验证。零导入时无需创建空回执文件。

## ABI 计数纠正

固定头中共 34 条 public 声明，但 Ntw16sSetWindowTheme 同时出现在 attachment.h
和 theme.h，签名相同。因此是 33 个不同源 symbol，31 个保留 public symbol，
2 个转私有；再新增 helper ordinal 1。S2 的 32 public 是“声明条数”，不能
当成 32 个不同导出。校验器保留两条来源声明记录，并拒绝同名签名/决定冲突。

自检包含 23 个结构/内容/路径/ABI/声明/回执负例及一个合法自编替代回执。
这些是内存 fixture，不会把原图/伪造产品文件写入工作区。
