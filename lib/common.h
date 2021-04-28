#pragma once

#ifndef MQP_COMMON_H_
#define MQP_COMMON_H_

#include "common.h"

namespace qm
{

template< typename Value>
class IConsumer;

template< typename Value>
using ConsumerPtr = std::shared_ptr< IConsumer < Value > >;

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

} // qm

#endif // MQP_COMMON_H_
