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

class arrange_counting_element : public mock_element {
public:
	using mock_element::mock_element;

	int arrange_count{0};

	void arrange() override {
		++arrange_count;
	}
};

class measured_element : public mock_element {
public:
	using mock_element::mock_element;

	void calc_content_size(const layer&) override {
		set_content_size({120, 35});
	}
};

static void run_layout_frame(layout& root, const vec2i& available) {
	element::begin_geometry_frame();
	root.prepare_geometry_frame();
	root.on_update();
	if (root.is_layout_dirty())
		root.calculate_content_sizes();
	element::begin_arranging();
	root.process_required_size(available);
	root.arrange();
	element::end_arranging();
	root.commit_geometry();
	element::end_geometry_frame();
}

class LinearTest : public testing::Test {
    theme previous_theme;
    void SetUp() override {
        auto& manager = style_manager::get_instance();
        previous_theme = manager.get_theme();
        manager.apply_theme({});
    }
    void TearDown() override {
        style_manager::get_instance().apply_theme(previous_theme);
    }
};

TEST_F(LinearTest, WrapContentMeasuresMatchParentChildrenByIntrinsicSize) {
    linear layout(orientation::vertical);
    layout.style.layout.require_mode(mode::wrap_content, mode::wrap_content);
    layout.compute_style();

    auto child = layout.add_scoped<mock_element>(
        0, 24, mode::match_parent, mode::wrap_content);
    child->set_content_size({320, 24});
    child->compute_style();

    layout.calc_content_size(layer::base);

    EXPECT_EQ(layout.get_content_size().x, 320);
    EXPECT_EQ(layout.get_content_size().y, 24);
}

TEST_F(LinearTest, MeasuresNestedWrapContentBeforeSizingItsParent) {
    linear root(orientation::vertical);
    root.style.layout.require_mode(mode::wrap_content, mode::wrap_content);
    root.compute_style();

    auto nested = root.add_scoped<linear>(orientation::vertical);
    nested->style.layout.require_mode(mode::wrap_content, mode::wrap_content);
    nested->compute_style();

    auto content = nested->add_scoped<measured_element>(
        0, 0, mode::wrap_content, mode::wrap_content);
    content->compute_style();

    root.calculate_content_sizes();
    root.process_required_size({500, 500});

    EXPECT_EQ(nested->get_content_size(), (vec2i{120, 35}));
    EXPECT_EQ(root.get_content_size(), (vec2i{120, 35}));
    EXPECT_EQ(root.processed_size(), (vec2i{120, 35}));
}

TEST_F(LinearTest, MeasuresContentInsideNonFlexLayouts) {
    layout root;
    auto& non_flex_container = root.add<linear>(orientation::vertical);
    non_flex_container.set_flex(false);
    non_flex_container.style.layout.require_mode(
        mode::wrap_content,
        mode::wrap_content
    );
    non_flex_container.compute_style();

    auto& content = non_flex_container.add<measured_element>(
        0,
        0,
        mode::wrap_content,
        mode::wrap_content
    );
    content.compute_style();

    root.calculate_content_sizes();

    EXPECT_EQ(non_flex_container.get_content_size(), (vec2i{120, 35}));
}

TEST_F(LinearTest, CleanLayoutsSkipArrangeUntilInvalidated) {
	linear root(orientation::vertical);
	root.style.layout.require_size(200, 200);
	root.style.layout.require_mode(mode::pixel, mode::pixel);
	root.compute_style();

	auto child = root.add_scoped<arrange_counting_element>(100, 40);
	child->compute_style();

	run_layout_frame(root, {200, 200});
	ASSERT_EQ(child->arrange_count, 1);

	run_layout_frame(root, {200, 200});
	EXPECT_EQ(child->arrange_count, 1);

	child->set_content_size({100, 60});
	run_layout_frame(root, {200, 200});
	EXPECT_EQ(child->arrange_count, 2);
}

TEST_F(LinearTest, VerticalLayoutFixedSizes) {
	linear layout(orientation::vertical);
	layout.style.layout.require_size(200, 400);
	layout.style.layout.require_mode(mode::pixel, mode::pixel);
	layout.process_required_size({200, 400});

	auto first = layout.add_scoped<mock_element>(100, 50);
	auto second = layout.add_scoped<mock_element>(100, 75);
	first->compute_style();
	second->compute_style();
	run_layout_frame(layout, {200, 400});

	EXPECT_EQ(first->processed_y(), 0);
	EXPECT_EQ(second->processed_y(), 50);
}

