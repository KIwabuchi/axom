/*
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Copyright (c) 2017-2018, Lawrence Livermore National Security, LLC.
 *
 * Produced at the Lawrence Livermore National Laboratory
 *
 * LLNL-CODE-741217
 *
 * All rights reserved.
 *
 * This file is part of Axom.
 *
 * For details about use and distribution, please read axom/LICENSE.
 *
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */

#ifndef MINT_ConnectivityArray_HPP_
#define MINT_ConnectivityArray_HPP_

// Axom includes
#include "axom/Macros.hpp"
#include "axom/Types.hpp"

// Mint includes
#include "mint/Array.hpp"
#include "mint/CellTypes.hpp"
#include "mint/ConnectivityArrayHelpers.hpp"
#include "mint/config.hpp"

// Slic includes
#include "slic/slic.hpp"

#ifdef MINT_USE_SIDRE
#include "sidre/sidre.hpp"
#endif

namespace axom
{

namespace mint
{

enum ConnectivityType
{
  NO_INDIRECTION,
  INDIRECTION,
  TYPED_INDIRECTION
};

/*!
 * \class ConnectivityArray
 *
 * \brief Provides an interface for general mesh connectivity.
 *
 *  The ConnectivityArray is a map between IDs and values where each
 *  ID can be a different type and have a different number of values.
 *  A ConnectivityArray with  N IDs has IDs from [0, N-1] whereas the values
 *  for each ID can be anything within the range of IndexType.
 *
 *  The ConnectivityArray object may be constructed using (a) native storage,
 *  (b) external storage, or, (c) Sidre:
 *
 *  * <b> Native Storage </b> <br />
 *
 *    When using native storage, the ConnectivityArray object owns all
 *    associated memory. The storage can dynamically grow as needed, e.g., when
 *    adding more cells. Typically, extra space is allocated to minimize the
 *    number of re-allocations. At any given instance, the total ID/value
 *    capacity can be queried by calling the getIDCapacity()/getValueCapacity()
 *    functions. The extra memory can be returned to the system by calling the
 *    shrink() method.
 *
 *    When all extra memory is exhausted, appending a new ID triggers a
 *    re-allocation. The amount of extra space that is allocated is according
 *    to the <em> resize_ratio </em> parameter, which is set to 2.0 by default.
 *    The <em> resize_ratio </em> may be queried and set to a different value
 *    by the getResizeRatio() and setResizeRatio() functions respectively.
 *
 *    When the ConnectivityArray object goes out-of-scope, all memory associated
 *    with the given instance is returned to the system.
 *
 *  * <b> External Storage </b> <br />
 *
 *    A ConnectivityArray object may also be constructed from external,
 *    user-supplied buffers that store the varius arrays. In this case,
 *    the memory is owned by the caller. The ConnectivityArray object just keeps
 *    pointers to the user-supplied buffers.
 *
 *    \warning Since the memory is not owned by the ConnectivityArray object
 *     when external buffers are supplied, the ConnectivityArray object cannot
 *     dynamically grow the storage. Consequently, the number of IDs/values the
 *     ConnectivityArray instance can hold is fixed. All calls to `shrink()` and
 *     `reserve()` will fail.
 *
 *    \warning Moreover, when the ConnectivityArray object goes out-of-scope,
 *     the associated buffers are not deleted. The caller owns the external data
 *     and has the responsibility of properly de-allocating the associated
 *     memory.
 *
 *  * <b> Sidre </b> <br />
 *
 *    A ConnectivityArray object may also be constructed from a sidre::Group
 *    which conforms to a topology of the
 *    <a href="http://llnl-conduit.readthedocs.io/en/latest/">
 *    mesh blueprint </a>.
 *
 *    A ConnectivityArray object that is bound to a particular sidre::Group
 *    supports all operations transparently including dynamically growing the
 *    storage to hold more nodes as needed, but, instead, Sidre owns the memory.
 *    All memory management operations are delegated to Sidre.
 *
 *    \warning Once the ConnectivityArray object goes out-of-scope, the data
 *     stays remains persistent in Sidre.
 *
 * \warning Reallocations tend to be costly operations in terms of performance.
 *  Use `reserveIDs()`/`reserveValues()` when the number of IDs/values is
 *  known a priori, or opt to use a constructor that takes an actual size and
 *  capacity when possible.
 *
 *  In this non-specialized ConnectivityArray it is assumed that each ID is of
 *  the same type and has the same number of values. Template specializations
 *  deal with the case where the number of values per ID differ but the type
 *  remains the same and the case where both differ.
 *
 * \tparam TYPE the type of the ConnectivityArray this class deals with the
 *  case of TYPE == NO_INDIRECTION.
 *
 * \see ConnectivityArray_indirection.hpp
 * \see ConnectivityArray_typed_indirection.hpp
 * \see ConnectivityArray_internal.hpp
 */
template < ConnectivityType TYPE >
class ConnectivityArray
{
  AXOM_STATIC_ASSERT( TYPE == NO_INDIRECTION );

public:

