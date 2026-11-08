# Custom panels with the BGUI facade

Include `<panel_facade.hpp>` for the focused API, or `<bgui.hpp>` for the
umbrella header. Link the existing `bgui` CMake target; no new dependency or
backend is required. The facade composes retained widgets once during UI
construction. It is not an immediate-mode API to call every frame.

```cpp
#include <panel_facade.hpp>

void build_tools(bgui::dock& dock, std::function<void()> save) {
    bgui::panel_facade panel(dock, "Tools", bgui::dock_area::right);
    panel.native().add_class("tools-panel");
    auto body = panel.content();
    body.label("Custom tools");
    body.property("Name", "Untitled", [](const std::string value) {
        // Validate value and dispatch an application command.
    });
    body.toggle("Visible", true, [](bool visible) {
        // Apply through the application's model/service.
    });
    auto actions = body.row();
    actions.action("Save", std::move(save));
}
```

`panel_facade(dock, title, area, scale)` delegates to `dock::add_window`.
The default area is `center` and the default title scale is `.3f`.
`panel_facade(window)` adds a vertical, scrollable body to an existing window;
it leaves existing content in place. Construct a facade only once for each
body you want to create. Reconstructing one appends another body.

The body fills the window's content area; headers and dock controls are kept
outside it. `content()` returns a borrowed `panel_content` value. Each `row()`
or `column()` creates a nested horizontal or vertical `linear` with
`match_parent` width and `wrap_content` height. For controls sharing a row,
set individual widths to `stretch` when they should divide the available space.
Change these defaults through `native().style.layout`, or disable scrolling
with `panel.content().native().set_scrollable(false)`.

| Method | Result |
| --- | --- |
| `label(value, scale = .3f)` | Native `text&` |
| `action(title, callback, scale = .3f)` | Native `button&` |
| `property(title, value, callback = {}, scale = .3f, input_mode = inputbox)` | Native `field&`, full width with a stretching input (26 logical pixels high for single-line input) |
| `toggle(title, value, callback = {}, scale = .3f)` | Native `checkbox&`, initialized by value |
| `add<T, Layer = layer::base>(args...)` | Native `T&`, forwards constructor arguments |
| `native()` | Underlying `linear&` (`panel_content`) or `window&` (`panel_facade`) |

Use `add<T>()` for custom widget classes and existing controls such as
`slider`, `color_picker`, `tree`, and `vector_field`. Include their headers
when using the focused facade header. Every returned reference supports the
original widget API, themes, classes and input behavior. The facade does not
apply an editor-specific theme or manage the BGUI context/frame loop.

For an existing layout, `bgui::panel_content content(linear)` adds no wrapper.
The `window&` overload targets `window.get_context()` so content does not get
inserted into the header. Window-level syntax highlighting should still be
configured through the native window API; generic additions to nested layouts
follow the existing layout API and do not add highlight propagation.

## Ownership and updates

The BGUI tree owns windows and widgets. Facades and content views are borrowed
builders: destroying or copying them neither removes nor duplicates widgets.
They and returned widget references must not be used after the corresponding
window/layout is removed, the root layout is replaced, or BGUI shuts down.
Remove dock windows with `dock::remove_window`, preserving its bookkeeping.
Do not remove a window from inside its active input callback: defer the action
with `bgui::add_function`, as with the native API.

Callbacks use the native widget semantics. They are not model bindings or
undo transactions. `property` forwards text edits without numeric parsing or
validation, including when using `input_mode::number`; validate application
values before accepting them. A toggle stores its own initial value and
reports changes; it does not hold a pointer to model storage. For dynamic
state, retain native widget references only while their owner is alive and
update them through the normal widget API. Validate entity/component lifetimes
inside callbacks rather than capturing component references. All construction
and widget changes belong on the UI thread.

## Headless regression tests

From the BGUI directory:

```sh
cmake -S . -B build-headless -DBGUI_USE_OPENGL=OFF -DBGUI_USE_GLFW=OFF \
    -DBGUI_USE_FREETYPE=OFF -DBGUI_BUILD_EXAMPLES=OFF -DBGUI_TESTS=ON
cmake --build build-headless --target all_tests
ctest --test-dir build-headless -R PanelFacadeTests --output-on-failure
```

This covers construction, dock registration, nested content, native extensions,
callbacks, resize behavior and widget lifetime after the builder leaves scope.
It does not validate interactive rendering or platform-specific behavior.

## Scrolling on both axes

`linear::set_scrollable(true)` clips overflowing content and enables both axes,
independently of layout orientation. Vertical wheel input retains its behavior;
horizontal trackpad input and Shift + wheel move horizontally. Each axis has a
thumb when needed. Inner scrollable layouts consume input before their parents,
then bubble at the corresponding boundary. Fixed widths/minimum widths can be
used to keep controls readable inside a narrower window.

`get_scroll_offset()` and `get_scrollbar_element()` retain their vertical meaning.
`get_horizontal_scroll_offset()` and `get_horizontal_scrollbar_element()` expose
the horizontal counterparts. Both thumb references borrow the native layout and
expire with it. Disabling scrolling resets offsets and hides both thumbs.
