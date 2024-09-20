// Copyright (c) 2017-2024, Lawrence Livermore National Security, LLC and
// other Axom Project Developers. See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: (BSD-3-Clause)

/*!
 ******************************************************************************
 *
 * \file ItemCollectionUmbrella.hpp
 *
 * \brief   Header file for ItemCollectionUmbrella.
 *
 *          This is a templated abstract base class defining an interface for
 *          classes holding a collection of items of a fixed
 *          type that can be accessed by string name or sidre::IndexType.
 *
 *          The primary intent is to decouple the implementation of the
 *          collections from the Group class which owns collections of
 *          View and child Group objects. They may have other uses,
 *          so they are not dependent on the Group class. This class is
 *          templated on the item type so that derived classes can be used
 *          to hold either View or Group object pointers without
 *          having to code a separate class for each.
 *
 *          Derived implemenations of this class can be used to explore
 *          alternative collection implementations for performance
 *          (insertion, lookup, etc.) and memory overhead.
 *
 *          \attention These classes should be robust against any potential
 *                     user interaction. They don't report errors and leave
 *                     checking of return values to calling code.
 *
 *          \attention The interface defined by this class is as follows:
 *
 *          \verbatim
 *
 *          - // Return number of items in collection.
 *
 *               size_t getNumItems() const;
 *
 *          - // Return first valid item index for iteration.
 *            // sidre::InvalidIndex returned if no items in collection
 *
 *               IndexType getFirstValidIndex() const;
 *
 *          - // Return next valid item index for iteration.
 *            // sidre::InvalidIndex returned if there are no more items
 *            // to be iterated over.
 *
 *               IndexType getNextValidIndex(IndexType idx) const;
 *
 *          - // Return true if item with given name in collection; else false.
 *
 *               bool hasItem(const std::string& name) const;
 *
 *          - // Return true if item with given index in collection; else false.
 *
 *               bool hasItem(IndexType idx) const;
 *
 *          - // Return pointer to item with given name (nullptr if none).
 *
 *               T* getItem(const std::string& name);
 *               T const* getItem(const std::string& name) const ;
 *
 *          - // Return pointer to item with given index (nullptr if none).
 *
 *               T* getItem(IndexType idx);
 *               T const* getItem(IndexType idx) const;
 *
 *          - // Return name of object with given index
 *            // (sidre::InvalidName if none).
 *
 *               std::string getItemName(IndexType idx) const;
 *
 *          - // Return index of object with given name
 *            // (sidre::InvalidIndex if none).
 *
 *               IndexType getItemIndex(const std::string& name) const;
 *
 *          - // Insert item with given name; return index if insertion
 *            // succeeded, and InvalidIndex otherwise.
 *
 *               IndexType insertItem(T* item, const std::string& name);
 *
 *          - // Remove item with given name if it exists and return a
 *            // pointer to it. If it doesn't exist, return nullptr.
 *
 *               T* removeItem(const std::string& name);
 *
 *          - // Remove item with given index if it exists and return a
 *            // pointer to it. If it doesn't exist, return nullptr.
 *
 *               T* removeItem(IndexType idx);
 *
 *          - // Remove all items (items not destroyed).
 *
 *               void removeAllItems();
 *
 *          - // Clear all items and destroy them.
 *
 *               void deleteAllItems();
 *
 *          \endverbatim
 *
 ******************************************************************************
 */

#ifndef SIDRE_ITEMCOLLECTIONS_HPP_
#define SIDRE_ITEMCOLLECTIONS_HPP_

// Other axom headers
#include "axom/config.hpp"
#include "axom/core/Types.hpp"
#include "axom/core/IteratorBase.hpp"

// Sidre project headers
#include "SidreTypes.hpp"
#include "Memory.hpp"
#include "IndexedCollectionCore.hpp"
#include "ListCollectionCore.hpp"
#include "MapCollectionCore.hpp"

namespace axom
{
namespace sidre
{
/*!
 *************************************************************************
 *
 * \class ItemCollectionUmbrella
 *
 * \brief ItemCollectionUmbrella is an abstract base class template for holding
 *        a collection of items of template parameter type T.  Derived
 *        child classes can determine how to specifically store the items.
 *
 *************************************************************************
 */
template <typename T>
class ItemCollectionUmbrella
{
public:
  using value_type = T;

