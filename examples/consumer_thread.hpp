#pragma once

#ifndef MQP_CONSUMER_THREAD_WORKER_H_
#define MQP_CONSUMER_THREAD_WORKER_H_

#include <thread>

#include <common.h>
#include <consumer/base_consumer.hpp>
#include <queue/block_concurrent_queue.hpp>
#include <manager/base_queue_manager.hpp>

namespace qm
{

std::atomic<int> consumer_counter_;

template<typename Key, typename Value>
class QueueConsumerThreadWorker : public IConsumer<Key, Value>, public SharedFactory< QueueConsumerThreadWorker< Key, Value > >
{
public:
     QueueConsumerThreadWorker() {};
     ~QueueConsumerThreadWorker() {};
     /*
     void Run()
     {
          //TODO add mp exception type
          if ( queue_ == nullptr ) throw std::runtime_error("Could not run consumer, queue is empty");

          consumer_thread_ = std::thread( []()
               {
                    while (!done || !queue_->Empty() ) {
                         auto value = queue_->Pop();

                         if ( value.is_initialized() )
                         {
                              Consume( value.get() )
                         }
                    }
               }
          );

     }*/

     void Consume( const Key &key, const Value &obj )
     {
          //std::string msg = "consumer queue: " + key + " value: " + std::to_string( obj ) + " \n";
          //std::cout << msg;
          //std::cout.flush();
          //std::cout << value;
          consumer_counter_++;
     };

     /*
     void Consume( )
     {
          while (!done || !q->Empty() ) {
               auto value = q->Pop();

               if ( value.is_initialized() )
               {
                    //std::string msg = "consumer: " + std::to_string( id ) + " value: " + std::to_string( value.get() ) + " \n";
                    //std::cout << msg;
                    ++consumer_count;
               }
          }
     }*/

     /*
     virtual void Stop()
     {
          is_enabled_ = false;
     }
private:
     std::atomic<bool> is_enabled_;
     std::thread consumer_thread_;*/
};



} // qm

#endif // MQP_CONSUMER_THREAD_WORKER_H_