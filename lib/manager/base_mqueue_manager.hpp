/// @brief Base template class for queues management
/// @author Denis Razinkin
#pragma once

#ifndef MQP_MULTI_QUEUE_MANAGER_H_
#define MQP_MULTI_QUEUE_MANAGER_H_

#include <mutex>

#include "common.h"
#include "producer/base_producer.hpp"

namespace qm
{

/// @brief Base class of multi queues management for multithreading consumers/producers models.
/// @tparam Key Type for queues map store. Key must be comparable by operator<
/// @tparam Value Type for queue store
template<typename Key, typename Value>
class IMultiQueueManager
{
public:
     using Producers = boost::container::flat_multimap< Key, ProducerPtr < Key, Value > >;
     using Consumers = boost::container::flat_multimap< Key, ConsumerPtr < Value > >;
     using Queues = boost::container::flat_map< Key, QueuePtr < Value > >;

     /// @brief Constructor
     explicit IMultiQueueManager();

     /// @brief Destructor
     virtual ~IMultiQueueManager() = default;

     /// @brief Copying is forbidden
     IMultiQueueManager( const IMultiQueueManager & ) = delete;

     // @brief Copying is forbidden
     IMultiQueueManager &operator=( const IMultiQueueManager & ) = delete;

public:
     /// @brief Stop all consumers, unregister all producers and disable queues
     virtual void StopProcessing();

     /// @brief Enable all consumers and queues
     virtual void StartProcessing();

     /// @brief Add new queue for management
     /// @param id Key to access and control queue
     /// @param queue Pointer to queue
     /// @return State value
     /// @details Thread safe
     State AddQueue( Key id, QueuePtr <Value> queue );

     /// @brief Get queue stored with specified id
     /// @param id Key to get queue
     /// @return State value
     /// @details Thread safe
     State RemoveQueue( Key id );

     /// @brief Get queue stored with specified id
     /// @param id Key to get queue
     /// @return State value
     /// @details Thread safe
     QueueResult <Value> GetQueue( Key id ) const;

     /// @brief Check are all queue empty
     /// @return true/false
     /// @details Thread safe
     bool AreAllQueuesEmpty() const;

     /// @brief Check are all producers done
     /// @return true/false
     /// @details Thread safe
     bool AreAllProducersDone() const;

public:
     /// @brief Subscribe new consumer to queue stored with id
     /// @param id Key to find required queue
     /// @param consumer Pointer to consumer
     /// @return State value
     /// @attention Thread-safe is required.
     virtual State Subscribe( Key id, ConsumerPtr < Value> consumer ) = 0;

     /// @brief Unsubscribe all consumers from queue
     /// @param id Key to find required queue
     /// @return State value
     /// @attention Thread-safe is required.
     virtual State Unsubscribe( Key id ) = 0;

     /// @brief Unsubscribe specified consumer from queue with id
     /// @param id Key to find required queue
     /// @param consumer Pointer to consumer
     /// @return State value
     /// @attention Thread-safe is required.
     virtual State Unsubscribe( Key id, ConsumerPtr < Value> consumer ) = 0;

public:
     /// @brief Register new producer for queue with id.
     /// This method sets queue to producer and make possible direct enqueue from producer's thread.
     /// Strongly recommended using instead of direct Enqueue(id,value)
     /// @param id Key to find required queue
     /// @param producer Pointer to producer
     /// @return State value
     /// @details Thread safe.
     virtual State RegisterProducer( Key id, ProducerPtr <Key, Value> producer );

     /// @brief Unregister producer from queue using.
     /// @param id id Key to find required queue
     /// @param producer Pointer to producer
     /// @return State value
     /// @details Thread safe.
     virtual State UnregisterProducer( Key id, ProducerPtr <Key, Value> producer );

     /// @brief Enqueue new value to queue with id
     /// @param id Lvalue key to find queue
     /// @param value Lvalue object to push
     /// @return State value
     /// @details Thread-safe
     /// @attention Use direct enqueue may cause performance reduce.
     /// RegisterProducer is recommended to use with directly push from producer's thread
     State Enqueue( Key &&id, Value &&value );

     /// @brief Enqueue new value to queue with id
     /// @param id Rvalue key to find queue
     /// @param value Rvalue object to push
     /// @return State value
     /// @details Thread-safe is required.
     /// @attention Use direct enqueue may cause performance reduce.
     /// RegisterProducer is recommended to use with directly push from producer's thread
     State Enqueue( const Key &id, const Value &value );

protected:
     /// @brief Method describes policy of new producers registration
     /// @param id key to find queue
     /// @return true/false
     virtual bool ProducerRegistrationAllowed( Key id ) const = 0;

protected:
     mutable std::recursive_mutex mtx_;
     std::atomic< bool > is_enabled_;

