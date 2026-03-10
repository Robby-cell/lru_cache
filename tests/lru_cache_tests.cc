#include <gtest/gtest.h>

#include <lru_cache/lru_cache.hpp>

TEST(LRUCache, Simple) {
    lru_cache::lru_cache<int, int> cache(0);
    auto it = cache.find(42);

    ASSERT_EQ(it, cache.end());
}

TEST(LRUCache, AddingElements) {
    lru_cache::lru_cache<int, int> cache(1);

    cache.set(42, 42);

    ASSERT_EQ(cache.get(42), 42);
    ASSERT_EQ(cache.find(69), cache.end());
}

TEST(LRUCache, OldValuesGetEvicted) {
    lru_cache::lru_cache<int, int> cache(3);

    for (int i = 0; i < 3; ++i) {
        cache.set(i, i);
    }

    cache.set(42, 42);

    ASSERT_FALSE(cache.contains(0));
    for (int i = 1; i < 3; ++i) {
        ASSERT_TRUE(cache.contains(i));
        ASSERT_EQ(cache.get(i), i);
    }
    ASSERT_TRUE(cache.contains(42));
    ASSERT_EQ(cache.get(42), 42);
}

TEST(LRUCache, EdgeCaseSizes) {
    lru_cache::lru_cache<int, int> cache(1);

    cache.set(42, 42);
    cache.set(0, 0);

    cache.evict_last();

    ASSERT_TRUE(cache.empty());

    cache.set(0, 0);

    size_t count = 0;
    for (const auto& p : cache) {
        auto&& k = p.first;
        auto&& v = p.second;

        // Since we only have 0,0 in the cache. We should be able to assume this
        // is true
        ASSERT_EQ(k, 0);
        ASSERT_EQ(v, 0);
        ++count;
    }

    ASSERT_EQ(count, 1);
}
