/**
 * @class LinearLayout
 * @brief A layout that arranges its child elements in a single direction (horizontal or vertical).
 *
 * | **Characteristic**         | **Description** |
 * |-----------------------------|-----------------|
 * | **Orientation**             | Elements are arranged either horizontally (left to right) or vertically (top to bottom). |
 * | **Order**                   | Child elements are displayed in the same order they were added to the layout. |
 * | **Alignment**               | Supports alignment along the main and cross axes (start, center, end, stretch). |
 * | **Spacing**                 | Allows fixed or dynamic spacing between elements (e.g., using `gap` or margins). |
 * | **Sizing**                  | Child elements can have fixed sizes or share available space proportionally. |
 * | **Flow**                    | Elements are stacked one after another without overlap. |
 * | **Margins & Padding**       | Supports outer margins and inner padding for precise element positioning. |
 * | **Auto-resizing**           | Elements can resize automatically when the parent layout changes size. |
 * | **Simplicity**              | Easy to use and predictable layout; ideal for lists, menus, and toolbars. |
 * | **Position Calculation**    | Each child’s position is based on the cumulative size of previous elements. |
 * | **Use Cases**               | Commonly used for vertical menus, horizontal toolbars, and UI element groups. |
 *
 * Example:
 * @code
 * linear_layout layout(orientation::vertical);
 * layout.set_spacing(8);
 * layout.set_alignment(alignment::center);
 * layout.add<elements::text>("My Text", 40);
 * @endcode
 */

#pragma once
#include "layout.hpp"

class linear_layout : public layout {
public:
    linear_layout(const orientation& ori);
    ~linear_layout() = default;

    void update() override;
    void get_draw_calls(std::vector<draw_call>& calls) override;
};