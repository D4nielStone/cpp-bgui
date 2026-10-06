#include <gtest/gtest.h>
#include "bgui.hpp"
#include <atomic>
#include <limits>
#include <thread>

TEST(ValueControlTest, ColorPickerConvertsColorsAndNotifiesChanges) {
    bgui::color_picker control({0.2f, 0.4f, 0.8f, 0.5f});
    EXPECT_NEAR(control.get_color().r, 0.2f, 0.001f);
    EXPECT_NEAR(control.get_color().g, 0.4f, 0.001f);
    EXPECT_NEAR(control.get_color().b, 0.8f, 0.001f);
    EXPECT_FLOAT_EQ(control.get_color().a, 0.5f);
    EXPECT_NEAR(control.get_hue(), 220.f, 0.01f);

    int callback_count = 0;
    bgui::color callback_color;
    control.set_on_change([&](const bgui::color& value) {
        ++callback_count;
        callback_color = value;
    });
    control.set_hue(120.f);

    EXPECT_FLOAT_EQ(control.get_hue(), 120.f);
    EXPECT_EQ(callback_count, 1);
    EXPECT_EQ(callback_color, control.get_color());
    EXPECT_NE(control.get_color(), (bgui::color{0.2f, 0.4f, 0.8f, 0.5f}));
}

TEST(ValueControlTest, ColorPickerNormalizesInputAndRejectsNonFiniteValues) {
    bgui::color_picker control;
    control.set_color({2.f, -1.f, 0.5f, 1.5f});

    EXPECT_EQ(control.get_color(), (bgui::color{1.f, 0.f, 0.5f, 1.f}));
    control.set_hue(-120.f);
    EXPECT_FLOAT_EQ(control.get_hue(), 240.f);
    EXPECT_THROW(
        control.set_color({0.f, std::numeric_limits<float>::quiet_NaN(), 0.f, 1.f}),
        std::invalid_argument
    );
    EXPECT_THROW(control.set_hue(std::numeric_limits<float>::infinity()), std::invalid_argument);
}

TEST(ValueControlTest, ColorPickerSelectsHueAndTriangleWithPointer) {
    bgui::scoped_interface interface;
    auto& context = bgui::get_context();
    bgui::color_picker control;
    control.set_final_rect(0, 0, 200, 200);

    context.m_mouse_position = {173, 142};
    control.on_pressed();
    EXPECT_NEAR(control.get_hue(), 240.f, 1.f);
    EXPECT_NEAR(control.get_color().r, 0.f, 0.01f);
    EXPECT_NEAR(control.get_color().g, 0.f, 0.01f);
    EXPECT_NEAR(control.get_color().b, 1.f, 0.01f);

    const bgui::color hue_color = control.get_color();
    control.on_released();
    context.m_mouse_position = {100, 100};
    control.on_pressed();
    EXPECT_NEAR(control.get_color().r, 1.f / 3.f, 0.01f);
    EXPECT_NEAR(control.get_color().g, 1.f / 3.f, 0.01f);
    EXPECT_NEAR(control.get_color().b, 2.f / 3.f, 0.01f);
    EXPECT_NE(control.get_color(), hue_color);
}

TEST(ValueControlTest, ColorPickerRendersSharedBackendTexture) {
    bgui::scoped_interface interface;
    bgui::color_picker control;
    control.set_final_rect(10, 20, 180, 180);
    control.compute_style();

    bgui::draw_data data;
    control.get_requires(&data);

    ASSERT_EQ(data.m_quad_requires.size(), 3U);
    const auto& draw = data.m_quad_requires.front();
    EXPECT_EQ(draw.m_rect, (bgui::vec4{10.f, 20.f, 180.f, 180.f}));
    ASSERT_TRUE(draw.m_material.m_use_tex);
    EXPECT_EQ(draw.m_material.m_texture.m_size, (bgui::vec2{128.f, 128.f}));
    EXPECT_EQ(draw.m_material.m_texture.m_buffer.size(), 128U * 128U * 4U);
    const auto initial_revision = draw.m_material.m_texture.m_revision;
    const auto initial_palette = draw.m_material.m_texture.m_buffer;

    bgui::draw_data unchanged_data;
    control.get_requires(&unchanged_data);
    ASSERT_EQ(unchanged_data.m_quad_requires.size(), 3U);
    EXPECT_EQ(
        unchanged_data.m_quad_requires.front().m_material.m_texture.m_revision,
        initial_revision
    );

    control.set_color({0.5f, 0.f, 0.f, 1.f});
    bgui::draw_data selection_data;
    control.get_requires(&selection_data);
    ASSERT_EQ(selection_data.m_quad_requires.size(), 3U);
    EXPECT_EQ(
        selection_data.m_quad_requires.front().m_material.m_texture.m_buffer,
        initial_palette
    );

    control.set_hue(120.f);
    bgui::draw_data updated_data;
    control.get_requires(&updated_data);
    ASSERT_EQ(updated_data.m_quad_requires.size(), 3U);
    EXPECT_NE(updated_data.m_quad_requires.front().m_material.m_texture.m_buffer, initial_palette);
    EXPECT_GT(
        updated_data.m_quad_requires.front().m_material.m_texture.m_revision,
        initial_revision
    );
}

