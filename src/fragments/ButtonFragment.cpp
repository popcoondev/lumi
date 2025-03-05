#include "ButtonFragment.h"

ButtonFragment::ButtonFragment(uint32_t id)
    : Fragment(id), 
      m_button(0, 0, 0, 0, "Button"),
      m_pressed(false),
      m_clickHandler(nullptr)
{
}

bool ButtonFragment::onCreate() {
    if (!Fragment::onCreate()) {
        return false;
    }
    
    // Update button position and size
    m_button = Button(getX(), getY(), getWidth(), getHeight(), "Button");
    
    return true;
}

void ButtonFragment::onDestroy() {
    Fragment::onDestroy();
}

bool ButtonFragment::handleEvent(const framework::Event& event) {
    // Check if base class handles the event
    if (Fragment::handleEvent(event)) {
        return true;
    }
    
    // Only process events if the fragment is visible and enabled
    if (!isVisible() || !isEnabled()) {
        return false;
    }
    
    // Handle touch events
    if (event.getType() == framework::EventType::TOUCH) {
        const framework::TouchEvent& touchEvent = static_cast<const framework::TouchEvent&>(event);
        
        int touchX = touchEvent.getX();
        int touchY = touchEvent.getY();
        
        // Check if touch is within button area
        if (touchX >= getX() && touchX < getX() + getWidth() &&
            touchY >= getY() && touchY < getY() + getHeight()) {
            
            if (touchEvent.getAction() == framework::TouchAction::DOWN) {
                // Touch down - set pressed state
                m_pressed = true;
                m_button.setPressed(true);
                draw();
                return true;
            }
            else if (touchEvent.getAction() == framework::TouchAction::UP) {
                // Touch up within button area - trigger click
                if (m_pressed) {
                    m_pressed = false;
                    m_button.setPressed(false);
                    draw();
                    
                    // Send button click event
                    framework::ButtonEvent buttonEvent(framework::ButtonAction::CLICK, getId());
                    framework::EventBus::getInstance().postEvent(buttonEvent);
                    
                    // Call click handler if set
                    if (m_clickHandler) {
                        m_clickHandler();
                    }
                    
                    return true;
                }
            }
        }
        else if (touchEvent.getAction() == framework::TouchAction::UP && m_pressed) {
            // Touch up outside button area - reset state without triggering click
            m_pressed = false;
            m_button.setPressed(false);
            draw();
            return true;
        }
    }
    
    return false;
}

void ButtonFragment::setLabel(const char* label) {
    m_button.setLabel(label);
    draw();
}

void ButtonFragment::setColor(uint16_t normalColor, uint16_t pressedColor) {
    m_button.setColor(normalColor, pressedColor);
    draw();
}

void ButtonFragment::setFontSize(uint8_t size) {
    m_button.setFontSize(size);
    draw();
}

void ButtonFragment::setType(uint8_t type) {
    m_button.setType(type);
    draw();
}

void ButtonFragment::setClickHandler(std::function<void()> handler) {
    m_clickHandler = handler;
}

void ButtonFragment::draw() {
    // Draw the button
    m_button.draw();
}
