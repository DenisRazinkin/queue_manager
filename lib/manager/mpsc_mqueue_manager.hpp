#pragma once

#ifndef MQP_MPSC_QUEUE_MANAGER_H_
#define MQP_MPSC_QUEUE_MANAGER_H_

#include <atomic>

#include <map>
#include <thread>

#include <boost/container/flat_map.hpp>

#include "queue/base_queue.hpp"
#include "consumer/base_consumer.hpp"
#include "manager/base_mqueue_manager.hpp"

namespace qm
{

/// @brief SMulti producer single consumer queue manager.
/// Used when required to create consumer thread for each queue and important that values will proceed consistently.
/// @tparam Key Type for queues map store. Key must be comparable by operator<
/// @tparam Value Type for queue store
template<typename Key, typename Value>
class MPSCQueueManager : public IMultiQueueManager< Key, Value >
{
public:
     /// @brief multi producer single consumer manager constructor
     explicit MPSCQueueManager() = default;

     /// @brief destructor
     virtual ~MPSCQueueManager();

     /// @brief Subscribe consumer from queue
     /// Starts new thread with
     /// @param id
     /// @param consumer
     /// @return
     State Subscribe( Key id, ConsumerPtr< Value > consumer ) override;

     /// @brief Unsubscribe consumer from queue
     /// @param id Key to find queue
     /// @return State value
     /// @attention Unsubscribe from blocking queue may lead to producers threads locks waiting for free space.
     /// If queue is not needed anymore, should use RemoveQueue method from base class.
     State Unsubscribe( Key id ) override;

     /// @brief Unsubscribe consumer from queue. Behaviour equal to function without consumer arg.
     /// @param id Key to find queue
     /// @param consumer Consumer ptr is ignored.
     /// @return State value
     /// @attention Unsubscribe from blocking queue may lead to producers threads locks waiting for free space.
     /// If queue is not needed anymore, should use RemoveQueue method from base class.
     State Unsubscribe( Key id, ConsumerPtr < Value> consumer ) override;

protected:
     bool ProducerRegistrationAllowed( Key ) const override;

private:
     boost::container::flat_map< Key, std::thread > consumer_threads_;
};

template<typename Key, typename Value>
MPSCQueueManager< Key, Value >::~MPSCQueueManager()
{
     IMultiQueueManager< Key, Value >::StopProcessing();
     for ( auto &thread : consumer_threads_ )
     {
          thread.second.join();
     }
}

template<typename Key, typename Value>
State MPSCQueueManager< Key, Value >::Subscribe( Key id, ConsumerPtr< Value > consumer )
{
     std::scoped_lock lock( IMultiQueueManager< Key, Value >::mtx_ );
     if ( IMultiQueueManager< Key, Value >::consumers_.find( id ) !=
          IMultiQueueManager< Key, Value >::consumers_.end() )
     {
          return State::QueueBusy;
     }

     auto queue_it = IMultiQueueManager< Key, Value >::queues_.find( id );
     if ( queue_it == IMultiQueueManager< Key, Value >::queues_.end())
     {
          return State::QueueAbsent;
     }

     IMultiQueueManager< Key, Value >::consumers_.emplace( id, consumer );

     auto queue = queue_it->second;
     auto thread_lambda = [ this, id, queue, consumer ]()
     {
          while ( ( consumer->Enabled() && IMultiQueueManager< Key, Value >::is_enabled_ && queue->Enabled()) ||
                    !queue->Empty() )
          {
               auto value = queue->Pop();
               if ( value.has_value())
               {
                    consumer->Consume( value.value());
               }
          }
     };
     consumer_threads_.emplace( id, std::thread( thread_lambda ) );

     return State::Ok;
}

template<typename Key, typename Value>
bool MPSCQueueManager< Key, Value >::ProducerRegistrationAllowed( Key ) const
{
     // the registration is always enabled for multi producers manager
     return true;
}

template<typename Key, typename Value>
State MPSCQueueManager< Key, Value >::Unsubscribe( Key id )
{
     std::scoped_lock lock( IMultiQueueManager< Key, Value >::mtx_ );
     auto consumer = IMultiQueueManager< Key, Value >::consumers_.find( id );
     if ( consumer != IMultiQueueManager< Key, Value >::consumers_.end())
     {
          consumer->second->Enabled( false );

          auto thread = consumer_threads_.find( id );
          if ( thread != consumer_threads_.end())
          {
               thread->second.join();
          }

          IMultiQueueManager< Key, Value >::consumers_.erase( id );
          return State::Ok;
     }

     return State::QueueAbsent;
}

template<typename Key, typename Value>
State MPSCQueueManager< Key, Value >::Unsubscribe( Key id, ConsumerPtr< Value > )
{
     return Unsubscribe( id );
}



} // namespace qm

#endif // MQP_MPSC_QUEUE_MANAGER_H_