     Queues queues_;
     Producers producers_;
     Consumers consumers_;

private:
     template< typename K, typename V >
     State EnqueueFwd( K&& id, V&& value );
};

template<typename Key, typename Value>
State IMultiQueueManager< Key, Value >::AddQueue( Key id, QueuePtr< Value > queue )
{
     std::scoped_lock lock( mtx_ );
     if ( queues_.find( id ) == queues_.end())
     {
          queues_.emplace( id, queue );
          queue->Enabled( true );
          return State::Ok;
     }

     return State::QueueExists;
}

template<typename Key, typename Value>
State IMultiQueueManager< Key, Value >::RegisterProducer( Key id, ProducerPtr< Key, Value > producer )
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

template<typename Key, typename Value>
State IMultiQueueManager< Key, Value >::UnregisterProducer( Key id, ProducerPtr <Key, Value> producer )
{
     std::scoped_lock lock_guard( mtx_ );
     auto queue_result = GetQueue( id );
     if ( queue_result.s_ != State::Ok )
     {
          return queue_result.s_;
     }

     auto range = producers_.equal_range( id );
     for ( auto it = range.first; it != range.second; it++ )
     {
          if ( it->second == producer )
          {
               producer->Enabled( false );
               producer->WaitThreadDone();
               producer->SetQueue( nullptr );
               producers_.erase( it );
               return State::Ok;
          }
     }

     return State::ProducerNotFound;
}

template<typename Key, typename Value>
State IMultiQueueManager< Key, Value >::RemoveQueue( Key id )
{
     std::scoped_lock lock( mtx_ );
     auto it = queues_.find( id );
     if ( it != queues_.end())
     {
          it->second->Enabled( false );
          queues_.erase( it );
     }
     else
     {
          return State::QueueAbsent;
     }

     Unsubscribe( id );
     consumers_.erase( id );

     auto range = producers_.equal_range( id );
     for ( auto p_it = range.first; p_it != range.second; p_it++ )
     {
          p_it->second->Enabled( false );
          p_it->second->WaitThreadDone();
          p_it->second->SetQueue( nullptr );
     }
     producers_.erase( id );

     return State::Ok;
}

template<typename Key, typename Value>
QueueResult <Value> IMultiQueueManager< Key, Value >::GetQueue( Key id ) const
{
     std::scoped_lock lock( IMultiQueueManager< Key, Value >::mtx_ );
     auto it = IMultiQueueManager< Key, Value >::queues_.find( id );
     return it == IMultiQueueManager< Key, Value >::queues_.end() ?
            QueueResult< Value >{ nullptr, State::QueueAbsent } :
            QueueResult< Value >{ it->second, State::Ok };
}

template<typename Key, typename Value>
bool IMultiQueueManager< Key, Value >::AreAllQueuesEmpty() const
{
     std::scoped_lock lock( IMultiQueueManager< Key, Value >::mtx_ );
     return std::all_of( IMultiQueueManager< Key, Value >::queues_.begin(),
                         IMultiQueueManager< Key, Value >::queues_.end(),
                         []( const auto &queue )
                         {
                              return queue.second && queue.second->Empty();
                         } );
}

template<typename Key, typename Value>
bool IMultiQueueManager< Key, Value >::AreAllProducersDone() const
{
     std::scoped_lock lock( IMultiQueueManager< Key, Value >::mtx_ );
     return std::all_of( IMultiQueueManager< Key, Value >::producers_.begin(),
                         IMultiQueueManager< Key, Value >::producers_.end(),
                         []( const auto &producer )
                         {
                              return producer.second && producer.second->Done();
                         } );
}

template<typename Key, typename Value>
State IMultiQueueManager< Key, Value >::Enqueue( const Key &id, const Value &value )
{
     return EnqueueFwd( id, value );
}

template<typename Key, typename Value>
State IMultiQueueManager< Key, Value >::Enqueue( Key &&id, Value &&value )
{
     return EnqueueFwd( std::move( id ), std::move( value ) );
}

template<typename Key, typename Value>
template<typename K, typename V>
State IMultiQueueManager< Key, Value >::EnqueueFwd( K &&id, V &&value )
{
     std::lock_guard<std::recursive_mutex> lock(IMultiQueueManager< Key, Value >::mtx_ );
     auto queue = IMultiQueueManager< Key, Value >::queues_.find(std::forward< K >(id ) );
     if (queue != IMultiQueueManager< Key, Value >::queues_.end() )
     {
          return queue->second->TryPush( std::forward< V >( value ) );
     }

     return State::QueueAbsent;
}

template<typename Key, typename Value>
void IMultiQueueManager< Key, Value >::StopProcessing()
{
     is_enabled_ = false;
     std::for_each( IMultiQueueManager< Key, Value >::queues_.begin(),
                    IMultiQueueManager< Key, Value >::queues_.end(), []( auto queue )
                    {
                         queue.second->Stop();
                    } );
     std::for_each( IMultiQueueManager< Key, Value >::consumers_.begin(),
                    IMultiQueueManager< Key, Value >::consumers_.end(), []( auto consumer )
                    {
                         consumer.second->Enabled( false );
                    } );
     std::for_each( IMultiQueueManager< Key, Value >::producers_.begin(),
                    IMultiQueueManager< Key, Value >::producers_.end(), [this]( auto producer )
                    {
                         UnregisterProducer( producer.first, producer.second );
                         //producer.second->Enabled( false );
                         //producer.second->SetQueue( nullptr );
                    } );
}

template<typename Key, typename Value>
void IMultiQueueManager< Key, Value >::StartProcessing()
{
     is_enabled_ = true;
     std::for_each( IMultiQueueManager< Key, Value >::queues_.begin(),
                    IMultiQueueManager< Key, Value >::queues_.end(), []( auto queue )
                    {
                         queue.second->Enabled( true );
                    } );
     std::for_each( IMultiQueueManager< Key, Value >::consumers_.begin(),
                    IMultiQueueManager< Key, Value >::consumers_.end(), []( auto consumer )
                    {
                         consumer.second->Enabled( true );
                    } );
}

template< typename Key, typename Value >
IMultiQueueManager< Key, Value >::IMultiQueueManager() : is_enabled_( true )
{
}

} // qm

#endif // MQP_MULTI_QUEUE_MANAGER_H_