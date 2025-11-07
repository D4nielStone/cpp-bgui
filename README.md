# **📝 Todo List Bubble**

[![Stars](https://img.shields.io/github/stars/d4nielstone/todo_list_bubble?style=social)](https://github.com/D4nielStone/todo_list_bubble/stargazers)
[![License](https://img.shields.io/github/license/d4nielstone/todo_list_bubble)](https://github.com/D4nielStone/todo_list_bubble/blob/master/LICENSE)
[![GitHub release (latest by tag)](https://img.shields.io/github/v/tag/d4nielstone/todo_list_bubble?style=badge&label=latest)](https://github.com/d4nielstone/todo_list_bubble/releases)
[![Issues](https://img.shields.io/github/issues/d4nielstone/todo_list_bubble?style=badge&)](https://github.com/D4nielStone/todo_list_bubble/issues)
[![Discussions](https://img.shields.io/github/discussions/d4nielstone/todo_list_bubble?style=badge&)](https://github.com/D4nielStone/todo_list_bubble/discussions)

A modern, fast and multi-plataform Todo application mada in C++. Made with the Bubble Graphic User Interface :).

---

## **Contributing**

This project is developed mainly on **Linux Mint** using **NeoVim**.

- Pull Requests are **welcome**!
- For major changes, please open a **discussion/issue** first.
- Constructive feedback is appreciated.

## **Compilation Guide:**
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
