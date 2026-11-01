#include <gtest/gtest.h>
#include "bgui.hpp"
#include <chrono>
#include <filesystem>
#include <fstream>

using namespace bgui;

TEST(DeclarativeStyleTest, ThemeAlignmentShorthandInitializesBothAxes) {
    const auto path = std::filesystem::temp_directory_path() /
        ("bgui-alignment-" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count()) + ".theme");
    struct cleanup {
        std::filesystem::path path;
        ~cleanup() { std::error_code error; std::filesystem::remove(path, error); }
    } cleanup{path};
    {
        std::ofstream file(path);
        file << "[base]\nlayout.align = center\n"
                "[type.single]\nlayout.align = end\n"
                "[type.pair]\nlayout.align = start, end\n";
        ASSERT_TRUE(file.good());
    }
    const auto loaded = load_theme(path.string());
    ASSERT_TRUE(loaded.base.layout.align);
    EXPECT_EQ(*loaded.base.layout.align, (vec<2, alignment>{alignment::center, alignment::center}));
    ASSERT_TRUE(loaded.types.at("single").layout.align);
    EXPECT_EQ(*loaded.types.at("single").layout.align, (vec<2, alignment>{alignment::end, alignment::end}));
    ASSERT_TRUE(loaded.types.at("pair").layout.align);
    EXPECT_EQ(*loaded.types.at("pair").layout.align, (vec<2, alignment>{alignment::start, alignment::end}));
}

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

TEST(DeclarativeStyleTest, VisibilityIsNotInheritedFromParent) {
	bgui::scoped_interface interface;

	layout parent;
	parent.style.visual.visible = false;
	auto& child = parent.add<element>();

	parent.compute_style();
	child.compute_style();

	EXPECT_FALSE(parent.computed_style.visual.visible);
	EXPECT_TRUE(child.computed_style.visual.visible);
}

TEST(ModalThemeTest, ModalAndBodyTextStylesComeFromTheme) {
	bgui::scoped_interface interface;
	auto& manager = style_manager::get_instance();
	manager.apply_theme(bgui::dark_theme());

	modal dialog;
	auto body = dialog.add_scoped<text>("A modal message", 0.3);
	dialog.compute_style();
	dialog.content().compute_style();
	body->compute_style();

	EXPECT_FALSE(dialog.style.layout.size_mode.has_value());
	EXPECT_FALSE(dialog.style.layout.align.has_value());
	EXPECT_FALSE(dialog.style.visual.background.normal.has_value());
	EXPECT_EQ(dialog.computed_style.layout.size_mode.x, mode::match_parent);
	EXPECT_EQ(dialog.computed_style.layout.align.x, alignment::center);
	EXPECT_EQ(dialog.content().computed_style.layout.size_mode.x, mode::wrap_content);
	ASSERT_TRUE(body->has_class("modal-text"));
	EXPECT_EQ(body->computed_style.layout.align.x, alignment::start);

	manager.apply_theme(bgui::light_theme());
	dialog.compute_style();
	dialog.content().compute_style();
	body->compute_style();

	EXPECT_EQ(dialog.computed_style.layout.size_mode.x, mode::match_parent);
	EXPECT_EQ(dialog.content().computed_style.layout.size_mode.x, mode::wrap_content);
	EXPECT_EQ(body->computed_style.layout.align.x, alignment::start);
}
