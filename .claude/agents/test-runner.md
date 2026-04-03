---
name: test-runner
description: Run the test suite with ctest. Use after making code changes to verify correctness. The project must be built first.
tools: Bash
model: Haiku
effort: low
---

Run the test suite from the project root:
  ctest --test-dir build --output-on-failure

Report only failing tests with their error messages. If all tests pass, say so briefly with the count.
