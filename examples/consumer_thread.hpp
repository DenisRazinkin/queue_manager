#pragma once

#ifndef MQP_CONSUMER_THREAD_WORKER_H_
#define MQP_CONSUMER_THREAD_WORKER_H_

#include <thread>

#include <common.h>
#include <consumer/base_consumer.hpp>
#include <queue/block_concurrent_queue.hpp>
#include <manager/base_mqueue_manager.hpp>

namespace qm
{

std::atomic<int> consumer_counter_;

template< typename Value >
class QueueConsumerThreadWorker : public IConsumer< Value >
{
public:
     QueueConsumerThreadWorker() {};
     ~QueueConsumerThreadWorker() {};

     void Consume( const Value &obj )
     {
          //std::string msg = "consumer queue: " + key + " value: " + std::to_string( obj ) + " \n";
          //std::cout << msg;
          //std::cout.flush();
          //std::cout << value;
          consumer_counter_++;
     };

};



} // qm

#endif // MQP_CONSUMER_THREAD_WORKER_H_