#include <gtest/gtest.h>
#include <glad/glad.h>
#include <bgui.hpp>
#include <GLFW/glfw3.h>
#include <array>
#include <filesystem>
#include <vector>
#include "os/asset_manager.hpp"

namespace {
class GL3Test : public testing::Test {
protected:
    GLFWwindow* window{};
    bgui::vec2i previous_size;

    void SetUp() override {
        if (!glfwInit())
            GTEST_SKIP() << "No GLFW display available";
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        window = glfwCreateWindow(64, 64, "BGUI rendering test", nullptr, nullptr);
        if (!window) {
            glfwTerminate();
            GTEST_SKIP() << "OpenGL 3.3 context unavailable";
        }
        glfwMakeContextCurrent(window);
        bgui::set_up_gl3();
        previous_size = bgui::get_context().m_size;
        bgui::get_context().m_size = {64, 64};
        glViewport(0, 0, 64, 64);
        glDisable(GL_FRAMEBUFFER_SRGB);
        glClearColor(0.f, 0.f, 0.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void TearDown() override {
        if (!window)
            return;
        bgui::get_context().m_size = previous_size;
        bgui::shutdown_gl3();
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    std::array<unsigned char, 4> pixel(int x, int y) {
        std::array<unsigned char, 4> result{};
        glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, result.data());
        return result;
    }
};
}

TEST_F(GL3Test, ImageRoundsCornersAndPreservesTextureAndBorder) {
    bgui::texture texture;
    texture.m_path = "rounded-image-test";
    texture.m_size = {1.f, 1.f};
    texture.m_has_alpha = true;
    texture.m_generate_mipmap = false;
    texture.m_buffer = {255, 0, 0, 255};
    bgui::image image(texture);
    image.style.visual.border_radius = 12.f;
    image.style.visual.border_size = 2.f;
    image.style.visual.border.normal = bgui::color{0.f, 1.f, 0.f, 1.f};
    image.compute_style();
    image.set_final_rect(8, 8, 48, 48);
    bgui::draw_data data;
    data.m_clip_rect = {0, 0, 64, 64};
    image.get_requires(&data);
    bgui::gl3_render(&data);

    EXPECT_EQ(pixel(8, 8)[0], 0); // Rounded corner exposes the background.
    EXPECT_EQ(pixel(55, 55)[0], 0);
    EXPECT_EQ(pixel(32, 32)[0], 255); // Texture remains visible in the center.
    EXPECT_EQ(pixel(32, 8)[1], 255); // Border follows the image contour.
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
    // Exercise another decoded image in the same GL context as the UI renderer.
    {
    glClear(GL_COLOR_BUFFER_BIT);
    const auto path = std::filesystem::path(__FILE__).parent_path() / "assets/rgb.jpg";
    bgui::image image(bgui::asset_manager::get_instance().load_texture(path.string()));
    image.compute_style();
    image.set_final_rect(8, 8, 48, 48);
    bgui::draw_data data;
    data.m_clip_rect = {0, 0, 64, 64};
    image.get_requires(&data);
    bgui::gl3_render(&data);
    EXPECT_GT(pixel(32, 32)[0], 240);
    EXPECT_LT(pixel(32, 32)[1], 10);
    EXPECT_LT(pixel(32, 32)[2], 10);
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}
}

TEST(GL3SetupTest, ReturnsWithoutACurrentContext) {
    ASSERT_EQ(glfwGetCurrentContext(), nullptr);
    EXPECT_NO_THROW(bgui::set_up_gl3());
}

namespace {
class green_geometry : public bgui::element {
public:
    void get_requires(bgui::draw_data* data) override {
        data->m_draw_list.set_color({0, 1, 0, 1});
        data->m_draw_list.add_triangle({4, 4}, {60, 4}, {4, 60});
    }
};

class mouse_probe : public bgui::element {
public:
    int clicks{};
    int presses{};
    int releases{};

