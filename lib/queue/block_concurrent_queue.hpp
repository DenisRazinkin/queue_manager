/// @brief Blocking concurrent queue with mutex and cv
/// @author Denis Razinkin
#pragma once

#ifndef MQP_BLOCKING_CONCURRENT_QUEUE_H_
#define MQP_BLOCKING_CONCURRENT_QUEUE_H_

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <utility>

#include "base_queue.hpp"

namespace qm
{

/// @brief Queue with concurrent access to queue from different threads.
/// @tparam Value Type for queue store
template<typename Value>
class BlockConcurrentQueue : public IQueue< Value >
{
public:
     /// @brief Constructor
     /// @param size Maximal size of queue
     explicit BlockConcurrentQueue( std::size_t size );

     /// @brief Destructor
     ~BlockConcurrentQueue();

     /// @brief Disable queue and stop waiting for threads
     /// Thread safe.
     void Stop();

     /// @brief Check is queue empty.
     /// Thread safe.
     /// @return true/false
     [[nodiscard]] bool Empty() const;

     /// @brief Get current size of queue
     /// Thread safe.
     /// @return Size
     std::size_t Size() const;

     /// @brief Blocking pop from queue. Thread will lock if queue empty until new value will come queue will be disabled.
     /// Thread safe.
     /// @return Object empty value if pop unsuccessfully
     std::optional< Value > Pop();

     /// @brief Blocking push until queue full or queue will be disabled.
     /// Thread safe.
     /// @param obj Lvalue object to push
     /// @return State::Ok or other state of queue on error
     State Push( const Value &obj );

     /// @brief Blocking push until queue full or queue will be disabled.
     /// Thread safe.
     /// @param obj Rvalue object to push
     /// @return State::Ok or other state of queue on error
     State Push( Value &&obj );

     /// @brief Nonblocking push. Used to avoid waiting when queue is full
     /// Thread safe.
     /// @param obj Lvalue object to push
     /// @return State::Ok or other state of queue on error
     State TryPush( const Value &obj );

     /// @brief Nonblocking push. Used to avoid waiting when queue is full
     /// Thread safe.
     /// @param obj Rvalue object to push
     /// @return State::Ok or other state of queue on error
     State TryPush( Value &&obj );

private:
     template<typename V>
     State TryPushFwd( V &&obj );

     template<typename V>
     State PushFwd( V &&obj );

private:
     std::queue< Value > queue_;

     mutable std::mutex mtx;
     std::condition_variable pop_cv_;
     std::condition_variable push_cv_;

};

template< typename Value >
BlockConcurrentQueue< Value >::BlockConcurrentQueue( std::size_t size ) : IQueue< Value >( size )
{}

template< typename Value >
BlockConcurrentQueue< Value >::~BlockConcurrentQueue()
{
     Stop();
}

template< typename Value >
void BlockConcurrentQueue< Value >::Stop()
{
     IQueue< Value >::Enabled( false );
     pop_cv_.notify_all();
     push_cv_.notify_all();
}

template< typename Value >
bool BlockConcurrentQueue< Value >::Empty() const
{
     std::unique_lock lock( mtx );
     return queue_.empty();
}

template< typename Value >
std::size_t BlockConcurrentQueue< Value >::Size() const
{
     std::unique_lock lock( mtx );
     return queue_.size();
}

template< typename Value >
std::optional< Value > BlockConcurrentQueue< Value >::Pop()
{
     Value result;
     {
          std::unique_lock lock( mtx );
          pop_cv_.wait( lock, [ this ]()
          {
               return !queue_.empty() || !IQueue< Value >::Enabled();
          } );

          if ( queue_.empty())
          {
               return std::nullopt;
          } else
          {
               result = queue_.front();
               queue_.pop();
          }
     }

     push_cv_.notify_one();
     return result;
}

template< typename Value >
State BlockConcurrentQueue< Value >::Push( const Value &obj )
{
     return PushFwd( obj );
}

template< typename Value >
State BlockConcurrentQueue< Value >::Push( Value &&obj )
{
     return PushFwd( std::move( obj ));
}

template< typename Value >
State BlockConcurrentQueue< Value >::TryPush( const Value &obj )
{
     return TryPushFwd( obj );
}

template< typename Value >
State BlockConcurrentQueue< Value >::TryPush( Value &&obj )
{
     return TryPushFwd( std::move( obj ));
}

template< typename Value >
template< typename V >
State BlockConcurrentQueue< Value >::TryPushFwd( V &&obj )
{
     std::unique_lock lock( mtx );
     if ( queue_.size() >= IQueue< Value >::MaxSize())
     {
          return State::QueueFull;
     }

     if ( !IQueue< Value >::Enabled())
     {
          return State::QueueDisabled;
     }

     queue_.emplace( std::forward< V >( obj ));
     pop_cv_.notify_one();
     return State::Ok;
}

template< typename Value >
template< typename V >
State BlockConcurrentQueue< Value >::PushFwd( V &&obj )
{
     //std::string msg = "push " + std::to_string( obj );
     //std::cout << msg;
     {
          std::unique_lock lock( mtx );
          //std::cout << std::this_thread::get_id() << " queue enabled_ " << IQueue<Value>::Enabled() << " obj " << obj << " queue size:" << queue_.size() << std::endl;
          push_cv_.wait( lock, [ this, obj ]()
          {
               //std::cout << "this queue " << this << " enabled " << IQueue<Value>::Enabled() << std::endl;
               return queue_.size() < IQueue< Value >::MaxSize() || !IQueue< Value >::Enabled();
          } );

          if ( queue_.size() >= IQueue< Value >::MaxSize())
          {
               return State::QueueFull;
          }

          if ( !IQueue< Value >::Enabled())
          {
               return State::QueueDisabled;
          }

          queue_.emplace( std::forward< V >( obj ));
     }

     pop_cv_.notify_one();
     return State::Ok;
}

} // qm

#endif // MQP_BLOCKING_CONCURRENT_QUEUE_H_