#include <gtest/gtest.h>
#include "bgui.hpp"

using namespace bgui;

TEST(ElementTest, requireSizeAndMode) {
    element elem;
    
    // 1. Request Size Test (default mode is pixel)
    elem.style.layout.require_size(150.f, 200.f);
    elem.compute_style();

    EXPECT_FLOAT_EQ(elem.computed_style.layout.size[0], 150.f);
    EXPECT_FLOAT_EQ(elem.computed_style.layout.size[1], 200.f);

    EXPECT_EQ(elem.computed_style.layout.size_mode[0], mode::pixel);
    EXPECT_EQ(elem.computed_style.layout.size_mode[1], mode::pixel);
    
    // 2. Test with percentual mode
    elem.style.layout.require_width(mode::percent, 50.f);
    elem.style.layout.require_height(mode::percent, 25.f);
    elem.compute_style();

    EXPECT_FLOAT_EQ(elem.computed_style.layout.size[0], 50.f);
    EXPECT_FLOAT_EQ(elem.computed_style.layout.size[1], 25.f);
    EXPECT_EQ(elem.computed_style.layout.size_mode[0], mode::percent);
    EXPECT_EQ(elem.computed_style.layout.size_mode[1], mode::percent);
}

TEST(ElementTest, FinalRectManipulation) {
    element elem;
    
    // 1. Manual final rect set
    elem.set_final_rect(10, 20, 100, 50);

    // 2. Verify getters
    EXPECT_EQ(elem.processed_x(), 10);
    EXPECT_EQ(elem.processed_y(), 20);
    EXPECT_EQ(elem.processed_width(), 100);
    EXPECT_EQ(elem.processed_height(), 50);

    EXPECT_EQ(elem.processed_position()[0], 10);
    EXPECT_EQ(elem.processed_size()[1], 50);
    EXPECT_EQ(elem.processed_rect().w, 50); // height

    elem.set_position(5, 15);
    EXPECT_EQ(elem.processed_x(), 5);
    EXPECT_EQ(elem.processed_y(), 15);
    EXPECT_EQ(elem.processed_width(), 100);

    elem.set_final_size(80, 40);
    EXPECT_EQ(elem.processed_width(), 80);
    EXPECT_EQ(elem.processed_height(), 40);
    EXPECT_EQ(elem.processed_x(), 5);
}

const vec2i AVAILABLE_SIZE = {400, 300};

TEST(ElementTest, UpdateSizeCalculation) {
    element elem;

    // 1. Test pixel mode
    elem.style.layout.require_size(100.f, 50.f);
    elem.compute_style();

    elem.process_required_size(AVAILABLE_SIZE); 
    
    EXPECT_EQ(elem.processed_width(), 100);
    EXPECT_EQ(elem.processed_height(), 50);

    // 2. Test percentual mode
    elem.style.layout.require_width(mode::percent, 50.f);
    elem.style.layout.require_height(mode::percent, 25.f);
    elem.compute_style();

    elem.process_required_size(AVAILABLE_SIZE);
    
    // 50% of 400 = 200
    EXPECT_EQ(elem.processed_width(), 200); 
    // 25% of 300 = 75
    EXPECT_EQ(elem.processed_height(), 75);

    // 3. Test match parent
    elem.style.layout.require_width(mode::match_parent, 0.f);
    elem.style.layout.require_height(mode::match_parent, 0.f);
    elem.compute_style();
    
    elem.process_required_size(AVAILABLE_SIZE);

    EXPECT_EQ(elem.processed_width(), 400); 
    EXPECT_EQ(elem.processed_height(), 300);

    // 4. Test percent and pixel
    elem.style.layout.require_width(mode::percent, 10.f);
    elem.style.layout.require_height(mode::pixel, 120.f);
    elem.compute_style();
    
    elem.process_required_size(AVAILABLE_SIZE);

    // 10% of 400 = 40
    EXPECT_EQ(elem.processed_width(), 40);
    EXPECT_EQ(elem.processed_height(), 120);

    // 5. Test padding and margin:
    elem.style.layout.set_padding(10, 10); // L, T, R, B
    elem.style.layout.set_margin(5, 5);     // L, T, R, B
    elem.style.layout.require_size(50.f, 50.f); 
    elem.style.layout.require_mode(mode::pixel, mode::pixel); 
    elem.compute_style();

    elem.process_required_size(AVAILABLE_SIZE);

    EXPECT_EQ(elem.processed_width(), 50);
    EXPECT_EQ(elem.processed_height(), 50);

    // 6. Test Stretched
    vec2i stretch_size = {150, 150};
    elem.style.layout.require_size(1.f, 1.f);
    elem.style.layout.require_mode(mode::stretch, mode::stretch);
    elem.compute_style();

    elem.process_required_size(stretch_size);
    
    EXPECT_EQ(elem.processed_width(), 150);
    EXPECT_EQ(elem.processed_height(), 150);
}

