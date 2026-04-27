# Changelog

All notable changes to this project will be documented in this file.

## [0.1.2] - 2026-04-27

### Added

- Threaded parallel counting with `--threads` and `--fork` options
- Performance optimizations: reduced stat calls and file reads

### Changed

- Applied clang-format code style formatting to parallel counting modules

## [0.1.1] - 2026-04-15

### Added

- `-v` short option for `--version`
- Centralized version management with `version.h` header
- Install and uninstall Makefile targets with configurable `PREFIX`
- Config language for common dotfiles (`.gitignore`, `.clang-format`, etc.)
- `--vcs=auto` as default behavior with `--no-vcs` option to disable
- Phase 5 extended diff features: alignment output, mode flags, and dynamic buffers
- Regex-based file and directory filtering (`--include`, `--exclude` patterns)
- Multi-format output: YAML, XML, HTML, and SQL
- Archive extraction support for zip and tar formats
- Custom language definition support via `--lang-def`
- Git commit statistics API (`--git` option)
- `--list-file` and `--force-lang` CLI options
- Config file loading with uniqueness detection and `skip_lines` support
- Phase 4 CLI features: strip mode, content filtering, diagnostics, and extended options

### Changed

- Rename default Makefile target from `all` to `build` for clarity
- Move build artifacts to `bin/` directory
- Parallel processing refactored to use fork/pipe instead of popen

### Fixed

- Comprehensive unit tests added for CLI, counter edge cases, and language definitions
- Fixed `skip_leading` extensions parsing in config file
- Temp file manager and execution helper modules added
- Unit tests for parallel, git, filter, diff, unique, config, util, vcs, and output modules

## [0.1.0] - 2026-04-14

### Added

- Core utility modules (path handling, file type detection, binary check)
- File processing modules (directory scanning, file list management)
- Multi-format output support (JSON, CSV)
- VCS file discovery (git, svn)
- `--exclude-list-file` option for file-based exclude patterns
- `--quiet` option and improved version output
- Git diff support for comparing commits
- Performance benchmark script
- Project README documentation
- Content-based language detection
- `.conf` extension to Config language
- Recognize `go.mod` and `go.sum` as Go files
- Lua language support
- nginx.conf language support

### Changed

- Code formatting with clang-format applied

### Fixed

- Fixed extension detection
- Block comment and continuation tests
- Language detection tests

### Project Setup

- AGENTS.md project documentation for AI assistants
- Makefile build configuration
- Main C source program (entry point, CLI parsing, counting logic)
- Test framework and comparison tool added
- Initial project configuration and build setup