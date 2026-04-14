<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-04-14 | Updated: 2026-04-14 -->

# tests

## Purpose
Unit tests for rloc using a custom lightweight test framework. Tests cover core modules: counter, language detection, filelist, string literal stripping, and comment handling.

## Key Files
| File | Description |
|------|-------------|
| `test_framework.h` | Test framework macros (TEST, ASSERT_EQ, ASSERT_STR, ASSERT_TRUE) |
| `test_framework.c` | Test registration and runner implementation |
| `test_counter.c` | Tests for line counting (blank, code, comment classification) |
| `test_language.c` | Tests for language detection by extension/shebang |
| `test_filelist.c` | Tests for directory scanning and file filtering |
| `test_strlit.c` | Tests for string literal stripping functionality |
| `test_block_comments.c` | Tests for block comment handling (/* */) |
| `test_continuation.c` | Tests for backslash line continuation handling |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `inputs/` | Test input files for various scenarios (see `inputs/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- Build tests: `make test` from root directory
- Individual test runners: `make test_runner`, `make test_strlit_runner`, etc.
- Run specific test: `./test_runner` after building
- Test registration: Each test file manually registers tests in `main()`

### Testing Requirements
- All tests must pass before changes are committed
- Use `ASSERT_EQ` for integers, `ASSERT_STR` for strings, `ASSERT_TRUE` for conditions
- Test function naming: `test_func_##name` via `TEST(name)` macro

### Common Patterns
- Test macro: `TEST(test_name) { ... }`
- Registration: `register_test("test_name", test_func_test_name)`
- Runner: `run_all_tests()` returns pass/fail count

## Dependencies

### Internal
- Test files depend on corresponding module (e.g., `test_counter.c` uses `counter.h`)
- All tests use `test_framework.h`

### External
- Standard C library only

<!-- MANUAL: Test-specific notes can be added below -->