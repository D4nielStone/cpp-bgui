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

class counting_element : public mock_element {
	int& m_draw_count;
public:
	counting_element(int height, int& draw_count)
		: mock_element(100, height), m_draw_count(draw_count) {}

	void get_requires(draw_data* data) override {
		++m_draw_count;
		element::get_requires(data);
	}
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

TEST(LinearTest, MatchParentUsesRemainingSpaceOnMainAxis) {
	linear layout(orientation::vertical);
	layout.style.layout.require_size(200, 400);
	layout.style.layout.require_mode(mode::pixel, mode::pixel);
	layout.style.layout.set_padding(0, 0);
	layout.compute_style();
	layout.process_required_size({200, 400});

	auto header = layout.add<mock_element>(200, 50);
	auto content = layout.add<mock_element>(200, 0, mode::pixel, mode::match_parent);
	header->compute_style();
	content->compute_style();
	layout.on_update();

	EXPECT_EQ(content->processed_y(), 50);
	EXPECT_EQ(content->processed_height(), 350);
	EXPECT_LE(content->processed_y() + content->processed_height(),
			  layout.processed_height());
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

TEST(LinearTest, ScrollableVerticalLayoutScrollsAndClamps) {
	linear panel(orientation::vertical);
	panel.style.layout.require_size(100, 100);
	panel.style.layout.require_mode(mode::pixel, mode::pixel);
	panel.style.layout.set_padding(0, 0);
	panel.compute_style();
	panel.process_required_size({100, 100});
	panel.set_scrollable(true);

	auto first = panel.add<mock_element>(100, 50);
	auto second = panel.add<mock_element>(100, 50);
	auto third = panel.add<mock_element>(100, 50);
	first->compute_style();
	second->compute_style();
	third->compute_style();

	auto& context = get_context();
	context.m_mouse_position = {10, 10};
	context.m_scroll_delta_y = -1.f;
	panel.on_update();

	EXPECT_EQ(panel.get_scroll_offset(), 40);
	EXPECT_EQ(first->processed_y(), -40);
	EXPECT_EQ(second->processed_y(), 10);
	EXPECT_EQ(third->processed_y(), 60);

	context.m_scroll_delta_y = -10.f;
	panel.on_update();
	EXPECT_EQ(panel.get_scroll_offset(), 50);
	EXPECT_EQ(first->processed_y(), -50);

	context.m_scroll_delta_y = 10.f;
	panel.on_update();
	EXPECT_EQ(panel.get_scroll_offset(), 0);
	EXPECT_EQ(first->processed_y(), 0);
	context.m_scroll_delta_y = 0.f;
}

TEST(LinearTest, ScrollableLayoutSkipsElementsOutsideViewport) {
	linear panel(orientation::vertical);
	panel.style.layout.require_size(100, 100);
	panel.style.layout.require_mode(mode::pixel, mode::pixel);
	panel.style.layout.set_padding(0, 0);
	panel.compute_style();
	panel.process_required_size({100, 100});
	panel.set_scrollable(true);

	int draw_counts[4]{};
	auto first = panel.add<counting_element>(50, draw_counts[0]);
	auto second = panel.add<counting_element>(50, draw_counts[1]);
	auto third = panel.add<counting_element>(50, draw_counts[2]);
	auto fourth = panel.add<counting_element>(50, draw_counts[3]);
	first->compute_style();
	second->compute_style();
	third->compute_style();
	fourth->compute_style();

	auto& context = get_context();
	context.m_mouse_position = {10, 10};
	context.m_scroll_delta_y = -10.f;
	panel.on_update();
	context.m_scroll_delta_y = 0.f;

	draw_data data;
	data.m_clip_rect = {0, 0, 100, 100};
	panel.get_requires(&data);

	EXPECT_EQ(draw_counts[0], 0);
	EXPECT_EQ(draw_counts[1], 0);
	EXPECT_EQ(draw_counts[2], 1);
	EXPECT_EQ(draw_counts[3], 1);
}

TEST(LinearTest, ScrollbarDraggingScrollsContent) {
	linear panel(orientation::vertical);
	panel.style.layout.require_size(100, 100);
	panel.style.layout.require_mode(mode::pixel, mode::pixel);
	panel.style.layout.set_padding(0, 0);
	panel.compute_style();
	panel.process_required_size({100, 100});
	panel.set_scrollable(true);

	auto first = panel.add<mock_element>(100, 50);
	auto second = panel.add<mock_element>(100, 50);
	auto third = panel.add<mock_element>(100, 50);
	first->compute_style();
	second->compute_style();
	third->compute_style();
	panel.on_update();

	auto& scrollbar = panel.get_scrollbar_element();
	ASSERT_TRUE(scrollbar.is_enabled());
	const int thumb_travel = 100 - scrollbar.processed_height();
	ASSERT_GT(thumb_travel, 0);

	scrollbar.set_drag({0, thumb_travel / 2});
	panel.on_update();

	EXPECT_EQ(panel.get_scroll_offset(), 25);
	EXPECT_EQ(first->processed_y(), -25);
	EXPECT_EQ(second->processed_y(), 25);
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