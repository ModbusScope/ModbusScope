# ModbusScope — Claude Code Guide

## Build

All commands run from the **project root** unless noted.

**Requirements:** Qt 6, C++20, Ninja. Compiler flags: `-Wall -Wextra -Werror`.

Configure and build (one-time setup, then just `ninja`):

```bash
mkdir -p build && cmake -GNinja -S . -B build
ninja -C build
```

Run tests (must build first):

```bash
ctest --test-dir build --output-on-failure
```

Run pre-commit checks (clang-format + clang-tidy + clazy — all must pass):

```bash
# For all checks (takes a while):
./scripts/run_precommit.sh
```

To run checks on a single file (faster during development):

```bash
clang-format -i src/path/to/file.cpp
./scripts/run_clang_tidy.sh src/path/to/file.cpp
./scripts/run_clazy.sh src/path/to/file.cpp
```

**Always run pre-commit after making source file changes.** All three checks must pass before the work is done.

## Project Structure

```text
src/
├── communication/   Communication (using external adapters)
├── models/          Data models (SettingsModel, GraphDataModel, DiagnosticModel, Device, Connection)
├── datahandling/    Expression parsing, graph data processing
├── importexport/    CSV export, MBS project files, MBC device config import
├── dialogs/         UI dialogs and MainWindow
├── customwidgets/   Qt custom widgets
├── graphview/       Graph rendering (qcustomplot)
└── util/            Logging (ScopeLogging), formatting helpers
tests/               Google Test + Qt Test; test files named tst_*.cpp
libraries/           Vendored: qcustomplot, muparser
```

## Architecture

- **SettingsModel** is the central config store — connections, devices, poll time
- **ModbusPoll** orchestrates polling: iterates connections, delegates to **ModbusMaster** per connection
- **ModbusMaster** manages one connection's read cycle via **ModbusConnection** (Qt Modbus)
- **GraphDataHandler** processes raw Modbus results and applies user expressions
- Models emit Qt signals; UI layers observe them — no direct model→UI calls

## Code Style

Enforced by `.clang-format` (Mozilla-based, C++20):

- 4 spaces, no tabs; 120-char line limit
- Braces on new line for classes/functions
- Pointer left-aligned: `Type* ptr`
- Private members: `_camelCase`; pointer members: `_pName`
- Classes: `PascalCase`; methods: `camelCase`
- Use Qt doxygen style for comments: `/*!` for multi-line blocks, `//!` for single-line; use `\brief`, `\param`, `\return` (backslash prefix, not `@`)
- Use `#ifndef`/`#define`/`#endif` include guards in all header files (not `#pragma once`); guard name is the filename uppercased with dots replaced by underscores (e.g. `FOO_BAR_H`)

## Key Conventions

- Prefer readability and maintainability over using the latest C++ features (avoid syntax sugar that may be less familiar to new contributors).
- Avoid lambda expressions with more than 2 captures or multiple statements; use named functions instead for clarity.
- Make sure to document public functions with brief Doxygen comments in the source file
- Only use early return for error handling, avoid deep nesting
- When fixing a bug, add a test that reproduces the issue before implementing the fix.

## Development

Three sub-agents are defined in `.claude/agents/` to keep build/test/lint output out of the main context:

- **`@agent-build`** - runs cmake + ninja; reports only errors and warnings.
- **`@agent-test-runner`** - runs ctest; reports only failing tests with their error messages.
- **`@agent-quality`** - runs clang-format, clang-tidy, and clazy; reports only violations.
- **`@agent-code-reviewer`** - reviews code for quality, safety, and best practices; provides specific, actionable feedback.

Always use these agents rather than running the commands directly. After making source file changes: build, then run tests, then run quality checks - all must pass before the work is done.
