#include <iostream>
#include <unordered_map>
#include <list>
#include <mutex>
#include <memory>

template<typename KeyType, typename ValueType>
class LRUCache {
public:
    // Constructor to init the cache w/ a given capacity
    explicit LRUCache(size_t size) : capacity(size) {}

    // Function to retrieve a value from the cache
    ValueType get(const KeyType& key) {
	std::lock_guard<std::mutex> lock(cache_mutex); // Lock for thread safety
        auto it = cache_map.find(key);  // Attempt to find the key in the hash map
        if (it == cache_map.end()) {
            throw std::range_error("Key not found");  // Key not found, throw exception
        }
        
        usage_list.splice(usage_list.begin(), usage_list, it->second); // Moves accessed node
        return it->second->second;  // Return the value associated with the key
    }

    // Function to insert or update a value in the cache
    void put(const KeyType& key, const ValueType& value) {
	std::lock_guard<std::mutex> lock(cache_mutex); // Lock for thread safety
        auto it = cache_map.find(key);  // Check if key already exists in the cache
        if (it != cache_map.end()) {
            // If key exists -> MRU
            usage_list.splice(usage_list.begin(), usage_list, it->second);
            it->second->second = value;  // Update the value
            return;
        }

        // If cache full, evict the LRU item
        if (usage_list.size() == capacity) {
            auto last = usage_list.end();
            last--;
            cache_map.erase(last->first);  // Remove from map
            usage_list.pop_back();  // Remove from list
        }

        // Inserts the new key-value pair at the front of the list
        usage_list.emplace_front(key, value);
        cache_map[key] = usage_list.begin();  // Update map to point to the new element in the list
    }

    // Function to remove an object from the cache if it exists
    void erase(const KeyType& key) {
	std::lock_guard<std::mutex> lock(cache_mutex); // Lock to ensure thread safety
        auto it = cache_map.find(key);  // Find the key in the map
        if (it != cache_map.end()) {
            usage_list.erase(it->second);  // Remove from list
            cache_map.erase(it);  // Remove from map
        }
    }

    // Function to dynamically adjust the cache's capacity
    void resize(size_t new_capacity) {
	std::lock_guard<std::mutex> lock(cache_mutex); // Lock to ensure thread safety
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
    std::list<std::pair<KeyType, ValueType>> usage_list;  
    // Map to quickly lookup elements in the list
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