  /*!
   * \brief Default constructor. Disabled.
   */
  ConnectivityArray() = delete;

/// \name Native Storage ConnectivityArray Constructors
/// @{

  /*!
   * \brief Constructs an empty ConnectivityArray instance.
   *
   * \param [in] ID_capacity the number of IDs to allocate space for.
   *
   * \post getIDCapacity() >= getNumberOfIDs()
   * \post getNumberOfIDs() == 0
   * \post getIDType() == TYPE
   */
  ConnectivityArray( CellType cell_type, IndexType ID_capacity=USE_DEFAULT ) :
    m_cell_type( cell_type ),
    m_stride( -1 ),
    m_values( AXOM_NULLPTR )
  {
    SLIC_ERROR_IF( m_cell_type == UNDEFINED_CELL,
                   "Cannot have an undefined cell type." );
    SLIC_ERROR_IF( cellTypeToInt(m_cell_type) >= NUM_CELL_TYPES,
                   "Unknown cell type." );

    m_stride = getCellInfo( cell_type ).num_nodes;
    m_values = new Array< IndexType >( internal::ZERO, m_stride, ID_capacity );
  }

/// @}

/// \name External Storage ConnectivityArray Constructors
/// @{

  /*!
   * \brief External constructor which creates a ConnectivityArray instance to
   *  wrap the given pointer.
   *
   * \param [in] n_IDs the number of IDs.
   * \param [in] values the array of values of length ID_capacity * STRIDE.
   * \param [in] ID_capacity the capacity of the values array in terms of IDs.
   *  If not specified the capacity is set to n_IDs.
   *
   * \pre n_IDs >= 0
   * \pre values != AXOM_NULLPTR
   *
   * \post getIDCapacity() >= getNumberOfIDs()
   * \post getNumberOfIDs() == n_IDs
   * \post getIDType() == TYPE
   */
  ConnectivityArray( CellType cell_type, IndexType n_IDs, IndexType* values,
                     IndexType ID_capacity=USE_DEFAULT ) :
    m_cell_type( cell_type ),
    m_stride( -1 ),
    m_values( AXOM_NULLPTR )
  {
    SLIC_ERROR_IF( m_cell_type == UNDEFINED_CELL,
                   "Cannot have an undefined cell type." );
    SLIC_ERROR_IF( cellTypeToInt(m_cell_type) >= NUM_CELL_TYPES, "Unknown cell type." );

    m_stride = getCellInfo( cell_type ).num_nodes;
    m_values = new Array< IndexType >( values, n_IDs, m_stride, ID_capacity );
  }

/// @}

/// \name Sidre Storage ConnectivityArray Constructors
/// @{

#ifdef MINT_USE_SIDRE

