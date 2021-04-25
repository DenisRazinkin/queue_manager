#pragma once

#ifndef MQP_QUEUE_H_
#define MQP_QUEUE_H_

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <utility>

#include "base_queue.hpp"

namespace qm
{

template<typename Value>
class BlockConcurrentQueue : public IQueue<Value>
{
public:
     explicit BlockConcurrentQueue( std::size_t size ) : IQueue<Value>( size ) { };

     ~BlockConcurrentQueue() {  Stop(); };

     void Stop()
     {
          IQueue<Value>::Enabled( false );
          pop_cv_.notify_all();
          push_cv_.notify_all();
     }

     bool Empty() const { return queue_.empty(); };

     std::optional<Value> Pop()
     {
          Value result;
          {
               std::unique_lock lock( mtx );
               //std::cout << "consumer " << std::this_thread::get_id() << " queue enabled_ " << IQueue<Value>::Enabled() << " queue size:" << queue_.size() << std::endl;
               pop_cv_.wait( lock, [this]()
               {

                    return !queue_.empty() || !IQueue<Value>::Enabled();
               } );

               if ( queue_.empty() )
               {
                    return std::nullopt;
               }
               else
               {
                    //!IQueue<Value>::Enabled()
                    result = queue_.front();
                    queue_.pop();
               }


          }

          push_cv_.notify_one();
          return result;
     };

     /// @brief blocking push until queue not empty or queue is disabled
     /// @param obj
     /// @return
     State Push( const Value &obj )
     {
          return PushFwd( obj );
     }

     State Push( Value&& obj )
     {
          return PushFwd( std::move( obj ) );
     }

     State TryPush( const Value& obj )
     {
          return TryPushFwd( obj );
     }

     /// Avoid waiting when queue is full
     State TryPush( Value&& obj )
     {
          return TryPushFwd( std::move( obj ) );
     }


private:
     template<typename V>
     State TryPushFwd( V&& obj )
     {
          std::unique_lock lock( mtx );
          if ( queue_.size() >= kQueueSize )
          {
               return State::QueueFull;
          }

          if ( !IQueue<Value>::Enabled() )
          {
               return State::QueueDisabled;
          }

          queue_.emplace( std::forward<V> ( obj ) );
          pop_cv_.notify_one();
          return State::Ok;
     }

     template<typename V>
     State PushFwd( V&& obj )
     {
          //std::string msg = "push " + std::to_string( obj );
          //std::cout << msg;
          {
               std::unique_lock lock( mtx );
               //std::cout << std::this_thread::get_id() << " queue enabled_ " << IQueue<Value>::Enabled() << " obj " << obj << " queue size:" << queue_.size() << std::endl;
               push_cv_.wait( lock, [ this, obj ] ()
               {
                    //std::cout << "this queue " << this << " enabled " << IQueue<Value>::Enabled() << std::endl;
                    return queue_.size() < kQueueSize || ! IQueue<Value>::Enabled();
               } );

               if ( queue_.size() >= kQueueSize )
               {
                    return State::QueueFull;
               }

               if ( !IQueue<Value>::Enabled() )
               {
                    return State::QueueDisabled;
               }

               queue_.emplace( std::forward<V> ( obj ) );
          }

          pop_cv_.notify_one();
          return State::Ok;
     }

private:
     std::queue<Value> queue_;
     const static unsigned int kQueueSize = 100;

     std::mutex mtx;
     std::condition_variable pop_cv_;
     std::condition_variable push_cv_;

};



} // qm

#endif // MQP_IQUEUE_H_