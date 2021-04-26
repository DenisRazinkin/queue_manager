#pragma once

#ifndef MQP_SINGLE_QUEUE_MANAGER_H_
#define MQP_SINGLE_QUEUE_MANAGER_H_

#include <mutex>

#include "common.h"

namespace qm
{

/*
template< typename Key, typename Value >
class ISingleQueueManager
{
public:
    using Producers = boost::container::flat_map< Key, ProducerPtr< Key, Value > >;
    using Consumers = boost::container::flat_map< Key, ConsumerPtr< Key, Value > >;
    using Queue = boost::container::flat_map< Key, QueuePtr< Value > >;

    explicit ISingleQueueManager( QueuePtr< std::pair<Key, Value> > queue ) = default;

    virtual ~ISingleQueueManager() = default;

    // forbid copying
    ISingleQueueManager(const ISingleQueueManager & ) = delete;
    ISingleQueueManager & operator =(const ISingleQueueManager & ) = delete;

public:
    virtual State Subscribe( ConsumerPtr< Key, Value > consumer, Key id ) = 0;
    virtual State Unsubscribe( Key id ) = 0;

    virtual State Enqueue( const Key &id, const Value &value ) = 0;
    virtual State Enqueue( Key &&id, Value &&value ) = 0;

public:

    QueueResult<Value> GetQueue( Key id ) const
    {
        std::scoped_lock lock(IMultiQueueManager< Key, Value >::mtx_ );
        auto it = IMultiQueueManager< Key, Value >::queues_.find(id );
        return it == IMultiQueueManager< Key, Value >::queues_.end() ? QueueResult<Value>{nullptr, State::QueueAbsent } :
               QueueResult<Value>{ it->second, State::Ok };
    }

    bool AreAllQueuesEmpty() const
    {
        std::scoped_lock lock(IMultiQueueManager< Key, Value >::mtx_ );
        return std::all_of(IMultiQueueManager< Key, Value >::queues_.begin(), IMultiQueueManager< Key, Value >::queues_.end(),
                           [] ( const auto& queue ) { return queue.second && queue.second->Empty(); } );
    }

    bool AreAllProducersDone() const
    {
        std::scoped_lock lock(IMultiQueueManager< Key, Value >::mtx_ );
        return std::all_of(IMultiQueueManager< Key, Value >::producers_.begin(), IMultiQueueManager< Key, Value >::producers_.end(),
                           [] ( const auto& producer ) { return producer.second && producer.second->Done(); } );
    }

    virtual State RegisterProducer( ProducerPtr< Key, Value > producer, Key id )
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

protected:
    virtual bool ProducerRegistrationAllowed( Key id ) const = 0;

protected:
    mutable std::recursive_mutex mtx_;

    Queues queue;
    Producers producers_;
    Consumers consumers_;
};
*/
} // qm

#endif // MQP_SINGLE_QUEUE_MANAGER_H_