  /*!
   * \brief Creates a ConnectivityArray instance from a sidre::Group which
   *  already has data.
   *
   * \param [in] group the sidre::Group to create the ConnectivityArray from.
   *
   * \note the given Group must conform to a single Blueprint topology.
   *
   * \pre group != AXOM_NULLPTR
   *
   * \post getIDCapacity() >= getNumberOfIDs()
   * \post getIDType() == TYPE
   */
  ConnectivityArray( sidre::Group* group ) :
    m_cell_type( UNDEFINED_CELL ),
    m_stride( -1 ),
    m_values( AXOM_NULLPTR )
  {
    m_cell_type = internal::initializeFromGroup( group, &m_values );

    SLIC_ERROR_IF( m_cell_type == UNDEFINED_CELL,
                   "Cannot have an undefined cell type." );


    m_stride = cell_info[ static_cast< int >( m_cell_type ) ].num_nodes;

    SLIC_ERROR_IF(
      m_values->numComponents() != m_stride,
      "values array must have " << m_stride << " components, is " <<
                   m_values->numComponents() << "." );
  }

  /*!
   * \brief Creates an empty ConnectivityArray instance from an empty
   *  sidre::Group.
   *
   * \param [in] group the sidre::Group to create the ConnectivityArray from.
   * \param [in] coordset the name of the Blueprint coordinate set to associate
   *  this ConnectivityArray with.
   * \param [in] ID_capacity the number of IDs to allocate space for.
   *
   * \pre group != AXOM_NULLPTR
   * \pre group->getNumGroups() == group->getNumViews() == 0
   *
   * \post getIDCapacity() >= getNumberOfIDs()
   * \post getIDType() == TYPE
   */
  ConnectivityArray( CellType cell_type, sidre::Group* group,
                     const std::string& coordset,
                     IndexType ID_capacity=USE_DEFAULT ) :
    m_cell_type( cell_type ),
    m_stride( cell_info[ static_cast< int >( m_cell_type ) ].num_nodes ),
    m_values( AXOM_NULLPTR )
  {
    SLIC_ERROR_IF( m_cell_type == UNDEFINED_CELL,
                   "Cannot have an undefined cell type." );


    internal::initializeGroup( group, coordset, m_cell_type );

    sidre::Group* elems_group = group->getGroup( "elements" );
    SLIC_ASSERT( elems_group != AXOM_NULLPTR );

    sidre::View* connec_view = elems_group->getView( "connectivity" );
    m_values = new Array< IndexType >( connec_view, 0, m_stride, ID_capacity );
    SLIC_ASSERT( m_values != AXOM_NULLPTR );
  }

#endif

/// @}

  /*!
   * \brief Destructor, free's the allocated vector.
   */
  ~ConnectivityArray()
  {
    if ( m_values != AXOM_NULLPTR )
    {
      delete m_values;
    }
    m_values = AXOM_NULLPTR;
  }

/// \name Attribute get/set Methods
/// @{

  /*!
   * \brief Returns the total number of IDs.
   */
  IndexType getNumberOfIDs() const
  { return m_values->size(); }

  /*!
   * \brief Return the number of IDs available for storage without resizing.
   */
  IndexType getIDCapacity() const
  { return m_values->capacity(); }

  /*!
   * \brief Returns the number of values in this ConnectivityArray instance.
   */
  IndexType getNumberOfValues() const
  { return m_values->size() * m_stride; }

  /*!
   * \brief Return the number of values available for storage without resizing.
   */
  IndexType getValueCapacity() const
  { return getIDCapacity() * m_stride; }

  /*!
   * \brief Reserve space for IDs and values.
   *
   * \param [in] ID_capacity the number of IDs to reserve space for.
   * \param [in] value_capacity not used, does not need to be specified.
   *
   * \post getIDCapacity() >= n_IDs
   */
  void reserve( IndexType ID_capacity,
                IndexType AXOM_NOT_USED(value_capacity)=0 )
  { m_values->reserve( ID_capacity ); }

  /*!
   * \brief Shrink the array so that there is no extra capacity.
   *
   * \post getIDCapacity() == getNumberOfIDs()
   */
  void shrink()
  { m_values->shrink(); }

  /*!
   * \brief Get the resize ratio.
   */
  double getResizeRatio() const
  { return m_values->getResizeRatio(); }

  /*!
   * \brief Set the resize ratio.
   *
   * \param [in] ratio the new resize ratio.
   *
   * \post getResizeRatio() == ratio
   */
  void setResizeRatio( double ratio )
  { m_values->setResizeRatio( ratio ); }

