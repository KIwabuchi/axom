#include <stack>

#include <metall/metall.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include <boost/container/vector.hpp>
#include <boost/container/string.hpp>
#include <boost/container/list.hpp>
#include <boost/container/deque.hpp>
#include <metall/container/string_key_store.hpp>

#include "Memory.hpp"

namespace axom::sidre::metall_container {

template <typename T, typename Allocator = metall::manager::scoped_fallback_allocator_type<T>>
using vector = boost::container::vector<T, Allocator>;

template <class Key, class Hash = std::hash<Key>,
          class KeyEqual = std::equal_to<Key>,
          class Allocator = metall::manager::scoped_fallback_allocator_type<Key>>
using unordered_set = boost::unordered_set<Key, Hash, KeyEqual, Allocator>;

template <class Key, class T, class Hash = std::hash<Key>,
          class KeyEqual = std::equal_to<Key>,
          class Allocator = metall::manager::scoped_fallback_allocator_type<std::pair<const Key, T>>>
using unordered_map = boost::unordered_map<Key, T, Hash, KeyEqual, Allocator>;

using string = boost::container::basic_string<char, std::char_traits<char>, metall::manager::scoped_fallback_allocator_type<char>>;

template <typename T, typename Allocator = metall::manager::scoped_fallback_allocator_type<T>>
using deque = boost::container::deque<T, Allocator>;

template <typename T, typename Container = deque<T>>
using stack = std::stack<T, Container>;

template <typename T, typename Allocator = metall::manager::scoped_fallback_allocator_type<T>>
using list = boost::container::list<T, Allocator>;

template <class Key, class Compare = std::less<Key>,
          class Allocator = metall::manager::scoped_fallback_allocator_type<Key>>
using set = boost::container::set<Key, Compare, Allocator>;

template <typename value_type,
          typename allocator_type =
              metall::manager::scoped_fallback_allocator_type<std::byte>>
using string_key_store =
    metall::container::string_key_store<value_type, allocator_type>;
}  // namespace axom::sidre::metall_container