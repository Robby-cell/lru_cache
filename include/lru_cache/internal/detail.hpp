#ifndef LRU_CACHE_LRU_CACHE_INTERNAL_DETAIL_HPP
#define LRU_CACHE_LRU_CACHE_INTERNAL_DETAIL_HPP

#include <lru_cache/internal/macros.hpp>
#include <type_traits>
#include <utility>

namespace lru_cache {
namespace internal {

template <class T, class... Args>
LRU_CACHE_CONSTEXPR_20 inline auto construct_in_place(T* ptr, Args&&... args) ->
    typename std::enable_if<std::is_constructible<T, Args&&...>::value,
                            void>::type {
    new (ptr) T(std::forward<Args>(args)...);
}

template <class T>
LRU_CACHE_CONSTEXPR_20 inline auto destroy_in_place(T* ptr) ->
    typename std::enable_if<std::is_trivially_destructible<T>::value,
                            void>::type {}

template <class T>
LRU_CACHE_CONSTEXPR_20 inline auto destroy_in_place(T* ptr) ->
    typename std::enable_if<!std::is_trivially_destructible<T>::value &&
                                std::is_destructible<T>::value,
                            void>::type {
    ptr->~T();
}

}  // namespace internal
}  // namespace lru_cache

#endif  // LRU_CACHE_LRU_CACHE_INTERNAL_DETAIL_HPP
