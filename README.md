# LRU Cache (C++)

A **header-only, high-performance Least Recently Used (LRU) cache** implemented in modern C++.
It provides **O(1) average lookup, insertion, and eviction**, making it suitable for performance-critical systems such as compilers, game engines, networking layers, and real-time systems.

The cache combines:

* a **hash set** for fast key lookup
* a **doubly linked list** to track recency of access

When the cache exceeds its maximum size, the **least recently used element is automatically evicted**.

---

## Features

* Header-only
* O(1) average `get`, `set`, `find`, and `contains`
* Automatic eviction when capacity is exceeded
* Bidirectional iterators
* Custom hash support
* Allocator support
* Cache order iteration (most recently used → least recently used)
* Move and copy semantics

---

## Basic Usage

```cpp
#include <lru_cache/lru_cache.hpp>

int main() {
    lru_cache::lru_cache<int, std::string> cache(3);

    cache.set(1, "one");
    cache.set(2, "two");
    cache.set(3, "three");

    // Accessing an element marks it as most recently used
    std::string value = cache.get(1);

    // Adding another element evicts the least recently used
    cache.set(4, "four");

    if (cache.contains(2)) {
        // ...
    }
}
```

---

## Example: Iterating the Cache

Iteration occurs from **most recently used → least recently used**.

```cpp
for (auto& [key, value] : cache) {
    std::cout << key << " -> " << value << "\n";
}
```

---

## API Overview

### Constructor

```cpp
lru_cache(size_type max_size);
```

Creates a cache with the given maximum number of elements.

---

### Insert / Update

```cpp
cache.set(key, value);
```

or

```cpp
cache.emplace(key, args...);
```

* Inserts a new value.
* Updates the value if the key already exists.
* Moves the element to **most recently used**.
* Evicts the least recently used item if capacity is exceeded.

---

### Access

```cpp
cache.get(key);
```

Returns the value associated with `key` and marks it as **most recently used**.

---

### Lookup

```cpp
cache.find(key);
cache.contains(key);
```

* `find` returns an iterator.
* `contains` returns a boolean.

---

### Eviction

```cpp
cache.evict_last();
```

Manually removes the **least recently used element**.

---

### Capacity / Size

```cpp
cache.size();
cache.capacity();
cache.max_size();
cache.empty();
```

---

## Template Parameters

```cpp
template <
    class K,
    class V,
    class Hash = default_hasher<K>,
    class Alloc = std::allocator<detail::lru_cache_node<K, V>>
>
class lru_cache;
```

| Parameter | Description                     |
| --------- | ------------------------------- |
| `K`       | Key type                        |
| `V`       | Value type                      |
| `Hash`    | Hash function for keys          |
| `Alloc`   | Allocator used for node storage |

---

## Custom Hash Example

```cpp
struct MyHasher {
    size_t operator()(const MyKey& k) const {
        return some_hash(k);
    }
};

lru_cache::lru_cache<MyKey, Value, MyHasher> cache(128);
```

---

## Implementation Notes

The cache internally stores entries in a structure containing:

* key/value pair
* previous/next pointers

A `flat_hash_set` is used for fast lookup while the linked list maintains **recency ordering**.

Operations such as `get()` move nodes to the **front of the list**, ensuring the least recently used node is always at the tail for efficient eviction.

---

## License

MIT License
