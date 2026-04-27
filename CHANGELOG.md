# Changelog

All notable changes to this project will be documented in this file.

## [Unreleased]

### Added

- `-v` short option for `--version`
- Centralized version management with `version.h` header
- Install and uninstall Makefile targets with configurable `PREFIX`
- Config language for common dotfiles (`.gitignore`, `.clang-format`, etc.)
- `--vcs=auto` as default behavior with `--no-vcs` option to disable
- Phase 5 extended diff features: alignment output, mode flags, and dynamic buffers

### Changed

- Rename default Makefile target from `all` to `build` for clarity
- Move build artifacts to `bin/` directory

### Fixed

- Comprehensive unit tests added for CLI, counter edge cases, and language definitions

---

## Development History

### 2026-04-15

**New Features**

- Regex-based file and directory filtering (`--include`, `--exclude` patterns)
- Multi-format output: YAML, XML, HTML, and SQL
- Archive extraction support for zip and tar formats
- Custom language definition support via `--lang-def`
- Git commit statistics API (`--git` option)
- `--list-file` and `--force-lang` CLI options
- Config file loading with uniqueness detection and `skip_lines` support
- Phase 4 CLI features: strip mode, content filtering, diagnostics, and extended options

**Architecture**

- Parallel processing refactored to use fork/pipe instead of popen
- Temp file manager and execution helper modules added
- Unit tests for parallel, git, filter, diff, unique, config, util, vcs, and output modules

**Fixes**

- Fixed `skip_leading` extensions parsing in config file

### 2026-04-14

**New Features**

- Core utility modules (path handling, file type detection, binary check)
- File processing modules (directory scanning, file list management)
- Multi-format output support (JSON, CSV)
- VCS file discovery (git, svn)
- `--exclude-list-file` option for file-based exclude patterns
- `--quiet` option and improved version output
- Git diff support for comparing commits
- Performance benchmark script
- Project README documentation

**Testing**

- Test framework and comparison tool added
- Block comment and continuation tests
- Language detection tests

**Fixes**

- Fixed extension detection

**Project Setup**

- AGENTS.md project documentation for AI assistants
- Code formatting with clang-format applied

### 2026-04-13

**Project Initialization**

- Initial project configuration and build setup
- Makefile build configuration
- Main C source program (entry point, CLI parsing, counting logic)
- Project documentation structure

