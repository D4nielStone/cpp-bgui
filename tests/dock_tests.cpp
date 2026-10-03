#include <gtest/gtest.h>
#include <algorithm>
#include <filesystem>

#include "bgui.hpp"
#include "elem/button.hpp"
#include "elem/window.hpp"
#include "lay/dock.hpp"

namespace {
    bgui::button* find_button(bgui::layout& parent, const std::string& class_name) {
        for (auto& [lay, elements] : parent.get_elements()) {
            for (auto& element : elements) {
                if (auto* button = dynamic_cast<bgui::button*>(element.get());
                    button && button->has_class(class_name))
                    return button;
                if (auto* child_layout = element->as_layout()) {
                    if (auto* button = find_button(*child_layout, class_name))
                        return button;
                }
            }
        }
        return nullptr;
    }
}

TEST(DockTest, PinnedWindowsTileAndResizeByDraggingDividers) {
    bgui::scoped_interface interface;
    auto& dock = bgui::get_layout().add_persistent<bgui::dock>();
    dock.compute_style();
    dock.process_required_size({900, 600});
    dock.set_final_rect(0, 0, 900, 600);

    auto& left = dock.add_window("Left", bgui::dock_area::left);
    auto& center = dock.add_window("Center", bgui::dock_area::center);
    dock.cascade_style();
    dock.on_update();

    EXPECT_FALSE(left.is_floating());
    EXPECT_FALSE(center.is_floating());
    EXPECT_GT(left.processed_width(), 0);
    EXPECT_EQ(left.processed_x(), 0);
    EXPECT_EQ(center.processed_x(), left.processed_width() + 6);

    bgui::element* left_divider = nullptr;
    for (auto& [lay, elements] : dock.get_elements()) {
        for (auto& element : elements) {
            if (element->has_class("dock-splitter") && element->is_enabled() &&
                element->processed_width() == 6 && element->processed_height() == 600) {
                left_divider = element.get();
                break;
            }
        }
        if (left_divider)
            break;
    }
    ASSERT_NE(left_divider, nullptr);

    const int initial_width = left.processed_width();
    left_divider->set_drag({40, 0});
    dock.on_update();
    dock.on_update();
    EXPECT_GT(left.processed_width(), initial_width);
}

TEST(DockTest, FloatingWindowsStayAbovePinnedWindowsRegardlessOfInsertionOrder) {
    bgui::scoped_interface interface;
    auto& dock = bgui::get_layout().add_persistent<bgui::dock>();
    auto& floating = dock.add_persistent<bgui::window>("Floating", true);
    auto& pinned = dock.add_window("Pinned", bgui::dock_area::center);
    dock.compute_style();
    dock.process_required_size({900, 600});
    dock.set_final_rect(0, 0, 900, 600);
    dock.cascade_style();
    dock.on_update();

    const auto& elements = dock.get_elements()[bgui::layer::base];
    const auto floating_position = std::find_if(elements.begin(), elements.end(), [&floating](const auto& element) {
        return element.get() == &floating;
    });
    const auto pinned_position = std::find_if(elements.begin(), elements.end(), [&pinned](const auto& element) {
        return element.get() == &pinned;
    });

    ASSERT_NE(floating_position, elements.end());
    ASSERT_NE(pinned_position, elements.end());
    EXPECT_LT(pinned_position, floating_position);

    for (auto splitter = elements.begin(); splitter != elements.end(); ++splitter) {
        if (splitter->get()->has_class("dock-splitter"))
            EXPECT_LT(splitter, floating_position);
    }
}

TEST(DockTest, FocusedFloatingWindowBecomesTopmost) {
    bgui::scoped_interface interface;
    auto& dock = bgui::get_layout().add_persistent<bgui::dock>();
    auto& focused = dock.add_persistent<bgui::window>("Focused", true);
    auto& other = dock.add_persistent<bgui::window>("Other", true);
    dock.compute_style();
    dock.process_required_size({900, 600});
    dock.set_final_rect(0, 0, 900, 600);
    dock.cascade_style();

    dock.focus_window(&focused);
    dock.on_update();

    const auto& elements = dock.get_elements()[bgui::layer::base];
    const auto focused_position = std::find_if(elements.begin(), elements.end(), [&focused](const auto& element) {
        return element.get() == &focused;
    });
    const auto other_position = std::find_if(elements.begin(), elements.end(), [&other](const auto& element) {
        return element.get() == &other;
    });

    ASSERT_NE(focused_position, elements.end());
    ASSERT_NE(other_position, elements.end());
    EXPECT_LT(other_position, focused_position);
}

