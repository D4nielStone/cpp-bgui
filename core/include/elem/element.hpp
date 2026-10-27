#pragma once
#include <vector>
#include <memory>
#include <limits.h>

#include "lay/layer.hpp"
#include "utils/style.hpp"
#include "utils/draw.hpp"
#include "utils/border.hpp"

/**
 * @namespace bgui
 * @brief Base namespace for the BGUI (Basic GUI) framework components.
 */
namespace bgui {
    class element;
    class layout;
    void cancel_interactions(element* subtree) noexcept;

    /**
     * @brief The small part of the UI.
     * * The element class is the fundamental building block for all UI components.
     * It handles style, size requires, layout results, and basic input events.
     */
    class element {
    private:
        bool m_style_dirty  = true;
        bool m_layout_dirty = true;
        bool m_render_dirty = true;
        bool m_render_cache_valid = false;
        vec4i m_render_cache_clip{0, 0, 0, 0};
        bool m_render_cache_draw_clip_enabled = false;
        vec4i m_render_cache_draw_clip{0, 0, 0, 0};
        vec4 m_render_cache_input_color{1.f, 1.f, 1.f, 1.f};
        vec4 m_render_cache_output_color{1.f, 1.f, 1.f, 1.f};
        std::vector<draw_require> m_cached_draw_requires;
        std::vector<draw_vertex> m_cached_draw_vertices;
    protected:
        // Boolean that false means that the element is disabled.
        bool m_enabled {true};
        // Pointer to the parent layout in the UI hierarchy. Not directly style, but crucial for layout calculation.
        layout* m_parent {nullptr};
        
        //TODO: remove material or change its responsabilities
        // The graphical material (shader, colors, textures) used for rendering the element.
        material m_material;
        // Flag indicating if the element should process mouse/keyboard input. Not strictly style, but related to interaction.
        bool m_receives_input{true};
        
        vec2i m_last_drag{0, 0}, m_content_size{0, 0};

        // FINAL COMPUTED RECT (layout writes this)
        // x, y, width, height - The final position and dimensions calculated by the layout.
        vec4i m_rect {0, 0, 0, 0};
        vec4i m_arranged_rect {0, 0, 0, 0};
        state m_state{state::normal};
        bool m_flex = true;
    public:
        static void begin_geometry_frame();
        static void begin_arranging();
        static void end_arranging();
        static void end_geometry_frame();

        void set_flex(bool f) {
            if (m_flex == f)
                return;
            m_flex = f;
            mark_layout_dirty();
        }
        bool is_flex() const {return m_flex;}
        void mark_style_dirty();
        void mark_layout_dirty();
        // Call after changing custom rendering data that is not tracked by BGUI setters.
        void mark_render_dirty();
        bool is_layout_dirty() const { return m_layout_dirty; }
        void clear_layout_dirty() { m_layout_dirty = false; }
        virtual bool supports_render_cache() const { return true; }
        // The input state of the element
        std::string type;                 // "button", "text", "linear"
        std::vector<std::string> classes; // {"primary", "rounded"}
        std::string id;                   // ex: "submitBtn"

        bgui::style style;                // inline style
        bgui::computed_style computed_style;       // final style

        state get_style_state() const {
            return m_state;
        }
        void set_style_state(const state s) {
            if (m_state == s)
                return;
            m_state = s;
            mark_style_dirty();
        }

        bool is_style_dirty() const { return m_style_dirty; }
        void clear_style_dirty()    { m_style_dirty = false; }
        /**
         * @brief Default constructor.
         */
        element() : type("element"), style() {}

        /// @brief Tag Constructor
        /// Creates a UID based on a string type (tag).
        /**
         * @brief Constructor that initializes the element with a tag.
         * @param tag A string identifier for the element's type.
         */

        /**
         * @brief Virtual destructor.
         */
        virtual ~element();

        void add_class(const std::string& cls);
        void remove_class(const std::string& cls);
        bool has_class(const std::string& cls) const;
        void clear_classes();
        void compute_style();
        void set_properties();

        void set_enable(bool);
        bool is_enabled() const {
            return m_enabled;
        }
        
        /**
         * @brief Returns the material of the element
         */
        material& get_material() {
            return m_material;
        };

        /**
         * @brief Calculates the size based on the available space and required size.
         * This is a layout-related method.
         * @param available_size The size provided by the parent layout.
         */
        void process_required_size(const vec2i& available_size);
        /**
         * @brief Sets the parent layout of this element.
         * @param p Pointer to the parent layout.
         */
        void set_parent(layout* p) { m_parent = p; }

        /**
         * @brief Gets the parent layout of this element.
         * @return Pointer to the parent layout, or nullptr if none.
         */
        layout* get_parent() const { return m_parent; }