// Mock element class for testing
class mock_element : public element {
public:
    mock_element(int w, int h, mode w_mode = mode::pixel, mode h_mode = mode::pixel) {
        // Atualiza o style bruto e computa na construção
        style.layout.require_size(static_cast<float>(w), static_cast<float>(h));
        style.layout.require_mode(w_mode, h_mode);
        style.layout.limit_min = {0, 0};
        compute_style();
    }

    void on_update() override {}
};

// Test: Empty layout
TEST(LinearTest, EmptyLayoutContentSize) {
    bgui::set_up();

    linear layout(orientation::vertical);
    layout.style.layout.set_padding(10, 20);
    layout.process_required_size({100, 100});
    layout.compute_style();

    // Content size should be 0 becuse exclude the padding, margin etc.
    EXPECT_FLOAT_EQ(layout.content_width(), 0.0f);
    EXPECT_FLOAT_EQ(layout.content_height(), 0.0f);
}

// Test: Vertical layout with fixed size elements
TEST(LinearTest, VerticalLayoutFixedSizes) {
    bgui::set_up();
    linear layout(orientation::vertical);
    layout.style.layout.require_size(200, 400);
    layout.style.layout.require_mode(mode::pixel, mode::pixel);
    layout.process_required_size({200, 400});
    
    auto& elem1 = layout.add<mock_element>(100, 50);
    auto& elem2 = layout.add<mock_element>(100, 75);
    auto& elem3 = layout.add<mock_element>(100, 100);

    elem1.compute_style();
    elem2.compute_style();
    elem3.compute_style();
    
    layout.on_update();
    
    // Check that elements are positioned vertically
    EXPECT_EQ(elem1.processed_y(), 0);
    EXPECT_EQ(elem2.processed_y(), 50);
    EXPECT_EQ(elem3.processed_y(), 125);
}

// Test: Horizontal layout with fixed size elements
TEST(LinearTest, HorizontalLayoutFixedSizes) {
    bgui::set_up();
    linear layout(orientation::horizontal);
    layout.style.layout.require_size(400, 200);
    layout.style.layout.require_mode(mode::pixel, mode::pixel);
    layout.process_required_size({400, 200});
    
    auto& elem1 = layout.add<mock_element>(50, 100);
    auto& elem2 = layout.add<mock_element>(75, 100);
    auto& elem3 = layout.add<mock_element>(100, 100);

    elem1.compute_style();
    elem2.compute_style();
    elem3.compute_style();
    
    layout.on_update();
    
    // Check that elements are positioned horizontally
    EXPECT_EQ(elem1.processed_x(), 0);
    EXPECT_EQ(elem2.processed_x(), 50);
    EXPECT_EQ(elem3.processed_x(), 125);
}

// Test: Stretch mode in vertical layout
TEST(LinearTest, VerticalLayoutWithStretch) {
    bgui::set_up();
    linear layout(orientation::vertical);
    layout.style.layout.require_size(200, 400);
    layout.style.layout.require_mode(mode::pixel, mode::pixel);
    layout.style.layout.set_padding(0, 0);
    layout.compute_style();
    layout.process_required_size({200, 400});
    
    auto& elem1 = layout.add<mock_element>(100, 50, mode::pixel, mode::pixel);
    auto& elem2 = layout.add<mock_element>(100, 0, mode::pixel, mode::stretch);
    auto& elem3 = layout.add<mock_element>(100, 50, mode::pixel, mode::pixel);

    elem1.compute_style();
    elem2.compute_style();
    elem3.compute_style();
    
    layout.on_update();
    
    // elem2 should stretch to fill remaining space: 400 - 50 - 50 = 300
    EXPECT_EQ(elem2.processed_height(), 300);
}