TEST_F(LinearTest, HorizontalLayoutFixedSizes) {
	linear layout(orientation::horizontal);
	layout.style.layout.require_size(400, 200);
	layout.style.layout.require_mode(mode::pixel, mode::pixel);
	layout.process_required_size({400, 200});

	auto first = layout.add_scoped<mock_element>(50, 100);
	auto second = layout.add_scoped<mock_element>(75, 100);
	first->compute_style();
	second->compute_style();
	run_layout_frame(layout, {400, 200});

	EXPECT_EQ(first->processed_x(), 0);
	EXPECT_EQ(second->processed_x(), 50);
}

TEST_F(LinearTest, StretchElementsShareRemainingSpace) {
	linear layout(orientation::vertical);
	layout.style.layout.require_size(200, 400);
	layout.style.layout.require_mode(mode::pixel, mode::pixel);
	layout.style.layout.set_padding(0, 0);
	layout.compute_style();
	layout.process_required_size({200, 400});

	auto first = layout.add_scoped<mock_element>(100, 0, mode::pixel, mode::stretch);
	auto second = layout.add_scoped<mock_element>(100, 0, mode::pixel, mode::stretch);
	first->compute_style();
	second->compute_style();
	run_layout_frame(layout, {200, 400});

	EXPECT_EQ(first->processed_height(), 200);
	EXPECT_EQ(second->processed_height(), 200);
}

TEST_F(LinearTest, MatchParentUsesRemainingSpaceOnMainAxis) {
	linear layout(orientation::vertical);
	layout.style.layout.require_size(200, 400);
	layout.style.layout.require_mode(mode::pixel, mode::pixel);
	layout.style.layout.set_padding(0, 0);
	layout.compute_style();
	layout.process_required_size({200, 400});

	auto header = layout.add_scoped<mock_element>(200, 50);
	auto content = layout.add_scoped<mock_element>(200, 0, mode::pixel, mode::match_parent);
	header->compute_style();
	content->compute_style();
	run_layout_frame(layout, {200, 400});

	EXPECT_EQ(content->processed_y(), 50);
	EXPECT_EQ(content->processed_height(), 350);
	EXPECT_LE(content->processed_y() + content->processed_height(),
			  layout.processed_height());
}

TEST_F(LinearTest, NonFlexElementsDoNotConsumeFlowSpace) {
	linear layout(orientation::vertical);
	layout.style.layout.require_size(200, 400);
	layout.style.layout.require_mode(mode::pixel, mode::pixel);
	layout.style.layout.set_padding(0, 0);
	layout.compute_style();

	auto header = layout.add_scoped<mock_element>(200, 50);
	auto overlay = layout.add_scoped<mock_element>(40, 40);
	auto content = layout.add_scoped<mock_element>(200, 0, mode::pixel, mode::stretch);
	header->compute_style();
	overlay->compute_style();
	content->compute_style();
	overlay->set_flex(false);
	overlay->set_final_rect(150, 20, 40, 40);

	run_layout_frame(layout, {200, 400});

	EXPECT_EQ(header->processed_y(), 0);
	EXPECT_EQ(content->processed_y(), 50);
	EXPECT_EQ(content->processed_height(), 350);
	EXPECT_EQ(overlay->processed_rect(), (vec4i{150, 20, 40, 40}));
}

TEST_F(LinearTest, PercentChildrenUseAvailableMainAxisSize) {
	linear layout(orientation::vertical);
	layout.style.layout.require_size(200, 400);
	layout.style.layout.require_mode(mode::pixel, mode::pixel);
	layout.style.layout.set_padding(0, 0);
	layout.compute_style();

	auto first = layout.add_scoped<mock_element>(200, 50, mode::pixel, mode::percent);
	auto second = layout.add_scoped<mock_element>(200, 50, mode::pixel, mode::percent);
	first->style.layout.require_height(mode::percent, 50.f);
	second->style.layout.require_height(mode::percent, 50.f);
	first->compute_style();
	second->compute_style();

	run_layout_frame(layout, {200, 400});

	EXPECT_EQ(first->processed_height(), 200);
	EXPECT_EQ(second->processed_height(), 200);
	EXPECT_EQ(first->processed_y(), 0);
	EXPECT_EQ(second->processed_y(), 200);
}

