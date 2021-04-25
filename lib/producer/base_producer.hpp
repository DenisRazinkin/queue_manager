#pragma once

#ifndef MQP_BASE_PRODUCER_H_
#define MQP_BASE_PRODUCER_H_

#include "common.h"
//#include "base_queue_manager.h"

#include <iostream>
#include <thread>

namespace qm
{

template<typename Key, typename Value>
class IProducer
{
public:
     explicit IProducer( Key id ) : id_( id ), done_( false ), enabled_( true ) {};
     virtual ~IProducer() = default;

     inline bool Enabled() const { return enabled_.load(); }
     inline void Enabled( bool enabled ) { enabled_.store( enabled ); }

     inline bool Done() const { return done_; }

     /*State Register( Key id, IQueueManager<Key, Value> *manager )
     {
          std::scoped_lock lock_guard( mtx );
          if ( manager == nullptr )
          {
               //TODO add state for null manager pointer?
               queue_ = nullptr;
               return State::QueueAbsent;
          }

          auto queue_result = manager->GetQueue( id );
          if ( queue_result.s_ != State::Ok )
          {
               queue_ = nullptr;
               return queue_result.s_;
          }

          SetQueue( queue_result.queue_ );
          id_ = id;
          return State::Ok;
     }*/

public:
     virtual void Produce() = 0;

protected:
     Key id_;
     bool done_;
     std::atomic<bool> enabled_ = true;
     QueuePtr<Value> queue_;

private:

     void SetQueue( QueuePtr<Value> queue )
     {
          queue_ = queue;
     }

     friend class IQueueManager< Key, Value >;

};

} // qm

#endif // MQP_BASE_CONSUMER_H_
