#include <iostream>
#include <unordered_map>
#include <list>
#include <mutex>
#include <memory>

template<typename KeyType, typename ValueType>
class LRUCache {
public:
    // Constructor to initialize the cache with a given capacity
    // Prevents compiler from using the constructor for implicit conversions
    // Helps prevent accidental type conversions that might lead to bugs
    explicit LRUCache(size_t size) : capacity(size) {}

    // Function to retrieve a value from the cache
    // Passed as const ref with '&' because there is no need to modify the object
    // Also copying is too "expensive". Using the & ref ensures that the function does not alter the data.
    // We do this because we don't want the program to alter the "keys" generated.
    ValueType get(const KeyType& key) { 
        // Lock for thread safety, prevents concurrent modification during the fetch
	std::lock_guard<std::mutex> lock(cache_mutex);
        auto it = cache_map.find(key);  // Attempt to find the key in the hash map
        if (it == cache_map.end()) {
            throw std::range_error("Key not found");  // Key not found, throw exception
        }
        // Moves the accessed node to the front of the list to mark it as most recently used
        usage_list.splice(usage_list.begin(), usage_list, it->second);
        return it->second->second;  // Return the value associated with the key
    }

    // Function to insert or update a value in the cache
    // If cache is full, it evicts the LRU item
    // New/updated item placed at the front of usage list
    void put(const KeyType& key, const ValueType& value) {
        // Lock for thread safety, ensures cache state is not corrupted by concurrent accesses and modifications
	std::lock_guard<std::mutex> lock(cache_mutex);
        auto it = cache_map.find(key);  // Check if key already exists in the cache
        if (it != cache_map.end()) {
            // If key exists, moves to the front to mark as most recently used
            usage_list.splice(usage_list.begin(), usage_list, it->second);
            it->second->second = value;  // Update the value
            return;
        }

        // If the cache is full, we need to evict the least recently used item
        if (usage_list.size() == capacity) {
            auto last = usage_list.end();
            last--;
            cache_map.erase(last->first);  // Remove from map
            usage_list.pop_back();  // Remove from list
        }

        // Insert the new key-value pair at the front of the list
        usage_list.emplace_front(key, value);
        cache_map[key] = usage_list.begin();  // Update map to point to the new element in the list
    }

    // Function to remove an object from the cache if it exists
    void erase(const KeyType& key) {
        // Lock to ensure thread safety
	// Ensures erase ops are performed safely w/o interference from other threads accessing/modifying data
	std::lock_guard<std::mutex> lock(cache_mutex);
        auto it = cache_map.find(key);  // Find the key in the map
        if (it != cache_map.end()) {
            usage_list.erase(it->second);  // Remove from list
            cache_map.erase(it);  // Remove from map
        }
    }

    // Function to dynamically adjust the cache's capacity
    // Evicts least recently used items until size matches new capacity
    void resize(size_t new_capacity) {
        // Lock to ensure thread safety
	// Ensures the resizing cache des not conflict with other ops happening concurrently
	std::lock_guard<std::mutex> lock(cache_mutex);
        while (usage_list.size() > new_capacity) {  // If current size is larger than new capacity, reduce size
            auto last = usage_list.end();
            last--;
            cache_map.erase(last->first);  // Remove least recently used items
            usage_list.pop_back();
        }
        capacity = new_capacity;  // Set the new capacity
    }

private:
    size_t capacity;  // Maximum number of elements in the cache
    // List to track the least recent to most recently used objects
    // Used doubly linked list to maintain the order from most recently used objects
    // Allows O(1) complexity for adding/removing elements from the end of list
    // This is crucial when using the LRU policy.
    std::list<std::pair<KeyType, ValueType>> usage_list;  
    // Map to quickly lookup elements in the list
    // Links keys to iterators of their corresponding positions in the usage_list
    // Enables 0(1) avg time complexity for accessing elements
    std::unordered_map<KeyType, typename std::list<std::pair<KeyType, ValueType>>::iterator> cache_map;
    std::mutex cache_mutex;  // Mutex to make class thread-safe
};

int main() {
    LRUCache<int, std::string> cache(2);  // Create a cache for up to 2 items
    cache.put(1, "data1");  // Insert item with key 1
    cache.put(2, "data2");  // Insert item with key 2
    try {
        std::cout << "1 -> " << cache.get(1) << std::endl;  // Access item with key 1
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    cache.put(3, "data3");  // Insert item with key 3, causing key 2 to be evicted
    try {
        std::cout << "2 -> " << cache.get(2) << std::endl;  // Attempt to access item with key 2
    } catch (const std::exception& e) {
        std::cout << "2 -> " << e.what() << std::endl;  // Should print "Key not found"
    }
    return 0;
}