TEST(ValueControlTest, SliderClampsValuesAndSnapsToStep) {
    bgui::slider control(-10.f, 10.f, 1.f);
    EXPECT_FLOAT_EQ(control.get_value(), 1.f);

    int callback_count = 0;
    float callback_value = 0.f;
    control.set_on_change([&](float value) {
        ++callback_count;
        callback_value = value;
    });

    control.set_step(2.f);
    EXPECT_FLOAT_EQ(control.get_value(), 2.f);
    EXPECT_EQ(callback_count, 1);
    EXPECT_FLOAT_EQ(callback_value, 2.f);

    control.set_value(20.f);
    EXPECT_FLOAT_EQ(control.get_value(), 10.f);
    EXPECT_FLOAT_EQ(callback_value, 10.f);
    control.set_value(10.f);
    EXPECT_EQ(callback_count, 2);
}

TEST(ValueControlTest, SliderRangeChangesClampCurrentValue) {
    bgui::slider control(0.f, 100.f, 80.f);
    control.set_range(-20.f, 40.f);

    EXPECT_FLOAT_EQ(control.get_minimum(), -20.f);
    EXPECT_FLOAT_EQ(control.get_maximum(), 40.f);
    EXPECT_FLOAT_EQ(control.get_value(), 40.f);
}

TEST(ValueControlTest, SliderRejectsInvalidRangesValuesAndSteps) {
    EXPECT_THROW(bgui::slider(2.f, 2.f), std::invalid_argument);
    EXPECT_THROW(bgui::slider(0.f, 1.f, std::numeric_limits<float>::infinity()), std::invalid_argument);
    EXPECT_THROW(
        bgui::slider(0.f, 1.f, 0.f, static_cast<bgui::orientation>(99)),
        std::invalid_argument
    );

    bgui::slider control;
    EXPECT_THROW(control.set_range(1.f, 1.f), std::invalid_argument);
    EXPECT_THROW(control.set_range(std::numeric_limits<float>::quiet_NaN(), 2.f), std::invalid_argument);
    EXPECT_THROW(control.set_value(std::numeric_limits<float>::infinity()), std::invalid_argument);
    EXPECT_THROW(control.set_step(-1.f), std::invalid_argument);
    EXPECT_THROW(control.set_step(std::numeric_limits<float>::quiet_NaN()), std::invalid_argument);

    EXPECT_FLOAT_EQ(control.get_minimum(), 0.f);
    EXPECT_FLOAT_EQ(control.get_maximum(), 1.f);
    EXPECT_FLOAT_EQ(control.get_value(), 0.f);
    EXPECT_FLOAT_EQ(control.get_step(), 0.f);
}

TEST(ValueControlTest, SliderPointerClickAndDragChangeValue) {
    bgui::scoped_interface interface;
    auto& context = bgui::get_context();
    context.m_mouse_position = {75, 10};

    bgui::slider control(0.f, 100.f);
    control.set_final_rect(0, 0, 100, 20);
    float callback_value = -1.f;
    control.set_on_change([&callback_value](float value) {
        callback_value = value;
    });
    control.on_clicked();
    EXPECT_NEAR(control.get_value(), 75.f, 0.01f);
    EXPECT_NEAR(callback_value, 75.f, 0.01f);

    context.m_mouse_position = {100, 10};
    control.set_drag({25, 0});
    EXPECT_FLOAT_EQ(control.get_value(), 100.f);
    EXPECT_FLOAT_EQ(callback_value, 100.f);
}

TEST(ValueControlTest, SliderRendersTrackAndThumb) {
    bgui::scoped_interface interface;
    bgui::slider control;
    control.set_value(0.5f);
    control.set_final_rect(10, 10, 100, 24);
    control.compute_style();

    bgui::draw_data data;
    control.get_requires(&data);

    EXPECT_EQ(data.m_draw_list.get_vertices().size(), 0U);
    EXPECT_EQ(data.m_quad_requires.size(), 3U);
    EXPECT_GT(data.m_quad_requires.front().m_rect.z, data.m_quad_requires.front().m_rect.w);
}

