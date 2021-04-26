#pragma once

#ifndef MQP_COMMON_H_
#define MQP_COMMON_H_

#include "common.h"

namespace qm
{

// forward declaration for alias ConsumerPtr
template<typename Key, typename Value>
class IConsumer;

template<typename Key, typename Value>
using ConsumerPtr = std::shared_ptr< IConsumer < Key, Value > >;

template<typename Key, typename Value>
class IProducer;

template<typename Key, typename Value>
using ProducerPtr = std::shared_ptr< IProducer< Key, Value > >;

template<typename Key, typename Value >
class IMultiQueueManager;

template<typename Key, typename Value >
using ManagerPtr = std::shared_ptr< IMultiQueueManager< Key, Value > >;

template < typename  T > using Ptr = std::shared_ptr< T >;
template < typename  T, typename... Args >
Ptr<T> MakePtr( Args... args )
{
     return std::make_shared< T >( args... );
}

// CRTP - curiously recurring template pattern
template< typename T >
struct SharedFactory
{
     using PointerType = std::shared_ptr< T >;

     template< typename... Args >
     static PointerType Make( Args&&... args )
     {
          return std::make_shared< T > ( std::forward< Args > ( args )... );
     }

     static PointerType Make()
     {
          return std::make_shared< T > ();
     }
};

} // qm

#endif // MQP_COMMON_H_
