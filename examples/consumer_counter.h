#pragma once

#ifndef MQP_CONSUMER_THREAD_WORKER_H_
#define MQP_CONSUMER_THREAD_WORKER_H_

#include <thread>

#include <common.h>
#include <consumer/base_consumer.hpp>
#include <queue/block_concurrent_queue.hpp>
#include <manager/base_mqueue_manager.hpp>

namespace qm::example
{

std::atomic<int> consumer_counter_;

template< typename Key, typename Value >
class ConsumerCounter : public IConsumer< Value >
{
public:
     explicit ConsumerCounter( Key id ) : id_( id) {};
     ~ConsumerCounter() = default;

     void Consume( const Value & )
     {
          consumer_counter_++;
     };

     Key id_;
};



} // qm

#endif // MQP_CONSUMER_THREAD_WORKER_H_