  /*!
   * \brief Checks if this CellConnecitivity instance has a variable number of
   *  values per ID.
   * \return false.
   */
  bool hasVariableValuesPerID() const
  { return false; }

  /*!
   * \brief Checks if this ConnectivityArray instance is empty.
   */
  bool empty() const
  { return m_values->empty(); }

  /*!
   * \brief Return true iff constructed via the external constructor.
   */
  bool isExternal() const
  { return m_values->isExternal(); }

  /*!
   * \brief Return true iff constructed via the sidre constructors.
   */
  bool isInSidre() const
  { return m_values->isInSidre(); }

  /*
   * \brief Return a const pointer to the sidre::Group that holds the data
   *  or AXOM_NULLPTR if the data is not in sidre.
   */
#ifdef MINT_USE_SIDRE
  const sidre::Group* getGroup() const
  {
    if ( !isInSidre() )
    {
      return AXOM_NULLPTR;
    }

    return m_values->getView()->getOwningGroup()->getParent();
  }
#endif

/// @}

/// \name Data Access Methods
/// @{

  /*!
   * \brief Returns the number of values for the given ID.
   *
   * \param [in] ID not used, does not need to be specified.
   */
  IndexType getNumberOfValuesForID( IndexType AXOM_NOT_USED(ID)=0 ) const
  { return m_stride; }

  /*!
   * \brief Returns the cell type of the given ID.
   *
   * \param [in] ID not used, does not need to be specified.
   */
  CellType getIDType( IndexType AXOM_NOT_USED(ID)=0 ) const
  { return m_cell_type;  }

  /*!
   * \brief Access operator for the values of the given ID.
   *
   * \param [in] ID the ID in question.
   *
   * \return pointer to the values of the given ID.
   *
   * \pre ID >= 0 && ID < getNumberOfIDs()
   * \post cell_ptr != AXOM_NULLPTR.
   */
  /// @{

  IndexType* operator[]( IndexType ID )
  {
    SLIC_ASSERT( ( ID >= 0 ) && ( ID < getNumberOfIDs() ) );
    return m_values->getData() + ID * m_stride;
  }

  const IndexType* operator[]( IndexType ID ) const
  {
    SLIC_ASSERT( ( ID >= 0 ) && ( ID < getNumberOfIDs() ) );
    return m_values->getData() + ID * m_stride;
  }

  /// @}

  /*!
   * \brief Returns a pointer to the values array, of length
   *getNumberOfValues().
   */
  /// @{

  IndexType* getValuePtr()
  { return m_values->getData(); }

  const IndexType* getValuePtr() const
  { return m_values->getData(); }

  /// @}

  /*!
   * \brief Returns a pointer to the offsets array, of length
   *  getNumberOfIDs() + 1.
   */
  /// @{

  IndexType* getOffsetPtr()
  { return AXOM_NULLPTR; }

  const IndexType* getOffsetPtr() const
  { return AXOM_NULLPTR; }

  /// @}

  /*!
   * \brief Returns a pointer to the types array, of length getNumberOfIDs().
   */
  /// @{

  CellType* getTypePtr()
  { return AXOM_NULLPTR; }

  const CellType* getTypePtr() const
  { return AXOM_NULLPTR; }

  /// @}

  /*!
   * \brief Adds a ID.
   *
   * \param [in] values pointer to the values to add, of length at least STRIDE.
   * \param [in] n_values not used, does not need to be specified.
   * \param [in] type not used, does not need to be specified.
   *
   * \pre values != AXOM_NULLPTR
   */
  void append( const IndexType* values, IndexType AXOM_NOT_USED(n_values)=0,
               CellType AXOM_NOT_USED(type)=UNDEFINED_CELL )
  { appendM( values, 1 ); }

  /*!
   * \brief Adds multiple IDs.
   *
   * \param [in] values pointer to the values to add, of length at least
   *  n_IDs * STRIDE.
   * \param [in] n_IDs the number of IDs to append.
   * \param [in] offsets not used, does not need to be specified.
   * \param [in] types not used, does not need to be specified.
   *
   * \pre values != AXOM_NULLPTR
   * \pre n_IDs >= 0
   */
  void appendM( const IndexType* values, IndexType n_IDs,
                const IndexType* AXOM_NOT_USED(offsets)=AXOM_NULLPTR,
                const CellType* AXOM_NOT_USED(types)=AXOM_NULLPTR )
  {
    SLIC_ASSERT( values != AXOM_NULLPTR );
    SLIC_ASSERT( n_IDs >= 0 );
    m_values->append( values, n_IDs );
  }

