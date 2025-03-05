#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <cstdint>
#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <functional>
#include <vector>
#include <algorithm>

namespace framework {

/**
 * @brief Resource type enumeration
 */
enum class ResourceType {
    NONE,       // No specific resource type
    IMAGE,      // Image resource
    SOUND,      // Sound resource
    FONT,       // Font resource
    TEXT,       // Text resource
    BINARY,     // Binary data resource
    CUSTOM      // Custom resource type
};

/**
 * @brief Base class for all resources
 */
class Resource {
public:
    /**
     * @brief Constructor with resource ID and type
     * @param id Resource ID
     * @param type Resource type
     */
    Resource(uint32_t id, ResourceType type = ResourceType::NONE);
    
    /**
     * @brief Virtual destructor
     */
    virtual ~Resource() = default;
    
    /**
     * @brief Get the resource ID
     * @return The resource ID
     */
    uint32_t getId() const { return m_id; }
    
    /**
     * @brief Get the resource type
     * @return The resource type
     */
    ResourceType getType() const { return m_type; }
    
    /**
     * @brief Get the resource size in bytes
     * @return The resource size
     */
    virtual size_t getSize() const = 0;
    
    /**
     * @brief Check if the resource is loaded
     * @return true if loaded, false otherwise
     */
    bool isLoaded() const { return m_loaded; }
    
    /**
     * @brief Set the loaded state
     * @param loaded The loaded state
     */
    void setLoaded(bool loaded) { m_loaded = loaded; }
    
    /**
     * @brief Get the resource path
     * @return The resource path
     */
    const std::string& getPath() const { return m_path; }
    
    /**
     * @brief Set the resource path
     * @param path The resource path
     */
    void setPath(const std::string& path) { m_path = path; }
    
    /**
     * @brief Update the last access time
     */
    void updateAccessTime() { m_lastAccessTime = getCurrentTime(); }
    
    /**
     * @brief Get the last access time
     * @return The last access time
     */
    uint64_t getLastAccessTime() const { return m_lastAccessTime; }

protected:
    uint32_t m_id;                   // Resource ID
    ResourceType m_type;             // Resource type
    bool m_loaded = false;           // Whether the resource is loaded
    std::string m_path;              // Resource path
    uint64_t m_lastAccessTime = 0;   // Last access time
    
    /**
     * @brief Get the current time in milliseconds
     * @return Current time
     */
    static uint64_t getCurrentTime();
};

// Inline implementation of constructor
inline Resource::Resource(uint32_t id, ResourceType type)
    : m_id(id), m_type(type), m_lastAccessTime(getCurrentTime()) {}

// Inline implementation of getCurrentTime
inline uint64_t Resource::getCurrentTime() {
    return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count());
}

/**
 * @brief Resource manager for loading and managing resources
 * 
 * ResourceManager is a singleton class that manages resource loading,
 * caching, and memory management. It provides methods for loading
 * resources from files or memory, and for releasing resources when
 * they are no longer needed.
 */
class ResourceManager {
public:
    /**
     * @brief Get the singleton instance
     * @return Reference to the singleton instance
     */
    static ResourceManager& getInstance();
    
    /**
     * @brief Destroy the singleton instance
     */
    static void destroyInstance();
    
    /**
     * @brief Load a resource from a file
     * @param path The file path
     * @param type The resource type
     * @return Pointer to the loaded resource, or nullptr if loading failed
     */
    Resource* loadResource(const std::string& path, ResourceType type);
    
    /**
     * @brief Load a resource from memory
     * @param data Pointer to the resource data
     * @param size Size of the resource data
     * @param type The resource type
     * @return Pointer to the loaded resource, or nullptr if loading failed
     */
    Resource* loadResourceFromMemory(const void* data, size_t size, ResourceType type);
    
    /**
     * @brief Get a resource by ID
     * @param id The resource ID
     * @return Pointer to the resource, or nullptr if not found
     */
    Resource* getResource(uint32_t id);
    
    /**
     * @brief Get a resource by path
     * @param path The resource path
     * @return Pointer to the resource, or nullptr if not found
     */
    Resource* getResourceByPath(const std::string& path);
    
