# **📝 Todo List Bubble**

[![Stars](https://img.shields.io/github/stars/d4nielstone/todo_list_bubble?style=social)](https://github.com/D4nielStone/todo_list_bubble/stargazers)
[![License](https://img.shields.io/github/license/d4nielstone/todo_list_bubble)](https://github.com/D4nielStone/todo_list_bubble/blob/master/LICENSE)
[![GitHub release (latest by tag)](https://img.shields.io/github/v/tag/d4nielstone/todo_list_bubble?style=badge&label=latest)](https://github.com/d4nielstone/todo_list_bubble/releases)
[![Issues](https://img.shields.io/github/issues/d4nielstone/todo_list_bubble?style=badge&)](https://github.com/D4nielStone/todo_list_bubble/issues)
[![Discussions](https://img.shields.io/github/discussions/d4nielstone/todo_list_bubble?style=badge&)](https://github.com/D4nielStone/todo_list_bubble/discussions)

<img width="542" height="250" alt="todo_list_bubble_banner" src="https://github.com/user-attachments/assets/c34ee92c-df71-498c-b65b-ecf2ba8e9f9e" />

A modern, fast and multi-plataform Todo application made in C++. 
> Note: Made with the Bubble Graphic User Interface :).

---

## **Contributing**

This project is developed mainly on **Linux Mint** using **NeoVim**.

- Pull Requests are **welcome**!
- For major changes, please open a **discussion/issue** first.
- Constructive feedback is appreciated.

## **Compilation Guide:**
## **Windows**
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
## **Linux**
You just need to install `build-essential` and `glfw` using your package manager.

### **Fedora**

```bash
dnf install glfw-devel cmake make freetype-devel automake gcc gcc-c++ kernel-devel mesa-libGL-devel mesa-libGLU-devel
```

### **Debian/Ubuntu**

```bash
apt install build-essential libglfw3-dev libfreetype6-dev
```

### **CMake command**:
```bash 
cmake -S . -B out
cmake --build out
```