  // Forward declare iterator classes and helpers
  class iterator;
  class const_iterator;
  class iterator_adaptor;
  class const_iterator_adaptor;

  using AllocatorType = metall::manager::fallback_allocator<void>;
  using VoidPtr = Ptr<typename AllocatorType::pointer, void>;

  using IndexedCollectionType = IndexedCollectionCore<T>;
  using MapCollectionType = MapCollectionCore<T>;
  using ListCollectionType = ListCollectionCore<T>;

  enum store_type { invalid, index, map, list };

 public:
  ItemCollectionUmbrella(const store_type t,
                         const AllocatorType& alloc)
   : m_type(t),
     m_alloc(alloc)
   {
    if (m_type == store_type::index) {
      m_index = rebind_construct<AllocatorType, IndexedCollectionType>(m_alloc, m_alloc);
    } else if (m_type == store_type::list) {
      m_list = rebind_construct<AllocatorType, ListCollectionType>(m_alloc, m_alloc);
    } else if (m_type == store_type::map) {
      m_map = rebind_construct<AllocatorType, MapCollectionType>(m_alloc, m_alloc);
    } else {
      assert(false);
    }
  }

  ~ItemCollectionUmbrella() {
    rebind_deallocate(m_alloc, m_index);
    m_index = nullptr;

    rebind_deallocate(m_alloc, m_list);
    m_list = nullptr;

    rebind_deallocate(m_alloc, m_map);
    m_map = nullptr;
  }

  //
  // Default compiler-generated ctor, dtor, copy ctor, and copy assignment
  // operator suffice for this class.
  //

  ///
  size_t getNumItems() const
  {
    if (m_type == store_type::index) {
      return m_index->getNumItems();
    } else if (m_type == store_type::list) {
      return m_list->getNumItems();
    } else if (m_type == store_type::map) {
      return m_map->getNumItems();
    }
    assert(false);
  }

  ///
  IndexType getFirstValidIndex() const
  {
    if (m_type == store_type::index) {
      return m_index->getFirstValidIndex();
    } else if (m_type == store_type::list) {
      return m_list->getFirstValidIndex();
    } else if (m_type == store_type::map) {
      return m_map->getFirstValidIndex();
    }
    assert(false);
  }

  ///
  IndexType getNextValidIndex(IndexType idx) const
  {
    if (m_type == store_type::index) {
      return m_index->getNextValidIndex(idx);
    } else if (m_type == store_type::list) {
      return m_list->getNextValidIndex(idx);
    } else if (m_type == store_type::map) {
      return m_map->getNextValidIndex(idx);
    }
    assert(false);
  }

  ///
  bool hasItem(IndexType idx) const
  {
    if (m_type == store_type::index) {
      return m_index->hasItem(idx);
    } else if (m_type == store_type::list) {
      return m_list->hasItem(idx);
    } else if (m_type == store_type::map) {
      return m_map->hasItem(idx);
    }
    assert(false);
  }

  bool hasItem(const std::string& name) const
  {
    if (m_type == store_type::map) {
      return m_map->hasItem(name);
    }
    assert(false);
  }

  ///
  T* getItem(IndexType idx)
  {
    if (m_type == store_type::index) {
      return m_index->getItem(idx);
    } else if (m_type == store_type::list) {
      return m_list->getItem(idx);
    } else if (m_type == store_type::map) {
      return m_map->getItem(idx);
    }
    assert(false);
  }

  ///
  T const* getItem(IndexType idx) const
  {
    if (m_type == store_type::index) {
      return m_index->getItem(idx);
    } else if (m_type == store_type::list) {
      return m_list->getItem(idx);
    } else if (m_type == store_type::map) {
      return m_map->getItem(idx);
    }
    assert(false);
  }

  T* getItem(const std::string& name)
  {
    if (m_type == store_type::map) {
      return m_map->getItem(name);
    }
    assert(false);
  }

  T const* getItem(const std::string& name) const
  {
    if (m_type == store_type::map) {
      return m_map->getItem(name);
    }
    assert(false);
  }

