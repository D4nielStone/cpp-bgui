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
 * linear layout(orientation::vertical);
 * layout.set_spacing(8);
 * layout.set_alignment(alignment::center);
 * layout.add<belem::text>("My Text", 0.5);
 * @endcode
 */

#pragma once
#include "layout.hpp"

namespace blay {
    class relative : public layout {
    public:
        relative(const butil::orientation& ori = butil::orientation::horizontal);
        ~relative() = default;
    
        void update() override;
        void fit_to_content() override;
        blay::layout* as_layout() override { return this; }
    };
}// namespace lay