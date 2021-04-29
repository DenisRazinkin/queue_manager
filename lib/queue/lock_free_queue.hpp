/// @brief Lock free queue for multi producers multi consumers model.
/// @author Denis Razinkin
#pragma once

#ifndef MQP_LOCK_FREE_QUEUE_H_
#define MQP_LOCK_FREE_QUEUE_H_

#include <boost/lockfree/queue.hpp>

#include <queue/base_queue.hpp>

namespace qm
{

/// @brief Lock free queue for multi producers multi consumers model.
/// @tparam Value Type for queue store
template< typename Value >
class LockFreeQueue : public IQueue< Value >
{
public:
     /// @brief Constructor
     /// @param size Maximal size of queue
     explicit LockFreeQueue( std::size_t size );

     /// @brief Destructor
     ~LockFreeQueue() = default;

     /// @brief Disable queue and stop waiting for threads
     /// Thread safe
     void Stop();

     /// @brief Check is queue empty.
     /// Thread safe.
     /// @return true/false
     [[nodiscard]] bool Empty() const;

     /// @brief Lock free pop from queue.
     /// Thread safe.
     /// @return Object empty value if pop unsuccessfully
     std::optional< Value > Pop();

     /// @brief Lock free push.
     /// Thread safe
     /// @param obj Lvalue object to push
     /// @return State::Ok or other state of queue on error
     State Push( const Value &obj );

     /// @brief Lock free push.
     /// Thread safe
     /// @param obj Rvalue object to push
     /// @return State::Ok or other state of queue on error
     State Push( Value &&obj );

     /// @brief Lock free push. Method similar to Push(const &)
     /// Thread safe
     /// @param obj Lvalue object to push
     /// @return State::Ok or other state of queue on error
     State TryPush( const Value &obj );

     /// @brief Lock free push. Method similar to Push(&&)
     /// Thread safe
     /// @param obj Rvalue object to push
     /// @return State::Ok or other state of queue on error
     State TryPush( Value &&obj );

private:
     boost::lockfree::queue< Value > queue_;

};

template< typename Value >
void LockFreeQueue< Value >::Stop()
{
     // Queue is nonblocking, nothing to do here
     IQueue< Value >::Enabled( false );
}

template< typename Value >
LockFreeQueue< Value >::LockFreeQueue( std::size_t size ) : IQueue< Value >( size ),
                                                            queue_( boost::lockfree::queue< Value >( size ))
{}

template< typename Value >
bool LockFreeQueue< Value >::Empty() const
{
     return queue_.empty();
}

template< typename Value >
std::optional< Value > LockFreeQueue< Value >::Pop()
{
     Value value;
     if ( queue_.pop( value ))
     {
          return value;
     }

     return std::nullopt;
}

template< typename Value >
State LockFreeQueue< Value >::Push( const Value &obj )
{
     if ( !IQueue< Value >::Enabled() ) return State::QueueDisabled;
     return queue_.bounded_push( obj ) ? State::Ok : State::QueueFull;
}

template< typename Value >
State LockFreeQueue< Value >::Push( Value &&obj )
{
     if ( !IQueue< Value >::Enabled() ) return State::QueueDisabled;
     return queue_.bounded_push( std::move( obj )) ? State::Ok : State::QueueFull;
}

template< typename Value >
State LockFreeQueue< Value >::TryPush( const Value &obj )
{
     if ( !IQueue< Value >::Enabled() ) return State::QueueDisabled;
     return queue_.bounded_push( obj ) ? State::Ok : State::QueueFull;
}

template< typename Value >
State LockFreeQueue< Value >::TryPush( Value &&obj )
{
     if ( !IQueue< Value >::Enabled() ) return State::QueueDisabled;
     return queue_.bounded_push( std::move( obj )) ? State::Ok : State::QueueFull;
}

} // qm

#endif // MQP_LOCK_FREE_QUEUE_H_