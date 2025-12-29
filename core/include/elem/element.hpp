#pragma once
#include <vector>
#include <memory>
#include <limits.h>

#include "utils/style.hpp"
#include "utils/draw.hpp"
#include "utils/uid.hpp"
#include "utils/border.hpp"

/**
 * @namespace bgui
 * @brief Base namespace for the BGUI (Basic GUI) framework components.
 */
namespace bgui {
    class layout;

    /**
     * @brief The small part of the UI.
     * * The element class is the fundamental building block for all UI components.
     * It handles style, size requests, layout results, and basic input events.
     */
    class element : public uid {
    protected:
        // Pointer to the parent layout in the UI hierarchy. Not directly style, but crucial for layout calculation.
        layout* m_parent {nullptr};
        // The graphical material (shader, colors, textures) used for rendering the element.
        material m_material;
        // Visibility state of the element.
        bool m_visible {true};
        // Flag indicating if the element should process mouse/keyboard input. Not strictly style, but related to interaction.
        bool m_recives_input{true};
        
        // Requested size measurement modes (pixel or relative).
        vec<2, mode> m_requested_mode {mode::pixel, mode::pixel};
        // The actual requested size values (can be pixels or percentage).
        vec2  m_requested_size {0.f, 0.f};

        // Internal space: left, top, right, bottom padding.
        vec4i m_padding {0,0,0,0}; 
        // External space: left, top, right, bottom margin.
        vec4i m_margin  {0,0,0,0}; 
        // Border properties (size, color, radius).
        border m_border;
        // Stored style object, usually inherited or default (colors, fonts, etc.).
        style m_style;

        // Minimum allowed size constraint (width, height).
        vec2i m_min_size {10, 10};
        // Maximum allowed size constraint (width, height).
        vec2i m_max_size {INT_MAX, INT_MAX};
        // Stores the last mouse delta during a drag operation. Not style, but input/interaction.
        vec2i m_last_drag{0, 0};

        // FINAL COMPUTED RECT (layout writes this)
        // x, y, width, height - The final position and dimensions calculated by the layout.
        vec4i m_rect {0, 0, 0, 0};
    public:
        /**
         * @brief Default constructor.
         */
        element() = default;

        /// @brief Tag Constructor
        /// Creates a UID based on a string type (tag).
        /**
         * @brief Constructor that initializes the element with a tag.
         * @param tag A string identifier for the element's type.
         */
        element(const std::string& tag) : uid(tag){};

        /**
         * @brief Virtual destructor.
         */
        virtual ~element() = default;

        // -- STYLE --
        /**
         * @brief Sets the margin (external spacing) around the element.
         * @param left Margin on the left side.
         * @param top Margin on the top side.
         * @param right Margin on the right side.
         * @param bottom Margin on the bottom side.
         */
        void set_margin(int left, int top, int right, int bottom);

        /**
         * @brief Sets uniform horizontal and vertical margins.
         * @param horizontal Margin for left and right sides.
         * @param vertical Margin for top and bottom sides.
         */
        void set_margin(int horizontal, int vertical);

        /**
         * @brief Gets all margins from the element
         */
        vec4i get_margin() const {
            return m_margin;
        };

        // Padding
        /**
         * @brief Sets the padding (internal spacing) inside the element.
         * @param left Padding on the left side.
         * @param top Padding on the top side.
         * @param right Padding on the right side.
         * @param bottom Padding on the bottom side.
         */
        void set_padding(int left, int top, int right, int bottom);

        /**
         * @brief Sets uniform horizontal and vertical padding.
         * @param horizontal Padding for left and right sides.
         * @param vertical Padding for top and bottom sides.
         */
        void set_padding(int horizontal, int vertical);

        /**
         * @brief Gets all the 4 paddings from the element.
         */
        vec4i get_padding() const {
            return m_padding;
        }
        // Borders
        /**
         * @brief Sets the border size (thickness).
         * @param x Horizontal border size (left/right).
         * @param y Vertical border size (top/bottom).
         */
        void set_border_size(int x, int y);

        /**
         * @brief Sets the color of the border.
         * @param c The color object.
         */
        void set_border_color(const color& c);

        /**
         * @brief Sets the border radius for rounded corners.
         * @param radius The radius value.
         */
        void set_border_radius(float radius);

        // Requested size
        /**
         * @brief Requests a specific size for the element using the currently set modes.
         * @param width The requested width value.
         * @param height The requested height value.
         */
        void request_size(float width, float height);
        
        /**
         * @brief Requests a specific size mode for the element using the currently set sizes.
         * @param width The requested width mode.
         * @param height The requested height mode.
         */
        void request_mode(mode width, mode height);

        /**
         * @brief Requests a specific width with a specified measurement mode.
         * @param m The measurement mode (e.g., pixel, percentage).
         * @param value The width value. Defaults to 100.f.
         */
        void request_width(mode m, float value = 100.f);

        /**
         * @brief Requests a specific height with a specified measurement mode.
         * @param m The measurement mode (e.g., pixel, percentage).
         * @param value The height value. Defaults to 100.f.
         */
        void request_height(mode m, float value = 100.f);

        // Material / visibility
        /**
         * @brief Sets the shader tag to be used by the element's material.
         * @param shd The string tag of the shader.
         */
        void set_shader_tag(const std::string& shd);

        /**
         * @brief Sets the entire material object for rendering.
         * @param m The material object.
         */
        void set_material(const material& m);

        /**
         * @brief Sets the visibility of the element.
         * @param v True to make the element visible, false otherwise.
         */
        void set_visible(bool v);

