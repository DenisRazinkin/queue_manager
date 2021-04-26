#pragma once

#ifndef MQP_MULTI_QUEUE_MANAGER_H_
#define MQP_MULTI_QUEUE_MANAGER_H_

#include "base_mqueue_manager.cpp"

#include <mutex>

#include "common.h"

namespace qm
{


template< typename Key, typename Value >
class IMultiQueueManager
{
public:
     using Producers = boost::container::flat_multimap< Key, ProducerPtr< Key, Value > >;
     using Consumers = boost::container::flat_multimap< Key, ConsumerPtr< Key, Value > >;
     using Queues = boost::container::flat_map< Key, QueuePtr< Value > >;

     explicit IMultiQueueManager() = default;

     virtual ~IMultiQueueManager() = default;

     // forbid copying
     IMultiQueueManager(const IMultiQueueManager & ) = delete;
     IMultiQueueManager & operator =(const IMultiQueueManager & ) = delete;

public:
     virtual State Subscribe( ConsumerPtr< Key, Value > consumer, Key id ) = 0;
     virtual State Unsubscribe( Key id ) = 0;

     virtual State Enqueue( const Key &id, const Value &value ) = 0;
     virtual State Enqueue( Key &&id, Value &&value ) = 0;

public:
     State AddQueue( Key id, QueuePtr< Value > queue )
     {
          std::scoped_lock lock(IMultiQueueManager< Key, Value >::mtx_ );
          if (IMultiQueueManager< Key, Value >::queues_.find(id ) == IMultiQueueManager< Key, Value >::queues_.end() )
          {
               IMultiQueueManager< Key, Value >::queues_.emplace(id, queue );
               return State::Ok;
          }

          return State::QueueBusy;
     }

     State RemoveQueue( Key id )
     {
          std::scoped_lock lock(IMultiQueueManager< Key, Value >::mtx_ );
          auto it = IMultiQueueManager< Key, Value >::queues_.find(id );
          if (it != IMultiQueueManager< Key, Value >::queues_.end() )
          {
               it->second->Enabled( false );
               IMultiQueueManager< Key, Value >::queues_.erase(it );
          }
          else
          {
               return State::QueueAbsent;
          }

          Unsubscribe( id );

          return State::Ok;
     }

     QueueResult<Value> GetQueue( Key id ) const
     {
          std::scoped_lock lock(IMultiQueueManager< Key, Value >::mtx_ );
          auto it = IMultiQueueManager< Key, Value >::queues_.find(id );
          return it == IMultiQueueManager< Key, Value >::queues_.end() ? QueueResult<Value>{nullptr, State::QueueAbsent } :
                 QueueResult<Value>{ it->second, State::Ok };
     }

     bool AreAllQueuesEmpty() const
     {
          std::scoped_lock lock(IMultiQueueManager< Key, Value >::mtx_ );
          return std::all_of(IMultiQueueManager< Key, Value >::queues_.begin(), IMultiQueueManager< Key, Value >::queues_.end(),
                             [] ( const auto& queue ) { return queue.second && queue.second->Empty(); } );
     }

     bool AreAllProducersDone() const
     {
          std::scoped_lock lock(IMultiQueueManager< Key, Value >::mtx_ );
          return std::all_of(IMultiQueueManager< Key, Value >::producers_.begin(), IMultiQueueManager< Key, Value >::producers_.end(),
                             [] ( const auto& producer ) { return producer.second && producer.second->Done(); } );
     }

     virtual State RegisterProducer( ProducerPtr< Key, Value > producer, Key id )
     {
          std::scoped_lock lock_guard( mtx_ );

          auto queue_result = GetQueue( id );
          if ( queue_result.s_ != State::Ok )
          {
               return queue_result.s_;
          }

          producer->SetQueue( queue_result.queue_ );
          if ( !ProducerRegistrationAllowed( id ) )
          {
               return State::QueueBusy;
          }

          producers_.emplace( id, producer );
          return State::Ok;
     }

protected:
     virtual bool ProducerRegistrationAllowed( Key id ) const = 0;

protected:
     mutable std::recursive_mutex mtx_;

     Queues queues_;
     Producers producers_;
     Consumers consumers_;
};

} // qm

#endif // MQP_QUEUE_MANAGER_H_