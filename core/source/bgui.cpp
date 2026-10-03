#include "bgui.hpp"
#include "os/os.hpp"
#include "os/style_manager.hpp"
#include "lay/dock.hpp"
#include <utils/vec.hpp>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <map>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <utility>
#include <vector>

static bool init_trigger = false;
std::unique_ptr<bgui::layout> bgui::s_main_layout;
static std::unique_ptr<bgui::draw_data> s_draw_data;
static std::queue<std::function<void()>> s_functions;
static bgui::element* s_keyboard_focused = nullptr;
static bgui::element* s_mouse_target = nullptr;
static float s_global_scale = 1.f;

namespace {
    constexpr const char* configuration_header = "cpp-bgui-ui 1";

    using configuration_sections = std::map<std::string, std::map<std::string, std::string>>;
    void collect_docks(bgui::layout& current, std::vector<bgui::dock*>& docks);

    std::string trim(const std::string& value) {
        const auto first = value.find_first_not_of(" \t\r\n");
        if (first == std::string::npos)
            return {};
        const auto last = value.find_last_not_of(" \t\r\n");
        return value.substr(first, last - first + 1);
    }

    bool parse_configuration_sections(
        std::istream& input,
        configuration_sections& sections)
    {
        std::string section = "interface";
        std::string line;
        while (std::getline(input, line)) {
            line = trim(line);
            if (line.empty() || line.front() == '#' || line.front() == ';')
                continue;
            if (line.front() == '[' && line.back() == ']') {
                section = trim(line.substr(1, line.size() - 2));
                if (section.empty())
                    return false;
                continue;
            }

            const auto separator = line.find('=');
            if (separator == std::string::npos)
                return false;
            const auto key = trim(line.substr(0, separator));
            const auto value = trim(line.substr(separator + 1));
            if (key.empty() || !sections[section].emplace(key, value).second)
                return false;
        }
        return input.eof();
    }

    template<typename T>
    bool read_configuration_value(
        const configuration_sections& sections,
        const std::string& section,
        const std::string& key,
        T& value)
    {
        const auto section_entry = sections.find(section);
        if (section_entry == sections.end())
            return false;
        const auto value_entry = section_entry->second.find(key);
        if (value_entry == section_entry->second.end())
            return false;

        std::istringstream input(value_entry->second);
        if (!(input >> value))
            return false;
        input >> std::ws;
        return input.eof();
    }

    bool read_configuration_string(
        const configuration_sections& sections,
        const std::string& section,
        const std::string& key,
        std::string& value)
    {
        const auto section_entry = sections.find(section);
        if (section_entry == sections.end())
            return false;
        const auto value_entry = section_entry->second.find(key);
        if (value_entry == section_entry->second.end())
            return false;

        std::istringstream input(value_entry->second);
        if (!(input >> std::quoted(value)))
            return false;
        input >> std::ws;
        return input.eof();
    }

    bool load_legacy_configuration(std::istream& input) {
        std::size_t dock_count = 0;
        if (!(input >> dock_count))
            return false;

        std::vector<bgui::dock::configuration> configurations(dock_count);
        for (auto& configuration : configurations) {
            std::size_t window_count = 0;
            if (!(input >> configuration.left_ratio >> configuration.right_ratio
                        >> configuration.top_ratio >> configuration.bottom_ratio >> window_count))
                return false;

            configuration.windows.reserve(window_count);
            for (std::size_t index = 0; index < window_count; ++index) {
                bgui::dock::window_configuration window;
                int area = 0;
                int floating = 0;
                if (!(input >> std::quoted(window.title) >> area >> window.weight >> floating
                            >> window.rect.x >> window.rect.y >> window.rect.z >> window.rect.w) ||
                    area < static_cast<int>(bgui::dock_area::left) ||
                    area > static_cast<int>(bgui::dock_area::center))
                    return false;
                window.area = static_cast<bgui::dock_area>(area);
                window.floating = floating != 0;
                configuration.windows.push_back(std::move(window));
            }
        }

        std::vector<bgui::dock*> docks;
        collect_docks(bgui::get_layout(), docks);
        for (std::size_t index = 0; index < std::min(docks.size(), configurations.size()); ++index)
            docks[index]->apply_configuration(configurations[index]);
        return true;
    }

