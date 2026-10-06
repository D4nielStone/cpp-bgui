#include <bgui.hpp>
#include <gtest/gtest.h>

TEST(ProfilingTest, OverlayIsOptInAndCanBeToggled) {
    bgui::scoped_interface interface;
    auto& root = bgui::get_layout();

    EXPECT_EQ(root.get_elements().count(bgui::layer::overlay), 0U);

    bgui::enable_proffiling();
    ASSERT_EQ(root.get_elements().count(bgui::layer::overlay), 1U);
    auto& overlays = root.get_elements().at(bgui::layer::overlay);
    ASSERT_EQ(overlays.size(), 1U);
    EXPECT_EQ(overlays.front()->id, "bgui.profiling_overlay");

    bgui::on_update();
    auto* overlay_layout = overlays.front()->as_layout();
    ASSERT_NE(overlay_layout, nullptr);
    ASSERT_EQ(overlay_layout->get_elements().at(bgui::layer::base).size(), 6U);

    bgui::enable_proffiling(false);
    EXPECT_TRUE(root.get_elements().at(bgui::layer::overlay).empty());
}