    mouse_probe() { receives_input(true); }
    void on_clicked() override { ++clicks; }
    void on_pressed() override { ++presses; }
    void on_released() override { ++releases; }
};

void host_char_callback(GLFWwindow*, unsigned int) {}

class AttachedWindow {
public:
    explicit AttachedWindow(GLFWwindow* window) { bgui::attach_glfw_window(window); }
    ~AttachedWindow() { bgui::detach_glfw_window(); }
};
}

TEST_F(GL3Test, CachedBaseGeometryStaysBehindOverlayQuads) {
    bgui::scoped_interface interface;
    auto& root = bgui::get_layout();
    auto& nested = root.add<bgui::layout>();
    nested.style.layout.require_size(64, 64);
    nested.add<green_geometry>().style.layout.require_size(64, 64);
    auto& overlay = root.add<bgui::element, bgui::layer::overlay>();
    overlay.style.layout.require_size(24, 24);
    overlay.style.visual.background.normal = bgui::color{1, 0, 0, 1};

    for (int frame = 0; frame < 2; ++frame) {
        glClear(GL_COLOR_BUFFER_BIT);
        auto& global = bgui::get_draw_list();
        global.set_color({0, 0, 1, 1});
        global.add_triangle({15, 15}, {23, 15}, {15, 23});
        bgui::on_update();
        bgui::gl3_render(bgui::get_draw_data());
        EXPECT_EQ(pixel(16, 47), (std::array<unsigned char, 4>{0, 0, 255, 255}));
        EXPECT_EQ(pixel(12, 51), (std::array<unsigned char, 4>{255, 0, 0, 255}));
        EXPECT_EQ(pixel(32, 51), (std::array<unsigned char, 4>{0, 255, 0, 255}));
    }
}

TEST_F(GL3Test, RenderRestoresCallerState) {
    GLuint textures[2]{}, vao{}, buffer{}, uniform_buffers[2]{};
    glGenBuffers(2, uniform_buffers);
    GLint alignment{};
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &alignment);
    glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffers[0]);
    glBufferData(GL_UNIFORM_BUFFER, alignment + 64, nullptr, GL_STATIC_DRAW);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, uniform_buffers[0], alignment, 64);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, uniform_buffers[0]);
    glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffers[1]);
    glGenTextures(2, textures);
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &buffer);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, textures[1]);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_SCISSOR_TEST);
    glScissor(3, 5, 7, 9);
    glDisable(GL_BLEND);
    glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ZERO, GL_ONE);
    glBlendEquationSeparate(GL_FUNC_REVERSE_SUBTRACT, GL_FUNC_SUBTRACT);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 8);

    bgui::draw_data data;
    data.m_clip_rect = {0, 0, 64, 64};
    bgui::texture texture;
    texture.m_external = true;
    texture.m_id = textures[1];
    texture.m_size = {1, 1};
    bgui::image image(texture);
    image.compute_style();
    image.set_final_rect(8, 8, 48, 48);
    image.get_requires(&data);
    bgui::gl3_render(&data);

    EXPECT_TRUE(glIsEnabled(GL_DEPTH_TEST));
    EXPECT_TRUE(glIsEnabled(GL_CULL_FACE));
    EXPECT_TRUE(glIsEnabled(GL_SCISSOR_TEST));
    EXPECT_FALSE(glIsEnabled(GL_BLEND));
    std::array<GLint, 4> scissor;
    glGetIntegerv(GL_SCISSOR_BOX, scissor.data());
    EXPECT_EQ(scissor, (std::array<GLint, 4>{3, 5, 7, 9}));
    const auto state = [](GLenum name) {
        GLint value{};
        glGetIntegerv(name, &value);
        return value;
    };
    EXPECT_EQ(state(GL_ACTIVE_TEXTURE), GL_TEXTURE3);
    EXPECT_EQ(state(GL_TEXTURE_BINDING_2D), textures[1]);
    glActiveTexture(GL_TEXTURE0);
    EXPECT_EQ(state(GL_TEXTURE_BINDING_2D), textures[0]);
    EXPECT_EQ(state(GL_VERTEX_ARRAY_BINDING), vao);
    EXPECT_EQ(state(GL_ARRAY_BUFFER_BINDING), buffer);
    EXPECT_EQ(state(GL_BLEND_SRC_RGB), GL_ONE);
    EXPECT_EQ(state(GL_BLEND_DST_ALPHA), GL_ONE);
    EXPECT_EQ(state(GL_BLEND_EQUATION_RGB), GL_FUNC_REVERSE_SUBTRACT);
    EXPECT_EQ(state(GL_BLEND_EQUATION_ALPHA), GL_FUNC_SUBTRACT);
    EXPECT_EQ(state(GL_UNPACK_ALIGNMENT), 8);
    EXPECT_EQ(state(GL_UNIFORM_BUFFER_BINDING), uniform_buffers[1]);
    for (GLuint binding : {0u, 1u}) {
        GLint bound{};
        glGetIntegeri_v(GL_UNIFORM_BUFFER_BINDING, binding, &bound);
        EXPECT_EQ(bound, uniform_buffers[0]);
    }
    GLint64 offset{}, size{};
    glGetInteger64i_v(GL_UNIFORM_BUFFER_START, 0, &offset);
    glGetInteger64i_v(GL_UNIFORM_BUFFER_SIZE, 0, &size);
    EXPECT_EQ(offset, alignment);
    EXPECT_EQ(size, 64);
    EXPECT_EQ(glGetError(), GL_NO_ERROR);

    glDeleteBuffers(2, uniform_buffers);
    glDeleteTextures(2, textures);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &buffer);
}