// Test: Multiple stretch elements share space equally
TEST(LinearTest, MultipleStretchElements) {
    bgui::set_up();
    linear layout(orientation::vertical);
    layout.style.layout.require_size(200, 400);
    layout.style.layout.require_mode(mode::pixel, mode::pixel);
    layout.style.layout.set_padding(0, 0);

    layout.compute_style();
    layout.process_required_size({200, 400});
    
    auto &elem1 = layout.add<mock_element>(100, 0, mode::pixel, mode::stretch);
    elem1.style.layout.margin = {0, 0};
    auto &elem2 = layout.add<mock_element>(100, 0, mode::pixel, mode::stretch);
    elem2.style.layout.margin = {0, 0};

    elem1.compute_style();
    elem2.compute_style();
    
    layout.on_update();
    
    // Each should get half: 400 / 2 = 200
    EXPECT_EQ(elem1.processed_height(), 200);
    EXPECT_EQ(elem2.processed_height(), 200);
}

// Test: Match parent mode
TEST(LinearTest, MatchParentCrossAxis) {
    linear layout(orientation::vertical);
    layout.style.layout.require_size(200, 400);
    layout.style.layout.set_padding(0, 0);
    layout.style.layout.require_mode(mode::pixel, mode::pixel);
    layout.compute_style();
    layout.process_required_size({200, 400});
    
    
    auto& elem1 = layout.add<mock_element>(0, 100, mode::match_parent, mode::pixel);
    elem1.compute_style();
    layout.on_update();
    
    // Width should match parent
    EXPECT_EQ(elem1.processed_width(), 200);
}

// Test: Padding affects layout
TEST(LinearTest, PaddingAffectsLayout) {
    linear layout(orientation::vertical);
    layout.style.layout.require_size(200, 400);
    layout.style.layout.require_mode(mode::pixel, mode::pixel);
    layout.style.layout.set_padding(10, 20); // left, top, right, bottom
    layout.compute_style();
    layout.process_required_size({200, 400});
    
    auto& elem1 = layout.add<mock_element>(100, 50);
    elem1.compute_style();
    layout.on_update();
    
    // Element should be positioned after top padding
    EXPECT_EQ(elem1.processed_y(), 20);
    EXPECT_EQ(elem1.processed_x(), 10);
}

// Test: Margins affect spacing
TEST(LinearTest, MarginsAffectSpacing) {
    linear layout(orientation::vertical);
    layout.style.layout.require_size(200, 400);
    layout.style.layout.require_mode(mode::pixel, mode::pixel);
    layout.style.layout.set_padding(0, 0);
    layout.compute_style();
    layout.process_required_size({200, 400});
        
    auto& elem1 = layout.add<mock_element>(100, 50);
    auto& elem2 = layout.add<mock_element>(100, 50);
    elem1.style.layout.set_margin(0, 10); // 10px margin top and bottom
    elem1.compute_style();
    elem2.compute_style();
    layout.on_update();
    
    // elem2 should be after elem1 height + margins: 10 + 50 + 10 = 70
    EXPECT_EQ(elem2.processed_y(), 70);
}

// Test: Start alignment
TEST(LinearTest, AlignmentStart) {
    bgui::set_up();
    linear layout(orientation::vertical);
    layout.style.layout.require_size(200, 400);
    layout.style.layout.set_padding(0, 0);
    layout.style.layout.require_mode(mode::pixel, mode::pixel);
    layout.style.layout.align = (alignment::start);
    layout.compute_style();
    layout.process_required_size({200, 400});
    
    auto& elem1 = layout.add<mock_element>(100, 50);
    elem1.compute_style();
    layout.on_update();
    
    EXPECT_EQ(elem1.processed_y(), 0);
}

// Test: Center alignment
TEST(LinearTest, AlignmentCenter) {
    bgui::set_up();
    linear layout(orientation::vertical);
    layout.style.layout.require_size(200, 400);
    layout.style.layout.set_padding(0, 0);
    layout.style.layout.require_mode(mode::pixel, mode::pixel);
    layout.style.layout.align = (alignment::center);
    layout.compute_style();
    layout.process_required_size({200, 400});
    
    auto& elem1 = layout.add<mock_element>(100, 100);
    elem1.compute_style();
    layout.on_update();
    
    // Element should be centered: (400 - 100) / 2 = 150
    EXPECT_EQ(elem1.processed_y(), 150);
}
// Test: End alignment
TEST(LinearTest, AlignmentEnd) {
    bgui::set_up();
    linear layout(orientation::vertical);
    layout.style.layout.require_size(200, 400);
    layout.style.layout.set_padding(0, 0);
    layout.style.layout.require_mode(mode::pixel, mode::pixel);
    layout.style.layout.align = (alignment::end);
    layout.compute_style();
    layout.process_required_size({200, 400});
    
    auto& elem1 = layout.add<mock_element>(100, 100);
    elem1.compute_style();
    layout.on_update();
    
    // Element should be at end: 400 - 100 = 300
    EXPECT_EQ(elem1.processed_y(), 300);
}

