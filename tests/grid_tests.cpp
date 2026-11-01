#include <gtest/gtest.h>
#include "bgui.hpp"

using namespace bgui;

namespace {
void frame(layout& root, vec2i size) {
    element::begin_geometry_frame();
    root.prepare_geometry_frame();
    root.cascade_style();
    root.calculate_content_sizes();
    element::begin_arranging();
    root.process_required_size(size);
    root.arrange();
    element::end_arranging();
    root.commit_geometry();
    element::end_geometry_frame();
}

element& cell(grid& grid) {
    auto& item = grid.add<element>();
    item.style.layout.require_mode(mode::match_parent, mode::match_parent);
    return item;
}
}

TEST(GridTest, WrapsAndReflowsInsideScrollableParent) {
    bgui::set_global_scale(1.f);
    linear parent(orientation::vertical);
    parent.style.layout.require_mode(mode::match_parent, mode::match_parent);
    parent.set_scrollable(true);
    auto& entries = parent.add<grid>(vec2i{100, 40}, vec2i{10, 5});
    entries.style.layout.require_mode(mode::match_parent, mode::wrap_content);
    auto& a = cell(entries);
    auto& b = cell(entries);
    auto& c = cell(entries);
    frame(parent, {210, 70});
    EXPECT_EQ(a.processed_position(), (vec2i{0, 0}));
    EXPECT_EQ(b.processed_position(), (vec2i{110, 0}));
    EXPECT_EQ(c.processed_position(), (vec2i{0, 45}));
    EXPECT_EQ(entries.processed_height(), 85);
    EXPECT_TRUE(parent.get_scrollbar_element().is_enabled());
    parent.mark_layout_dirty();
    frame(parent, {100, 70});
    EXPECT_EQ(b.processed_position(), (vec2i{0, 45}));
    EXPECT_EQ(c.processed_position(), (vec2i{0, 90}));
    EXPECT_EQ(entries.processed_height(), 130);
}

TEST(GridTest, SkipsDisabledChildrenAndHandlesNarrowAndEmptyLayouts) {
    grid entries({100, 40}, {10, 5});
    entries.style.layout.require_mode(mode::match_parent, mode::match_parent);
    auto& a = cell(entries);
    auto& hidden = cell(entries);
    auto& b = cell(entries);
    hidden.set_enable(false);
    frame(entries, {50, 200});
    EXPECT_EQ(a.processed_width(), 50);
    EXPECT_EQ(b.processed_y(), 45);
    entries.remove(&a);
    entries.remove(&hidden);
    entries.remove(&b);
    frame(entries, {0, 0});
    EXPECT_EQ(entries.get_content_size(), (vec2i{0, 0}));
}

TEST(GridTest, ScalesCellsAndGapsWithTheInterface) {
    bgui::set_global_scale(2.f);
    grid entries({100, 40}, {10, 5});
    entries.style.layout.require_mode(mode::match_parent, mode::match_parent);
    auto& a = cell(entries);
    auto& b = cell(entries);
    auto& c = cell(entries);
    frame(entries, {420, 200});
    EXPECT_EQ(a.processed_size(), (vec2i{200, 80}));
    EXPECT_EQ(b.processed_position(), (vec2i{220, 0}));
    EXPECT_EQ(c.processed_position(), (vec2i{0, 90}));
    bgui::set_global_scale(1.f);
}