TEST_F(GL3Test, TwoClicksBetweenUpdatesDispatchAllTransitionsAtTheEventPosition) {
    bgui::scoped_interface interface;
    AttachedWindow attached(window);
    double x{}, y{};
    glfwGetCursorPos(window, &x, &y);
    auto& probe = bgui::get_layout().add<mouse_probe>();
    probe.set_final_rect(static_cast<int>(x) - 2, static_cast<int>(y) - 2, 5, 5);
    for (int click = 0; click < 2; ++click) {
        bgui::glfw_mouse_button_callback(window, GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS, 0);
        bgui::glfw_mouse_button_callback(window, GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE, 0);
    }

    auto& io = bgui::get_context();
    io.m_mouse_position = {static_cast<int>(x) + 10, static_cast<int>(y) + 10};
    bgui::on_update();
    EXPECT_EQ(probe.clicks, 2);
    EXPECT_EQ(probe.presses, 2);
    EXPECT_EQ(probe.releases, 2);
    EXPECT_FALSE(bgui::get_pressed(bgui::input_key::mouse_left));
    EXPECT_EQ(bgui::get_mouse_target(), nullptr);
}

TEST_F(GL3Test, DetachRestoresHostCallbackAndIgnoresForwardedCharacters) {
    glfwSetCharCallback(window, host_char_callback);
    AttachedWindow attached(window);
    auto& io = bgui::get_context();
    io.m_char_buffer.clear();
    bgui::glfw_char_callback(window, 'A');
    ASSERT_EQ(io.m_char_buffer, "A");

    bgui::detach_glfw_window();
    EXPECT_EQ(glfwSetCharCallback(window, nullptr), host_char_callback);
    bgui::glfw_char_callback(window, 'B');
    EXPECT_TRUE(io.m_char_buffer.empty());
}

namespace {
class InstanceDraws {
    static inline PFNGLDRAWARRAYSINSTANCEDPROC original{};
    static inline std::vector<GLsizei> counts;

    static void APIENTRY record(GLenum mode, GLint first, GLsizei vertices, GLsizei instances) {
        counts.push_back(instances);
        original(mode, first, vertices, instances);
    }

public:
    InstanceDraws() {
        counts.clear();
        original = glDrawArraysInstanced;
        glDrawArraysInstanced = record;
    }
    ~InstanceDraws() { glDrawArraysInstanced = original; }
    const auto& instance_counts() const { return counts; }
};
}