// Test: Cross-axis center alignment
TEST(LinearTest, CrossAxisCenterAlignment) {
    bgui::set_up();
    linear layout(orientation::vertical);
    layout.style.layout.require_size(200, 400);
    layout.style.layout.set_padding(0, 0);
    layout.style.layout.require_mode(mode::pixel, mode::pixel);
    layout.style.layout.align = (alignment::start, alignment::center);
    layout.compute_style();
    layout.process_required_size({200, 400});
    
    auto& elem1 = layout.add<mock_element>(100, 50);
    elem1.compute_style();
    layout.on_update();
    
    // Element should be centered horizontally: (200 - 100) / 2 = 50
    EXPECT_EQ(elem1.processed_x(), 50);
}

// Test: Content width calculation for vertical layout
TEST(LinearTest, ContentWidthVertical) {
    bgui::set_up();
    linear layout(orientation::vertical);
    layout.style.layout.set_padding(5, 10);
    
    auto& elem1 = layout.add<mock_element>(100, 50);
    auto& elem2 = layout.add<mock_element>(150, 50);
    auto& elem3 = layout.add<mock_element>(80, 50);
    
    elem1.process_required_size({100, 50});
    elem2.process_required_size({150, 50});
    elem3.process_required_size({80, 50});
    layout.on_update();
    
    // Width should be max element width: 150
    EXPECT_FLOAT_EQ(layout.content_width(), 150.0f);
}

// Test: Content height calculation for horizontal layout
TEST(LinearTest, ContentHeightHorizontal) {
    bgui::set_up();
    linear layout(orientation::horizontal);
    layout.style.layout.set_padding(5, 10);
    
    auto& elem1 = layout.add<mock_element>(50, 100);
    auto& elem2 = layout.add<mock_element>(50, 150);
    auto& elem3 = layout.add<mock_element>(50, 80);
    
    elem1.process_required_size({50, 100});
    elem2.process_required_size({50, 150});
    elem3.process_required_size({50, 80});
    layout.on_update();
    
    // Height should be max element height: 150
    EXPECT_FLOAT_EQ(layout.content_height(), 150.0f);
}

// Test: Percent mode
TEST(LinearTest, PercentMode) {
    bgui::set_up();
    linear layout(orientation::vertical);
    layout.style.layout.set_padding(0, 0);
    layout.style.layout.require_size(200, 400);
    layout.style.layout.require_mode(mode::pixel, mode::pixel);
    layout.compute_style();
    layout.process_required_size({200, 400});
    
    auto& elem1 = layout.add<mock_element>(50, 25, mode::percent, mode::percent);
    elem1.compute_style();
    
    layout.on_update();
    
    EXPECT_EQ(elem1.processed_width(), 100);  // 50% of 200
    EXPECT_EQ(elem1.processed_height(), 100); // 25% of 400
}

// Test: Negative space handling
TEST(LinearTest, NegativeSpaceHandling) {
    bgui::set_up();
    linear layout(orientation::vertical);
    layout.style.layout.set_padding(0, 0);
    layout.style.layout.require_size(200, 100);
    layout.style.layout.require_mode(mode::pixel, mode::pixel);
    layout.compute_style();
    layout.process_required_size({200, 100});
    
    // Add elements that exceed available space
    auto& elem1 = layout.add<mock_element>(100, 80);
    auto& elem2 = layout.add<mock_element>(100, 80);
    elem1.compute_style();
    elem2.compute_style();
    
    // Should not crash and handle gracefully
    EXPECT_NO_THROW(layout.on_update());
}

// Test: Basic style resolution
TEST(StyleVisualTest, ResolveBackgroundColor) {
    bgui::set_up();

    auto& sm = bgui::style_manager::get_instance();

    bgui::element elem;

    elem.style.visual.background.normal = {1.f, 0.f, 0.f, 1.f}; // red
    elem.compute_style();

    const auto& cs = elem.computed_style.visual;

    EXPECT_FLOAT_EQ(cs.background.r, 1.f);
    EXPECT_FLOAT_EQ(cs.background.g, 0.f);
    EXPECT_FLOAT_EQ(cs.background.b, 0.f);
    EXPECT_FLOAT_EQ(cs.background.a, 1.f);
}

