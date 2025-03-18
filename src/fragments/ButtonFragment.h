#ifndef BUTTON_FRAGMENT_H
#define BUTTON_FRAGMENT_H

#include "../framework/Fragment.h"
#include "../ui/components/Button.h"
#include "../framework/EventBus.h"
#include <functional>

/**
 * @brief Button fragment class
 * 
 * ButtonFragment is a reusable UI component that wraps a Button and provides
 * Fragment lifecycle and event handling capabilities.
 */
class ButtonFragment : public framework::Fragment {
public:
    /**
     * @brief Constructor with optional ID
     * @param id Optional fragment ID, auto-generated if not provided
     */
    ButtonFragment(uint32_t id = 0);
    
    /**
     * @brief Virtual destructor
     */
    virtual ~ButtonFragment() = default;
    
    /**
     * @brief Lifecycle method called when fragment is created
     * @return true if creation was successful, false otherwise
     */
    virtual bool onCreate() override;
    
    /**
     * @brief Lifecycle method called when fragment is about to be destroyed
     */
    virtual void onDestroy() override;
    
    /**
     * @brief Handle an event
     * @param event The event to handle
     * @return true if the event was handled, false otherwise
     */
    virtual bool handleEvent(const framework::Event& event) override;
    
    /**
     * @brief Set the button's label
     * @param label The new label to set
     */
    void setLabel(const char* label);
    
    /**
     * @brief Set the button's colors
     * @param normalColor Color when not pressed
     * @param pressedColor Color when pressed
     */
    void setColor(uint16_t normalColor, uint16_t pressedColor);
    
    /**
     * @brief Set the button's font size
     * @param size The new font size to set
     */
    void setFontSize(uint8_t size);
    
    /**
     * @brief Set the button's type
     * @param type The new type to set (BUTTON_TYPE_OUTLINE or BUTTON_TYPE_TEXT)
     */
    void setType(uint8_t type);
    
    /**
     * @brief Set a click handler for this button
     * @param handler The click handler function
     */
    void setClickHandler(std::function<void()> handler);
    
    /**
     * @brief Draw the button
     */
    void draw();
    
    /**
     * @brief Set the fragment's display area
     * @param x X-coordinate
     * @param y Y-coordinate
     * @param width Width
     * @param height Height
     */
    void setDisplayArea(int x, int y, int width, int height);
    
private:
    Button m_button;                      // Internal button instance
    bool m_pressed;                       // Button pressed state
    std::function<void()> m_clickHandler; // Click handler
    String m_label;                       // Button label
    uint16_t m_normalColor;               // Normal color
    uint16_t m_pressedColor;              // Pressed color
    uint8_t m_fontSize;                   // Font size
    uint8_t m_type;                       // Button type
    
    /**
     * @brief Update the internal button's geometry based on fragment's position and size
     */
    void updateButtonGeometry();
};

#endif // BUTTON_FRAGMENT_H
