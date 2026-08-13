# ScopeCanvas.Editor

Optional editor modules for [ScopeCanvas](https://github.com/Iso83/ScopeCanvas).

ScopeCanvas.Editor provides reusable editing components built on top of ScopeCanvas while remaining an independent, self-contained project.

## Modules

### editor/text

Text editing and visualization components, including:

* Document and text editing
* Caret and selection handling
* Syntax highlighting
* Annotations and gutter rendering
* Zoom and viewport handling
* Row projection and diff visualization support

## Building

ScopeCanvas.Editor includes ScopeCanvas as a Git submodule so the project can be built and tested independently.

```bash
git clone --recursive https://github.com/Iso83/ScopeCanvas.Editor.git
cd ScopeCanvas.Editor

cmake -S . -B build
cmake --build build
ctest --test-dir build
```

## Requirements

* C++20
* CMake
* ScopeCanvas