// Test: Visual Inherit
TEST(StyleVisualTest, InheritBackgroundFromParent) {
    bgui::set_up();

    bgui::linear parent(bgui::orientation::vertical);
    parent.style.visual.background.normal = {0.f, 0.f, 1.f, 1.f}; // blue
    parent.compute_style();

    auto& child = parent.add<bgui::element>();
    child.compute_style();

    const auto& parent_bg = parent.computed_style.visual.background;
    const auto& child_bg  = child.computed_style.visual.background;

    EXPECT_EQ(parent_bg, child_bg);
}

// Test: Visual Child Override
TEST(StyleVisualTest, ChildOverridesVisualStyle) {
    bgui::set_up();

    bgui::linear parent(bgui::orientation::vertical);
    parent.style.visual.background.normal = {0.f, 0.f, 0.f, 1.f};
    parent.compute_style();

    auto& child = parent.add<bgui::element>();
    child.style.visual.background.normal = {0.f, 1.f, 0.f, 1.f}; // green
    child.compute_style();

    EXPECT_NE(parent.computed_style.visual.background,
              child.computed_style.visual.background);

    EXPECT_FLOAT_EQ(child.computed_style.visual.background.g, 1.f);
}

// Test: Computed border
TEST(StyleVisualTest, BorderStyleResolution) {
    bgui::set_up();

    bgui::element elem;
    elem.style.visual.border = {1.f, 1.f, 1.f, 1.f};
    elem.style.visual.border_radius = 6.f;

    elem.compute_style();

    const auto& cs = elem.computed_style.visual;

    EXPECT_FLOAT_EQ(cs.border_radius, 6.f);
    EXPECT_FLOAT_EQ(cs.border.a, 1.f);
}

// Test: Integration layout + visual
TEST(StyleVisualTest, VisualStyleDoesNotBreakLayoutUpdate) {
    bgui::set_up();

    bgui::linear layout(bgui::orientation::vertical);
    layout.style.layout.set_padding(0, 0);
    layout.style.layout.require_size(200, 100);
    layout.style.layout.require_mode(bgui::mode::pixel, bgui::mode::pixel);
    layout.style.visual.background = {0.2f, 0.2f, 0.2f, 1.f};

    layout.compute_style();
    layout.process_required_size({200, 100});

    auto& elem1 = layout.add<mock_element>(100, 80);
    elem1.style.visual.background = {1.f, 0.f, 0.f, 1.f};
    elem1.compute_style();

    EXPECT_NO_THROW(layout.on_update());
}

// Test: StyleMnager singleton
TEST(StyleVisualTest, StyleManagerSingleton) {
    auto& sm1 = bgui::style_manager::get_instance();
    auto& sm2 = bgui::style_manager::get_instance();

    EXPECT_EQ(&sm1, &sm2);
}

// Test: Basic class style application
TEST(DeclarativeStyleTest, ApplyClassStyle) {
    bgui::set_up();
    auto& sm = style_manager::get_instance();
    
    // Define a class style
    style button_style;
    button_style.layout.require_size(100.f, 40.f);
    button_style.visual.background.normal = {0.2f, 0.4f, 0.8f, 1.f};
    sm.set_class("button", button_style);
    
    // Create element with class
    element elem;
    elem.classes = {"button"};
    elem.compute_style();
    
    EXPECT_FLOAT_EQ(elem.computed_style.layout.size[0], 100.f);
    EXPECT_FLOAT_EQ(elem.computed_style.layout.size[1], 40.f);
    EXPECT_FLOAT_EQ(elem.computed_style.visual.background.b, 0.8f);
}

// Test: Multiple classes cascade
TEST(DeclarativeStyleTest, MultipleClassesCascade) {
    bgui::set_up();
    auto& sm = style_manager::get_instance();
    
    style base_style;
    base_style.layout.require_width(mode::pixel, 200.f);
    base_style.visual.background.normal = {0.5f, 0.5f, 0.5f, 1.f};
    sm.set_class("base", base_style);
    
    style highlight_style;
    highlight_style.visual.background.normal = {1.f, 1.f, 0.f, 1.f}; // yellow
    sm.set_class("highlight", highlight_style);
    
    element elem;
    elem.classes = {"base", "highlight"};
    elem.compute_style();
    
    // Should have width from base, but color from highlight (later override)
    EXPECT_FLOAT_EQ(elem.computed_style.layout.size[0], 200.f);
    EXPECT_FLOAT_EQ(elem.computed_style.visual.background.g, 1.f);
    EXPECT_FLOAT_EQ(elem.computed_style.visual.background.b, 0.f);
}

