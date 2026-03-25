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
./scripts/run_precommit.sh
```

**Always run pre-commit after making source file changes.** All three checks must pass before the work is done.

## Project Structure

```text
src/
├── communication/   Modbus protocol layer (ModbusPoll, ModbusMaster, ModbusConnection)
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
- Make sure to document public functions with brief Doxygen comments in the source file
- Only use early return for error handling, avoid deep nesting
- When fixing a bug, add a test that reproduces the issue before implementing the fix.