TEST_F(LinearTest, StretchChildrenShareSpaceAfterMargins) {
	linear layout(orientation::vertical);
	layout.style.layout.require_size(200, 400);
	layout.style.layout.require_mode(mode::pixel, mode::pixel);
	layout.style.layout.set_padding(0, 0);
	layout.compute_style();

	auto header = layout.add_scoped<mock_element>(200, 50);
	auto first = layout.add_scoped<mock_element>(200, 0, mode::pixel, mode::stretch);
	auto second = layout.add_scoped<mock_element>(200, 0, mode::pixel, mode::stretch);
	header->compute_style();
	first->style.layout.set_margin(0, 10);
	second->style.layout.set_margin(0, 10);
	first->compute_style();
	second->compute_style();

	run_layout_frame(layout, {200, 400});

	EXPECT_EQ(first->processed_height(), 155);
	EXPECT_EQ(second->processed_height(), 155);
	EXPECT_EQ(first->processed_y(), 60);
	EXPECT_EQ(second->processed_y(), 235);
}

TEST_F(LinearTest, PaddingAffectsLayout) {
	linear layout(orientation::vertical);
	layout.style.layout.require_size(200, 400);
	layout.style.layout.require_mode(mode::pixel, mode::pixel);
	layout.style.layout.set_padding(10, 20);
	layout.compute_style();
	layout.process_required_size({200, 400});

	auto child = layout.add_scoped<mock_element>(100, 50);
	child->compute_style();
	run_layout_frame(layout, {200, 400});

	EXPECT_EQ(child->processed_x(), 10);
	EXPECT_EQ(child->processed_y(), 20);
}

TEST_F(LinearTest, ScrollableVerticalLayoutScrollsAndClamps) {
	linear panel(orientation::vertical);
	panel.style.layout.require_size(100, 100);
	panel.style.layout.require_mode(mode::pixel, mode::pixel);
	panel.style.layout.set_padding(0, 0);
	panel.compute_style();
	panel.process_required_size({100, 100});
	panel.set_scrollable(true);

	auto first = panel.add_scoped<mock_element>(100, 50);
	auto second = panel.add_scoped<mock_element>(100, 50);
	auto third = panel.add_scoped<mock_element>(100, 50);
	first->compute_style();
	second->compute_style();
	third->compute_style();

	auto& context = get_context();
	context.m_mouse_position = {10, 10};
	run_layout_frame(panel, {100, 100});
	context.m_scroll_delta_y = -1.f;
	run_layout_frame(panel, {100, 100});

	EXPECT_EQ(panel.get_scroll_offset(), 40);
	EXPECT_EQ(first->processed_y(), -40);
	EXPECT_EQ(second->processed_y(), 10);
	EXPECT_EQ(third->processed_y(), 60);

	context.m_scroll_delta_y = -10.f;
	run_layout_frame(panel, {100, 100});
	EXPECT_EQ(panel.get_scroll_offset(), 50);
	EXPECT_EQ(first->processed_y(), -50);

	context.m_scroll_delta_y = 10.f;
	run_layout_frame(panel, {100, 100});
	EXPECT_EQ(panel.get_scroll_offset(), 0);
	EXPECT_EQ(first->processed_y(), 0);
	context.m_scroll_delta_y = 0.f;
}

