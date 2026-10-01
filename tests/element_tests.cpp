#include <gtest/gtest.h>
#include "bgui.hpp"

using namespace bgui;

namespace {
const vec2i available_size = {400, 300};
}

TEST(ElementTest, RequireSizeAndMode) {
	element elem;
	elem.style.layout.require_size(150.f, 200.f);
	elem.compute_style();

	EXPECT_FLOAT_EQ(elem.computed_style.layout.size[0], 150.f);
	EXPECT_FLOAT_EQ(elem.computed_style.layout.size[1], 200.f);
	EXPECT_EQ(elem.computed_style.layout.size_mode[0], mode::pixel);
	EXPECT_EQ(elem.computed_style.layout.size_mode[1], mode::pixel);
}

TEST(ElementTest, FinalRectManipulation) {
	element elem;
	elem.set_final_rect(10, 20, 100, 50);

	EXPECT_EQ(elem.processed_x(), 10);
	EXPECT_EQ(elem.processed_y(), 20);
	EXPECT_EQ(elem.processed_width(), 100);
	EXPECT_EQ(elem.processed_height(), 50);

	elem.set_position(5, 15);
	elem.set_final_size(80, 40);
	EXPECT_EQ(elem.processed_x(), 5);
	EXPECT_EQ(elem.processed_y(), 15);
	EXPECT_EQ(elem.processed_width(), 80);
	EXPECT_EQ(elem.processed_height(), 40);
}

TEST(ElementTest, UpdateSizeCalculation) {
	element elem;
	elem.style.layout.require_width(mode::percent, 50.f);
	elem.style.layout.require_height(mode::percent, 25.f);
	elem.compute_style();
	elem.process_required_size(available_size);

	EXPECT_EQ(elem.processed_width(), 200);
	EXPECT_EQ(elem.processed_height(), 75);
}

TEST(ElementTest, ScopedInterfaceInitializesAndShutsDown) {
	EXPECT_THROW(bgui::get_layout(), std::runtime_error);

	{
		bgui::scoped_interface interface;
		EXPECT_NO_THROW(bgui::get_layout());
	}

	EXPECT_THROW(bgui::get_layout(), std::runtime_error);
}

TEST(ElementTest, ScopedElementIsRemovedWhenHandleLeavesScope) {
	layout root;
	EXPECT_TRUE(root.get_elements().empty());

	{
		auto handle = root.add<element>();
		EXPECT_TRUE(handle);
		EXPECT_EQ(root.get_elements().at(layer::base).size(), 1U);
	}

	EXPECT_TRUE(root.get_elements().at(layer::base).empty());
}

TEST(DrawDataTest, EnqueueCapturesIntersectedClipRect) {
	const auto clip = intersect_rect(vec4i{10, 20, 80, 60}, vec4i{40, 0, 80, 50});
	EXPECT_EQ(clip, (vec4i{40, 20, 50, 30}));

	element elem;
	draw_data data;
	data.m_clip_rect = clip;
	data.enqueue({elem.get_material()});

	ASSERT_EQ(data.m_quad_requires.size(), 1U);
	EXPECT_EQ(data.m_quad_requires.front().m_clip_rect, clip);
}