TEST(DockTest, PinnedWindowCanBeUnpinnedFromItsHeader) {
    bgui::scoped_interface interface;
    auto& dock = bgui::get_layout().add_persistent<bgui::dock>();
    auto& panel = dock.add_window("Panel", bgui::dock_area::center);

    auto* unpin = find_button(panel, "window-unpin-button");
    auto* close = find_button(panel, "window-close-button");
    ASSERT_NE(unpin, nullptr);
    ASSERT_NE(close, nullptr);
    EXPECT_TRUE(unpin->is_enabled());
    EXPECT_FALSE(close->is_enabled());

    dock.compute_style();
    dock.process_required_size({900, 600});
    dock.set_final_rect(0, 0, 900, 600);
    dock.cascade_style();
    dock.on_update();

    unpin->on_released();
    bgui::on_update();
    dock.on_update();

    EXPECT_TRUE(panel.is_floating());
    EXPECT_FALSE(unpin->is_enabled());
    EXPECT_TRUE(close->is_enabled());

    bgui::linear* header = nullptr;
    for (auto& [lay, elements] : panel.get_elements()) {
        for (auto& element : elements) {
            if (element->has_class("window-header"))
                header = dynamic_cast<bgui::linear*>(element.get());
        }
    }
    ASSERT_NE(header, nullptr);
    EXPECT_EQ(close->processed_x() + close->processed_width(),
              header->processed_x() + header->processed_width());
}

TEST(DockTest, ConfigurationCanSaveAndRestoreWindowAndDockState) {
    bgui::scoped_interface interface;
    auto& root = bgui::set_layout<bgui::layout>();
    auto& dock = root.add_persistent<bgui::dock>();
    auto& left = dock.add_window("Left", bgui::dock_area::left);
    auto& floating = dock.add_persistent<bgui::window>("Floating", true);
    floating.set_final_rect(45, 55, 280, 190);

    auto saved = dock.get_configuration();
    saved.left_ratio = 0.31f;
    for (auto& window : saved.windows) {
        if (window.title == "Left")
            window.weight = 0.72f;
        if (window.title == "Floating")
            window.rect = {45, 55, 280, 190};
    }
    dock.apply_configuration(saved);

    const auto config_path = std::filesystem::temp_directory_path() /
        "cpp-bgui-dock-test.cfg";
    ASSERT_TRUE(bgui::save_configuration(config_path.string()));

    auto changed = dock.get_configuration();
    changed.left_ratio = 0.48f;
    for (auto& window : changed.windows) {
        if (window.title == "Left")
            window.area = bgui::dock_area::right;
        if (window.title == "Floating")
            window.rect = {5, 10, 150, 120};
    }
    dock.apply_configuration(changed);
    ASSERT_TRUE(bgui::load_configuration(config_path.string()));
    std::filesystem::remove(config_path);

    const auto restored = dock.get_configuration();
    EXPECT_NEAR(restored.left_ratio, 0.31f, 0.001f);
    ASSERT_EQ(restored.windows.size(), 2u);
    const auto left_state = std::find_if(restored.windows.begin(), restored.windows.end(), [](const auto& item) {
        return item.title == "Left";
    });
    ASSERT_NE(left_state, restored.windows.end());
    EXPECT_EQ(left_state->area, bgui::dock_area::left);
    EXPECT_NEAR(left_state->weight, 0.72f, 0.001f);

    const auto floating_state = std::find_if(restored.windows.begin(), restored.windows.end(), [](const auto& item) {
        return item.title == "Floating";
    });
    ASSERT_NE(floating_state, restored.windows.end());
    EXPECT_TRUE(floating_state->floating);
    EXPECT_EQ(floating_state->rect.x, 45);
    EXPECT_EQ(floating_state->rect.y, 55);
    EXPECT_EQ(floating_state->rect.z, 280);
    EXPECT_EQ(floating_state->rect.w, 190);
    EXPECT_FALSE(left.is_floating());
    EXPECT_TRUE(floating.is_floating());
}