        /**
         * @brief Returns the material of the element
         */
        material& get_material() {
            return m_material;
        };

        // Size constraints
        /**
         * @brief Sets the minimum size constraints.
         * @param width The minimum width in pixels.
         * @param height The minimum height in pixels.
         */
        void set_min_size(int width, int height);

        /**
         * @brief Sets the maximum size constraints.
         * @param width The maximum width in pixels.
         * @param height The maximum height in pixels.
         */
        void set_max_size(int width, int height);
        // -- /STYLE --

        // Size constraints
        /**
         * @brief Gets the minimum size constraint.
         * @return A vec2i containing the minimum width and height.
         */
        vec2i get_min_size() const { return m_min_size; }
        
        /**
         * @brief Gets the maximum size constraint.
         * @return A vec2i containing the maximum width and height.
         */
        vec2i get_max_size() const { return m_max_size; }

        // Requested size
        /**
         * @brief Gets the raw requested size values (pixels or percentage).
         * @return A vec2 containing the requested width and height.
         */
        vec2 requested_size() const { return m_requested_size; }
        
        /**
         * @brief Gets the requested size measurement modes.
         * @return A vec<2,mode> containing the width and height modes.
         */
        vec<2,mode> get_requested_mode() const { return m_requested_mode; }

        // Core Style Application
        /**
         * @brief Applies a given style object to the element, updating material properties.
         * @param style The style object to apply.
         */
        virtual void apply_style(const style& style);

        /**
         * @brief Getter to enable user to edit elemnt's local style.
         * @returns A style containing informations from it's style.
         */
        style& get_style() {
            return m_style;
        }

        /**
         * @brief Calculates the size based on the available space and requested size.
         * This is a layout-related method.
         * @param available_size The size provided by the parent layout.
         */
        void update_size(const vec2i& available_size);
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
        // Final rect (computed by layout)
        /**
         * @brief Gets the final computed X position (left edge) of the element.
         * @return The X coordinate in pixels.
         */
        int processed_x()     const { return m_rect[0]; }
        
        /**
         * @brief Gets the final computed Y position (top edge) of the element.
         * @return The Y coordinate in pixels.
         */
        int processed_y()     const { return m_rect[1]; }
        
        /**
         * @brief Gets the final computed width of the element.
         * @return The width in pixels.
         */
        int processed_width() const { return m_rect[2]; }
        
        /**
         * @brief Gets the final computed height of the element.
         * @return The height in pixels.
         */
        int processed_height()const { return m_rect[3]; }

        /**
         * @brief Sets the final computed position and size of the element.
         * * This method is typically called by the parent layout.
         * @param x The final X position.
         * @param y The final Y position.
         * @param w The final width.
         * @param h The final height.
         */
        void set_final_rect(int x, int y, int w, int h) {m_rect = vec4i({x, y, w, h}); }
        
        /**
         * @brief Sets the final computed position (X and Y).
         * @param x The final X position.
         * @param y The final Y position.
         */
        void set_position(int x, int y) {m_rect[0] = x; m_rect[1] = y;}
        
        /**
         * @brief Sets the final computed size (width and height).
         * @param x The final width.
         * @param y The final height.
         */
        void set_final_size(int x, int y) {m_rect[2] = x; m_rect[3] = y;}

        /**
         * @brief Gets the final computed position as a 2D vector.
         * @return A vec2i containing the X and Y position.
         */
        vec2i processed_position() const { return vec2i({m_rect[0], m_rect[1]}); }
        
        /**
         * @brief Gets the final computed size as a 2D vector.
         * @return A vec2i containing the width and height.
         */
        vec2i processed_size()     const { return vec2i({m_rect[2], m_rect[3]}); }
        
        /**
         * @brief Gets the final computed rectangle (X, Y, W, H).
         * @return A vec4i containing the processed rect.
         */
        vec4i processed_rect()     const { return m_rect; }
        // input
        /**
         * @brief Enables or disables input reception for the element.
         * @param b True to enable input, false to disable.
         */
        void recives_input(bool b) {
            m_recives_input = b;
        };

        /**
         * @brief Checks if the element is currently set to receive input.
         * @return True if input is enabled, false otherwise.
         */
        bool recives_input() const {
            return m_recives_input;
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

        /**
         * @brief Calculates the content width (excluding padding, margin, border).
         * @return The content width as a float.
         */
        virtual float content_width(){return 0.f;};

        /**
         * @brief Calculates the content height (excluding padding, margin, border).
         * @return The content height as a float.
         */
        virtual float content_height(){return 0.f;};

        /**
         * @brief Callback invoked when the element is initially pressed (mouse down).
         */
        virtual void on_pressed() {};

        /**
         * @brief Callback invoked when the mouse is moved while pressed over the element.
         * @param mouse_delta The change in mouse position since the last frame.
         */
        virtual void on_drag(const vec2i& mouse_delta) {m_last_drag = mouse_delta;};

        /**
         * @brief Callback invoked when the element is clicked (pressed and released).
         */
        virtual void on_clicked() {};

        /**
         * @brief Callback invoked when the press is released (mouse up).
         */
        virtual void on_released() {};

        /**
         * @brief Callback invoked when the mouse cursor hovers over the element.
         */
        virtual void on_mouse_hover() {};

        /**
         * @brief Gets the minimum required size for the element's content.
         * @return A vec2i representing the content size. Defaults to get_min_size().
         */
        virtual vec2i get_content_size() {return get_min_size(); }

        /**
         * @brief Collects draw calls required to render this element.
         * @param calls A pointer to the structure where draw data is collected.
         */
        virtual void get_requests(draw_data* calls);
    };

} // namespace bgui