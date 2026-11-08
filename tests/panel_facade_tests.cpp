#include "panel_facade.hpp"
#include "bgui.hpp"

#include <gtest/gtest.h>

TEST(PanelFacadeTest, KeepsWidgetsAfterBuilderDestructionAndUsesWindowContent) {
    bgui::scoped_interface interface;
    auto& dock = bgui::get_layout().add<bgui::dock>();
    bgui::window* window{};
    bgui::text* label{};
    {
        bgui::panel_facade panel(dock, "Custom", bgui::dock_area::right);
        window = &panel.native();
        label = &panel.content().label("Hello");
        EXPECT_EQ(panel.content().native().get_parent(), &window->get_context());
        EXPECT_FALSE(window->is_floating());
        EXPECT_TRUE(panel.content().native().clips_children());
    }
    EXPECT_EQ(label->get_buffer(), "Hello");
    const auto config = dock.get_configuration();
    ASSERT_EQ(config.windows.size(), 1u);
    EXPECT_EQ(config.windows.front().title, "Custom");
    EXPECT_EQ(config.windows.front().area, bgui::dock_area::right);
    EXPECT_TRUE(dock.remove_window(window));
}

TEST(PanelFacadeTest, ComposesRowsColumnsAndNativeCustomWidgets) {
    bgui::scoped_interface interface;
    auto& dock = bgui::get_layout().add<bgui::dock>();
    auto& window = dock.add_window("Existing");
    bgui::panel_facade panel(window);
    auto row = panel.content().row();
    auto column = row.column();
    EXPECT_EQ(row.native().get_parent(), &panel.content().native());
    EXPECT_EQ(column.native().get_parent(), &row.native());
    auto& progress = column.add<bgui::progress_bar>(0.f, 100.f, 25.f);
    progress.set_value(75.f);
    EXPECT_FLOAT_EQ(progress.get_value(), 75.f);
    EXPECT_EQ(progress.get_parent(), &column.native());
    EXPECT_EQ(row.native().style.layout.size_mode.value()[0], bgui::mode::match_parent);
    EXPECT_EQ(row.native().style.layout.size_mode.value()[1], bgui::mode::wrap_content);
    bgui::panel_content direct(window);
    auto& label = direct.label("Direct content");
    EXPECT_EQ(label.get_parent(), &window.get_context());
}

TEST(PanelFacadeTest, ForwardsActionsAndKeepsToggleStateInWidget) {
    bgui::scoped_interface interface;
    auto& dock = bgui::get_layout().add<bgui::dock>();
    bgui::panel_facade panel(dock, "Controls");
    auto body = panel.content();
    int clicks = 0;
    auto& action = body.action("Apply", [&] { ++clicks; });
    action.on_clicked();
    action.on_released();
    EXPECT_EQ(clicks, 0);
    bgui::on_update();
    EXPECT_EQ(clicks, 1);
    bool enabled = false;
    auto& toggle = body.toggle("Enabled", false, [&](bool value) { enabled = value; });
    toggle.on_clicked();
    EXPECT_TRUE(enabled);
    EXPECT_TRUE(toggle.checked());
    std::string accepted;
    auto& property = body.property("Name", "Initial",
        [&](const std::string value) { accepted = value; });
    EXPECT_EQ(property.get_label().get_buffer(), "Name");
    EXPECT_EQ(property.get_inputbox().get_buffer(), "Initial");
    EXPECT_EQ(property.get_inputbox().style.layout.size_mode.value()[0], bgui::mode::stretch);
    auto& input = property.get_inputbox();
    input.set_buffer("Edited");
    input.set_focused(true);
    auto& context = bgui::get_context();
    context.m_input_map.clear();
    context.m_char_buffer.clear();
    context.m_input_map[bgui::input_key::enter] = bgui::input_action::press;
    input.on_update();
    bgui::on_update();
    EXPECT_EQ(accepted, "Edited");
    context.m_input_map.clear();
}

TEST(PanelFacadeTest, BodyFollowsDockResizeAndRemainsScrollable) {
    bgui::scoped_interface interface;
    auto& dock = bgui::get_layout().add<bgui::dock>();
    bgui::panel_facade panel(dock, "Resizable");
    auto body = panel.content();
    for (int i = 0; i < 30; ++i)
        body.label("Item").style.layout.require_height(bgui::mode::pixel, 24.f);
    auto arrange = [&](int width, int height) {
        dock.compute_style();
        dock.process_required_size({width, height});
        dock.set_final_rect(0, 0, width, height);
        dock.cascade_style();
        dock.on_update();
        bgui::element::begin_arranging();
        dock.arrange();
        bgui::element::end_arranging();
    };
    arrange(800, 600);
    const auto initial_width = body.native().processed_width();
    EXPECT_GT(initial_width, 0);
    EXPECT_GT(body.native().processed_height(), 0);
    arrange(400, 300);
    EXPECT_GT(body.native().processed_width(), 0);
    EXPECT_LT(body.native().processed_width(), initial_width);
    EXPECT_LE(body.native().processed_height(), panel.native().processed_height());
    EXPECT_TRUE(body.native().clips_children());
}

TEST(PanelFacadeTest, SingleLinePropertyHasCompactHeight) {
    bgui::scoped_interface interface;
    auto& dock = bgui::get_layout().add<bgui::dock>();
    bgui::panel_facade panel(dock, "Properties");
    auto& field = panel.content().property("Duration", "5.0");
    dock.compute_style();
    dock.process_required_size({800, 600});
    dock.set_final_rect(0, 0, 800, 600);
    dock.cascade_style();
    dock.on_update();
    bgui::element::begin_arranging();
    dock.arrange();
    bgui::element::end_arranging();
    EXPECT_GT(field.get_inputbox().processed_height(), 0);
    EXPECT_LT(field.get_inputbox().processed_height(), 64);
}
