#include <gtest/gtest.h>
#include "bgui.hpp"
#include "os/os.hpp"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

using namespace bgui;

namespace {
const vec2i available_size = {400, 300};

class drag_probe final : public element {
public:
	int drag_updates{0};

	void set_drag(const vec2i&) override {
		++drag_updates;
	}
};

class capture_probe final : public element {
public:
	explicit capture_probe(int& release_count) : m_release_count(release_count) {
		recives_input(true);
	}

	void on_released() override {
		++m_release_count;
	}

private:
	int& m_release_count;
};
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

TEST(InteractionTest, RemovingCapturedElementAndAncestorCancelsCapture) {
	bgui::scoped_interface interface;
	auto& root = bgui::get_layout();
	auto& context = bgui::get_context();
	context.m_input_map.clear();
	context.m_mouse_position = {10, 10};
	context.m_input_map[input_key::mouse_left] = input_action::press;

	int direct_release_count = 0;
	auto& direct = root.add_persistent<capture_probe>(direct_release_count);
	direct.set_final_rect(0, 0, 50, 50);
	bgui::on_update();
	EXPECT_EQ(bgui::get_mouse_target(), &direct);
	ASSERT_TRUE(root.remove(&direct));
	EXPECT_EQ(bgui::get_mouse_target(), nullptr);

	context.m_input_map[input_key::mouse_left] = input_action::release;
	bgui::on_update();
	EXPECT_EQ(direct_release_count, 0);

	context.m_input_map[input_key::mouse_left] = input_action::press;
	auto& parent = root.add_persistent<layout>();
	parent.set_final_rect(0, 0, 200, 200);
	int nested_release_count = 0;
	auto& nested = parent.add_persistent<capture_probe>(nested_release_count);
	nested.set_final_rect(0, 0, 50, 50);
	bgui::on_update();
	EXPECT_EQ(bgui::get_mouse_target(), &nested);
	ASSERT_TRUE(root.remove(&parent));
	EXPECT_EQ(bgui::get_mouse_target(), nullptr);

	context.m_input_map[input_key::mouse_left] = input_action::release;
	bgui::on_update();
	context.m_input_map.clear();
}

TEST(ConfigurationTest, ReplacesExistingFileWithoutLeavingTemporaryFiles) {
	bgui::scoped_interface interface;
	const auto path = std::filesystem::temp_directory_path() / "bgui-configuration-atomic-test.cfg";
	{
		std::ofstream previous(path, std::ios::binary | std::ios::trunc);
		ASSERT_TRUE(previous);
		previous << "previous";
	}

	ASSERT_TRUE(bgui::save_configuration(path.string()));
	std::ifstream saved(path, std::ios::binary);
	const std::string contents(std::istreambuf_iterator<char>(saved), {});
	EXPECT_NE(contents.find("[interface]"), std::string::npos);

	std::size_t files = 0;
	for (const auto& entry : std::filesystem::directory_iterator(path.parent_path())) {
		if (entry.path().filename().string().starts_with("bgui-configuration-atomic-test.cfg"))
			++files;
	}
	EXPECT_EQ(files, 1U);

	const auto directory_path = std::filesystem::temp_directory_path() / "bgui-configuration-atomic-test-directory";
	std::filesystem::create_directories(directory_path);
	{
		std::ofstream marker(directory_path / "preserved.txt", std::ios::binary | std::ios::trunc);
		ASSERT_TRUE(marker);
		marker << "preserved";
	}
	EXPECT_FALSE(bgui::save_configuration(directory_path.string()));
	EXPECT_TRUE(std::filesystem::is_regular_file(directory_path / "preserved.txt"));

	std::error_code ignored;
	std::filesystem::remove(path, ignored);
	std::filesystem::remove_all(directory_path, ignored);
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

TEST(DrawListTest, TessellatesPrimitivesWithEditableColor) {
	draw_list list;
	const vec4 color{0.2f, 0.4f, 0.6f, 0.8f};
	list.set_color(color);
	list.add_line({0.f, 0.f}, {10.f, 0.f}, 2.f);
	list.add_polyline({{0.f, 0.f}, {5.f, 5.f}, {10.f, 0.f}}, 1.f);
	list.add_triangle({0.f, 0.f}, {10.f, 0.f}, {5.f, 10.f});
	list.add_quad({0.f, 0.f}, {10.f, 0.f}, {10.f, 10.f}, {0.f, 10.f});
	list.add_circle({0.f, 0.f}, 5.f, 4);
	list.add_rect_filled({0.f, 0.f}, {10.f, 10.f});
	list.add_rect({0.f, 0.f}, {10.f, 10.f});
	list.add_circle_filled({0.f, 0.f}, 5.f, 8);
	list.add_convexpolyfilled({{0.f, 0.f}, {10.f, 0.f}, {5.f, 10.f}});

	ASSERT_EQ(list.get_vertices().size(), 6U + 12U + 3U + 6U + 24U + 6U + 24U + 24U + 3U);
	EXPECT_EQ(list.get_color(), color);
	for (const auto& vertex : list.get_vertices())
		EXPECT_EQ(vertex.m_color, color);
}