// Test: ID style has higher priority than class
TEST(DeclarativeStyleTest, IDOverridesClass) {
    bgui::set_up();
    auto& sm = style_manager::get_instance();
    
    style class_style;
    class_style.layout.require_height(mode::pixel, 50.f);
    sm.set_class("normal", class_style);
    
    style id_style;
    id_style.layout.require_height(mode::pixel, 100.f);
    sm.set_id("special", id_style);
    
    element elem;
    elem.classes = {"normal"};
    elem.id = ("special");
    elem.compute_style();
    
    // ID should override class
    EXPECT_FLOAT_EQ(elem.computed_style.layout.size[1], 100.f);
}

// Test: Inline style has highest priority
TEST(DeclarativeStyleTest, InlineOverridesAll) {
    bgui::set_up();
    auto& sm = style_manager::get_instance();
    
    style class_style;
    class_style.visual.background.normal = {1.f, 0.f, 0.f, 1.f}; // red
    sm.set_class("themed", class_style);
    
    element elem;
    elem.classes = {"themed"};
    elem.style.visual.background.normal = {0.f, 1.f, 0.f, 1.f}; // green inline
    elem.compute_style();
    
    // Inline should win
    EXPECT_FLOAT_EQ(elem.computed_style.visual.background.r, 0.f);
    EXPECT_FLOAT_EQ(elem.computed_style.visual.background.g, 1.f);
}

// Test: Type-based styles
TEST(DeclarativeStyleTest, TypeStyleApplication) {
    bgui::set_up();
    auto& sm = style_manager::get_instance();
    
    style button_type_style;
    button_type_style.layout.require_size(120.f, 40.f);
    button_type_style.visual.border_radius = 8.f;
    sm.set_type("button", button_type_style);
    
    element elem;
    elem.type = ("button");
    elem.compute_style();
    
    EXPECT_FLOAT_EQ(elem.computed_style.layout.size[0], 120.f);
    EXPECT_FLOAT_EQ(elem.computed_style.visual.border_radius, 8.f);
}

// Test: Default style applies to all elements
TEST(DeclarativeStyleTest, DefaultStyleApplication) {
    bgui::set_up();
    auto& sm = style_manager::get_instance();
    
    style default_style;
    default_style.visual.font = std::make_optional("default");
    default_style.visual.text.normal = {0.9f, 0.9f, 0.9f, 1.f};
    sm.set_default(default_style);
    
    element elem;
    elem.compute_style();
    
    EXPECT_EQ(elem.computed_style.visual.font, "default");
    EXPECT_FLOAT_EQ(elem.computed_style.visual.text.r, 0.9f);
}

// Test: Cascade priority order (default < type < class < id < inline)
TEST(DeclarativeStyleTest, CompleteCascadePriority) {
    bgui::set_up();
    auto& sm = style_manager::get_instance();
    
    style default_style;
    default_style.layout.require_width(mode::pixel, 50.f);
    sm.set_default(default_style);
    
    style type_style;
    type_style.layout.require_width(mode::pixel, 100.f);
    sm.set_type("widget", type_style);
    
    style class_style;
    class_style.layout.require_width(mode::pixel, 150.f);
    sm.set_class("large", class_style);
    
    style id_style;
    id_style.layout.require_width(mode::pixel, 200.f);
    sm.set_id("mega", id_style);
    
    element elem;
    elem.type = ("widget");
    elem.classes = {"large"};
    elem.id = ("mega");
    elem.style.layout.require_width(mode::pixel, 300.f); // inline
    elem.compute_style();
    
    // Inline should win (highest priority)
    EXPECT_FLOAT_EQ(elem.computed_style.layout.size[0], 300.f);
    
    // Test without inline
    element elem2;
    elem2.type = ("widget");
    elem2.classes = {"large"};
    elem2.id = ("mega");
    elem2.compute_style();
    
    // ID should win
    EXPECT_FLOAT_EQ(elem2.computed_style.layout.size[0], 200.f);
}

// Test: Theme application
TEST(DeclarativeStyleTest, ThemeApplication) {
    bgui::set_up();
    auto& sm = style_manager::get_instance();
    
    theme dark_theme;
    dark_theme.base.visual.background.normal = {0.1f, 0.1f, 0.1f, 1.f};
    dark_theme.base.visual.text.normal = {0.9f, 0.9f, 0.9f, 1.f};
    
    style dark_button;
    dark_button.visual.background.normal = {0.2f, 0.2f, 0.3f, 1.f};
    dark_theme.types["button"] = dark_button;
    
    style primary_class;
    primary_class.visual.background.normal = {0.2f, 0.4f, 0.9f, 1.f};
    dark_theme.classes["primary"] = primary_class;
    
    sm.apply_theme(dark_theme);
    
    element elem;
    elem.type = ("button");
    elem.classes = {"primary"};
    elem.compute_style();
    
    // Should have primary color (class overrides type)
    EXPECT_FLOAT_EQ(elem.computed_style.visual.background.b, 0.9f);
    
    // Text should come from base theme
    EXPECT_FLOAT_EQ(elem.computed_style.visual.text.r, 0.9f);
}

