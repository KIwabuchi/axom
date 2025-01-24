#pragma once

#include <memory>
#include <scoped_allocator>

#include <metall/metall.hpp>

template <typename P, typename T>
using Ptr = typename std::pointer_traits<P>::template rebind<T>;

template <typename A, typename T>
using RebindAlloc = typename std::allocator_traits<A>::template rebind_alloc<T>;

template <typename A, typename T>
using RebindScpAlloc = std::scoped_allocator_adaptor<RebindAlloc<A, T>>;

template <typename A, typename T>
inline T* rebind_alloc(A& alloc, std::size_t n = 1)
{
  using AllocT = RebindAlloc<A, T>;
  AllocT a(alloc);
  auto ptr = std::allocator_traits<AllocT>::allocate(a, n);
  return metall::to_raw_pointer(ptr);
}

template <typename A, typename T>
inline T* rebind_realloc(A& alloc, T* p, std::size_t old_n, std::size_t n)
{
  using AllocT = RebindAlloc<A, T>;
  AllocT a(alloc);
  auto ptr = std::allocator_traits<AllocT>::allocate(a, n);
  std::memcpy(metall::to_raw_pointer(ptr), p, std::min(old_n, n) * sizeof(T));
  std::allocator_traits<AllocT>::deallocate(a, metall::to_raw_pointer(ptr), old_n);
  return metall::to_raw_pointer(ptr);
}

template <typename A, typename P>
inline void rebind_deallocate(A& alloc, P ptr, std::size_t n = 1)
{
  using T = typename std::pointer_traits<P>::element_type;
  using AllocT = RebindAlloc<A, T>;
  AllocT a(alloc);

  std::allocator_traits<AllocT>::deallocate(a, ptr, n);
}

template <typename A, typename T, typename... Args>
inline T* rebind_construct(A& alloc, Args&&... args)
{
  T* ptr = rebind_alloc<A, T>(alloc, 1);

  using AllocT = RebindAlloc<A, T>;
  AllocT a(alloc);
  std::allocator_traits<AllocT>::construct(a, ptr, std::forward<Args>(args)...);

  return ptr;
}

template <typename A, typename P>
inline void rebind_delete(A& alloc, P ptr)
{
  using T = typename std::pointer_traits<P>::element_type;
  ptr->~T();
  rebind_deallocate(alloc, ptr);
}