#ifndef LRU_CACHE_LRU_CACHE_HPP
#define LRU_CACHE_LRU_CACHE_HPP

// LRU Cache
#include <lru_cache/internal/macros.hpp>

// Hash set
#include <hmm/city-hash.hpp>
#include <hmm/flat-hash-set.hpp>

// Std
#include <cstddef>
#include <functional>
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>

namespace lru_cache {

template <class T = void>
struct default_hasher {
    size_t operator()(const T& value) const noexcept {
        return hmm::CityHash<T>{}(value);
    }
};

namespace detail {

struct uninitialized_value_t {};

constexpr uninitialized_value_t uninitialized_value{};

template <class, template <class...> class, class...>
struct is_specialization : std::false_type {};

template <template <class...> class Template, class... Args>
struct is_specialization<Template<Args...>, Template> : std::true_type {};

template <class K, class V>
struct lru_cache_node {
    using Modifiable = std::pair<K, V>;
    using Accessible = std::pair<const K, V>;

    lru_cache_node(const lru_cache_node& that)
        : lru_cache_node(that.modifiable) {}  // NOLINT

    lru_cache_node(lru_cache_node&& that) noexcept
        : lru_cache_node(std::move(that.modifiable)) {}  // NOLINT

    lru_cache_node& operator=(const lru_cache_node& that) {
        if (this != std::addressof(that)) {
            modifiable = that.modifiable;  // NOLINT
        }
        return *this;
    }

    lru_cache_node& operator=(lru_cache_node&& that) noexcept {
        if (this != std::addressof(that)) {
            modifiable = std::move(that.modifiable);  // NOLINT
        }
        return *this;
    }

    LRU_CACHE_CONSTEXPR_20 ~lru_cache_node() {
        modifiable.~Modifiable();  // NOLINT
    }

    LRU_CACHE_CONSTEXPR_20 lru_cache_node(const Modifiable& p)
        : modifiable(p) {}

    LRU_CACHE_CONSTEXPR_20 lru_cache_node(Modifiable&& p)
        : modifiable(std::move(p)) {}

    template <class Key>
    LRU_CACHE_CONSTEXPR_20 lru_cache_node(
        uninitialized_value_t /* uninitialized */, Key&& key)
        : _raw_key(std::forward<Key>(key)) {}

    const K& key() const noexcept {
        return accessible.first;  // NOLINT
    }

    V& value() noexcept {
        return accessible.second;  // NOLINT
    }

    const V& value() const noexcept {
        return accessible.second;  // NOLINT
    }

    union {
        Modifiable modifiable;
        Accessible accessible;
        K _raw_key;  // NOLINT
    };
    lru_cache_node* previous = nullptr;
    lru_cache_node* next = nullptr;
};

template <class Hash>
struct hasher_for : private Hash {
    template <class Node>
    LRU_CACHE_CONSTEXPR_20 auto operator()(const Node& node) const noexcept ->
        typename std::enable_if<is_specialization<Node, lru_cache_node>::value,
                                size_t>::type {
        return Hash::operator()(node.key());
    }

    template <class Node>
    LRU_CACHE_CONSTEXPR_20 auto operator()(const Node* node) const noexcept ->
        typename std::enable_if<is_specialization<Node, lru_cache_node>::value,
                                size_t>::type {
        return Hash::operator()(node->key());
    }

    template <class K>
    LRU_CACHE_CONSTEXPR_20 auto operator()(const K& key) const noexcept ->
        typename std::enable_if<!is_specialization<K, lru_cache_node>::value,
                                size_t>::type {
        return Hash::operator()(key);
    }
};

template <class T>
struct extractor {
    static const T& get(const T& v) noexcept {
        return v;  // NOLINT(bugprone-return-const-ref-from-parameter)
    }
};

template <class K, class V>
struct extractor<lru_cache_node<K, V>> {
    static const K& get(const lru_cache_node<K, V>& n) noexcept {
        return n.key();
    }
};

template <class Eq>
struct equality_for : private Eq {
    template <class L, class R>
    LRU_CACHE_CONSTEXPR_20 bool operator()(const L& lhs,
                                           const R& rhs) const noexcept {
        return Eq::operator()(extractor<L>::get(lhs), extractor<R>::get(rhs));
    }
};

struct node_extractor;

template <class T, class Const>
struct lru_cache_iterator;

template <class K, class V, class Const>
class lru_cache_iterator<lru_cache_node<K, V>, Const> {
   private:
    using node_type = lru_cache_node<K, V>;

   public:
    using value_type = std::pair<const K, V>;
    using reference = typename std::conditional<Const{}(), const value_type&,
                                                value_type&>::type;
    using pointer = typename std::conditional<Const{}(), const value_type*,
                                              value_type*>::type;
    using size_type = size_t;
    using difference_type = ptrdiff_t;