        /**
         * @brief Attempts to cast the element to a layout type.
         * * Used for type checking in the hierarchy traversal.
         * @return Pointer to layout if the element is a layout, otherwise nullptr.
         */
        virtual layout* as_layout() { return nullptr; }
        virtual bool clips_children() const { return false; }
        virtual vec4i get_children_clip_rect() const { return processed_rect(); }
        // Final rect (computed by layout)
        /**
         * @brief Gets the final computed X position (left edge) of the element.
         * @return The X coordinate in pixels.
         */
        int processed_x()     const;
        
        /**
         * @brief Gets the final computed Y position (top edge) of the element.
         * @return The Y coordinate in pixels.
         */
        int processed_y()     const;

        
        /**
         * @brief Gets the final computed width of the element.
         * @return The width in pixels.
         */
        int processed_width() const;
        
        /**
         * @brief Gets the final computed height of the element.
         * @return The height in pixels.
         */
        int processed_height()const;

        /**
         * @brief Sets the final computed position and size of the element.
         * * This method is typically called by the parent layout.
         * @param x The final X position.
         * @param y The final Y position.
         * @param w The final width.
         * @param h The final height.
         */
        void set_final_rect(int x, int y, int w, int h);
        
        /**
         * @brief Sets the final computed position (X and Y).
         * @param x The final X position.
         * @param y The final Y position.
         */
        void set_position(int x, int y);
        
        /**
         * @brief Sets the final computed size (width and height).
         * @param x The final width.
         * @param y The final height.
         */
        void set_final_size(int x, int y);

        /**
         * @brief Gets the final computed position as a 2D vector.
         * @return A vec2i containing the X and Y position.
         */
        vec2i processed_position() const;
        
        /**
         * @brief Gets the final computed size as a 2D vector.
         * @return A vec2i containing the width and height.
         */
        vec2i processed_size()     const;
        
        /**
         * @brief Gets the final computed rectangle (X, Y, W, H).
         * @return A vec4i containing the processed rect.
         */
        vec4i processed_rect()     const;
        // input
        /**
         * @brief Enables or disables input reception for the element.
         * @param b True to enable input, false to disable.
         */
        void receives_input(bool b) {
            m_receives_input = b;
        };

        /**
         * @brief Checks if the element is currently set to receive input.
         * @return True if input is enabled, false otherwise.
         */
        bool receives_input() const {
            return m_receives_input;
        }

        /**
         * @brief Checks if the element is currently being dragged.
         * * This likely returns the drag delta if a drag is active.
         * @return A vec2i representing the drag vector, or (0, 0) if not dragging.
         */
        vec2i is_drag() const;
        
        // Update methods
        /**
         * @brief Pure virtual update method called every frame or tick.
         * * Derived classes should override this for continuous logic.
         */
        virtual void on_update();
        virtual void arrange();
        virtual void prepare_geometry_frame();
        virtual void commit_geometry();

        /**
         * @brief Calculates the content width (excluding padding, margin, border).
         * @return The content width as a float.
         */
        float content_width(){return get_content_size().x;};

        /**
         * @brief Calculates the content height (excluding padding, margin, border).
         * @return The content height as a float.
         */
        float content_height(){return get_content_size().y;};

        /**
         * @brief Callback invoked when the element is initially pressed (mouse down).
         */
        virtual void on_pressed() {
            set_style_state(state::pressed);
        };

        /**
         * @brief Callback invoked when the mouse is moved while pressed over the element.
         * @param mouse_delta The change in mouse position since the last frame.
         */
        virtual void set_drag(const vec2i& mouse_delta) {m_last_drag = mouse_delta;};

        /**
         * @brief Callback invoked when the element is clicked (pressed and released).
         */
        virtual void on_clicked() {
            set_style_state(state::pressed);
        };

        /**
         * @brief Callback invoked when the press is released (mouse up).
         */
        virtual void on_released() {
            set_style_state(state::normal);
        };

        /**
         * @brief Callback invoked when the mouse cursor hovers over the element.
         */
        virtual void on_mouse_hover() {
            set_style_state(state::hover);
        };

        /**
         * @brief Callback invoked when the mouse cursor leaves the element.
         */
        virtual void on_mouse_leave() {
            if (m_state == state::hover)
                set_style_state(state::normal);
        };

        virtual void calc_content_size(const layer& lay) {};

        /**
         * @brief Gets the minimum required size for the element's content.
         * @return A vec2i representing the content size. Defaults to get_min_size().
         */
        vec2i get_content_size() const {
            return m_content_size; 
        }
        void set_content_size(const vec2i& content) {
            if (m_content_size == content)
                return;
            m_content_size = content;
            mark_layout_dirty();
        };

        /**
         * @brief Collects draw calls required to render this element.
         * @param calls A pointer to the structure where draw data is collected.
         */
        virtual void get_requires(draw_data* calls);
        // Reuses cached draw data until rendering inputs, geometry, or clipping change.
        void get_cached_requires(draw_data* calls);
    };

} // namespace bgui