    bool load_cfg_configuration(std::istream& input) {
        configuration_sections sections;
        if (!parse_configuration_sections(input, sections))
            return false;

        std::string format;
        int version = 0;
        std::size_t dock_count = 0;
        if (!read_configuration_string(sections, "interface", "format", format) ||
            format != "cpp-bgui-ui" ||
            !read_configuration_value(sections, "interface", "version", version) ||
            version != 1 ||
            !read_configuration_value(sections, "interface", "docks", dock_count))
            return false;

        std::vector<bgui::dock::configuration> configurations(dock_count);
        for (std::size_t dock_index = 0; dock_index < dock_count; ++dock_index) {
            const auto section = "dock." + std::to_string(dock_index);
            auto& configuration = configurations[dock_index];
            std::size_t window_count = 0;
            if (!read_configuration_value(sections, section, "left_ratio", configuration.left_ratio) ||
                !read_configuration_value(sections, section, "right_ratio", configuration.right_ratio) ||
                !read_configuration_value(sections, section, "top_ratio", configuration.top_ratio) ||
                !read_configuration_value(sections, section, "bottom_ratio", configuration.bottom_ratio) ||
                !read_configuration_value(sections, section, "windows", window_count))
                return false;

            configuration.windows.reserve(window_count);
            for (std::size_t window_index = 0; window_index < window_count; ++window_index) {
                const auto window_section = section + ".window." + std::to_string(window_index);
                bgui::dock::window_configuration window;
                int area = 0;
                std::string floating;
                if (!read_configuration_string(sections, window_section, "title", window.title) ||
                    !read_configuration_value(sections, window_section, "area", area) ||
                    area < static_cast<int>(bgui::dock_area::left) ||
                    area > static_cast<int>(bgui::dock_area::center) ||
                    !read_configuration_value(sections, window_section, "weight", window.weight) ||
                    !read_configuration_value(sections, window_section, "floating", floating) ||
                    (floating != "true" && floating != "false") ||
                    !read_configuration_value(sections, window_section, "rect_x", window.rect.x) ||
                    !read_configuration_value(sections, window_section, "rect_y", window.rect.y) ||
                    !read_configuration_value(sections, window_section, "rect_width", window.rect.z) ||
                    !read_configuration_value(sections, window_section, "rect_height", window.rect.w))
                    return false;
                window.area = static_cast<bgui::dock_area>(area);
                window.floating = floating == "true";
                configuration.windows.push_back(std::move(window));
            }
        }

        std::vector<bgui::dock*> docks;
        collect_docks(bgui::get_layout(), docks);
        for (std::size_t index = 0; index < std::min(docks.size(), configurations.size()); ++index)
            docks[index]->apply_configuration(configurations[index]);
        return true;
    }

    void collect_docks(bgui::layout& current, std::vector<bgui::dock*>& docks) {
        if (auto* dock = dynamic_cast<bgui::dock*>(&current))
            docks.push_back(dock);

        for (auto& [layer, elements] : current.get_elements()) {
            for (auto& element : elements) {
                if (auto* child = element->as_layout())
                    collect_docks(*child, docks);
            }
        }
    }
}

bool bgui::load_configuration(const std::string& path) {
    if (!init_trigger)
        return false;

    std::ifstream input(path);
    std::string header;
    std::size_t dock_count = 0;
    if (!input || !std::getline(input, header) || header != configuration_header ||
        !(input >> dock_count))
        return false;

    std::vector<dock::configuration> configurations(dock_count);
    for (auto& configuration : configurations) {
        std::size_t window_count = 0;
        if (!(input >> configuration.left_ratio >> configuration.right_ratio
                    >> configuration.top_ratio >> configuration.bottom_ratio >> window_count))
            return false;

        configuration.windows.reserve(window_count);
        for (std::size_t index = 0; index < window_count; ++index) {
            dock::window_configuration window;
            int area = 0;
            int floating = 0;
            if (!(input >> std::quoted(window.title) >> area >> window.weight >> floating
                        >> window.rect.x >> window.rect.y >> window.rect.z >> window.rect.w) ||
                area < static_cast<int>(dock_area::left) ||
                area > static_cast<int>(dock_area::center))
                return false;
            window.area = static_cast<dock_area>(area);
            window.floating = floating != 0;
            configuration.windows.push_back(std::move(window));
        }
    }

    std::vector<dock*> docks;
    collect_docks(get_layout(), docks);
    for (std::size_t index = 0; index < std::min(docks.size(), configurations.size()); ++index)
        docks[index]->apply_configuration(configurations[index]);
    return true;
}