    /**
     * @brief Release a resource
     * @param resource The resource to release
     * @return true if the resource was released, false otherwise
     */
    bool releaseResource(Resource* resource);
    
    /**
     * @brief Release a resource by ID
     * @param id The ID of the resource to release
     * @return true if the resource was released, false otherwise
     */
    bool releaseResourceById(uint32_t id);
    
    /**
     * @brief Release a resource by path
     * @param path The path of the resource to release
     * @return true if the resource was released, false otherwise
     */
    bool releaseResourceByPath(const std::string& path);
    
    /**
     * @brief Release all resources
     */
    void releaseAllResources();
    
    /**
     * @brief Release all resources of a specific type
     * @param type The resource type
     */
    void releaseResourcesByType(ResourceType type);
    
    /**
     * @brief Get the total memory used by all resources
     * @return The total memory used in bytes
     */
    size_t getTotalMemoryUsed() const;
    
    /**
     * @brief Get the memory used by resources of a specific type
     * @param type The resource type
     * @return The memory used in bytes
     */
    size_t getMemoryUsedByType(ResourceType type) const;
    
    /**
     * @brief Set the memory limit for resource caching
     * @param limit The memory limit in bytes
     */
    void setMemoryLimit(size_t limit) { m_memoryLimit = limit; }
    
    /**
     * @brief Get the memory limit for resource caching
     * @return The memory limit in bytes
     */
    size_t getMemoryLimit() const { return m_memoryLimit; }
    
    /**
     * @brief Register a custom resource loader
     * @param type The resource type
     * @param loader The loader function
     * @return true if registration was successful, false otherwise
     */
    bool registerResourceLoader(ResourceType type, std::function<Resource*(const std::string&)> loader);
    
    /**
     * @brief Unregister a custom resource loader
     * @param type The resource type
     * @return true if unregistration was successful, false otherwise
     */
    bool unregisterResourceLoader(ResourceType type);

private:
    /**
     * @brief Private constructor (singleton pattern)
     */
    ResourceManager() = default;
    
    /**
     * @brief Private destructor (singleton pattern)
     */
    ~ResourceManager();
    
    /**
     * @brief Private copy constructor (singleton pattern)
     */
    ResourceManager(const ResourceManager&) = delete;
    
    /**
     * @brief Private assignment operator (singleton pattern)
     */
    ResourceManager& operator=(const ResourceManager&) = delete;
    
    /**
     * @brief Generate a unique ID for a resource
     * @return A unique ID
     */
    uint32_t generateResourceId();
    
    /**
     * @brief Check if memory limit is exceeded and release resources if needed
     * @param requiredSize The size of the resource to be loaded
     * @return true if enough memory is available, false otherwise
     */
    bool ensureMemoryAvailable(size_t requiredSize);
    
    /**
     * @brief Release least recently used resources to free memory
     * @param requiredSize The amount of memory to free
     * @return true if enough memory was freed, false otherwise
     */
    bool releaseLRUResources(size_t requiredSize);
    
    /**
     * @brief Update the access order of a resource
     * @param resource The resource that was accessed
     */
    void updateResourceAccessOrder(Resource* resource);
    
    static ResourceManager* s_instance;                // Singleton instance
    static std::mutex s_mutex;                         // Mutex for thread safety
    
    std::unordered_map<uint32_t, std::unique_ptr<Resource>> m_resources;  // Map of resource ID to resource
    std::unordered_map<std::string, Resource*> m_resourcesByPath;         // Map of resource path to resource
    std::unordered_map<ResourceType, std::function<Resource*(const std::string&)>> m_loaders;  // Map of resource type to loader
    
    std::vector<Resource*> m_resourceAccessOrder;      // List of resources in order of last access
    size_t m_memoryLimit = 1024 * 1024 * 100;          // Memory limit (100 MB by default)
    size_t m_memoryUsed = 0;                           // Current memory used
};

// Static member initialization
inline ResourceManager* ResourceManager::s_instance = nullptr;
inline std::mutex ResourceManager::s_mutex;