// Test: Class removal and recomputation
TEST(DeclarativeStyleTest, ClassRemovalRecomputation) {
    bgui::set_up();
    auto& sm = style_manager::get_instance();
    
    style active_style;
    active_style.visual.background.normal = {0.f, 1.f, 0.f, 1.f};
    sm.set_class("active", active_style);
    
    element elem;
    elem.classes = {"active"};
    elem.compute_style();
    
    EXPECT_FLOAT_EQ(elem.computed_style.visual.background.g, 1.f);
    
    // Remove class and recompute
    elem.remove_class("active");
    elem.compute_style();
    
    // Should revert to default (no green)
    EXPECT_NE(elem.computed_style.visual.background.g, 1.f);
}

// Test: Pseudo-class states (hover, active, disabled)
TEST(DeclarativeStyleTest, InputStateResolution) {
    bgui::set_up();
    auto& sm = style_manager::get_instance();
    
    style button_style;
    button_style.visual.background.normal = {0.5f, 0.5f, 0.5f, 1.f};
    button_style.visual.background.hover = {0.7f, 0.7f, 0.7f, 1.f};
    button_style.visual.background.pressed = {0.3f, 0.3f, 0.3f, 1.f};
    sm.set_class("button", button_style);
    
    element elem;
    elem.classes = {"button"};
    
    // Normal state
    elem.set_state(state::normal);
    elem.compute_style();
    EXPECT_FLOAT_EQ(elem.computed_style.visual.background.r, 0.5f);
    
    // Hover state
    elem.set_state(state::hover);
    elem.compute_style();
    EXPECT_FLOAT_EQ(elem.computed_style.visual.background.r, 0.7f);
    
    // Active state
    elem.set_state(state::pressed);
    elem.compute_style();
    EXPECT_FLOAT_EQ(elem.computed_style.visual.background.r, 0.3f);
}

// Test: Layout in vertical with declarative classes
TEST(DeclarativeStyleTest, LayoutWithDeclarativeClasses) {
    bgui::set_up();
    auto& sm = style_manager::get_instance();
    
    style sidebar_style;
    sidebar_style.layout.require_width(mode::pixel, 250.f);
    sidebar_style.layout.require_height(mode::match_parent);
    sm.set_class("sidebar", sidebar_style);
    
    style button_style;
    button_style.layout.require_width(mode::match_parent);
    button_style.layout.require_height(mode::pixel, 40.f);
    button_style.layout.align = alignment::center;
    sm.set_class("sidebar-button", button_style);
    
    linear layout(orientation::vertical);
    layout.add_class("sidebar");
    layout.style.layout.require_size(250, 500);
    layout.style.layout.set_padding(15, 10);
    layout.compute_style();
    layout.process_required_size({250, 500});
    
    auto& btn1 = layout.add<element>();
    btn1.add_class("sidebar-button");
    btn1.compute_style();
    
    auto& btn2 = layout.add<element>();
    btn2.add_class("sidebar-button");
    btn2.compute_style();
    
    layout.on_update();

    // Buttons should match parent width minus padding
    EXPECT_EQ(btn1.processed_width(), 220);
    EXPECT_EQ(btn1.processed_height(), 40);
    EXPECT_EQ(btn2.processed_height(), 40);
    
    // Second button positioned after first
    EXPECT_EQ(btn2.processed_y(), 50); // 10 (top padding) + 40 (btn1 height) + some spacing
}

// Test: Global style caching
TEST(DeclarativeStyleTest, GlobalComputedStyleCaching) {
    bgui::set_up();
    auto& sm = style_manager::get_instance();
    
    theme app_theme;
    app_theme.base.visual.font = std::make_optional("default");
    app_theme.base.visual.text.normal = {1.f, 1.f, 1.f, 1.f};
    sm.apply_theme(app_theme);
    
    auto global = sm.get_global();
    
    EXPECT_EQ(global.visual.font, "default");
    EXPECT_FLOAT_EQ(global.visual.text.normal->r, 1.f);
}

