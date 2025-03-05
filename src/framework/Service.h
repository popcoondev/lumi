#ifndef SERVICE_H
#define SERVICE_H

#include <cstdint>
#include <string>

namespace framework {

/**
 * @brief Service state enumeration
 */
enum class ServiceState {
    UNINITIALIZED,  // Service has been created but not initialized
    INITIALIZING,   // Service is in the process of initializing
    RUNNING,        // Service is running normally
    STOPPING,       // Service is in the process of shutting down
    STOPPED         // Service has been stopped
};

/**
 * @brief Base class for all services
 * 
 * Service is the foundation class for all services in the framework.
 * It provides ID management, lifecycle methods, and state management.
 */
class Service {
public:
    /**
     * @brief Constructor with optional ID
     * @param id Optional service ID, auto-generated if not provided
     */
    Service(uint32_t id = 0);
    
    /**
     * @brief Virtual destructor
     */
    virtual ~Service() = default;
    
    /**
     * @brief Get the service's ID
     * @return The service ID
     */
    uint32_t getId() const { return m_id; }
    
    /**
     * @brief Set the service's ID
     * @param id The new ID to set
     */
    void setId(uint32_t id) { m_id = id; }
    
    /**
     * @brief Initialize the service
     * @return true if initialization was successful, false otherwise
     */
    virtual bool initialize();
    
    /**
     * @brief Shutdown the service
     */
    virtual void shutdown();
    
    /**
     * @brief Get the current service state
     * @return The current state
     */
    ServiceState getState() const { return m_state; }
    
    /**
     * @brief Check if service is initialized
     * @return true if initialized, false otherwise
     */
    bool isInitialized() const { 
        return m_state == ServiceState::RUNNING; 
    }
    
    /**
     * @brief Get the service type name
     * @return The service type name
     */
    virtual const char* getServiceTypeName() const = 0;

protected:
    uint32_t m_id;                                   // Service unique identifier
    ServiceState m_state = ServiceState::UNINITIALIZED; // Current service state
    
    /**
     * @brief Set the service state
     * @param state The new state to set
     */
    void setState(ServiceState state) { m_state = state; }
    
    /**
     * @brief Generate a unique ID for the service
     * @return A unique ID
     */
    static uint32_t generateId();
};

// Inline implementation of constructor
inline Service::Service(uint32_t id) : m_id(id == 0 ? generateId() : id) {}

// Inline implementation of initialize
inline bool Service::initialize() {
    if (m_state != ServiceState::UNINITIALIZED) {
        return false;
    }
    
    setState(ServiceState::INITIALIZING);
    setState(ServiceState::RUNNING);
    return true;
}

// Inline implementation of shutdown
inline void Service::shutdown() {
    if (m_state == ServiceState::RUNNING || m_state == ServiceState::INITIALIZING) {
        setState(ServiceState::STOPPING);
        setState(ServiceState::STOPPED);
    }
}

// Inline implementation of ID generator
inline uint32_t Service::generateId() {
    static uint32_t nextId = 1;
    return nextId++;
}

} // namespace framework

#endif // SERVICE_H