    using iterator_category = std::bidirectional_iterator_tag;

    lru_cache_iterator() = default;

    template <class ThatConst, typename std::enable_if<
                                   Const{}() && !ThatConst{}(), int>::type = 0>
    constexpr lru_cache_iterator(
        const lru_cache_iterator<node_type, ThatConst>& that)
        : current_(that.current_) {}

    template <class ThatConst, typename std::enable_if<
                                   Const{}() && !ThatConst{}(), int>::type = 0>
    constexpr lru_cache_iterator& operator=(
        const lru_cache_iterator<node_type, ThatConst>& that) {
        current_ = that.current_;
    }

    constexpr explicit lru_cache_iterator(node_type* current)
        : current_(current) {}

    LRU_CACHE_CONSTEXPR_14 lru_cache_iterator& operator++() noexcept {
        current_ = current_->next;
        return *this;
    }

    LRU_CACHE_CONSTEXPR_14 lru_cache_iterator operator++(int) noexcept {
        auto tmp = *this;
        ++*this;
        return tmp;
    }

    LRU_CACHE_CONSTEXPR_14 lru_cache_iterator& operator--() noexcept {
        current_ = current_->previous;
        return *this;
    }

    LRU_CACHE_CONSTEXPR_14 lru_cache_iterator operator--(int) noexcept {
        auto tmp = *this;
        --*this;
        return tmp;
    }

    constexpr bool operator==(const lru_cache_iterator& that) const noexcept {
        return current_ == that.current_;
    }

    constexpr bool operator!=(const lru_cache_iterator& that) const noexcept {
        return !(*this == that);
    }

    reference operator*() const noexcept {
        return current_->accessible;
    }

    pointer operator->() const noexcept {
        return std::addressof(current_->accessible);
    }

   private:
    node_type* current_ = nullptr;

    friend node_extractor;
};

struct node_extractor {
    template <class K, class V, class Const>
    static lru_cache_node<K, V>* extract(
        lru_cache_iterator<lru_cache_node<K, V>, Const> it) noexcept {
        return it.current_;
    }
};

}  // namespace detail

template <class K, class V, class Hash = default_hasher<K>,
          class Alloc = std::allocator<detail::lru_cache_node<K, V>>>
class lru_cache {
   public:
    using node_type = detail::lru_cache_node<K, V>;

   private:
    using Underlying =
        hmm::flat_hash_set<node_type, detail::hasher_for<Hash>,
                           detail::equality_for<std::equal_to<>>, Alloc>;

   public:
    using allocator_type =
        typename std::allocator_traits<Alloc>::template rebind_alloc<node_type>;

    using size_type = typename Underlying::size_type;
    using difference_type = typename Underlying::difference_type;

    using iterator = detail::lru_cache_iterator<node_type, std::false_type>;
    using const_iterator =
        detail::lru_cache_iterator<node_type, std::true_type>;

    using value_type = typename iterator::value_type;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using reference = value_type&;
    using const_reference = const value_type&;

    using key_type = K;
    using mapped_type = V;

   private:
    template <class It>
    static node_type* set_iterator_to_node(It it) {
        return const_cast<node_type*>(std::addressof(*it));
    }

   public:
    lru_cache(size_type max_size) : max_(max_size) {}

    lru_cache(const lru_cache& that) : set_(that.set_), max_(that.max_) {
        // NOLINTBEGIN(cppcoreguidelines-prefer-member-initializer)
        first_ = set_iterator_to_node(set_.find(that.first_->key()));
        last_ = set_iterator_to_node(set_.find(that.last_->key()));
        // NOLINTEND(cppcoreguidelines-prefer-member-initializer)
    }

    lru_cache& operator=(const lru_cache& that) {
        if (this != std::addressof(that)) {
            set_ = that.set_;
            max_ = that.max_;
            first_ = set_iterator_to_node(set_.find(that.first_->key()));
            last_ = set_iterator_to_node(set_.find(that.last_->key()));
        }

        return *this;
    }

    lru_cache(lru_cache&& that) noexcept
        : set_(std::move(that.set_)), max_(std::exchange(that.max_, 0)) {
        // NOLINTBEGIN(cppcoreguidelines-prefer-member-initializer)
        first_ = set_iterator_to_node(
            set_.find(std::exchange(that.first_, nullptr)->key()));
        last_ = set_iterator_to_node(
            set_.find(std::exchange(that.last_, nullptr)->key()));
        // NOLINTEND(cppcoreguidelines-prefer-member-initializer)
    }

    lru_cache& operator=(lru_cache&& that) noexcept {
        set_ = std::move(that.set_);
        max_ = std::exchange(that.max_, 0);
        first_ = set_iterator_to_node(set_.find(that.first_->key()));
        last_ = set_iterator_to_node(set_.find(that.last_->key()));

        return *this;
    }