TEST_F(GL3Test, BatchesDifferentWidgetStylesWithoutCrossingClipOrGeometry) {
    bgui::element red;
    red.style.visual.background.normal = bgui::color{1, 0, 0, 1};
    red.compute_style();
    red.set_final_rect(0, 0, 48, 64);

    bgui::element blue;
    blue.style.visual.background.normal = bgui::color{0, 0, 1, 0.5f};
    blue.style.visual.border.normal = bgui::color{1, 1, 0, 1};
    blue.style.visual.border_radius = 6.f;
    blue.style.visual.border_size = 2.f;
    blue.compute_style();
    blue.set_final_rect(16, 0, 48, 64);

    bgui::draw_data data;
    data.m_clip_rect = {0, 0, 48, 64};
    red.get_requires(&data);
    blue.get_requires(&data);
    data.m_draw_list.set_color({0, 1, 0, 1});
    data.m_draw_list.add_rect_filled({20, 20}, {64, 44});
    data.m_widget_vertex_end = data.m_draw_list.get_vertices().size();
    blue.get_requires(&data);
    data.m_clip_rect = {48, 0, 16, 64};
    data.enqueue({.m_material = red.get_material(), .m_rect = {0, 0, 64, 16}});
    data.m_clip_rect = {0, 0, 64, 64};

    InstanceDraws draws;
    bgui::gl3_render(&data);

    EXPECT_EQ(draws.instance_counts(), (std::vector<GLsizei>{2, 1, 1}));
    EXPECT_EQ(pixel(8, 48)[0], 255);
    EXPECT_EQ(pixel(16, 40), (std::array<unsigned char, 4>{255, 255, 0, 255}));
    EXPECT_NEAR(pixel(24, 48)[0], 63, 1);
    EXPECT_NEAR(pixel(24, 48)[2], 192, 1);
    EXPECT_EQ(pixel(24, 32)[0], 0);
    EXPECT_NEAR(pixel(24, 32)[1], 127, 1);
    EXPECT_NEAR(pixel(24, 32)[2], 128, 1);
    EXPECT_EQ(pixel(56, 56)[0], 255);
    EXPECT_EQ(pixel(56, 47)[2], 0);
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(GL3Test, TexturedInstancesKeepTheirUvBoundsAndSplitOnTextureChanges) {
    bgui::material atlas;
    atlas.m_shader_tag = "ui::image";
    atlas.m_use_tex = true;
    atlas.m_texture.m_path = "instance-atlas";
    atlas.m_texture.m_size = {2, 1};
    atlas.m_texture.m_has_alpha = true;
    atlas.m_texture.m_generate_mipmap = false;
    atlas.m_texture.m_buffer = {255, 0, 0, 255, 0, 255, 0, 255};

    bgui::material other = atlas;
    other.m_texture.m_path = "instance-other";
    other.m_texture.m_size = {1, 1};
    other.m_texture.m_buffer = {0, 0, 255, 255};

    bgui::draw_data data;
    data.m_clip_rect = {0, 0, 64, 64};
    data.enqueue({.m_material = atlas, .m_rect = {0, 0, 16, 16},
                  .m_uv_min = {0.25f, 0}, .m_uv_max = {0.25f, 1}});
    data.enqueue({.m_material = atlas, .m_rect = {16, 0, 16, 16},
                  .m_uv_min = {0.75f, 0}, .m_uv_max = {0.75f, 1}});
    data.enqueue({.m_material = other, .m_rect = {32, 0, 16, 16}});

    InstanceDraws draws;
    bgui::gl3_render(&data);

    EXPECT_EQ(draws.instance_counts(), (std::vector<GLsizei>{2, 1}));
    EXPECT_EQ(pixel(8, 56), (std::array<unsigned char, 4>{255, 0, 0, 255}));
    EXPECT_EQ(pixel(24, 56), (std::array<unsigned char, 4>{0, 255, 0, 255}));
    EXPECT_EQ(pixel(40, 56), (std::array<unsigned char, 4>{0, 0, 255, 255}));
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(GL3Test, LegacyQuadInheritsUniformsFromLastStyledInstance) {
    bgui::element red;
    red.style.visual.background.normal = bgui::color{1, 0, 0, 1};
    red.compute_style();
    red.set_final_rect(0, 0, 16, 16);

    bgui::element green;
    green.style.visual.background.normal = bgui::color{0, 1, 0, 1};
    green.compute_style();
    green.set_final_rect(16, 0, 16, 16);

    bgui::material legacy;
    bgui::draw_data data;
    data.m_clip_rect = {0, 0, 64, 64};
    red.get_requires(&data);
    green.get_requires(&data);
    data.enqueue({.m_material = legacy, .m_count = 3, .m_rect = {32, 0, 16, 16}});

    InstanceDraws draws;
    bgui::gl3_render(&data);

    EXPECT_EQ(draws.instance_counts(), (std::vector<GLsizei>{2}));
    EXPECT_EQ(pixel(36, 59), (std::array<unsigned char, 4>{0, 255, 0, 255}));
    EXPECT_EQ(pixel(45, 51), (std::array<unsigned char, 4>{0, 0, 0, 255}));
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(GL3Test, TextMaterialBlockAppliesOverrideAndProjectionChanges) {
    bgui::material glyph;
    glyph.m_shader_tag = "ui::text";
    glyph.m_use_tex = true;
    glyph.m_texture.m_path = "ubo-glyph";
    glyph.m_texture.m_size = {1, 1};
    glyph.m_texture.m_generate_mipmap = false;
    glyph.m_texture.m_use_red_channel = true;
    glyph.m_texture.m_buffer = {255};
    glyph.set("text_color", bgui::vec4{1, 0, 0, 1});
    glyph.set("projection", bgui::mat4{});

    bgui::draw_data data;
    data.m_clip_rect = {0, 0, 64, 64};
    data.enqueue({.m_material = glyph, .m_rect = {0, 0, 16, 16}});
    data.enqueue({.m_material = glyph, .m_rect = {16, 0, 16, 16}}, bgui::vec4{0, 1, 0, 1});
    bgui::gl3_render(&data);
    EXPECT_EQ(pixel(8, 56), (std::array<unsigned char, 4>{255, 0, 0, 255}));
    EXPECT_EQ(pixel(24, 56), (std::array<unsigned char, 4>{0, 255, 0, 255}));

    glClear(GL_COLOR_BUFFER_BIT);
    bgui::get_context().m_size = {32, 64};
    data.enqueue({.m_material = glyph, .m_rect = {16, 0, 16, 16}});
    bgui::gl3_render(&data);
    EXPECT_EQ(pixel(24, 56), (std::array<unsigned char, 4>{0, 0, 0, 255}));
    EXPECT_EQ(pixel(48, 56), (std::array<unsigned char, 4>{255, 0, 0, 255}));
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}
