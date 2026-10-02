#include <gtest/gtest.h>
#include "bgui.hpp"
#include "os/os.hpp"

using namespace bgui;

class mock_element : public element {
public:
	mock_element(int width, int height, mode width_mode = mode::pixel,
				 mode height_mode = mode::pixel) {
		style.layout.require_size(static_cast<float>(width), static_cast<float>(height));
		style.layout.require_mode(width_mode, height_mode);
		style.layout.limit_min = {0, 0};
		compute_style();
	}

	void on_update() override {}
};

TEST(LinearTest, VerticalLayoutFixedSizes) {
	linear layout(orientation::vertical);
	layout.style.layout.require_size(200, 400);
	layout.style.layout.require_mode(mode::pixel, mode::pixel);
	layout.process_required_size({200, 400});

	auto first = layout.add<mock_element>(100, 50);
	auto second = layout.add<mock_element>(100, 75);
	first->compute_style();
	second->compute_style();
	layout.on_update();

	EXPECT_EQ(first->processed_y(), 0);
	EXPECT_EQ(second->processed_y(), 50);
}

TEST(LinearTest, HorizontalLayoutFixedSizes) {
	linear layout(orientation::horizontal);
	layout.style.layout.require_size(400, 200);
	layout.style.layout.require_mode(mode::pixel, mode::pixel);
	layout.process_required_size({400, 200});

	auto first = layout.add<mock_element>(50, 100);
	auto second = layout.add<mock_element>(75, 100);
	first->compute_style();
	second->compute_style();
	layout.on_update();

	EXPECT_EQ(first->processed_x(), 0);
	EXPECT_EQ(second->processed_x(), 50);
}

TEST(LinearTest, StretchElementsShareRemainingSpace) {
	linear layout(orientation::vertical);
	layout.style.layout.require_size(200, 400);
	layout.style.layout.require_mode(mode::pixel, mode::pixel);
	layout.style.layout.set_padding(0, 0);
	layout.compute_style();
	layout.process_required_size({200, 400});

	auto first = layout.add<mock_element>(100, 0, mode::pixel, mode::stretch);
	auto second = layout.add<mock_element>(100, 0, mode::pixel, mode::stretch);
	first->compute_style();
	second->compute_style();
	layout.on_update();

	EXPECT_EQ(first->processed_height(), 200);
	EXPECT_EQ(second->processed_height(), 200);
}

TEST(LinearTest, PaddingAffectsLayout) {
	linear layout(orientation::vertical);
	layout.style.layout.require_size(200, 400);
	layout.style.layout.require_mode(mode::pixel, mode::pixel);
	layout.style.layout.set_padding(10, 20);
	layout.compute_style();
	layout.process_required_size({200, 400});

	auto child = layout.add<mock_element>(100, 50);
	child->compute_style();
	layout.on_update();

	EXPECT_EQ(child->processed_x(), 10);
	EXPECT_EQ(child->processed_y(), 20);
}

TEST(LinearTest, ResizablePanelCanBeEnabledAndResized) {
	linear panel(orientation::vertical);
	panel.style.layout.require_size(200, 160);
	panel.style.layout.require_mode(mode::pixel, mode::pixel);
	panel.style.layout.limit_min = {40, 40};
	panel.compute_style();
	panel.process_required_size({200, 160});

	EXPECT_FALSE(panel.is_resizable());
	panel.set_resizable(true);
	EXPECT_TRUE(panel.is_resizable());
	panel.on_update();

	element* bottom_right = nullptr;
	for (auto& [lay, elements] : panel.get_elements()) {
		for (auto& elem : elements) {
			if (elem->has_class("resize-right") && elem->has_class("resize-bottom"))
				bottom_right = elem.get();
		}
	}
	ASSERT_NE(bottom_right, nullptr);
	bottom_right->set_drag({20, 10});
	bottom_right->on_update();

	ASSERT_TRUE(panel.style.layout.size.has_value());
	EXPECT_FLOAT_EQ((*panel.style.layout.size)[0], 220.f);
	EXPECT_FLOAT_EQ((*panel.style.layout.size)[1], 170.f);

	panel.set_resizable(false);
	EXPECT_FALSE(panel.is_resizable());
	bool has_resize_handle = false;
	for (auto& [lay, elements] : panel.get_elements()) {
		for (auto& elem : elements) {
			has_resize_handle = has_resize_handle || elem->type == "resize_handle";
		}
	}
	EXPECT_FALSE(has_resize_handle);
}

TEST(LinearTest, ResizingOneEdgePreservesTheOtherInitialDimension) {
	bgui::set_up();
	auto& manager = style_manager::get_instance();
	style themed_size;
	themed_size.layout.require_size(200.f, 160.f);
	themed_size.layout.require_mode(mode::pixel, mode::pixel);
	manager.set_type("resizable_test", themed_size);

	linear panel(orientation::vertical);
	panel.type = "resizable_test";
	panel.compute_style();
	panel.process_required_size({400, 300});
	panel.set_resizable(true);
	panel.on_update();

	element* right_edge = nullptr;
	element* bottom_right_corner = nullptr;
	for (auto& [lay, elements] : panel.get_elements()) {
		for (auto& elem : elements) {
			if (elem->has_class("resize-right") &&
				!elem->has_class("resize-top") && !elem->has_class("resize-bottom"))
				right_edge = elem.get();
			if (elem->has_class("resize-right") && elem->has_class("resize-bottom"))
				bottom_right_corner = elem.get();
		}
	}
	ASSERT_NE(right_edge, nullptr);
	ASSERT_NE(bottom_right_corner, nullptr);

	right_edge->on_mouse_hover();
	EXPECT_EQ(bgui::get_context().m_actual_cursor, cursor::resize_horizontal);
	right_edge->set_drag({20, 0});
	right_edge->on_update();

	ASSERT_TRUE(panel.style.layout.size.has_value());
	EXPECT_FLOAT_EQ((*panel.style.layout.size)[0], 220.f);
	EXPECT_FLOAT_EQ((*panel.style.layout.size)[1], 160.f);

	bottom_right_corner->on_mouse_hover();
	EXPECT_EQ(bgui::get_context().m_actual_cursor, cursor::resize_nwse);
}

TEST(DetailsTest, PreservesContentWhileCollapsingAndExpanding) {
	bgui::details section("Models");
	auto& item = section.content().add_persistent<bgui::text>("cube.obj", 0.35f);

	ASSERT_TRUE(section.style.layout.size_mode.has_value());
	EXPECT_EQ((*section.style.layout.size_mode)[1], bgui::mode::wrap_content);
	ASSERT_TRUE(section.content().style.layout.size_mode.has_value());
	EXPECT_EQ((*section.content().style.layout.size_mode)[1], bgui::mode::wrap_content);
	EXPECT_FALSE(section.is_open());
	EXPECT_FALSE(section.content().is_enabled());

	section.set_open(true);
	EXPECT_TRUE(section.is_open());
	EXPECT_TRUE(section.content().is_enabled());
	ASSERT_EQ(section.content().get_elements().begin()->second.size(), 1u);
	EXPECT_EQ(section.content().get_elements().begin()->second.front().get(), &item);

	section.set_open(false);
	EXPECT_FALSE(section.content().is_enabled());
	EXPECT_EQ(section.content().get_elements().begin()->second.front().get(), &item);
}