bool bgui::save_configuration(const std::string& path) {
    if (!init_trigger)
        return false;

    std::vector<dock*> docks;
    collect_docks(get_layout(), docks);
    std::ofstream output(path, std::ios::trunc);
    if (!output)
        return false;

    output << configuration_header << '\n' << docks.size() << '\n'
           << std::setprecision(9);
    for (auto* dock : docks) {
        const auto configuration = dock->get_configuration();
        output << configuration.left_ratio << ' ' << configuration.right_ratio << ' '
               << configuration.top_ratio << ' ' << configuration.bottom_ratio << ' '
               << configuration.windows.size() << '\n';
        for (const auto& window : configuration.windows) {
            output << std::quoted(window.title) << ' ' << static_cast<int>(window.area) << ' '
                   << window.weight << ' ' << static_cast<int>(window.floating) << ' '
                   << window.rect.x << ' ' << window.rect.y << ' '
                   << window.rect.z << ' ' << window.rect.w << '\n';
        }
    }
    return static_cast<bool>(output);
}

static void shutdown_interface() noexcept {
    init_trigger = false;
    s_keyboard_focused = nullptr;
    s_mouse_target = nullptr;
    bgui::s_main_layout.reset();
    s_draw_data.reset();
    std::queue<std::function<void()>> empty;
    s_functions.swap(empty);
}

static void set_keyboard_focus(bgui::element* element) {
    if (s_keyboard_focused == element) {
        if (auto* input = dynamic_cast<bgui::input_area*>(element)) {
            input->set_focused(true);
        }
        return;
    }

    if (auto* input = dynamic_cast<bgui::input_area*>(s_keyboard_focused)) {
        input->set_focused(false);
    } else if (s_keyboard_focused) {
        s_keyboard_focused->set_style_state(bgui::state::normal);
    }

    s_keyboard_focused = element;
    if (auto* input = dynamic_cast<bgui::input_area*>(element)) {
        input->set_focused(true);
    } else if (element) {
        element->set_style_state(bgui::state::focused);
    }
}

bgui::layout& bgui::get_layout() {
    if(!init_trigger) throw std::runtime_error("[BGUI] You must initialize the library.");
    return *s_main_layout;
}

void bgui::set_up() {
    //std::cout << "[BGUI] Setting-up the library.\n";
    init_trigger = true;
    // Create the default layout
    if(!s_main_layout)
        set_layout<bgui::layout>();
    // Create draw data structure
    if(!s_draw_data)
        s_draw_data = std::make_unique<bgui::draw_data>();
#ifdef BGUI_USE_FREETYPE
    bgui::set_up_freetype();
    bgui::load_font_queue();
#endif
    // Set up default theme
    auto& sm = style_manager::get_instance();
}

bgui::scoped_interface::scoped_interface() {
    if (init_trigger)
        throw std::runtime_error("[BGUI] The library is already initialized.");
    try {
        bgui::set_up();
    } catch (...) {
        shutdown_interface();
        throw;
    }
}

bgui::scoped_interface::~scoped_interface() noexcept {
    if (init_trigger)
        shutdown_interface();
}

void bgui::cascade_style() {
    if(!init_trigger)
        throw std::runtime_error("[BGUI] You must initialize the library.");

    auto& sm = style_manager::get_instance();

    bgui::s_main_layout->style.layout.require_height(bgui::mode::match_parent);
    bgui::s_main_layout->style.layout.require_width(bgui::mode::match_parent);
    s_main_layout->cascade_style();
}

bgui::draw_data* bgui::get_draw_data() {
    if(!init_trigger) throw std::runtime_error("[BGUI] You must initialize the library.");
    return s_draw_data.get();
}

bgui::draw_list& bgui::get_draw_list() {
    return get_draw_data()->m_draw_list;
}
bool bgui::shutdown_lib() {
    if(!init_trigger) throw std::runtime_error("[BGUI] You must initialize the library.");
    shutdown_interface();
    return true;
}

static void clear_stale_mouse_hover(bgui::layout& lay, bgui::element* target) {
    for (auto& [layer, elements] : lay.get_elements()) {
        for (auto& owned_element : elements) {
            auto* element = owned_element.get();
            if (element != target && element->get_style_state() == bgui::state::hover)
                element->on_mouse_leave();

            if (auto* child_layout = element->as_layout())
                clear_stale_mouse_hover(*child_layout, target);
        }
    }
}

