#ifndef SLIDER_FRAGMENT_H
#define SLIDER_FRAGMENT_H

#include "../framework/Fragment.h"
#include "../framework/EventBus.h"
#include "../ui/views/LumiView.h" // 依存関係に必要
#include <functional>

/**
 * @brief Slider fragment class
 * 
 * SliderFragment is a reusable UI component that wraps a Slider and provides
 * Fragment lifecycle and event handling capabilities.
 */
class SliderFragment : public framework::Fragment {
public:
    /**
     * @brief Constructor with optional ID
     * @param id Optional fragment ID, auto-generated if not provided
     */
    SliderFragment(uint32_t id = 0);
    
    /**
     * @brief Virtual destructor
     */
    virtual ~SliderFragment() = default;
    
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
     * @brief Set the slider's title
     * @param title The new title to set
     */
    void setTitle(const String& title);
    
    /**
     * @brief Set the slider's value
     * @param value The new value to set (0-100)
     */
    void setValue(int value);
    
    /**
     * @brief Get the slider's value
     * @return The current value (0-100)
     */
    int getValue() const;
    
    /**
     * @brief Set a value change handler for this slider
     * @param handler The value change handler function
     */
    void setValueChangeHandler(std::function<void(int)> handler);
    
    /**
     * @brief Draw the slider
     */
    void draw();
    
private:
    Slider m_slider;                            // Internal slider instance
    bool m_isDragging;                          // Slider dragging state
    std::function<void(int)> m_valueChangeHandler;  // Value change handler
};

#endif // SLIDER_FRAGMENT_H