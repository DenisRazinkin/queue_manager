#pragma once

#ifndef MQP_COMMON_H_
#define MQP_COMMON_H_

#include "common.h"

namespace qm
{

template< typename Value>
class IQueue;

/// @brief Alias name for shared pointer to queue
template<typename Value>
using QueuePtr = std::shared_ptr< IQueue< Value > >;

template< typename Value>
class IConsumer;

/// @brief Alias name for shared pointer to consumer
template< typename Value>
using ConsumerPtr = std::shared_ptr< IConsumer < Value > >;

template<typename Key, typename Value>
class IProducer;

/// @brief Alias name for shared pointer to producer
template<typename Key, typename Value>
using ProducerPtr = std::shared_ptr< IProducer< Key, Value > >;

template<typename Key, typename Value >
class IMultiQueueManager;

/// @brief Alias name for shared pointer to queue manager
template<typename Key, typename Value >
using ManagerPtr = std::shared_ptr< IMultiQueueManager< Key, Value > >;

template < typename  T > using Ptr = std::shared_ptr< T >;
template < typename  T, typename... Args >
Ptr<T> MakePtr( Args... args )
{
     return std::make_shared< T >( args... );
}

} // qm

#endif // MQP_COMMON_H_