bool update_inputs(bgui::layout &lay){
    // global element for last capture
    static bgui::element* g_mouse_captured = nullptr;

    auto m = bgui::get_mouse_position();
    float mx = m[0];
    float my = m[1];

    bool mouse_now = bgui::get_pressed(bgui::input_key::mouse_left);
    bool mouse_click = (mouse_now && !bgui::get_context().m_last_mouse_left);
    bool mouse_released = (!mouse_now && bgui::get_context().m_last_mouse_left);

    if (g_mouse_captured) {
        s_mouse_target = g_mouse_captured;
        if (mouse_released) {
            g_mouse_captured->on_released();
            g_mouse_captured = nullptr;
        } else if (mouse_now) {
            g_mouse_captured->set_drag(m - bgui::get_context().m_last_mouse_pos);
        }
        return true;
    }

    if (bgui::get_pressed(bgui::input_key::escape) && s_keyboard_focused) {
        set_keyboard_focus(nullptr);
    }

    if (mouse_click) {
        if (auto* window = dynamic_cast<bgui::window*>(&lay)) {
            const auto rect = window->processed_rect();
            const bool inside = mx >= rect.x && mx <= rect.x + rect.z &&
                my >= rect.y && my <= rect.y + rect.w;
            if (inside) {
                if (auto* parent_dock = dynamic_cast<bgui::dock*>(window->get_parent()))
                    parent_dock->focus_window(window);
            }
        }
    }
    
    for(size_t i = lay.get_elements().size(); i-- > 0; ) {
        // iterate through elements in reverse order to prioritize topmost elements
        auto& elements = lay.get_elements()[static_cast<bgui::layer>(i)];
        for (size_t i = elements.size(); i-- > 0; ) {
            auto elem = elements[i].get();

            if (!elem->is_enabled())
                continue;

            if (auto* cast = elem->as_layout())
                if(update_inputs(*cast)) {
                    return true;
                }
            
            // inside test
            float x = elem->processed_x();
            float y = elem->processed_y();
            float w = elem->processed_width();
            float h = elem->processed_height();

            bool inside =
                mx >= x &&
                mx <= x + w &&
                my >= y &&
                my <= y + h;

                if (inside) {
                if(!elem->recives_input()) {
                    // A non-interactive child bubbles the hit to its
                    // interactive parent, but a front-most sibling blocks
                    // elements behind it from receiving the click.
                    if (elem->get_parent() && elem->get_parent()->recives_input()) {
                        return false;
                    }
                    if (mouse_click && s_keyboard_focused) {
                        set_keyboard_focus(nullptr);
                    }
                    return true;
                }
                s_mouse_target = elem;
                elem->on_mouse_hover();
                if (mouse_click) {
                    g_mouse_captured = elem; // start capture
                    if (elem->type == "inputarea") {
                        set_keyboard_focus(elem);
                    } else {
                        set_keyboard_focus(nullptr);
                    }
                    elem->on_clicked();
                    elem->on_pressed();
                }
                if(mouse_released) {
                    elem->on_released();
                    if (g_mouse_captured == elem)
                        g_mouse_captured = nullptr; // release capture when mouse release
                }
                return true;
            }
        }
    }
    if (mouse_click && s_keyboard_focused) {
        set_keyboard_focus(nullptr);
    }
    return false;
}
// Updates the main layout
void bgui::on_update() {
    if(!init_trigger) throw std::runtime_error("[BGUI] You must initialize the library.");

    bgui::vec2i w_size = bgui::get_context_size();

    // the main layout must to be resized based on the window size by default.
    while(!s_functions.empty()) {
        auto& f = s_functions.front();
        f();
        s_functions.pop();
    }

    // cascade style
    cascade_style();

    // Resolve mouse focus before updating keyboard-driven elements.
    bgui::s_main_layout->process_required_size(w_size);
    get_context().m_actual_cursor = cursor::arrow;
    s_mouse_target = nullptr;
    update_inputs(*bgui::s_main_layout);
    clear_stale_mouse_hover(*bgui::s_main_layout, s_mouse_target);
    bgui::s_main_layout->on_update();

    if (!s_keyboard_focused) {
        bgui::get_context().m_char_buffer.clear();
    }

    bgui::get_context().m_last_mouse_left = bgui::get_pressed(bgui::input_key::mouse_left);
    bgui::get_context().m_last_mouse_pos = bgui::get_mouse_position();

    // get new requires
    if(!get_draw_data()->m_quad_requires.empty()) std::cout << "[BGUI] Warning: draw data not empty at beginning of frame.\nMake sure you are resetting draw data each frame.\n";
    get_draw_data()->m_clip_rect = {0, 0, w_size.x, w_size.y};
    bgui::s_main_layout->get_requires(get_draw_data());
}

bgui::element* bgui::get_mouse_target() {
    return s_mouse_target;
}

void bgui::add_function(const std::function<void()>& f) {
    if(!init_trigger) throw std::runtime_error("[BGUI] You must initialize the library.");
    s_functions.push(f);
}

void bgui::set_global_scale(float scale) {
    if (scale <= 0.f) {
        throw std::invalid_argument("[BGUI] Global scale must be greater than zero.");
    }
    s_global_scale = scale;
    if (s_main_layout) {
        s_main_layout->mark_style_dirty();
    }
}

float bgui::get_global_scale() {
    return s_global_scale;
}
