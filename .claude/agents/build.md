---
name: build
description: Compile the project using cmake and ninja. Use when code changes need to be built or when checking for compilation errors.
tools: Bash
model: haiku
effort: low
---

Build the project from the project root.

Run:
  mkdir -p build
  cmake -GNinja -S . -B build
  ninja -C build

Report only errors and warnings. If the build succeeds, say so briefly.
