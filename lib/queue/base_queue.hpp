#pragma once

#ifndef MQP_BASE_IQUEUE_H_
#define MQP_BASE_IQUEUE_H_

#include <boost/optional.hpp>

namespace qm
{

enum class State
{
     Ok,
     QueueFull,
     QueueEmpty,
     QueueBusy,
     QueueAbsent,
     QueueDisabled
};

std::string StateStr( State s )
{
     switch ( s )
     {
          case State::QueueFull:
               return "queue is full";
          case State::QueueEmpty:
               return "queue is empty";
          case State::QueueBusy:
               return "queue is busy";
          case State::QueueAbsent:
               return "queue is absent";

          default:
               return "unknown state";
     }
}

template<typename Value>
class IQueue
{
public:
     explicit IQueue( std::size_t size ) : size_( size ), enabled_( true ){};
     virtual ~IQueue() { Stop(); };


     IQueue( const IQueue & ) = delete;

     IQueue &operator=( const IQueue & ) = delete;

     virtual void Stop() { Enabled( false ); };

     inline bool Enabled() const { return enabled_.load(); }
     inline void Enabled( bool enabled ) { enabled_.store( enabled ); }

     //virtual Value PopWait() = 0;
     virtual std::optional<Value> Pop() = 0;

     /// Blocking push
     virtual State Push( const Value &obj ) = 0;
     virtual State Push( Value&& obj ) = 0;

     /// Nonblocking push
     virtual State TryPush( const Value &obj ) = 0;
     virtual State TryPush( Value&& obj ) = 0;

     virtual bool Empty() const = 0;
     //virtual boost::optional<Value> TryPop() = 0;
     //virtual bool TryPush( Value&& obj ) = 0;
private:
     std::size_t size_;
     std::atomic<bool> enabled_;
     //< typename Queue >
          //std::enable_if< std::is_base_of< Queue, IQueue < Value > >::value>, Value >
     //const auto MakeQueuePtr = std::make_shared< Queue >;
};

template<typename Value>
using QueuePtr = std::shared_ptr< IQueue< Value > >;

template<typename Value>
struct QueueResult
{
     QueuePtr< Value > queue_;
     State s_;
};
//static_assert( std::is_base_of< Queue, IQueue>::Value, "Not derived";

} // qm

#endif // MQP_IQUEUE_H_