  ///
  const std::string& getItemName(IndexType idx) const
  {
    if (m_type == store_type::map) {
      return m_map->getItemName(idx);
    }
    assert(false);
  }

  ///
  IndexType getItemIndex(const std::string& name) const
  {
    if (m_type == store_type::map) {
      return m_map->getItemIndex(name);
    }
    assert(false);
  }

  ///
  IndexType insertItem(T* item, const std::string& name = "")
  {
    if (m_type == store_type::index) {
      return m_index->insertItem(item, name);
    } else if (m_type == store_type::list) {
      return m_list->insertItem(item, name);
    } else if (m_type == store_type::map) {
      return m_map->insertItem(item, name);
    }
    assert(false);
  }

  IndexType insertItem(T* item, IndexType idx)
  {
    if (m_type == store_type::index) {
      return m_index->insertItem(item, idx);
    }
    assert(false);
  }

  ///
  T* removeItem(IndexType idx)
  {
    if (m_type == store_type::index) {
      return m_index->removeItem(idx);
    } else if (m_type == store_type::list) {
      return m_list->removeItem(idx);
    } else if (m_type == store_type::map) {
      return m_map->removeItem(idx);
    }
    assert(false);
  }

  T* removeItem(const std::string& name)
  {
    if (m_type == store_type::map) {
      return m_map->removeItem(name);
    }
    assert(false);
  }

  ///
  void removeAllItems()
  {
    if (m_type == store_type::index) {
      return m_index->removeAllItems();
    } else if (m_type == store_type::list) {
      return m_list->removeAllItems();
    } else if (m_type == store_type::map) {
      return m_map->removeAllItems();
    }
    assert(false);
  }

  IndexType getValidEmptyIndex()
  {
    if (m_type == store_type::index) {
      return m_index->getValidEmptyIndex();
    }
    assert(false);
  }

public:
  iterator begin() { return iterator(this, true); }
  iterator end() { return iterator(this, false); }

  const_iterator cbegin() const { return const_iterator(this, true); }
  const_iterator cend() const { return const_iterator(this, false); }

  const_iterator begin() const { return const_iterator(this, true); }
  const_iterator end() const { return const_iterator(this, false); }

  /// Returns an adaptor wrapping this collection in support of iteration
  iterator_adaptor getIteratorAdaptor() { return iterator_adaptor(this); }

  /// Returns a const adaptor wrapping this collection in support of iteration
  const_iterator_adaptor getIteratorAdaptor() const
  {
    return const_iterator_adaptor(this);
  }

 private:
  store_type m_type{store_type::invalid};
  AllocatorType m_alloc;
  Ptr<VoidPtr, IndexedCollectionType> m_index{nullptr};
  Ptr<VoidPtr, ListCollectionType> m_list{nullptr};
  Ptr<VoidPtr, MapCollectionType> m_map{nullptr};
};

/*!
 * \brief An std-compliant forward iterator for an ItemCollectionUmbrella
 */
template <typename T>

class ItemCollectionUmbrella<T>::iterator : public IteratorBase<iterator, IndexType>
{
private:
  using BaseType = IteratorBase<iterator, IndexType>;
  using CollectionType = ItemCollectionUmbrella<T>;

public:
  // Iterator traits required to satisfy LegacyRandomAccessIterator concept
  // before C++20
  // See: https://en.cppreference.com/w/cpp/iterator/iterator_traits
  using difference_type = IndexType;
  using value_type = typename std::remove_cv<T>::type;
  using reference = T&;
  using pointer = Ptr<VoidPtr, CollectionType>;
  using iterator_category = std::forward_iterator_tag;

public:
  iterator(Ptr<VoidPtr, CollectionType> coll, bool is_first)
    : m_collection(coll)
  {
    SLIC_ASSERT(coll != nullptr);

    BaseType::m_pos = is_first ? coll->getFirstValidIndex() : sidre::InvalidIndex;
  }

  IndexType index() const { return BaseType::m_pos; }

  pointer operator->() { return m_collection->getItem(BaseType::m_pos); }

