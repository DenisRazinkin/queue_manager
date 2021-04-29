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

     /// @brief Stop all producers and consumers
     void StopProcessing() override;

     /// @brief Enable all consumers and queues, start consumers threads
     void StartProcessing() override;

     /// @brief Subscribe consumer from queue
     /// Starts a new thread that pop data from the bounded queue and executes a handler Consume() for it
     /// @param id id Key to find queue
     /// @param consumer Consumer for subscribe.
     /// @return State value
     /// @details Thread safe
     State Subscribe( Key id, ConsumerPtr< Value > consumer ) override;

     /// @brief Unsubscribe consumer from queue
     /// @param id Key to find queue
     /// @return State value
     /// @attention Unsubscribe from blocking queue may lead to producers threads locks waiting for free space.
     /// If queue is not needed anymore, should use RemoveQueue method from base class.
     /// @details Thread safe
     State Unsubscribe( Key id ) override;

     /// @brief Unsubscribe consumer from queue. Behaviour is equal to function without consumer arg.
     /// @param id Key to find queue
     /// @param consumer Consumer ptr is ignored.
     /// @return State value
     /// @attention Unsubscribe from blocking queue may lead to producers threads locks waiting for free space.
     /// If queue is not needed anymore, should use RemoveQueue method from base class.
     /// @details Thread safe
     State Unsubscribe( Key id, ConsumerPtr < Value> consumer ) override;

protected:
     bool ProducerRegistrationAllowed( Key ) const override;

private:
     State StartConsumerThread( Key id, ConsumerPtr <Value> consumer, QueuePtr <Value> queue );

     boost::container::flat_map< Key, std::thread > consumer_threads_;
};

template<typename Key, typename Value>
MPSCQueueManager< Key, Value >::~MPSCQueueManager()
{
     StopProcessing();
}

template<typename Key, typename Value>
void MPSCQueueManager< Key, Value >::StopProcessing()
{
     IMultiQueueManager< Key, Value >::StopProcessing();
     for ( auto &thread : consumer_threads_ )
     {
          if ( thread.second.joinable() )
          {
               thread.second.join();
          }
     }

     consumer_threads_.clear();
}

template<typename Key, typename Value>
void MPSCQueueManager< Key, Value >::StartProcessing()
{
     if ( IMultiQueueManager< Key, Value >::is_enabled_ )
     {
          return;

     }

     IMultiQueueManager< Key, Value >::StartProcessing();
     std::for_each( IMultiQueueManager< Key, Value >::consumers_.begin(),
                    IMultiQueueManager< Key, Value >::consumers_.end(), [ this ]( auto consumer )
                    {
                         auto queue = IMultiQueueManager< Key, Value >::queues_.find( consumer.first );
                         if ( queue != IMultiQueueManager< Key, Value >::queues_.end() )
                         {
                              StartConsumerThread( consumer.first, consumer.second, queue->second );
                         }
                    } );
}

template<typename Key, typename Value>
State MPSCQueueManager< Key, Value >::StartConsumerThread( Key id, ConsumerPtr< Value > consumer, QueuePtr< Value > queue )
{
     auto thread_lambda = [ this, id, queue, consumer ]()
     {
          while ( ( consumer->Enabled() && IMultiQueueManager< Key, Value >::is_enabled_ && queue->Enabled() ) ||
                 !queue->Empty() )
          {
               auto value = queue->Pop();
               if ( value.has_value() )
               {
                    consumer->Consume( value.value() );
               }
          }
     };

     consumer_threads_.emplace( id, std::thread( thread_lambda ) );
     return State::Ok;
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
     if ( queue_it == IMultiQueueManager< Key, Value >::queues_.end() )
     {
          return State::QueueAbsent;
     }

     IMultiQueueManager< Key, Value >::consumers_.emplace( id, consumer );
     return StartConsumerThread( id, consumer, queue_it->second );
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
     if ( consumer != IMultiQueueManager< Key, Value >::consumers_.end() )
     {
          consumer->second->Enabled( false );

          auto thread = consumer_threads_.find( id );
          if ( thread != consumer_threads_.end() )
          {
               thread->second.join();
          }

          consumer_threads_.erase( id );

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