#pragma once

#ifndef MQP_QUEUE_MANAGER_H_
#define MQP_QUEUE_MANAGER_H_

#include "base_queue_manager.cpp"

#include <mutex>

#include "common.h"

namespace qm
{

//template<typename Key, typename Value, typename ProduceConsumeModel, typename ThreadModel >
template< typename Key, typename Value >
class IQueueManager
{
public:
     using Producers = boost::container::flat_multimap< Key, ProducerPtr< Key, Value > >;
     using Consumers = boost::container::flat_multimap< Key, ConsumerPtr< Key, Value > >;
     using Queues = boost::container::flat_map< Key, QueuePtr< Value > >;

     explicit IQueueManager() = default;

     virtual ~IQueueManager() = default;

     // forbid copying
     IQueueManager( const IQueueManager & ) = delete;
     IQueueManager & operator =( const IQueueManager & ) = delete;

public:
     virtual State Subscribe( ConsumerPtr< Key, Value > consumer, Key id ) = 0;
     virtual State Unsubscribe( Key id ) = 0;

     virtual State Enqueue( const Key &id, const Value &value ) = 0;
     virtual State Enqueue( Key &&id, Value &&value ) = 0;

public:
     State AddQueue( Key id, QueuePtr< Value > queue )
     {
          std::scoped_lock lock( IQueueManager< Key, Value >::mtx_ );
          if ( IQueueManager< Key, Value >::queues_.find( id ) == IQueueManager< Key, Value >::queues_.end() )
          {
               IQueueManager< Key, Value >::queues_.emplace( id, queue );
               return State::Ok;
          }

          return State::QueueBusy;
     }

     State RemoveQueue( Key id )
     {
          std::scoped_lock lock( IQueueManager< Key, Value >::mtx_ );
          auto it = IQueueManager< Key, Value >::queues_.find( id );
          if ( it != IQueueManager< Key, Value >::queues_.end() )
          {
               it->second->Enabled( false );
               IQueueManager< Key, Value >::queues_.erase( it );
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
          std::scoped_lock lock( IQueueManager< Key, Value >::mtx_ );
          auto it = IQueueManager< Key, Value >::queues_.find( id );
          return it == IQueueManager< Key, Value >::queues_.end() ? QueueResult<Value>{ nullptr, State::QueueAbsent } :
                 QueueResult<Value>{ it->second, State::Ok };
     }

     bool AreAllQueuesEmpty() const
     {
          std::scoped_lock lock( IQueueManager< Key, Value >::mtx_ );
          return std::all_of( IQueueManager< Key, Value >::queues_.begin(), IQueueManager< Key, Value >::queues_.end(),
                              [] ( const auto& queue ) { return queue.second && queue.second->Empty(); } );
     }

     bool AreAllProducersDone() const
     {
          std::scoped_lock lock( IQueueManager< Key, Value >::mtx_ );
          return std::all_of( IQueueManager< Key, Value >::producers_.begin(), IQueueManager< Key, Value >::producers_.end(),
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