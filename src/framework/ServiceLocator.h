#ifndef SERVICE_LOCATOR_H
#define SERVICE_LOCATOR_H

#include "Service.h"
#include <unordered_map>
#include <string>
#include <mutex>
#include <type_traits>
#include <algorithm>

namespace framework {

/**
 * @brief Service locator for dependency injection
 * 
 * ServiceLocator is a singleton class that manages service registration and lookup.
 * It allows services to be registered and retrieved by type in a type-safe manner.
 */
class ServiceLocator {
public:
    /**
     * @brief Get the singleton instance
     * @return Reference to the singleton instance
     */
    static ServiceLocator& getInstance();
    
    /**
     * @brief Destroy the singleton instance
     */
    static void destroyInstance();
    
    /**
     * @brief Register a service
     * @param service The service to register
     * @return true if registration was successful, false otherwise
     */
    bool registerService(Service* service);
    
    /**
     * @brief Unregister a service
     * @param service The service to unregister
     * @return true if unregistration was successful, false otherwise
     */
    bool unregisterService(Service* service);
    
    /**
     * @brief Unregister a service by ID
     * @param id The ID of the service to unregister
     * @return true if unregistration was successful, false otherwise
     */
    bool unregisterServiceById(uint32_t id);
    
    /**
     * @brief Get a service by ID
     * @param id The ID of the service to get
     * @return Pointer to the service, or nullptr if not found
     */
    Service* getServiceById(uint32_t id) const;
    
    /**
     * @brief Get a service by type
     * @tparam T The service type
     * @return Pointer to the service, or nullptr if not found
     */
    template<typename T>
    T* getService() const;
    
    /**
     * @brief Initialize all registered services
     * @return true if all services were initialized successfully, false otherwise
     */
    bool initializeAll();
    
    /**
     * @brief Shutdown all registered services
     */
    void shutdownAll();

private:
    /**
     * @brief Private constructor (singleton pattern)
     */
    ServiceLocator() = default;
    
    /**
     * @brief Private destructor (singleton pattern)
     */
    ~ServiceLocator();
    
    /**
     * @brief Private copy constructor (singleton pattern)
     */
    ServiceLocator(const ServiceLocator&) = delete;
    
    /**
     * @brief Private assignment operator (singleton pattern)
     */
    ServiceLocator& operator=(const ServiceLocator&) = delete;
    
    static ServiceLocator* s_instance;                // Singleton instance
    static std::mutex s_mutex;                        // Mutex for thread safety
    
    std::unordered_map<uint32_t, Service*> m_services; // Map of service ID to service
    std::unordered_map<std::string, Service*> m_servicesByType; // Map of service type name to service
};

// Static member initialization
inline ServiceLocator* ServiceLocator::s_instance = nullptr;
inline std::mutex ServiceLocator::s_mutex;

// Inline implementation of getInstance
inline ServiceLocator& ServiceLocator::getInstance() {
    std::lock_guard<std::mutex> lock(s_mutex);
    if (!s_instance) {
        s_instance = new ServiceLocator();
    }
    return *s_instance;
}

// Inline implementation of destroyInstance
inline void ServiceLocator::destroyInstance() {
    std::lock_guard<std::mutex> lock(s_mutex);
    if (s_instance) {
        delete s_instance;
        s_instance = nullptr;
    }
}

// Inline implementation of destructor
inline ServiceLocator::~ServiceLocator() {
    // We don't delete services here, as they might be owned elsewhere
    m_services.clear();
    m_servicesByType.clear();
}

// Inline implementation of registerService
inline bool ServiceLocator::registerService(Service* service) {
    if (!service) return false;
    
    uint32_t id = service->getId();
    const char* typeName = service->getServiceTypeName();
    
    // Check if service with this ID is already registered
    if (m_services.find(id) != m_services.end()) {
        return false;
    }
    
    // Check if service of this type is already registered
    if (m_servicesByType.find(typeName) != m_servicesByType.end()) {
        return false;
    }
    
    m_services[id] = service;
    m_servicesByType[typeName] = service;
    
    return true;
}

// Inline implementation of unregisterService
inline bool ServiceLocator::unregisterService(Service* service) {
    if (!service) return false;
    
    return unregisterServiceById(service->getId());
}

// Inline implementation of unregisterServiceById
inline bool ServiceLocator::unregisterServiceById(uint32_t id) {
    auto it = m_services.find(id);
    if (it != m_services.end()) {
        Service* service = it->second;
        const char* typeName = service->getServiceTypeName();
        
        m_services.erase(it);
        m_servicesByType.erase(typeName);
        
        return true;
    }
    
    return false;
}

// Inline implementation of getServiceById
inline Service* ServiceLocator::getServiceById(uint32_t id) const {
    auto it = m_services.find(id);
    return (it != m_services.end()) ? it->second : nullptr;
}

// Template implementation of getService
template<typename T>
T* ServiceLocator::getService() const {
    static_assert(std::is_base_of<Service, T>::value, "T must be derived from Service");
    
    // Create a temporary instance to get the type name
    T temp;
    const char* typeName = temp.getServiceTypeName();
    
    auto it = m_servicesByType.find(typeName);
    if (it != m_servicesByType.end()) {
        return static_cast<T*>(it->second);
    }
    
    return nullptr;
}

// Inline implementation of initializeAll
inline bool ServiceLocator::initializeAll() {
    bool allInitialized = true;
    
    for (auto& pair : m_services) {
        Service* service = pair.second;
        if (!service->isInitialized()) {
            if (!service->initialize()) {
                allInitialized = false;
            }
        }
    }
    
    return allInitialized;
}

// Inline implementation of shutdownAll
inline void ServiceLocator::shutdownAll() {
    for (auto& pair : m_services) {
        Service* service = pair.second;
        if (service->isInitialized()) {
            service->shutdown();
        }
    }
}

} // namespace framework

#endif // SERVICE_LOCATOR_H
