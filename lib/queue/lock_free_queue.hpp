#pragma once

#ifndef MQP_LOCK_FREE_QUEUE_H_
#define MQP_LOCK_FREE_QUEUE_H_

#include <boost/lockfree/queue.hpp>

#include <queue/base_queue.hpp>

namespace qm
{

template<typename Value>
class LockFreeQueue : public IQueue<Value>
{
public:
     explicit LockFreeQueue( std::size_t size ) : IQueue<Value>( size ), queue_( boost::lockfree::queue<Value>( size ) ) { };

     void Stop()
     {
     }

     bool Empty() const { return queue_.empty(); };

     std::optional<Value> Pop()
     {
          Value value;
          if ( queue_.pop( value ) )
          {
               return value;
          }

          return std::nullopt;
     };

     State Push( const Value &obj )
     {
          return queue_.push( obj ) ? State::Ok : State::QueueFull;
     }

     /// Push
     State Push( Value &&obj )
     {
          return queue_.push( std::move( obj ) ) ? State::Ok : State::QueueFull;
     }

     State TryPush( const Value &obj )
     {
          return queue_.bounded_push( obj ) ? State::Ok : State::QueueFull;
     }

     State TryPush( Value &&obj )
     {
          return queue_.bounded_push( std::move( obj ) ) ? State::Ok : State::QueueFull;
     }
private:
     boost::lockfree::queue<Value> queue_;

};

} // qm

#endif // MQP_LOCK_FREE_QUEUE_H_