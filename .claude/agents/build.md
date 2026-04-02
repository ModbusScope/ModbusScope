---
name: build
description: Compile the project using cmake and ninja. Use when code changes need to be built or when checking for compilation errors.
tools: Bash
---

Build the project from the project root.

If the build directory doesn't exist yet, run:
  mkdir -p build && cmake -GNinja -S . -B build

Then always run:
  ninja -C build

Report only errors and warnings. If the build succeeds, say so briefly.
