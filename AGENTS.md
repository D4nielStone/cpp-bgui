# BGUI agent instructions

## Project
BGUI is the retained-mode C++20 UI library used by Bubble Engine. It contains
the UI core, layout and element implementations, platform backends, and tests.

## Rules
1. Treat suspected defects as hypotheses. Confirm each one in the current code
   and, when possible, reproduce it with a failing test before changing code.
2. Keep fixes focused and preserve the public API. Avoid unrelated refactors.
3. Add regression coverage for fixes when practical. Prefer headless tests for
   behavior that does not require a graphics context.
4. Do not claim a fix works unless the relevant target builds and its tests run.
   State clearly when display, GPU, or platform validation is unavailable.
5. Do not rewrite layout/rendering architecture or add concurrency or
   micro-optimizations as part of routine bug fixing.
6. Keep changes within the requested BGUI scope and preserve unrelated working
   tree changes.
7. For changes made through cpp-bengine, update `docs/STABILIZATION.md` with
   the finding, evidence, fix, tests, and remaining validation limits.

## Build and tests
The parent project's CI uses `bash scripts/build.sh <build-dir> --test`, which
configures and builds cpp-bgui together with cpp-bengine and runs CTest. BGUI's
standalone CMake project also supports `BGUI_TESTS=ON`; its test dependency is
fetched by CMake FetchContent.
