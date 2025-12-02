# Bubble GUI

<p align="center"><b><i>"I moved mountains to align a button."</i></b></p>

---

[![build without backend](https://github.com/D4nielStone/cpp-bgui/actions/workflows/cmake-multi-platform.yml/badge.svg)](https://github.com/D4nielStone/cpp-bgui/actions/workflows/cmake-multi-platform.yml)
[![Stars](https://img.shields.io/github/stars/D4nielStone/cpp-bgui?style=social)](https://github.com/D4nielStone/cpp-bgui/stargazers)

| [Overview](#overview) – [Releases & Changelogs](#releases--changelogs) – [Usage](#usage) - [Back End(#back-end) |
|:--:|

---

## Overview

Bubble GUI is a graphics library for developing desktop applications with a graphical user interface.  
This project was created to improve my skills in **C++** and **OpenGL**.

Bubble GUI is designed (but not limited) to simplify the creation of **web-like interfaces** using native rendering.

### Programming Model

Bubble GUI works with a **retained** logic model:

| Part          | Model                  |
| ------------- | ---------------------- |
| Initial State | Store configuration    |
| Layout        | Processes positioning  |
| Rendering     | Render the drawdata queue |
| Final State   | Store position to compare in future updates |

> The final visual state is calculated only when generating draw commands and is **not stored**.

---

## Releases & Changelogs

You can find the latest version on the [**Releases**](https://github.com/D4nielStone/cpp-bgui/releases/) page.

---

## Usage

**Bubble GUI uses the power of C++ templates to create an intuitive and easy-to-use API.**  
After initializing the library, you can set the main layout and add UI elements to it.

### Example

*Using GLFW & Opengl:*

- Initialize glfw and creates a window
- Initialize the library
- Set up the glfw and opengl/vulkan configs

#### GLFW and Opengl Preset

```cpp
/* backends are included based on the cmake build settings */
#include <bgui.hpp>
#include <iostream>

int main() {
    bgui::set_up();

    GLFWwindow* window = bgui::set_up_glfw(1280, 720, "BGUI Exemple");

    bgui::set_up_freetype();
    bgui::set_up_opengl3();
    [...]
```

- Configure the layout as you want

```cpp

    auto& root = bgui::set_layout<bgui::relative>(bgui::orientation::horizontal); 
    // Supported layouts: linear, absolute (base), relative, and more.

    // Lateral panel: vertical linear layout
    auto& panel = root.add<bgui::linear>(bgui::orientation::vertical);

    // Cross alignment (horizontal)
    panel.set_cross_alignment(bgui::alignment::stretch);
    panel.set_width(300/*, bgui::pixel*/); // Pixel is default
    panel.set_height(1.f, bgui::mode::relative);

    // Adding elements
    panel.add<bgui::text>("Hello World!", 0.5f);
```

#### Main Loop

```cpp
    [...]
    while (!glfwWindowShouldClose(window)) {
        bgui::glfw_update(); // update events
        bgui::update();       // update layout
        bgui::opengl3_render(
            bgui::get_draw_data() // render the layout data
        );
        glfwSwapBuffers(window);
    }

    // don't forget to clean-up the trash!
    bgui::shutdown_lib();
    bgui::shutdown_opengl3();
    bgui::shutdown_freetype();
    bgui::shutdown_glfw();
    return 0;
}
```

## Back End

Bubble's GUI is library-agnostic, so if you want to create a system window or render the elements, you must use ***back ends***.
You'll be able to set these options on the cmake configuration:
```cmake
BGUI_USE_OPENGL
BGUI_USE_GLFW
BGUI_USE_FREETYPE
BGUI_USE_VULKAN
```

---
