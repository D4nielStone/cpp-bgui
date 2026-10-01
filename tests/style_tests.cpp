#include <gtest/gtest.h>
#include "bgui.hpp"

using namespace bgui;

TEST(StyleVisualTest, ResolveBackgroundColor) {
	bgui::set_up();
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

TEST(DeclarativeStyleTest, InlineStyleOverridesClassStyle) {
	bgui::set_up();
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
	bgui::set_up();
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