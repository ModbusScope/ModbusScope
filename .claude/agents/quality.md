---
name: quality
description: Run pre-commit quality checks (clang-format, clang-tidy, clazy) on source files. Use after making code changes before finishing a task.
tools: Bash
model: Haiku
---

Run quality checks from the project root.

For a specific file (faster):
  clang-format -i <file>
  ./scripts/run_clang_tidy.sh <file>
  ./scripts/run_clazy.sh <file>

For all files (slow):
  ./scripts/run_precommit.sh

Report only violations with file, line, and error message. If all checks pass, say so briefly.
