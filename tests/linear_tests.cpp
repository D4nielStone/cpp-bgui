#include <gtest/gtest.h>
#include "bgui.hpp"

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

	auto& first = layout.add<mock_element>(100, 50);
	auto& second = layout.add<mock_element>(100, 75);
	first.compute_style();
	second.compute_style();
	layout.on_update();

	EXPECT_EQ(first.processed_y(), 0);
	EXPECT_EQ(second.processed_y(), 50);
}

TEST(LinearTest, HorizontalLayoutFixedSizes) {
	linear layout(orientation::horizontal);
	layout.style.layout.require_size(400, 200);
	layout.style.layout.require_mode(mode::pixel, mode::pixel);
	layout.process_required_size({400, 200});

	auto& first = layout.add<mock_element>(50, 100);
	auto& second = layout.add<mock_element>(75, 100);
	first.compute_style();
	second.compute_style();
	layout.on_update();

	EXPECT_EQ(first.processed_x(), 0);
	EXPECT_EQ(second.processed_x(), 50);
}

TEST(LinearTest, StretchElementsShareRemainingSpace) {
	linear layout(orientation::vertical);
	layout.style.layout.require_size(200, 400);
	layout.style.layout.require_mode(mode::pixel, mode::pixel);
	layout.style.layout.set_padding(0, 0);
	layout.compute_style();
	layout.process_required_size({200, 400});

	auto& first = layout.add<mock_element>(100, 0, mode::pixel, mode::stretch);
	auto& second = layout.add<mock_element>(100, 0, mode::pixel, mode::stretch);
	first.compute_style();
	second.compute_style();
	layout.on_update();

	EXPECT_EQ(first.processed_height(), 200);
	EXPECT_EQ(second.processed_height(), 200);
}

TEST(LinearTest, PaddingAffectsLayout) {
	linear layout(orientation::vertical);
	layout.style.layout.require_size(200, 400);
	layout.style.layout.require_mode(mode::pixel, mode::pixel);
	layout.style.layout.set_padding(10, 20);
	layout.compute_style();
	layout.process_required_size({200, 400});

	auto& child = layout.add<mock_element>(100, 50);
	child.compute_style();
	layout.on_update();

	EXPECT_EQ(child.processed_x(), 10);
	EXPECT_EQ(child.processed_y(), 20);
}