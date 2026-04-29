# rloc

rloc (Rusty/Recursive Line of Code) 是一个 C11 实现的代码行数计数器，类似于 cloc。它能够统计项目中的空白行、注释行和代码行数，支持 24 种编程语言的智能注释检测。

## 特性

- **多语言支持**：覆盖 C/C++、Python、Java、JavaScript、Go、Rust 等 24 种主流语言
- **智能注释检测**：准确识别单行注释、多行块注释，并正确处理字符串中的注释标记
- **字符串字面量保护**：避免将字符串内的注释标记误判为真实注释
- **续行符处理**：正确处理 C 预处理器和 Shell 脚本中的反斜杠续行
- **多检测策略**：通过内容模式、扩展名、精确文件名、shebang 四种方式识别语言
- **零外部依赖**：仅使用 C11 标准库和 POSIX 函数
- **命令行过滤**：支持按语言、扩展名、目录进行灵活过滤

## 构建

```bash
# 编译主程序
make

# 运行所有测试
make test

# 格式化代码（需要 clang-format）
make fmt

# 清理构建产物
make clean
```

### 编译要求

- C11 编译器（gcc、clang 等）
- POSIX 兼容系统（Linux、macOS）
- 可选：clang-format（用于代码格式化）

## 使用方法

```bash
# 统计单个文件
./rloc main.c

# 统计目录（递归）
./rloc src/

# 统计多个路径
./rloc src/ include/ main.c

# 排除特定目录
./rloc . --exclude-dir vendor --exclude-dir node_modules

# 仅统计特定语言
./rloc . --include-lang Python,C

# 接受特定扩展名
./rloc . --accept-ext .inc

# 拒绝特定扩展名
./rloc . --reject-ext .min.js

# 不递归子目录
./rloc . --no-recurse

# 显示帮助
./rloc --help

# 显示版本
./rloc --version
```

## 输出格式

输出为文本表格，按语言分组显示统计结果：

```
-------------------------------------------------------------------------------
Language             files    blank   comment       code
-------------------------------------------------------------------------------
C                       10       120        89       1456
Python                   5        45        67        312
JavaScript               3        28        34        198
-------------------------------------------------------------------------------
SUM                     18       193       190       1966
-------------------------------------------------------------------------------
```

## 支持的语言

| 语言 | 扩展名 | 注释类型 |
|------|--------|----------|
| C | .c, .h | 单行 `//` + 块 `/* */` |
| C++ | .cpp, .cxx, .cc, .hpp, .hxx, .hh | 单行 `//` + 块 `/* */` |
| Python | .py, .pyw | 单行 `#` |
| Java | .java | 单行 `//` + 块 `/* */` |
| JavaScript | .js, .mjs | 单行 `//` + 块 `/* */` |
| TypeScript | .ts, .tsx | 单行 `//` + 块 `/* */` |
| Go | .go | 单行 `//` |
| Rust | .rs | 单行 `//` + 块 `/* */` |
| Ruby | .rb | 单行 `#` |
| PHP | .php | 单行 `//`, `#` + 块 `/* */` |
| Shell | .sh, .bash, .zsh, .ksh | 单行 `#` |
| Perl | .pl, .pm | 单行 `#` |
| Swift | .swift | 单行 `//` |
| Kotlin | .kt, .kts | 单行 `//` |
| C# | .cs | 单行 `//` + 块 `/* */` |
| CSS | .css | 块 `/* */` |
| HTML | .html, .htm | 块 `<!-- -->` |
| XML | .xml | 块 `<!-- -->` |
| SQL | .sql | 单行 `--` |
| TOML | .toml | 单行 `#` |
| YAML | .yaml, .yml | 单行 `#` |
| JSON | .json | 无注释支持 |
| Markdown | .md, .markdown | 无注释支持 |
| Vue | .vue | 复用 JavaScript 规则 |
| Makefile | Makefile | 单行 `#` |
| Dockerfile | Dockerfile | 单行 `#` |

## 项目架构