  reference operator*() { return *m_collection->getItem(BaseType::m_pos); }

private:
  // Remove backwards iteration functions
  using BaseType::operator--;
  using BaseType::operator-=;

protected:
  /// Implementation of advance() as required by IteratorBase
  void advance(IndexType n)
  {
    for(int i = 0; i < n; ++i)
    {
      BaseType::m_pos = m_collection->getNextValidIndex(BaseType::m_pos);
    }
  }

private:
  Ptr<VoidPtr, CollectionType> m_collection;
};

/*!
 * \brief An std-compliant forward iterator for a const ItemCollectionUmbrella
 */
template <typename T>
class ItemCollectionUmbrella<T>::const_iterator
  : public IteratorBase<const_iterator, IndexType>
{
private:
  using BaseType = IteratorBase<const_iterator, IndexType>;
  using CollectionType = ItemCollectionUmbrella<T>;

public:
  // Iterator traits required to satisfy LegacyRandomAccessIterator concept
  // before C++20
  // See: https://en.cppreference.com/w/cpp/iterator/iterator_traits
  using difference_type = IndexType;
  using value_type = typename std::remove_cv<T>::type;
  using reference = const T&;
  using pointer = Ptr<VoidPtr, const CollectionType>;
  using iterator_category = std::forward_iterator_tag;

public:
  const_iterator(Ptr<VoidPtr, const CollectionType> coll, bool is_first)
    : m_collection(coll)
  {
    SLIC_ASSERT(coll != nullptr);

    BaseType::m_pos = is_first ? coll->getFirstValidIndex() : sidre::InvalidIndex;
  }

  IndexType index() const { return BaseType::m_pos; }

  pointer operator->() { return m_collection->getItem(BaseType::m_pos); }

  reference operator*() { return *m_collection->getItem(BaseType::m_pos); }

private:
  // Remove backwards iteration functions
  using BaseType::operator--;
  using BaseType::operator-=;

protected:
  /// Implementation of advance() as required by IteratorBase
  void advance(IndexType n)
  {
    for(int i = 0; i < n; ++i)
    {
      BaseType::m_pos = m_collection->getNextValidIndex(BaseType::m_pos);
    }
  }

private:
  Ptr<VoidPtr, const CollectionType> m_collection;
};

/*!
 * \brief Utility class to wrap an ItemCollectionUmbrella in support of iteration
 */
template <typename T>
class ItemCollectionUmbrella<T>::iterator_adaptor
{
public:
  using CollectionType = ItemCollectionUmbrella<T>;

public:
  iterator_adaptor(Ptr<VoidPtr, CollectionType> coll) : m_collection(coll) { }

  std::size_t size() const
  {
    return m_collection ? m_collection->getNumItems() : 0;
  }

  iterator begin() { return iterator(m_collection, true); }
  iterator end() { return iterator(m_collection, false); }

  const_iterator cbegin() const { return const_iterator(m_collection, true); }
  const_iterator cend() const { return const_iterator(m_collection, false); }

  operator const_iterator_adaptor() const
  {
    return const_iterator_adaptor(m_collection);
  }

private:
  Ptr<VoidPtr, CollectionType> m_collection {nullptr};
};

/*!
 * \brief Utility class to wrap a const ItemCollectionUmbrella in support of iteration
 */
template <typename T>
class ItemCollectionUmbrella<T>::const_iterator_adaptor
{
public:
  using CollectionType = ItemCollectionUmbrella<T>;

public:
  const_iterator_adaptor(const Ptr<VoidPtr, CollectionType> coll)
    : m_collection(coll)
  { }

  std::size_t size() const
  {
    return m_collection ? m_collection->getNumItems() : 0;
  }

  const_iterator begin() { return const_iterator(m_collection, true); }
  const_iterator end() { return const_iterator(m_collection, false); }

  const_iterator cbegin() const { return const_iterator(m_collection, true); }
  const_iterator cend() const { return const_iterator(m_collection, true); }

private:
  const Ptr<VoidPtr, CollectionType> m_collection {nullptr};
};

} /* end namespace sidre */
} /* end namespace axom */

#endif /* SIDRE_ITEMCOLLECTIONS_HPP_ */
