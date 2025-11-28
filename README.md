# Bubble GUI

<p align="center"><b><i>"I moved mountains to align a button."</i></b></p>

---

[![Build Status](https://github.com/D4nielStone/cpp-bgui/workflows/build/badge.svg)](https://github.com/D4nielStone/cpp-bgui/actions?workflow=build)
[![Stars](https://img.shields.io/github/stars/D4nielStone/cpp-bgui?style=social)](https://github.com/D4nielStone/cpp-bgui/stargazers)

| [Overview](#overview) – [Releases & Changelogs](#releases--changelogs) – [Usage](#usage) |
|:--:|

---

## Overview

Bubble GUI is a graphics library for developing desktop applications with a graphical user interface.  
This project was created to improve my skills in **C++** and **OpenGL**.

Bubble GUI is designed (but not limited) to simplify the creation of **web-like interfaces** using native rendering.

### Programming Model

Bubble GUI works with a **retained + immediate** hybrid logic model:

| Part          | Model                  |
| ------------- | ---------------------- |
| Initial State | Retained               |
| Layout        | Retained + Immediate   |
| Rendering     | Immediate              |
| Final State   | Discarded              |

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
- Set up the glfw and opengl configs

#### Preset

```cpp
#include <bgui_backend_glfw.hpp>
#include <bgui_backend_opengl3.hpp>
#include <bgui.hpp>
#include <GLFW/glfw3.h>

if (!glfwInit()) {
        std::cerr << "Failed to init GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "BGUI Example", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    bgui::init_lib();

    bgui_set_opengl3();
    bgui_set_glfw(window);
```

- Configure the layout as you want

```cpp
auto& root = bgui::set_layout<blay::relative>(); 
// Supported layouts: linear, absolute (base), relative, and more.

// Lateral panel: vertical linear layout
auto& panel = root.add<blay::linear>(butil::orientation::vertical);

// Cross alignment (horizontal)
panel.set_cross_alignment(butil::alignment::stretch);
panel.set_width(300/*, butil::pixel*/); // Pixel is default
panel.set_height(1.f, butil::relative);

// Adding elements
panel.add<bbelem::text>("Hello World!", 0.5f);
```

#### Main Loop

> GLFW & Opengl exemple

```cpp
while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    bgui_glfw_update_inputs(); // Generates input data
    bgui::update();
    bgui_opengl3_render(
        bgui::get_draw_data() // Generates draw request data
    );
    glfwSwapBuffers(window);
}
```

> Note: The final layout and draw state are not stored.
> They are recalculated every frame and immediately sent to the backend as draw commands.

> Note: The bgui just updates the layout and store draw commands information.