#include "SliderFragment.h"

SliderFragment::SliderFragment(uint32_t id)
    : Fragment(id), 
      m_slider(0, 0, 40, 80),
      m_isDragging(false),
      m_valueChangeHandler(nullptr)
{
}

bool SliderFragment::onCreate() {
    if (!Fragment::onCreate()) {
        return false;
    }
    
    // Update slider position and size
    m_slider = Slider(getX(), getY(), getWidth(), getHeight());
    
    return true;
}

void SliderFragment::onDestroy() {
    Fragment::onDestroy();
}

bool SliderFragment::handleEvent(const framework::Event& event) {
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
        
        // Check if slider contains the touch point
        bool containsPoint = (touchX >= getX() && touchX < getX() + getWidth() &&
                              touchY >= getY() && touchY < getY() + getHeight());
        
        // デバッグ用の詳細情報を表示
        Serial.println("SliderFragment::handleEvent - id: " + String(getId()) + 
                       ", x: " + String(touchX) + 
                       ", y: " + String(touchY) + 
                       ", containsPoint: " + String(containsPoint ? "true" : "false") + 
                       ", action: " + String((int)touchEvent.getAction()));
        
        if (touchEvent.getAction() == framework::TouchAction::DOWN && containsPoint) {
            // Touch down - start dragging
            Serial.println("SliderFragment: Touch DOWN received in bounds");
            m_isDragging = true;
            bool valueChanged = m_slider.handleTouch(touchX, touchY, true);
            
            // If value changed, send slider event and call value change handler
            if (valueChanged) {
                int value = m_slider.getValue();
                Serial.println("SliderFragment: Value changed to " + String(value));
                
                // Call value change handler if set
                if (m_valueChangeHandler) {
                    m_valueChangeHandler(value);
                }
            }
            
            draw();
            return true;
        }
        else if (touchEvent.getAction() == framework::TouchAction::MOVE && m_isDragging) {
            // Touch move - continue dragging
            Serial.println("SliderFragment: Touch MOVE received while dragging");
            bool valueChanged = m_slider.handleTouch(touchX, touchY, true);
            
            // If value changed, send slider event and call value change handler
            if (valueChanged) {
                int value = m_slider.getValue();
                Serial.println("SliderFragment: Value changed to " + String(value));
                
                // Call value change handler if set
                if (m_valueChangeHandler) {
                    m_valueChangeHandler(value);
                }
            }
            
            draw();
            return true;
        }
        else if (touchEvent.getAction() == framework::TouchAction::UP && m_isDragging) {
            // Touch up - end dragging
            Serial.println("SliderFragment: Touch UP received while dragging");
            m_isDragging = false;
            m_slider.handleTouch(touchX, touchY, false);
            draw();
            return true;
        }
    }
    
    return false;
}

void SliderFragment::setTitle(const String& title) {
    m_slider.setTitle(title);
    draw();
}

void SliderFragment::setValue(int value) {
    m_slider.setValue(value);
    draw();
}

int SliderFragment::getValue() const {
    // 直接メンバをアクセスして問題を回避
    return m_slider.value;
}

void SliderFragment::setValueChangeHandler(std::function<void(int)> handler) {
    m_valueChangeHandler = handler;
}

void SliderFragment::draw() {
    // Draw the slider
    m_slider.draw();
}