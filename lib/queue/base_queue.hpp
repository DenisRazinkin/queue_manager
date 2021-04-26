#pragma once

#ifndef MQP_BASE_IQUEUE_H_
#define MQP_BASE_IQUEUE_H_

#include <atomic>

#include <boost/optional.hpp>

#include "queue_state.hpp"

namespace qm
{

/// @brief General base template class for queue processing
/// @tparam Value - type for queue store
template<typename Value>
class IQueue
{
public:
     explicit IQueue( std::size_t size );
     virtual ~IQueue();

     /// @brief copy constructor is disabled
     IQueue( const IQueue & ) = delete;

     /// @brief copy operator is disabled
     IQueue &operator=( const IQueue & ) = delete;

     /// @brief Stop queue handling
     virtual void Stop();

     /// @brief Is queue enabled
     /// @return true/false
     [[nodiscard]] inline bool Enabled() const;

     /// @brief Set is queue enabled
     /// @param enabled - true/false
     inline void Enabled( bool enabled );

     /// @brief - queue maximal size
     /// @return - size_t
     [[nodiscard]] std::size_t MaxSize() const;

public:
     /// @brief Try pop value from queue
     /// @return Value if pop successful, boost::none otherwise
     virtual std::optional<Value> Pop() = 0;

     /// @brief Push to the queue ( may block )
     /// @param obj - lvalue const object to push
     /// @return - State value
     virtual State Push( const Value &obj ) = 0;

     /// @brief Push to the queue ( may block )
     /// @param obj - rvalue object to push
     /// @return - State value
     virtual State Push( Value&& obj ) = 0;

     /// @brief Nonblocking push to the queue
     /// @param obj - lvalue const object to push
     /// @return - State value
     virtual State TryPush( const Value &obj ) = 0;

     /// @brief Nonblocking push to the queue
     /// @param obj - rvalue object to push
     /// @return - State value
     virtual State TryPush( Value&& obj ) = 0;

     /// @brief Is queue empty
     /// @return true/false
     [[nodiscard]] virtual bool Empty() const = 0;

private:
     std::size_t size_;
     std::atomic<bool> enabled_;
};

/// @brief Alias name for shared pointer to queue
template<typename Value>
using QueuePtr = std::shared_ptr< IQueue< Value > >;

template<typename Value>
struct QueueResult
{
     QueuePtr< Value > queue_;
     State s_;
};

template<typename Value>
IQueue<Value>::IQueue(std::size_t size) : size_( size ), enabled_( true ) {}

template<typename Value>
IQueue<Value>::~IQueue()
{
     enabled_ = false;
}

template<typename Value>
void IQueue<Value>::Stop()
{
     Enabled( false );
}

template<typename Value>
void IQueue<Value>::Enabled(bool enabled)
{
     enabled_.store( enabled );
}

template<typename Value>
bool IQueue<Value>::Enabled() const
{
     return enabled_.load();
}

template<typename Value>
std::size_t IQueue<Value>::MaxSize() const
{
     return size_;
}

} // namespace qm

#endif // MQP_BASE_IQUEUE_H_