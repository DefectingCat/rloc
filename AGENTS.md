<!-- Generated: 2026-04-14 | Updated: 2026-04-14 -->

# rloc

## Purpose
A C11 implementation of a line-of-code counter, similar to cloc. Counts blank lines, comment lines, and code lines across multiple programming languages with language-aware comment detection.

## Key Files
| File | Description |
|------|-------------|
| `main.c` | Entry point - orchestrates CLI parsing, file scanning, counting, and output |
| `cli.c` / `cli.h` | Command-line argument parsing (--help, --exclude-dir, --include-lang, etc.) |
| `counter.c` / `counter.h` | Core line counting logic with language-aware comment detection |
| `language.c` / `language.h` | Language detection by extension, filename, or shebang |
| `lang_defs.c` / `lang_defs.h` | Language definitions for 24 languages (Python, C, Java, etc.) |
| `filelist.c` / `filelist.h` | Directory scanning and file list management |
| `strlit.c` / `strlit.h` | String literal stripping to avoid false comment detection |
| `output.c` / `output.h` | Result formatting and text table output |
| `util.c` / `util.h` | Utility functions (path handling, file type detection, binary check) |
| `Makefile` | Build configuration - compiles main program and test runners |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `tests/` | Unit tests and test framework (see `tests/AGENTS.md`) |
| `docs/` | Documentation files (see `docs/AGENTS.md`) |
| `tools/` | Helper scripts for comparison (see `tools/AGENTS.md`) |
| `cloc/` | Original cloc repository reference (see `cloc/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- Build: `make` creates `rloc` binary
- Test: `make test` runs all test runners
- Format: `make fmt` applies clang-format to all source files
- Clean: `make clean` removes binaries
- Use C11 standard with `-Wall -Wextra` warnings
- All source files include corresponding headers first

### Testing Requirements
- Run `make test` before committing changes
- Tests use a custom framework in `tests/test_framework.h`
- Each module has dedicated test file (test_counter.c, test_language.c, etc.)

### Common Patterns
- Language detection: `detect_language(filepath)` returns `Language*`
- Counting: `count_file_with_lang(filepath, lang, &result)`
- Filter types: `FILTER_REMOVE_INLINE` (// comments), `FILTER_REMOVE_BETWEEN` (/* */ blocks)
- String stripping: `strip_string_literals()` handles language-specific string delimiters

## Dependencies

### Internal
- All modules depend on `util.h` for path/file utilities
- `counter.c` depends on `strlit.c` for string literal handling
- `language.c` depends on `lang_defs.c` for language definitions

### External
- Standard C11 library only - no external dependencies
- POSIX functions: `stat`, `dirent`, `strtok_r`, `strcasecmp`

<!-- MANUAL: Project-specific notes can be added below -->