// Test: Empty classes don't break resolution
TEST(DeclarativeStyleTest, EmptyClassesHandling) {
    bgui::set_up();
    
    element elem;
    elem.classes = {};
    
    EXPECT_NO_THROW(elem.compute_style());
}

// Test: Non-existent class doesn't break resolution
TEST(DeclarativeStyleTest, NonExistentClassHandling) {
    bgui::set_up();
    
    element elem;
    elem.classes = {"non-existent-class"};
    
    EXPECT_NO_THROW(elem.compute_style());
}

// Test: Partial style override
TEST(DeclarativeStyleTest, PartialStyleOverride) {
    bgui::set_up();
    auto& sm = style_manager::get_instance();
    
    style base_style;
    base_style.layout.require_size(100.f, 100.f);
    base_style.visual.background.normal = {1.f, 0.f, 0.f, 1.f};
    base_style.visual.border_radius = 5.f;
    sm.set_class("base", base_style);
    
    style modifier_style;
    modifier_style.visual.background.normal = {0.f, 0.f, 1.f, 1.f}; // only background
    sm.set_class("blue", modifier_style);
    
    element elem;
    elem.classes = {"base", "blue"};
    elem.compute_style();
    
    // Size and border from base, color from blue
    EXPECT_FLOAT_EQ(elem.computed_style.layout.size[0], 100.f);
    EXPECT_FLOAT_EQ(elem.computed_style.visual.border_radius, 5.f);
    EXPECT_FLOAT_EQ(elem.computed_style.visual.background.b, 1.f);
}

// Test: Declarative style with stretch mode
TEST(DeclarativeStyleTest, StretchModeWithClasses) {
    // Set up
    bgui::set_up();
    auto& sm = style_manager::get_instance();
    
    // Creates stretch style
    style fill_style;
    fill_style.layout.require_mode(mode::stretch, mode::stretch);
    // Set it's class
    sm.set_class("fill", fill_style);
    
    // Creates the layout to be the parent
    linear layout(orientation::vertical);
    layout.style.layout.require_size(300, 400);
    layout.style.layout.set_padding(0, 0);
    layout.compute_style();
    layout.process_required_size({300, 400});
    
    // Give the "fill" class to the child elemnt
    auto& elem = layout.add<element>();
    elem.add_class("fill");

    // Compute styles
    layout.cascade_style();
    
    // Update sizes
    layout.on_update();
    
    // Width must be the minimum size becuse only works with the main axis (vertical).
    EXPECT_EQ(elem.processed_width(), 15);
    EXPECT_EQ(elem.processed_height(), 400);
}

// Test: Class toggle behavior
TEST(DeclarativeStyleTest, ClassToggle) {
    bgui::set_up();
    auto& sm = style_manager::get_instance();
    
    style hidden_style;
    hidden_style.visual.visible = false;
    sm.set_class("hidden", hidden_style);
    
    element elem;
    elem.compute_style();
    
    EXPECT_TRUE(elem.computed_style.visual.visible);
    
    elem.add_class("hidden");
    elem.compute_style();
    
    EXPECT_FALSE(elem.computed_style.visual.visible);
    
    elem.remove_class("hidden");
    elem.compute_style();
    
    EXPECT_TRUE(elem.computed_style.visual.visible);
}

// Test: Complex nested layout with declarative styles
TEST(DeclarativeStyleTest, ComplexNestedLayoutWithClasses) {
    bgui::set_up();
    auto& sm = style_manager::get_instance();
    
    style container_style;
    container_style.layout.set_padding(20, 20);
    container_style.visual.background.normal = {0.1f, 0.1f, 0.1f, 1.f};
    sm.set_class("container", container_style);
    
    style card_style;
    card_style.layout.require_width(mode::match_parent);
    card_style.layout.set_padding(10, 10);
    card_style.visual.background.normal = {0.2f, 0.2f, 0.2f, 1.f};
    card_style.visual.border_radius = 8.f;
    sm.set_class("card", card_style);
    
    linear root(orientation::vertical);
    root.add_class("container");
    root.style.layout.require_size(400, 600);
    root.compute_style();
    root.process_required_size({400, 600});
    
    auto& card = root.add<linear>(orientation::vertical);
    card.add_class("card");
    card.compute_style();
    
    auto& content = card.add<element>();
    content.compute_style();
    
    root.on_update();
    
    // Card should match parent width minus container padding
    EXPECT_EQ(card.processed_width(), 360); // 400 - 20 - 20
}