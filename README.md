# threadSafe
C++ thread safe container class (Scalable) {Comment Heavy}

Stores recently accessed objects. 
Supports caching objects of different types
Implements a resize method to dynamically adjust cache size.
Provides methods for inserting/removing data
Minimizes memory allocation/deallocation
Supports concurrent access & updates from multiple threads, and uses the LRU eviction policy.