// Inline implementation of getInstance
inline ResourceManager& ResourceManager::getInstance() {
    std::lock_guard<std::mutex> lock(s_mutex);
    if (!s_instance) {
        s_instance = new ResourceManager();
    }
    return *s_instance;
}

// Inline implementation of destroyInstance
inline void ResourceManager::destroyInstance() {
    std::lock_guard<std::mutex> lock(s_mutex);
    if (s_instance) {
        delete s_instance;
        s_instance = nullptr;
    }
}

// Inline implementation of destructor
inline ResourceManager::~ResourceManager() {
    releaseAllResources();
}

// Inline implementation of generateResourceId
inline uint32_t ResourceManager::generateResourceId() {
    static uint32_t nextId = 1;
    return nextId++;
}

// Inline implementation of loadResource
inline Resource* ResourceManager::loadResource(const std::string& path, ResourceType type) {
    // Check if resource is already loaded
    auto it = m_resourcesByPath.find(path);
    if (it != m_resourcesByPath.end()) {
        Resource* resource = it->second;
        updateResourceAccessOrder(resource);
        return resource;
    }
    
    // Find a loader for this resource type
    auto loaderIt = m_loaders.find(type);
    if (loaderIt != m_loaders.end()) {
        // Use the registered loader
        Resource* resource = loaderIt->second(path);
        if (resource) {
            resource->setPath(path);
            resource->setLoaded(true);
            
            // Ensure we have enough memory
            if (!ensureMemoryAvailable(resource->getSize())) {
                delete resource;
                return nullptr;
            }
            
            // Add to resources
            uint32_t id = resource->getId();
            m_resources[id] = std::unique_ptr<Resource>(resource);
            m_resourcesByPath[path] = resource;
            m_memoryUsed += resource->getSize();
            updateResourceAccessOrder(resource);
            
            return resource;
        }
    }
    
    return nullptr;
}

// Inline implementation of loadResourceFromMemory
inline Resource* ResourceManager::loadResourceFromMemory(const void* data, size_t size, ResourceType type) {
    // This is a simplified implementation
    // In a real implementation, you would create a resource from the memory data
    
    // Generate a unique path for this memory resource
    std::string path = "memory://" + std::to_string(reinterpret_cast<uintptr_t>(data)) + 
                       "_" + std::to_string(size);
    
    // Check if resource is already loaded
    auto it = m_resourcesByPath.find(path);
    if (it != m_resourcesByPath.end()) {
        Resource* resource = it->second;
        updateResourceAccessOrder(resource);
        return resource;
    }
    
    // For now, just return nullptr as we don't have a concrete implementation
    return nullptr;
}

// Inline implementation of getResource
inline Resource* ResourceManager::getResource(uint32_t id) {
    auto it = m_resources.find(id);
    if (it != m_resources.end()) {
        Resource* resource = it->second.get();
        updateResourceAccessOrder(resource);
        return resource;
    }
    return nullptr;
}

// Inline implementation of getResourceByPath
inline Resource* ResourceManager::getResourceByPath(const std::string& path) {
    auto it = m_resourcesByPath.find(path);
    if (it != m_resourcesByPath.end()) {
        Resource* resource = it->second;
        updateResourceAccessOrder(resource);
        return resource;
    }
    return nullptr;
}

// Inline implementation of releaseResource
inline bool ResourceManager::releaseResource(Resource* resource) {
    if (!resource) return false;
    
    return releaseResourceById(resource->getId());
}

// Inline implementation of releaseResourceById
inline bool ResourceManager::releaseResourceById(uint32_t id) {
    auto it = m_resources.find(id);
    if (it != m_resources.end()) {
        Resource* resource = it->second.get();
        std::string path = resource->getPath();
        
        // Update memory usage
        m_memoryUsed -= resource->getSize();
        
        // Remove from access order
        auto orderIt = std::find(m_resourceAccessOrder.begin(), m_resourceAccessOrder.end(), resource);
        if (orderIt != m_resourceAccessOrder.end()) {
            m_resourceAccessOrder.erase(orderIt);
        }
        
        // Remove from maps
        m_resourcesByPath.erase(path);
        m_resources.erase(it);
        
        return true;
    }
    
    return false;
}

