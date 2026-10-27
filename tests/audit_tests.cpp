#include <gtest/gtest.h>
#include "bgui.hpp"

std::u32string utf8_to_utf32(const std::string&, std::vector<size_t>*);

namespace {
class style_probe : public bgui::layout {
public:
    int visits = 0;
    bgui::layout* as_layout() override { ++visits; return this; }
};
class triangle_probe : public bgui::element {
public:
    int draws = 0;
    void get_requires(bgui::draw_data* data) override {
        ++draws;
        data->m_draw_list.add_triangle({0, 0}, {100, 0}, {0, 100});
    }
};
}

TEST(AuditTest, StyleInvalidationVisitsNestedLayoutsOnce) {
    bgui::scoped_interface interface;
    style_probe root;
    std::vector<style_probe*> children;
    auto* parent = &root;
    for (int depth = 0; depth < 8; ++depth) {
        parent = &parent->add<style_probe>();
        children.push_back(parent);
    }
    for (auto* child : children) {
        child->visits = 0;
        child->clear_style_dirty();
    }
    root.mark_style_dirty();
    for (auto* child : children) {
        EXPECT_EQ(child->visits, 1);
        EXPECT_TRUE(child->is_style_dirty());
    }
}

TEST(AuditTest, RenderCacheTracksPrimitiveClipState) {
    bgui::scoped_interface interface;
    triangle_probe triangle;
    bgui::draw_data initial;
    initial.m_clip_rect = {0, 0, 100, 100};
    triangle.get_cached_requires(&initial);
    ASSERT_EQ(initial.m_draw_list.get_vertices().size(), 3U);

    bgui::draw_data clipped;
    clipped.m_clip_rect = initial.m_clip_rect;
    clipped.m_draw_list.set_clip_rect({0, 0, 10, 10});
    triangle.get_cached_requires(&clipped);
    for (const auto& vertex : clipped.m_draw_list.get_vertices()) {
        EXPECT_LE(vertex.m_position.x, 10.f);
        EXPECT_LE(vertex.m_position.y, 10.f);
    }
    bgui::draw_data same_clip;
    same_clip.m_clip_rect = initial.m_clip_rect;
    same_clip.m_draw_list.set_clip_rect({0, 0, 10, 10});
    triangle.get_cached_requires(&same_clip);
    EXPECT_EQ(triangle.draws, 2);
    bgui::draw_data wider;
    wider.m_clip_rect = initial.m_clip_rect;
    wider.m_draw_list.set_clip_rect({0, 0, 20, 20});
    triangle.get_cached_requires(&wider);
    float max_x = 0;
    for (const auto& vertex : wider.m_draw_list.get_vertices())
        max_x = std::max(max_x, vertex.m_position.x);
    EXPECT_FLOAT_EQ(max_x, 20.f);
    bgui::draw_data unclipped;
    unclipped.m_clip_rect = initial.m_clip_rect;
    triangle.get_cached_requires(&unclipped);
    ASSERT_EQ(unclipped.m_draw_list.get_vertices().size(), 3U);
    EXPECT_FLOAT_EQ(unclipped.m_draw_list.get_vertices()[1].m_position.x, 100.f);
}

TEST(AuditTest, Utf8RejectsInvalidSequencesWithoutReadingPastInput) {
    for (const auto& input : {std::string("\xF0"), std::string("\xE2\x82"),
                             std::string("\xC0\xAF"), std::string("\xED\xA0\x80"),
                             std::string("\xF4\x90\x80\x80"), std::string("\x80"),
                             std::string("\xE2" "A")}) {
        std::vector<size_t> lengths;
        auto decoded = utf8_to_utf32(input, &lengths);
        ASSERT_EQ(decoded.size(), lengths.size());
        EXPECT_EQ(decoded.front(), U'\uFFFD');
        size_t consumed = 0;
        for (auto length : lengths) consumed += length;
        EXPECT_EQ(consumed, input.size());
    }
    for (int byte = 0x80; byte <= 0xFF; ++byte) {
        std::string input(64, 'a');
        input.push_back(static_cast<char>(byte));
        input.shrink_to_fit();
        std::vector<size_t> consumed;
        const auto decoded = utf8_to_utf32(input, &consumed);
        EXPECT_EQ(decoded, std::u32string(64, U'a') + U'\uFFFD');
        EXPECT_EQ(consumed, std::vector<size_t>(65, 1));
    }
    std::vector<size_t> lengths;
    EXPECT_EQ(utf8_to_utf32("A\xC3\xA7\xF0\x9F\x98\x80", &lengths), U"Aç😀");
    EXPECT_EQ(lengths, (std::vector<size_t>{1, 2, 4}));
}

TEST(AuditTest, ScaleChangeDuringUpdateReflowsInTheSameFrame) {
    bgui::scoped_interface interface;
    bgui::set_global_scale(1.f);
    auto& element = bgui::get_layout().add<bgui::element>();
    element.style.layout.require_size(100.f, 40.f);
    bgui::on_update();
    bgui::get_draw_data()->m_quad_requires = {};
    bgui::get_draw_data()->m_draw_list.clear();
    class zoom_on_update : public bgui::element {
        void on_update() override { bgui::set_global_scale(1.1f); }
    };
    bgui::get_layout().add<zoom_on_update>();
    bgui::on_update();
    EXPECT_EQ(element.processed_width(), 110);
    EXPECT_EQ(element.processed_height(), 44);
    bgui::set_global_scale(1.f);
}

TEST(AuditTest, ScaleCascadeDoesNotRepeatedlyInvalidateDirtyDescendants) {
    bgui::scoped_interface interface;
    bgui::set_global_scale(1.f);
    auto* parent = &bgui::get_layout();
    std::vector<style_probe*> nodes;
    for (int depth = 0; depth < 16; ++depth) {
        auto& node = parent->add<style_probe>();
        node.style.layout.require_size(100.f, 40.f);
        nodes.push_back(&node);
        parent = &node;
    }
    bgui::cascade_style();
    bgui::set_global_scale(1.1f);
    for (auto* node : nodes) node->visits = 0;
    bgui::cascade_style();
    for (auto* node : nodes) {
        EXPECT_LE(node->visits, 3);
        EXPECT_FALSE(node->is_style_dirty());
        EXPECT_FLOAT_EQ(node->computed_style.layout.size.x, 110.f);
    }
    bgui::set_global_scale(1.f);
}

TEST(AuditTest, UnchangedScaleDoesNotInvalidateStyles) {
    bgui::scoped_interface interface;
    bgui::set_global_scale(2.f);
    bgui::cascade_style();
    ASSERT_FALSE(bgui::get_layout().is_style_dirty());
    bgui::set_global_scale(2.f);
    EXPECT_FALSE(bgui::get_layout().is_style_dirty());
    bgui::set_global_scale(1.f);
}
