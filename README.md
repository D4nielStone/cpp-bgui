# TodoListBubble
Is a `Todo List` software used to develop the `BubbleGUI` library.

[![CMake (Linux + Windows MinGW)](https://github.com/D4nielStone/todo_list_bubble/actions/workflows/cmake-multi-platform.yml/badge.svg)](https://github.com/D4nielStone/todo_list_bubble/actions/workflows/cmake-multi-platform.yml)

# Instalation:
You can find the `binaries` here:

[![Version](https://img.shields.io/github/v/tag/d4nielstone/todo_list_bubble)](https://github.com/D4nielStone/todo_list_bubble/tags)

---

# Compilation Guide:

## Windows
You need:

 - CMake → the build system generator
   - 🔗 Download from https://cmake.org/download/
 - Compiler toolchain:
   - MinGW or LLVM/Clang → if you prefer GCC-style.
   - 🔗 Download from https://sourceforge.net/projects/mingw/

**cmake command**:
```bash 
cmake -S . -B out -G "MinGW Makefiles"
cmake --build out
```

Don't forget to clone and compile glfw:
```bash
git clone https://github.com/glfw/glfw.git
    cmake -S glfw -B glfw/build -G "MinGW Makefiles" -DBUILD_SHARED_LIBS=OFF
    cmake --build glfw/build --config Release
    mkdir -p libs/lib
    cp glfw/build/src/libglfw3.a libs/lib/
```

---
## Linux
You will just need to install `build-essential` and `libglfw3-dev` with your package manager.
**cmake command**:
```bash 
cmake -S . -B out
cmake --build out
```
