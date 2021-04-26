#pragma once

#ifndef MQP_BASE_CONSUMER_H_
#define MQP_BASE_CONSUMER_H_

#include "common.h"
//#include "base_queue_manager.h"

#include <iostream>

namespace qm
{



template<typename Key, typename Value>
class IConsumer
{
public:
     explicit IConsumer() = default;
     virtual ~IConsumer() = default;

     inline bool Enabled() const { return enabled_.load(); }
     inline void Enabled( bool enabled ) { enabled_.store( enabled ); }

     //virtual void Run() = 0;

     //virtual void Stop() = 0;
     //{
          //if ( queue_ == nullptr ) return QueueSubscriptionAbsent;
          //return Consume();
     //}

     //virtual friend class IMultiQueueManager<Key, Value>;
     //friend State QueueManager<Key, Value>::Subscribe( ConsumerPtr<Key, Value> consumer, Key id );

public:
     virtual void Consume( const Key &key, const Value &obj ) = 0;
private:
     std::atomic<bool> enabled_ = true;
/*
     ConsumerPtr<Key, Value> Subscribe( QueuePtr<Value> queue )
     {
          queue_ = queue;
          return this;
     }

     ConsumerPtr<Key, Value> Unsubscribe()
     {
          queue_.reset();
          return this;
     }*/

     Key id;
     //QueuePtr<Value> queue_;

};

} // qm

#endif // MQP_BASE_CONSUMER_H_
