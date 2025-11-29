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

#### GLFW and Opengl Preset

```cpp
/* backends are included based on the cmake build settings */
#include <bgui.hpp>
#include <iostream>

int main() {
    bgui::set_up();

    GLFWwindow* window = bkend::set_up_glfw(1280, 720, "BGUI Exemple");

    bkend::set_up_freetype();
    bkend::set_up_opengl3();
    [...]
```

- Configure the layout as you want

```cpp

    auto& root = bgui::set_layout<blay::relative>(butil::orientation::horizontal); 
    // Supported layouts: linear, absolute (base), relative, and more.

    // Lateral panel: vertical linear layout
    auto& panel = root.add<blay::linear>(butil::orientation::vertical);

    // Cross alignment (horizontal)
    panel.set_cross_alignment(butil::alignment::stretch);
    panel.set_width(300/*, butil::pixel*/); // Pixel is default
    panel.set_height(1.f, butil::mode::relative);

    // Adding elements
    panel.add<belem::text>("Hello World!", 0.5f);
```

#### Main Loop

```cpp
    [...]
    while (!glfwWindowShouldClose(window)) {
        bkend::glfw_update(); // update events
        bgui::update();       // update layout
        bkend::opengl3_render(
            bgui::get_draw_data() // render the layout data
        );
        glfwSwapBuffers(window);
    }

    // don't forget to clean-up the trash!
    bgui::shutdown_lib();
    bkend::shutdown_opengl3();
    bkend::shutdown_freetype();
    bkend::shutdown_glfw();
    return 0;
}
```

---

> Note: The final layout and draw state are not stored.
> They are recalculated every frame and immediately sent to the backend as draw commands.

> Note: The bgui just updates the layout and store draw commands information.