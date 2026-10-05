#include <gtest/gtest.h>
#include "bgui.hpp"

using namespace bgui;

TEST(StyleVisualTest, ResolveBackgroundColor) {
	bgui::scoped_interface interface;
	element elem;
	elem.style.visual.background.normal = {1.f, 0.f, 0.f, 1.f};
	elem.compute_style();

	EXPECT_FLOAT_EQ(elem.computed_style.visual.background.r, 1.f);
	EXPECT_FLOAT_EQ(elem.computed_style.visual.background.g, 0.f);
}

TEST(StyleVisualTest, StyleManagerIsSingleton) {
	auto& first = style_manager::get_instance();
	auto& second = style_manager::get_instance();
	EXPECT_EQ(&first, &second);
}

TEST(StyleVisualTest, StateColorUpdatesAfterInteraction) {
	bgui::scoped_interface interface;
	auto& manager = style_manager::get_instance();

	style button_style;
	button_style.visual.background.normal = {0.1f, 0.1f, 0.1f, 1.f};
	button_style.visual.background.hover = {0.2f, 0.2f, 0.2f, 1.f};
	button_style.visual.background.pressed = {0.3f, 0.3f, 0.3f, 1.f};
	manager.set_type("button", button_style);

	element test_button;
	test_button.type = "button";
	test_button.compute_style();
	EXPECT_FLOAT_EQ(test_button.computed_style.visual.background.r, 0.1f);

	test_button.on_mouse_hover();
	test_button.get_requires(bgui::get_draw_data());
	EXPECT_FLOAT_EQ(test_button.computed_style.visual.background.r, 0.2f);
	while (!bgui::get_draw_data()->m_quad_requires.empty())
		bgui::get_draw_data()->m_quad_requires.pop();

	test_button.on_mouse_leave();
	test_button.get_requires(bgui::get_draw_data());
	EXPECT_FLOAT_EQ(test_button.computed_style.visual.background.r, 0.1f);
	while (!bgui::get_draw_data()->m_quad_requires.empty())
		bgui::get_draw_data()->m_quad_requires.pop();

	test_button.on_mouse_hover();
	test_button.on_pressed();
	test_button.get_requires(bgui::get_draw_data());
	EXPECT_FLOAT_EQ(test_button.computed_style.visual.background.r, 0.3f);

	test_button.on_released();
	test_button.get_requires(bgui::get_draw_data());
	EXPECT_FLOAT_EQ(test_button.computed_style.visual.background.r, 0.1f);
}

TEST(DeclarativeStyleTest, InlineStyleOverridesClassStyle) {
	bgui::scoped_interface interface;
	auto& manager = style_manager::get_instance();

	style class_style;
	class_style.visual.background.normal = {1.f, 0.f, 0.f, 1.f};
	manager.set_class("themed", class_style);

	element elem;
	elem.classes = {"themed"};
	elem.style.visual.background.normal = {0.f, 1.f, 0.f, 1.f};
	elem.compute_style();

	EXPECT_FLOAT_EQ(elem.computed_style.visual.background.r, 0.f);
	EXPECT_FLOAT_EQ(elem.computed_style.visual.background.g, 1.f);
}

TEST(DeclarativeStyleTest, ClassToggleRecomputesStyle) {
	bgui::scoped_interface interface;
	auto& manager = style_manager::get_instance();
	style hidden;
	hidden.visual.visible = false;
	manager.set_class("hidden", hidden);

	element elem;
	elem.compute_style();
	EXPECT_TRUE(elem.computed_style.visual.visible);

	elem.add_class("hidden");
	elem.compute_style();
	EXPECT_FALSE(elem.computed_style.visual.visible);
}