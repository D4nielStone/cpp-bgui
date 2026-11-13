# Hover tooltips

Every `bgui::element` supports an optional tooltip:

```cpp
button.set_tooltip("Save model (Ctrl+S)");
button.set_tooltip("Save model", std::chrono::milliseconds{800});
button.set_tooltip(""); // Disable the tooltip.
```

The default delay is 650 milliseconds of continuous hover over the same input
target, measured with the monotonic clock. Negative delays are clamped to zero.
The text is copied into the element; callers do not retain an overlay or widget
reference. Decorative children of icon buttons should use `receives_input(false)`
so the button remains the input target.

BGUI draws the hint after the widget tree using its existing text renderer. The
hint does not participate in hit testing, docking, layout sizing, keyboard focus,
or mouse capture. It wraps UTF-8 text within the context width, stays within the
context, and moves above its target if there is insufficient room below.

Leaving the target, pressing a mouse button, disabling/removing its subtree, or
replacing the root cancels the pending hint. Queued left clicks restart the delay.
The existing `cancel_interactions` lifetime path invalidates the borrowed target;
the renderer owns only the reusable text widget.

## Title and explanation

Use the additive overload to show a short action name above explanatory text:

```cpp
button.set_tooltip("Save model", "Writes pending geometry and rig changes to the model asset.");
button.set_tooltip("Save model", "Writes the current asset.", std::chrono::milliseconds{800});
```

`get_tooltip()` continues to return the title; `get_tooltip_description()`
returns the explanation. Updating the title with the original overload retains
its description, which supports changing state labels. An empty title hides
both. Existing title-only calls remain valid. The renderer uses a smaller title
and body, an explicitly visible opaque dark surface and a separate gap between
text sections. Both sections wrap inside the context and scale with UI zoom.
