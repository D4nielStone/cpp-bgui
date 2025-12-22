Bubble GUI
===

<p align="center"><b><i><q>I moved mountains to align a button.</i></b></p>
<img width="1280" height="699" alt="image" src="https://github.com/user-attachments/assets/a296eca6-570a-47bc-a033-1389c798c8d6" />

---

[![build without backend](https://github.com/D4nielStone/cpp-bgui/actions/workflows/cmake-multi-platform.yml/badge.svg)](https://github.com/D4nielStone/cpp-bgui/actions/workflows/cmake-multi-platform.yml)
[![Stars](https://img.shields.io/github/stars/D4nielStone/cpp-bgui?style=social)](https://github.com/D4nielStone/cpp-bgui/stargazers)

| [Overview](#overview) – [Releases & Changelogs](#releases--changelogs) – [Usage](#usage) - [Back end](#back-end) |
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

> The final visual state is calculated only when generating draw requests.
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
    bgui::set_up_gl3();
    [...]
```

- Configure the layout as you want

```cpp
    bgui::layout& root = bgui::get_layout();

    // Adding elements
    auto& panel = root.add<bgui::linear>(bgui::orientation::vertical);
    panel.set_padding(10, 2);
    panel.request_width(bgui::mode::pixel, 300.f);
    panel.request_height(bgui::mode::match_parent);

    // layout are invisible by default
    panel.set_visible(true);

    auto& txt = panel.add<bgui::text>("Linear Layout Exemple", 0.5f);
    txt.request_width(bgui::mode::match_parent);
    txt.set_alignment(bgui::alignment::center);
    auto& button = panel.add<bgui::button>("Button Exemple", 0.5f, [](){});
    button.request_width(bgui::mode::match_parent);

    // window widget
    auto& win = root.add<bgui::window>("Hello Bubble!");
    root.add<bgui::window>("win2");
    root.add<bgui::window>("win3");
    root.add<bgui::window>("win4");
    auto& context = win.add<bgui::linear>(bgui::orientation::vertical);
    context.request_height(bgui::mode::stretch);
    context.request_width(bgui::mode::match_parent);
    context.set_padding(10, 10);

    context.add<bgui::text>("This is a window widget exemple.", 0.5f);
    auto& txt2 = context.add<bgui::text>("Centered text", 0.5f);
    txt2.set_alignment(bgui::alignment::center);
    txt2.request_width(bgui::mode::stretch);
    auto& button2 = context.add<bgui::button>("Button inside window", 0.5f, [](){});
    button2.request_width(bgui::mode::match_parent);

    // style must be applyed in the end
    bgui::apply_style(bgui::dark_style);
```

#### Main Loop

```cpp
    [...]
    while (!glfwWindowShouldClose(window)) {
        bgui::glfw_update(bgui::get_context());           // update events
        bgui::update();                 // update layout
        bgui::gl3_render(
            bgui::get_draw_data()       // render the layout data
        );
        glfwSwapBuffers(window);
    }

    bgui::shutdown_lib();
    bgui::shutdown_gl3();
    bgui::shutdown_freetype();
    bgui::shutdown_glfw();
    return 0;
}
```

## Back end

Bubble's GUI is library-agnostic, so if you want to create a system window or render the elements, you must use ***back ends***.
You'll be able to set these options on the cmake configuration:
```cmake
BGUI_USE_OPENGL
BGUI_USE_GLFW
BGUI_USE_FREETYPE
BGUI_USE_VULKAN
```

---