TEST(ValueControlTest, VerticalSliderAndProgressBarRenderTallTracks) {
    bgui::scoped_interface interface;
    bgui::slider control(0.f, 1.f, 0.5f, bgui::orientation::vertical);
    control.set_final_rect(0, 0, 20, 100);
    control.compute_style();
    bgui::draw_data slider_data;
    control.get_requires(&slider_data);
    ASSERT_FALSE(slider_data.m_quad_requires.empty());
    EXPECT_LT(slider_data.m_quad_requires.front().m_rect.z, slider_data.m_quad_requires.front().m_rect.w);

    bgui::progress_bar progress(0.f, 1.f, 0.5f, bgui::orientation::vertical);
    progress.set_final_rect(0, 0, 20, 100);
    progress.compute_style();
    bgui::draw_data progress_data;
    progress.get_requires(&progress_data);
    ASSERT_FALSE(progress_data.m_quad_requires.empty());
    EXPECT_LT(progress_data.m_quad_requires.front().m_rect.z, progress_data.m_quad_requires.front().m_rect.w);
}

TEST(ValueControlTest, DarkThemeProvidesIntrinsicSliderHeight) {
    bgui::scoped_interface interface;
    bgui::style_manager::get_instance().apply_theme(bgui::dark_theme());

    bgui::slider control;
    control.compute_style();

    EXPECT_EQ(control.computed_style.layout.size_mode.y, bgui::mode::pixel);
    EXPECT_FLOAT_EQ(control.computed_style.layout.size.y, 24.f);
}

TEST(ValueControlTest, ProgressBarClampsValuesAndSupportsRanges) {
    bgui::progress_bar control(0.f, 100.f, 50.f);
    EXPECT_FALSE(control.recives_input());
    EXPECT_FLOAT_EQ(control.get_value(), 50.f);

    control.set_value(120.f);
    EXPECT_FLOAT_EQ(control.get_value(), 100.f);
    control.set_range(-10.f, 10.f);
    EXPECT_FLOAT_EQ(control.get_minimum(), -10.f);
    EXPECT_FLOAT_EQ(control.get_maximum(), 10.f);
    EXPECT_FLOAT_EQ(control.get_value(), 10.f);
}

TEST(ValueControlTest, ProgressBarValueCanBeUpdatedFromAnotherThread) {
    bgui::progress_bar control(0.f, 100.f);
    std::atomic_bool finished{false};
    std::thread worker([&]() {
        for (int value = 0; value <= 100000; ++value)
            control.set_value(static_cast<float>(value % 101));
        control.set_value(100.f);
        finished.store(true, std::memory_order_release);
    });

    while (!finished.load(std::memory_order_acquire)) {
        const float value = control.get_value();
        EXPECT_GE(value, 0.f);
        EXPECT_LE(value, 100.f);
    }
    worker.join();

    EXPECT_FLOAT_EQ(control.get_value(), 100.f);
}

TEST(ValueControlTest, ProgressBarRejectsInvalidRangesAndValues) {
    EXPECT_THROW(bgui::progress_bar(1.f, 1.f), std::invalid_argument);
    EXPECT_THROW(bgui::progress_bar(0.f, 1.f, std::numeric_limits<float>::quiet_NaN()), std::invalid_argument);
    EXPECT_THROW(
        bgui::progress_bar(0.f, 1.f, 0.f, static_cast<bgui::orientation>(99)),
        std::invalid_argument
    );

    bgui::progress_bar control;
    EXPECT_THROW(control.set_range(2.f, 1.f), std::invalid_argument);
    EXPECT_THROW(control.set_value(std::numeric_limits<float>::infinity()), std::invalid_argument);
    EXPECT_FLOAT_EQ(control.get_minimum(), 0.f);
    EXPECT_FLOAT_EQ(control.get_maximum(), 100.f);
}

TEST(ValueControlTest, ProgressBarRendersTrackAndFill) {
    bgui::scoped_interface interface;
    bgui::progress_bar control(0.f, 100.f, 50.f);
    control.set_final_rect(10, 10, 100, 14);
    control.compute_style();

    bgui::draw_data data;
    control.get_requires(&data);

    EXPECT_EQ(data.m_draw_list.get_vertices().size(), 0U);
    EXPECT_EQ(data.m_quad_requires.size(), 2U);
}