    ~lru_cache() = default;

    iterator begin() noexcept {
        return iterator(first_);
    }

    iterator end() noexcept {
        return iterator(nullptr);
    }

    const_iterator begin() const noexcept {
        return const_iterator(first_);
    }

    const_iterator end() const noexcept {
        return const_iterator(nullptr);
    }

    const_iterator cbegin() const noexcept {
        return begin();
    }

    const_iterator cend() const noexcept {
        return end();
    }

   private:
    template <class Self, class Key>
    static auto get_impl(Self&& self, Key&& key) -> typename std::conditional<
        std::is_const<typename std::remove_reference<Self>::type>::value,
        const_iterator, iterator>::type {
        using iterator_type = typename std::conditional<
            std::is_const<typename std::remove_reference<Self>::type>::value,
            const_iterator, iterator>::type;

        auto it = std::forward<Self>(self).find(std::forward<Key>(key));
        if (it == self.end()) {
            return self.end();
        }

        auto node = detail::node_extractor::extract(it);
        if (node->previous) {
            node->previous->next = node->next;
        }
        if (node->next) {
            node->next->previous = node->previous;
        }

        auto old = self.first_;
        self.first_ = node;
        old->previous = node;

        if (!node->next) {
            self.last_ = node->previous;
        }

        node->previous = nullptr;
        node->next = old;

        return iterator_type{node};
    }

    void evict(node_type* node) {
        if (node->previous) {
            node->previous->next = node->next;
        }
        if (node->next) {
            node->next->previous = node->previous;
        }

        if (first_ == node) {
            first_ = node->next;
        }
        if (last_ == node) {
            last_ = node->previous;
        }

        set_.erase_element(node->key());

        if (empty()) {
            first_ = nullptr;
            last_ = nullptr;
        }
    }

   public:
    void evict_last() {
        evict(last_);
    }

    mapped_type& get(const key_type& key) noexcept {
        return get_impl(*this, key)->second;
    }

    template <class Key, class... Args>
    iterator emplace(Key&& key, Args&&... args) {
        auto it = get_impl(*this, key);

        if (it == end()) {
            auto emplaced = set_.emplace(detail::uninitialized_value,
                                         std::forward<Key>(key));
            auto&& p = set_iterator_to_node(emplaced.first);
            new (std::addressof(p->value()))
                mapped_type(std::forward<Args>(args)...);

            p->next = first_;
            if (first_) {
                first_->previous = p;
            }
            first_ = p;
            if (!last_) {
                last_ = p;
            }

            if (size() > max_size()) {
                evict_last();
            }

            return iterator(p);
        } else {  // NOLINT(readability-else-after-return)
            it->second = mapped_type{std::forward<Args>(args)...};

            auto node = detail::node_extractor::extract(it);
            if (node->previous) {
                node->previous->next = node->next;
            }
            if (node->next) {
                node->next->previous = node->previous;
            }
            node->next = first_;

            first_ = node;
            return iterator(it);
        }
    }

    template <class Key, class Value>
    void set(Key&& key, Value&& value) {
        emplace(std::forward<Key>(key), std::forward<Value>(value));
    }

   private:
    template <class Self, class Key>
    static auto find_impl(Self&& self, const Key& key)  // NOLINT
        -> typename std::conditional<
            std::is_const<typename std::remove_reference<Self>::type>::value,
            const_iterator, iterator>::type {
        using iterator_type = typename std::conditional<
            std::is_const<typename std::remove_reference<Self>::type>::value,
            const_iterator, iterator>::type;

        auto it = self.set_.find(key);
        if (it != self.set_.end()) {
            return iterator_type(set_iterator_to_node(it));
        }
        return self.end();
    }

   public:
    iterator find(const key_type& key) {
        return find_impl(*this, key);
    }

    const_iterator find(const key_type& key) const {
        return find_impl(*this, key);
    }

    template <class Key>
    iterator find(const Key& key) {
        return find_impl(*this, key);
    }

    template <class Key>
    const_iterator find(const Key& key) const {
        return find_impl(*this, key);
    }

    bool contains(const key_type& key) const {
        return find(key) != end();
    }

    template <class Key>
    bool contains(const Key& key) const {
        return find(key) != end();
    }

    constexpr bool empty() const noexcept {
        return size() == 0;
    }

    constexpr size_type size() const noexcept {
        return set_.size();
    }

    constexpr size_type capacity() const noexcept {
        return set_.capacity();
    }

    constexpr size_type max_size() const noexcept {
        return max_;
    }

   private:
    Underlying set_;
    node_type* first_ = nullptr;
    node_type* last_ = nullptr;
    size_type max_ = 0;
};

}  // namespace lru_cache

#endif  // LRU_CACHE_LRU_CACHE_HPP