TEST_F(LinearTest, NestedScrollableLayoutGetsWheelBeforeParent) {
	linear parent(orientation::vertical);
	parent.style.layout.require_size(100, 100);
	parent.style.layout.require_mode(mode::pixel, mode::pixel);
	parent.style.layout.set_padding(0, 0);
	parent.compute_style();
	parent.process_required_size({100, 100});
	parent.set_scrollable(true);

	auto inner = parent.add_scoped<linear>(orientation::vertical);
	inner->style.layout.require_size(100, 100);
	inner->style.layout.require_mode(mode::pixel, mode::pixel);
	inner->style.layout.set_padding(0, 0);
	inner->compute_style();
	inner->set_scrollable(true);
	auto first = inner->add_scoped<mock_element>(100, 50);
	auto second = inner->add_scoped<mock_element>(100, 50);
	auto third = inner->add_scoped<mock_element>(100, 50);
	first->compute_style();
	second->compute_style();
	third->compute_style();

	auto sibling = parent.add_scoped<mock_element>(100, 50);
	sibling->compute_style();

	auto& context = get_context();
	context.m_mouse_position = {10, 10};
	context.m_scroll_delta_y = 0.f;
	run_layout_frame(parent, {100, 100});

	context.m_scroll_delta_y = -10.f;
	run_layout_frame(parent, {100, 100});
	EXPECT_EQ(inner->get_scroll_offset(), 50);
	EXPECT_EQ(parent.get_scroll_offset(), 0);

	context.m_scroll_delta_y = -1.f;
	run_layout_frame(parent, {100, 100});
	EXPECT_EQ(inner->get_scroll_offset(), 50);
	EXPECT_EQ(parent.get_scroll_offset(), 40);
	context.m_scroll_delta_y = 0.f;
}

TEST_F(LinearTest, FractionalWheelDeltasAccumulate) {
	linear panel(orientation::vertical);
	panel.style.layout.require_size(100, 100);
	panel.style.layout.require_mode(mode::pixel, mode::pixel);
	panel.style.layout.set_padding(0, 0);
	panel.compute_style();
	panel.process_required_size({100, 100});
	panel.set_scrollable(true);

	auto first = panel.add_scoped<mock_element>(100, 50);
	auto second = panel.add_scoped<mock_element>(100, 50);
	auto third = panel.add_scoped<mock_element>(100, 50);
	first->compute_style();
	second->compute_style();
	third->compute_style();

	auto& context = get_context();
	context.m_mouse_position = {10, 10};
	context.m_scroll_delta_y = 0.f;
	run_layout_frame(panel, {100, 100});

	context.m_scroll_delta_y = -0.01f;
	run_layout_frame(panel, {100, 100});
	EXPECT_EQ(panel.get_scroll_offset(), 0);
	EXPECT_EQ(context.m_scroll_delta_y, 0.f);

	context.m_scroll_delta_y = -0.02f;
	run_layout_frame(panel, {100, 100});
	EXPECT_EQ(panel.get_scroll_offset(), 1);
	context.m_scroll_delta_y = 0.f;
}

TEST_F(LinearTest, ScrollableLayoutSkipsElementsOutsideViewport) {
	linear panel(orientation::vertical);
	panel.style.layout.require_size(100, 100);
	panel.style.layout.require_mode(mode::pixel, mode::pixel);
	panel.style.layout.set_padding(0, 0);
	panel.compute_style();
	panel.process_required_size({100, 100});
	panel.set_scrollable(true);

	int draw_counts[4]{};
	auto first = panel.add_scoped<counting_element>(50, draw_counts[0]);
	auto second = panel.add_scoped<counting_element>(50, draw_counts[1]);
	auto third = panel.add_scoped<counting_element>(50, draw_counts[2]);
	auto fourth = panel.add_scoped<counting_element>(50, draw_counts[3]);
	first->compute_style();
	second->compute_style();
	third->compute_style();
	fourth->compute_style();

	auto& context = get_context();
	context.m_mouse_position = {10, 10};
	context.m_scroll_delta_y = 0.f;
	run_layout_frame(panel, {100, 100});
	context.m_scroll_delta_y = -10.f;
	run_layout_frame(panel, {100, 100});
	context.m_scroll_delta_y = 0.f;

	draw_data data;
	data.m_clip_rect = {0, 0, 100, 100};
	panel.get_requires(&data);

	EXPECT_EQ(draw_counts[0], 0);
	EXPECT_EQ(draw_counts[1], 0);
	EXPECT_EQ(draw_counts[2], 1);
	EXPECT_EQ(draw_counts[3], 1);
}