// Inline implementation of releaseResourceByPath
inline bool ResourceManager::releaseResourceByPath(const std::string& path) {
    auto it = m_resourcesByPath.find(path);
    if (it != m_resourcesByPath.end()) {
        Resource* resource = it->second;
        return releaseResourceById(resource->getId());
    }
    
    return false;
}

// Inline implementation of releaseAllResources
inline void ResourceManager::releaseAllResources() {
    m_resourcesByPath.clear();
    m_resourceAccessOrder.clear();
    m_resources.clear();
    m_memoryUsed = 0;
}

// Inline implementation of releaseResourcesByType
inline void ResourceManager::releaseResourcesByType(ResourceType type) {
    // Collect resources of the specified type
    std::vector<uint32_t> resourcesToRelease;
    
    for (const auto& pair : m_resources) {
        Resource* resource = pair.second.get();
        if (resource->getType() == type) {
            resourcesToRelease.push_back(resource->getId());
        }
    }
    
    // Release collected resources
    for (uint32_t id : resourcesToRelease) {
        releaseResourceById(id);
    }
}

// Inline implementation of getTotalMemoryUsed
inline size_t ResourceManager::getTotalMemoryUsed() const {
    return m_memoryUsed;
}

// Inline implementation of getMemoryUsedByType
inline size_t ResourceManager::getMemoryUsedByType(ResourceType type) const {
    size_t memoryUsed = 0;
    
    for (const auto& pair : m_resources) {
        Resource* resource = pair.second.get();
        if (resource->getType() == type) {
            memoryUsed += resource->getSize();
        }
    }
    
    return memoryUsed;
}

// Inline implementation of registerResourceLoader
inline bool ResourceManager::registerResourceLoader(ResourceType type, std::function<Resource*(const std::string&)> loader) {
    if (!loader) return false;
    
    m_loaders[type] = loader;
    return true;
}

// Inline implementation of unregisterResourceLoader
inline bool ResourceManager::unregisterResourceLoader(ResourceType type) {
    auto it = m_loaders.find(type);
    if (it != m_loaders.end()) {
        m_loaders.erase(it);
        return true;
    }
    
    return false;
}

// Inline implementation of ensureMemoryAvailable
inline bool ResourceManager::ensureMemoryAvailable(size_t requiredSize) {
    // Check if we have enough memory
    if (m_memoryUsed + requiredSize <= m_memoryLimit) {
        return true;
    }
    
    // Try to free up memory
    return releaseLRUResources(requiredSize);
}

// Inline implementation of releaseLRUResources
inline bool ResourceManager::releaseLRUResources(size_t requiredSize) {
    // Sort resources by last access time
    std::sort(m_resourceAccessOrder.begin(), m_resourceAccessOrder.end(),
              [](Resource* a, Resource* b) {
                  return a->getLastAccessTime() < b->getLastAccessTime();
              });
    
    // Release resources until we have enough memory
    size_t freedMemory = 0;
    std::vector<uint32_t> resourcesToRelease;
    
    for (Resource* resource : m_resourceAccessOrder) {
        freedMemory += resource->getSize();
        resourcesToRelease.push_back(resource->getId());
        
        if (m_memoryUsed - freedMemory + requiredSize <= m_memoryLimit) {
            break;
        }
    }
    
    // Release collected resources
    for (uint32_t id : resourcesToRelease) {
        releaseResourceById(id);
    }
    
    // Check if we have enough memory now
    return (m_memoryUsed + requiredSize <= m_memoryLimit);
}

// Inline implementation of updateResourceAccessOrder
inline void ResourceManager::updateResourceAccessOrder(Resource* resource) {
    if (!resource) return;
    
    // Update the resource's access time
    resource->updateAccessTime();
    
    // Remove the resource from the access order if it's already there
    auto it = std::find(m_resourceAccessOrder.begin(), m_resourceAccessOrder.end(), resource);
    if (it != m_resourceAccessOrder.end()) {
        m_resourceAccessOrder.erase(it);
    }
    
    // Add the resource to the end of the access order (most recently used)
    m_resourceAccessOrder.push_back(resource);
}

} // namespace framework

#endif // RESOURCE_MANAGER_H