  /*!
   * \brief Sets the values of the given ID.
   *
   * \param [in] values pointer to the values to set, of length at least STRIDE.
   * \param [in] ID the ID of the values to set.
   *
   * \pre ID >= 0 && ID < getNumberOfIDs()
   * \pre values != AXOM_NULLPTR
   */
  void set( const IndexType* values, IndexType ID )
  {
    SLIC_ASSERT( ID >= 0 );
    SLIC_ASSERT( ID < getNumberOfIDs() );
    SLIC_ASSERT( values != AXOM_NULLPTR );
    m_values->set( values, 1, ID );
  }

  /*!
   * \brief Sets the values of multiple IDs starting with the given ID.
   *
   * \param [in] values pointer to the values to set, of length at least
   *  n_IDs * STRIDE.
   * \param [in] start_ID the ID to start at.``
   * \param [in] n_IDs the number of IDs to set.
   *
   * \pre start_ID >= 0 && start_ID + n_IDs < getNumberOfIDs()
   * \pre values != AXOM_NULLPTR
   */
  void setM( const IndexType* values, IndexType start_ID, IndexType n_IDs )
  {
    SLIC_ASSERT( start_ID >= 0 );
    SLIC_ASSERT( start_ID + n_IDs <= getNumberOfIDs() );
    SLIC_ASSERT( values != AXOM_NULLPTR );
    m_values->set( values, n_IDs, start_ID );
  }

  /*!
   * \brief Insert the values of a new ID before the given ID.
   *
   * \param [in] values pointer to the values to insert, of length at least
   *  STRIDE.
   * \param [in] start_ID the ID to start at.
   * \param [in] n_values not used, does not need to be specified.
   * \param [in] type not used, does not need to be specified.
   *
   * \pre start_ID >= 0 && start_ID <= getNumberOfIDs()
   * \pre values != AXOM_NULLPTR
   */
  void insert( const IndexType* values, IndexType ID,
               IndexType AXOM_NOT_USED(n_values)=0,
               CellType AXOM_NOT_USED(type)=UNDEFINED_CELL )
  { insertM( values, ID, 1 ); }

  /*!
   * \brief Insert the values of multiple IDs before the given ID.
   *
   * \param [in] values pointer to the values to set, of length at least
   *  n_IDs * STRIDE.
   * \param [in] start_ID the ID to start at.
   * \param [in] n_IDs the number of IDs to set.
   * \param [in] offsets not used, does not need to be specified.
   * \param [in] types not used, does not need to be specified.
   *
   * \pre start_ID >= 0 && start_ID <= getNumberOfIDs()
   * \pre n_IDs >= 0
   * \pre values != AXOM_NULLPTR
   */
  void insertM( const IndexType* values, IndexType start_ID, IndexType n_IDs,
                const IndexType* AXOM_NOT_USED(offsets)=AXOM_NULLPTR,
                const CellType* AXOM_NOT_USED(types)=AXOM_NULLPTR )
  {
    SLIC_ASSERT( start_ID >= 0 );
    SLIC_ASSERT( start_ID <= getNumberOfIDs() );
    SLIC_ASSERT( values != AXOM_NULLPTR );
    m_values->insert( values, n_IDs, start_ID );
  }

/// @}

private:

  CellType m_cell_type;
  IndexType m_stride;
  Array< IndexType >* m_values;

  DISABLE_COPY_AND_ASSIGNMENT( ConnectivityArray );
  DISABLE_MOVE_AND_ASSIGNMENT( ConnectivityArray );
};


} /* namespace mint */
} /* namespace axom */

#include "mint/ConnectivityArray_indirection.hpp"
#include "mint/ConnectivityArray_typed_indirection.hpp"

#endif /* MINT_ConnectivityArray_HPP_ */
