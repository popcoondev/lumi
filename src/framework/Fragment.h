#ifndef FRAGMENT_H
#define FRAGMENT_H

#include "Component.h"
#include <memory>

namespace framework {

/**
 * @brief Base class for reusable UI fragments
 * 
 * Fragment represents a reusable UI component that can be attached to
 * and detached from other components. It extends Component with additional
 * lifecycle methods and display area management.
 */
class Fragment : public Component {
public:
    /**
     * @brief Constructor with optional ID
     * @param id Optional fragment ID, auto-generated if not provided
     */
    Fragment(uint32_t id = 0);
    
    /**
     * @brief Virtual destructor
     */
    virtual ~Fragment() = default;
    
    /**
     * @brief Lifecycle method called when fragment is attached to a parent
     * @param parent The parent component
     * @return true if attachment was successful, false otherwise
     */
    virtual bool onAttach(Component* parent);
    
    /**
     * @brief Lifecycle method called when fragment is detached from its parent
     */
    virtual void onDetach();
    
    /**
     * @brief Check if fragment is attached to a parent
     * @return true if attached, false otherwise
     */
    bool isAttached() const { return m_parent != nullptr; }
    
    /**
     * @brief Get the parent component
     * @return Pointer to the parent component, or nullptr if not attached
     */
    Component* getParent() const { return m_parent; }
    
    /**
     * @brief Set the fragment's display area
     * @param x X-coordinate
     * @param y Y-coordinate
     * @param width Width
     * @param height Height
     */
    void setDisplayArea(int x, int y, int width, int height);
    
    /**
     * @brief Get the fragment's X-coordinate
     * @return X-coordinate
     */
    int getX() const { return m_x; }
    
    /**
     * @brief Get the fragment's Y-coordinate
     * @return Y-coordinate
     */
    int getY() const { return m_y; }
    
    /**
     * @brief Get the fragment's width
     * @return Width
     */
    int getWidth() const { return m_width; }
    
    /**
     * @brief Get the fragment's height
     * @return Height
     */
    int getHeight() const { return m_height; }
    
    /**
     * @brief Set the fragment's position
     * @param x X-coordinate
     * @param y Y-coordinate
     */
    void setPosition(int x, int y);
    
    /**
     * @brief Set the fragment's size
     * @param width Width
     * @param height Height
     */
    void setSize(int width, int height);

protected:
    Component* m_parent = nullptr; // Parent component
    int m_x = 0;                   // X-coordinate
    int m_y = 0;                   // Y-coordinate
    int m_width = 0;               // Width
    int m_height = 0;              // Height
};

// Inline implementation of constructor
inline Fragment::Fragment(uint32_t id) : Component(id) {}

// Inline implementation of onAttach
inline bool Fragment::onAttach(Component* parent) {
    if (parent) {
        m_parent = parent;
        return true;
    }
    return false;
}

// Inline implementation of onDetach
inline void Fragment::onDetach() {
    m_parent = nullptr;
}

// Inline implementation of setDisplayArea
inline void Fragment::setDisplayArea(int x, int y, int width, int height) {
    m_x = x;
    m_y = y;
    m_width = width;
    m_height = height;
}

// Inline implementation of setPosition
inline void Fragment::setPosition(int x, int y) {
    m_x = x;
    m_y = y;
}

// Inline implementation of setSize
inline void Fragment::setSize(int width, int height) {
    m_width = width;
    m_height = height;
}

} // namespace framework

#endif // FRAGMENT_H
