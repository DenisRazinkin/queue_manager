#pragma once

#ifndef MQP_MPSC_QUEUE_MANAGER_H_
#define MQP_MPSC_QUEUE_MANAGER_H_

#include <atomic>

#include <map>
#include <thread>

#include <boost/container/flat_map.hpp>

#include "queue/base_queue.hpp"
#include "consumer/base_consumer.hpp"
#include "manager/base_queue_manager.hpp"

namespace qm
{

//template< typename Key, typename Value, template< Key, typename > class KVStore >
template< typename Key, typename Value >
class MPSCQueueManager : public IQueueManager< Key, Value >
{
public:


     explicit MPSCQueueManager()
          :  is_enabled_ ( true ) {};

     virtual ~MPSCQueueManager()
     {
          StopProcessing();
          //producer_threads_.join_all();
          //consumer_threads_.join_all();
     };




     void StopProcessing()
     {
          std::cout << "stop processing..\n";
          is_enabled_ = false;
          std::for_each( IQueueManager< Key, Value >::queues_.begin(), IQueueManager< Key, Value >::queues_.end(), [] ( auto queue ) { queue.second->Stop(); } );
          std::for_each( IQueueManager< Key, Value >::consumers_.begin(), IQueueManager< Key, Value >::consumers_.end(), [] ( auto consumer ) { consumer.second->Enabled( false ); } );
          std::for_each( IQueueManager< Key, Value >::producers_.begin(), IQueueManager< Key, Value >::producers_.end(), [] ( auto producer ) { producer.second->Enabled( false ); } );

          for ( auto & thread : consumer_threads_ )
          {
               thread.second.join();
          }
          //std::for_each( consumer_threads_.begin(), consumer_threads_.end(), [] ( auto thread ) { thread.second.join(); } );

     }

     State Enqueue( Key &&id, Value &&value ) override
     {
          return EnqueueFwd( std::move( id ), std::move( value ) );
     }

     State Enqueue( const Key &id, const Value &value ) override
     {
          return EnqueueFwd( id, value );
     }

     virtual State Subscribe( ConsumerPtr<Key, Value> consumer, Key id ) override
     {
          //auto new_subscriber = Consumers::value_type( id, consumer );
          //std::cout << id << " subscribe.. \n";

          std::scoped_lock lock( IQueueManager< Key, Value >::mtx_ );
          if (  IQueueManager< Key, Value >::consumers_.find( id ) != IQueueManager< Key, Value >::consumers_.end() )
          {
               return State::QueueBusy;
          }

          auto queue_it = IQueueManager< Key, Value >::queues_.find( id );
          if ( queue_it == IQueueManager< Key, Value >::queues_.end() )
          {
               return State::QueueAbsent;
          }

          IQueueManager< Key, Value >::consumers_.emplace( id, consumer );

          auto queue = queue_it->second;
          consumer_threads_.emplace( id, std::thread([ this, id, queue, consumer ]()
          {
               std::cout << "consumer " << std::hex << std::this_thread::get_id() << std::dec << " consumer enabled_ " << consumer->Enabled() << " queue->Empty():" << queue->Empty() << std::endl;
               //std::cout << "manager enabled_ " << is_enabled_ << " queue enabled_ " <<queue->Enabled()  << std::endl;
               while ( ( consumer->Enabled() && is_enabled_ && queue->Enabled() ) || !queue->Empty() ) {
                    //std::cout << id << " thread \n";
                    //std::cout << "consumer " << std::this_thread::get_id() << " consumer enabled_ " << consumer->Enabled() << " queue->Empty():" << queue->Empty() << std::endl;
                    auto value = queue->Pop();

                    if ( value.has_value() )
                    {
                         consumer->Consume( id, value.value() );
                    }
               }
               //std::cout << "consumer " << std::this_thread::get_id() << " consumer enabled_ " << consumer->Enabled() << " queue->Empty():" << queue->Empty() << std::endl;
               //std::cout << "manager enabled_ " << is_enabled_ << " queue enabled_ " <<queue->Enabled()  << std::endl;
          } ) );

          //consumer->Subscribe( queue )->Run();
          return State::Ok;
     }

     virtual State Unsubscribe( Key id ) override
     {
          std::scoped_lock lock( IQueueManager< Key, Value >::mtx_ );
          auto consumer = IQueueManager< Key, Value >::consumers_.find( id );
          if ( consumer != IQueueManager< Key, Value >::consumers_.end() )
          {
               //consumer->Unsubscribe()->Stop();
               consumer->second->Enabled( false );

               auto thread = consumer_threads_.find( id );
               if ( thread != consumer_threads_.end() )
               {
                    thread->second.join();
               }

               IQueueManager< Key, Value >::consumers_.erase( id );
               return State::Ok;
          }

          return State::QueueAbsent;
     }

protected:
     bool ProducerRegistrationAllowed( Key id ) const override
     {
          // for multi producers manager registration has always enabled
          return true;
     }

private:
     template< typename K, typename V >
     State EnqueueFwd( K&& id, V&& value )
     {
          std::lock_guard<std::recursive_mutex> lock( IQueueManager< Key, Value >::mtx_ );
          auto queue = IQueueManager< Key, Value >::queues_.find( std::forward< K >( id ) );
          if ( queue != IQueueManager< Key, Value >::queues_.end() )
          {
               return queue->second->TryPush( std::forward< V >( value ) );
          }

          return State::QueueAbsent;
     }

private:
     std::atomic<bool> is_enabled_;

     //std::map<Key, std::thread> consumers_threads_:
     //std::map<Key, std::thread> producer_threads_;

     boost::container::flat_map<Key, std::thread> consumer_threads_;
};

} // namespace qm

#endif // MQP_MPSC_QUEUE_MANAGER_H_