```
rloc/
├── main.c           # 程序入口，协调各模块
├── cli.c / cli.h    # 命令行参数解析
├── counter.c / counter.h  # 核心行计数逻辑
├── counter_interface.h  # Counter 接口抽象层
├── counter_ops.c / counter_ops.h  # 计数操作工具函数
├── language.c / language.h  # 语言检测
├── lang_defs.c / lang_defs.h  # 语言定义数据
├── filelist.c / filelist.h  # 目录扫描
├── strlit.c / strlit.h  # 字符串字面量处理
├── output.c / output.h  # 结果格式化输出
├── output_writer.c / output_writer.h  # Output Writer 接口
├── scanner.c / scanner.h  # 输入路径扫描模块
├── input_handler.c / input_handler.h  # 输入处理模块
├── util.c / util.h  # 工具函数
├── error.c / error.h  # 错误处理
├── file_processor.c / file_processor.h  # 文件处理协调
├── report.c / report.h  # 报告生成
├── vcs.c / vcs.h  # 版本控制系统检测
├── vcs_ops.c / vcs_ops.h  # VCS 操作
├── diff.c / diff.h  # Diff 统计
├── temp_manager.c / temp_manager.h  # 临时文件管理
├── exec_helper.c / exec_helper.h  # 命令执行辅助
├── archive.c / archive.h  # 归档文件处理
├── parallel.c / parallel.h  # 并行处理
├── unique.c / unique.h  # 去重工具
├── config.c / config.h  # 配置文件处理
├── threaded_counter.c / threaded_counter.h  # 多线程计数
├── coro_scanner.c / coro_scanner.h  # 协程扫描器
├── coco/            # Coco 协程库
├── Makefile         # 构建配置
├── tests/           # 测试代码
├── docs/            # 文档
└── tools/           # 辅助脚本
```

### 模块依赖关系

```
main.c (入口点，协调各模块)
  ├── cli.h           # 参数解析
  ├── config.h        # 配置加载
  ├── scanner.h       # 输入路径扫描
  │     ├── filelist.h    # 文件发现
  │     ├── vcs_ops.h     # VCS 操作
  │     └── coro_scanner.h # 协程扫描
  ├── file_processor.h  # 文件处理协调
  │     ├── counter_interface.h  # Counter 接口
  │     │     └── counter.h  # 行计数实现
  │     └── language.h  # 语言检测
  ├── report.h        # 报告生成
  │     └── output_writer.h  # Output Writer 接口
  │           └── output.h  # 输出格式化
  └── error.h         # 错误处理
```

## 核心算法

### 行分类流程

1. **字符串预处理**：将字符串字面量内容替换为空格，保留换行符
2. **状态机扫描**：追踪块注释状态、续行状态，逐行分类
3. **行类型判定**：
   - 空白行：仅包含空白字符
   - 注释行：纯注释内容或续行继承
   - 代码行：包含非注释的可执行代码

### 续行符处理

支持 C 预处理器和 Shell 的反斜杠续行语义：

```c
// 注释续行仍为注释 \
int x = 1;  // 这行仍是注释行

#define MACRO \
    value    // 这两行都是代码行
```

### 语言检测优先级

```
内容模式匹配 (读取前512字节，需匹配至少2个特征模式)
    → 精确文件名匹配 (Makefile, Dockerfile)
    → 扩展名匹配 (.py, .c, .rs)
    → Shebang 检测 (#!/usr/bin/python)
```

## 添加新语言

在 `lang_defs.c` 中添加定义：

```c
static const GenericFilter lua_filters[] = {
    {FILTER_REMOVE_INLINE, "--", NULL},
    {FILTER_REMOVE_BETWEEN, "--[[", "]]"},
};

// 在 g_languages 数组中添加
{
    .name = "Lua",
    .extensions = "lua",
    .generic_filters = lua_filters,
    .generic_filter_count = 2,
    .str_delimiters = "\"'",
    .str_escape = "\\",
}
```

更新 `lang_defs.h` 中的 `NUM_LANGUAGES` 宏。

## 测试

项目使用自定义轻量级测试框架：

```bash
# 运行所有测试
make test

# 测试覆盖的模块
# - counter.c (行计数核心)
# - filelist.c (文件扫描)
# - language.c (语言检测)
# - strlit.c (字符串处理)
# - block_comments (块注释)
# - continuation (续行符)
# - archive.c (归档处理)
# - temp_manager.c (临时文件管理)
# - parallel.c (并行处理)
# - diff.c (Diff 统计)
# - vcs.c (VCS 检测)
# - cli.c (命令行解析)
# - config.c (配置处理)
# - util.c (工具函数)
# - unique.c (去重)
# - output.c (输出格式化)
# - lang_defs.c (语言定义)
# - coro_scanner.c (协程扫描)
# - counter_interface.h (Counter 接口)
# - output_writer.c (Output Writer 接口)
# - scanner.c (输入扫描模块)
```

### 与 cloc 对比

```bash
# 使用对比脚本
tools/compare_cloc.sh <file_or_dir>
```

## 已知限制

- 不支持嵌套块注释（如 `/* /* nested */ */`）
- Python docstring 未特殊处理（计为代码）
- 不支持文档注释标记区分（如 `/** */` vs `/* */`）
- 大文件处理采用完整读取，内存峰值较高

## 许可证

MIT License

## 参考

本项目灵感来源于 [cloc](https://github.com/AlDanial/cloc)，旨在提供一个轻量、无外部依赖的 C 语言实现。