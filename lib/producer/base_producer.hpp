#pragma once

#ifndef MQP_BASE_PRODUCER_H_
#define MQP_BASE_PRODUCER_H_

#include "common.h"

#include <iostream>
#include <thread>

#include "manager/base_mqueue_manager.hpp"

namespace qm
{

template< typename Key, typename Value >
class IProducer
{
public:
     explicit IProducer( Key id );

     virtual ~IProducer() = default;

     [[nodiscard]] inline bool Enabled() const;

     inline void Enabled( bool enabled );

     [[nodiscard]] inline bool Done() const;

public:
     virtual void Produce() = 0;

     virtual void WaitDone() = 0;

protected:
     Key id_;
     bool done_;
     std::atomic< bool > enabled_ = true;
     QueuePtr <Value> queue_;

private:

     void SetQueue( QueuePtr <Value> queue )
     {
          queue_ = queue;
     }

     friend class IMultiQueueManager< Key, Value >;

};

template< typename Key, typename Value >
IProducer< Key, Value >::IProducer( Key id ) : id_( id ), done_( false ), enabled_( true )
{}

template< typename Key, typename Value >
void IProducer< Key, Value >::Enabled( bool enabled )
{
     enabled_.store( enabled );
}

template< typename Key, typename Value >
bool IProducer< Key, Value >::Enabled() const
{
     return enabled_.load();
}

template< typename Key, typename Value >
bool IProducer< Key, Value >::Done() const
{
     return done_;
}

} // qm

#endif // MQP_BASE_PRODUCER_H_
