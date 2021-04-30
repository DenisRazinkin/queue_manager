#pragma once

#ifndef MQP_ENQUEUE_PRODUCER_H_
#define MQP_ENQUEUE_PRODUCER_H_

#include <producer/base_producer.hpp>

namespace qm::example
{

std::atomic< int > produce_counter_;

template< typename Key, typename Value >
class EnqueueProducerThread : public IProducer< Key, Value >
{
public:
     EnqueueProducerThread( Key id, int loops, ManagerPtr <Key, Value> manager )
          : IProducer< Key, Value >( id ), manager_( manager ), loops_( loops )
     {};

     ~EnqueueProducerThread()
     {
          if ( thread_.joinable()) thread_.join();
     };

     void WaitThreadDone() override
     {
          if ( thread_.joinable()) thread_.join();
     }

     void Produce()
     {
          if ( manager_ == nullptr )
          {
               std::cout << "Manager is null\n";
               IProducer< Key, Value >::done_ = true;
               return;
          }

          auto producer = [ this ] ()
          {
               for ( int i = 0; i != loops_; ++i )
               {
                    if ( !IProducer< Key, Value >::enabled_.load() ) return;
                    produce_counter_++;

                    qm::State state;
                    while ( IProducer< Key, Value >::enabled_.load() &&
                            ( state = manager_->Enqueue( IProducer< Key, Value >::id_, produce_counter_ ) ) != qm::State::Ok )
                    {
                         if ( state == qm::State::QueueDisabled ||
                              state == qm::State::QueueAbsent )
                         {
                              break;
                         }
                    }
               }

               IProducer< Key, Value >::done_ = true;
          };

          thread_ = std::thread( producer );
     }

private:
     ManagerPtr <Key, Value> manager_;
     std::thread thread_;
     int loops_;
};


template< typename Key, typename Value >
class SimpleLoopProducerThread : public IProducer< Key, Value >
{
public:
     SimpleLoopProducerThread( Key id, int loops ) : IProducer< Key, Value >( id ), loops_( loops )
     {};

     ~SimpleLoopProducerThread()
     {
          if ( thread_.joinable()) thread_.join();
     };

     void WaitThreadDone() override
     {
          thread_.join();
     }

     void Produce() override
     {
          if ( IProducer< Key, Value >::queue_ == nullptr )
          {
               std::cout << "queue is null!";
               return;
          }

          auto producer = [ this ] ()
          {
               for ( int i = 0; i != loops_; ++i )
               {
                    if ( !IProducer< Key, Value >::enabled_.load()) return;
                    produce_counter_++;

                    qm::State state;
                    while ( IProducer< Key, Value >::enabled_.load() &&
                            ( state = IProducer< Key, Value >::queue_->Push( produce_counter_ ) ) !=
                            qm::State::Ok )
                    {
                         if ( state == qm::State::QueueDisabled )
                         {
                              break;
                         }
                    }
               }

               IProducer< Key, Value >::done_ = true;
          };

          thread_ = std::thread( producer );
     }

private:
     std::thread thread_;
     int loops_;
};

} // namespace qm::example

#endif // MQP_ENQUEUE_PRODUCER_H_