TEST_F(LinearTest, ResizablePanelCanBeEnabledAndResized) {
	linear panel(orientation::vertical);
	panel.style.layout.require_size(200, 160);
	panel.style.layout.require_mode(mode::pixel, mode::pixel);
	panel.style.layout.limit_min = {40, 40};
	panel.compute_style();
	panel.process_required_size({200, 160});

	EXPECT_FALSE(panel.is_resizable());
	panel.set_resizable(true);
	EXPECT_TRUE(panel.is_resizable());
	run_layout_frame(panel, {100, 100});

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

TEST_F(LinearTest, WrapContentIncludesChildMinimumSize) {
    scoped_interface interface;
    auto& parent = get_layout().add<linear>(orientation::vertical);
    parent.style.layout.require_mode(mode::wrap_content, mode::wrap_content);
    auto& child = parent.add<mock_element>(1, 1, mode::wrap_content, mode::wrap_content);
    child.style.layout.limit_min = {40, 32};
    on_update();
    EXPECT_GE(parent.processed_width(), child.processed_width());
    EXPECT_GE(parent.processed_height(), child.processed_height());
    EXPECT_GE(parent.get_content_size().x, 40);
    EXPECT_GE(parent.get_content_size().y, 32);
}

TEST_F(LinearTest, HorizontalOverflowSupportsTrackpadShiftWheelThumbAndResize) {
    scoped_interface interface;
    linear panel(orientation::vertical);
    panel.style.layout.require_size(100, 100);
    panel.style.layout.require_mode(mode::pixel, mode::pixel);
    panel.style.layout.set_padding(0, 0);
    panel.compute_style(); panel.process_required_size({100, 100}); panel.set_scrollable(true);
    auto& child = panel.add<mock_element>(300, 150);
    auto& io = get_context(); io.m_mouse_position = {10, 10};
    run_layout_frame(panel, {100, 100});
    EXPECT_TRUE(panel.get_horizontal_scrollbar_element().is_enabled());
    io.m_scroll_delta_x = -1;
    run_layout_frame(panel, {100, 100});
    EXPECT_EQ(panel.get_horizontal_scroll_offset(), 40);
    EXPECT_EQ(child.processed_x(), -40);
    EXPECT_EQ(panel.get_scroll_offset(), 0);
    io.m_input_map[input_key::left_shift] = input_action::press;
    io.m_scroll_delta_y = -1;
    run_layout_frame(panel, {100, 100});
    EXPECT_EQ(panel.get_horizontal_scroll_offset(), 80);
    EXPECT_EQ(panel.get_scroll_offset(), 0);
    io.m_input_map[input_key::left_shift] = input_action::release;
    panel.get_horizontal_scrollbar_element().set_drag({500, 0});
    run_layout_frame(panel, {100, 100});
    EXPECT_EQ(panel.get_horizontal_scroll_offset(), 200);
    child.style.layout.require_size(50, 50); child.mark_style_dirty(); child.compute_style();
    run_layout_frame(panel, {100, 100});
    EXPECT_EQ(panel.get_horizontal_scroll_offset(), 0);
    EXPECT_FALSE(panel.get_horizontal_scrollbar_element().is_enabled());
}

TEST_F(LinearTest, NestedHorizontalScrollBubblesAtTheBoundary) {
    scoped_interface interface;
    linear parent(orientation::vertical);
    parent.style.layout.require_size(100, 100);
    parent.style.layout.require_mode(mode::pixel, mode::pixel);
    parent.style.layout.set_padding(0, 0);
    parent.compute_style(); parent.process_required_size({100,100}); parent.set_scrollable(true);
    auto& inner = parent.add<linear>(orientation::horizontal);
    inner.style.layout.require_mode(mode::pixel, mode::pixel);
    inner.style.layout.require_size(200, 60); inner.style.layout.set_padding(0,0);
    inner.compute_style(); inner.set_scrollable(true);
    inner.add<mock_element>(300, 60);
    auto& io = get_context(); io.m_mouse_position = {10,10};
    run_layout_frame(parent, {100,100});
    io.m_scroll_delta_x = -10;
    run_layout_frame(parent, {100,100});
    EXPECT_EQ(inner.get_horizontal_scroll_offset(),100);
    EXPECT_EQ(parent.get_horizontal_scroll_offset(),0);
    io.m_scroll_delta_x = -1;
    run_layout_frame(parent, {100,100});
    EXPECT_EQ(parent.get_horizontal_scroll_offset(),40);
}
