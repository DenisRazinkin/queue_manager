#pragma once

#ifndef MQP_ENQUEUE_PRODUCER_H_
#define MQP_ENQUEUE_PRODUCER_H_

#include <producer/base_producer.hpp>


namespace qm
{

std::atomic< int > counter_;

template< typename Key, typename Value >
class EnqueueProducerThread : public IProducer< Key, Value >
{
public:
     EnqueueProducerThread( Key id, int loops, ManagerPtr <Key, Value> manager ) : IProducer< Key, Value >( id ),
                                                                                   manager_( manager ), loops_( loops )
     {};

     ~EnqueueProducerThread()
     { if ( thread_.joinable()) thread_.join(); };

     void WaitThreadDone() override
     {
          thread_.join();
     }

     void Produce()
     {
          if ( manager_ == nullptr )
          {
               std::cout << "Manager is null\n";
               IProducer< Key, Value >::done_ = true;
               return;
          }

          thread_ = std::thread( [ this ]()
          {
               for ( int i = 0; i != loops_; ++i )
               {
                    if ( !IProducer< Key, Value >::enabled_.load() ) return;
                    counter_++;

                    qm::State state;

                    //std::string msg =  " queue " + IProducer<Key, Value>::id_ + " loops_: " + std::to_string ( loops_ ) + " push value " + std::to_string (counter_ );
                    //std::cout << "producer " << std::hex << std::this_thread::get_id() << std::dec << msg << std::endl;
                    while ( IProducer< Key, Value >::enabled_.load() &&
                          ( state = manager_->Enqueue( IProducer< Key, Value >::id_, counter_ )) != qm::State::Ok )
                    {
                         if ( state == qm::State::QueueDisabled ||
                            state == qm::State::QueueAbsent )
                         {
                              break;
                         }
                    }
               }

               IProducer< Key, Value >::done_ = true;
          } );
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
     { if ( thread_.joinable()) thread_.join(); };

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

          thread_ = std::thread( [ this ]()
                                 {

                                      for ( int i = 0; i != loops_; ++i )
                                      {
                                           if ( !IProducer< Key, Value >::enabled_.load()) return;
                                           counter_++;

                                           qm::State state;

                                           //std::cout << "queue " << IProducer<Key, Value>::id_ << " loops_: " << loops_ << " push value " << counter_ << std::endl;
                                           while ( IProducer< Key, Value >::enabled_.load() &&
                                                   ( state = IProducer< Key, Value >::queue_->Push( counter_ )) !=
                                                   qm::State::Ok )
                                           {

                                                if ( state == qm::State::QueueDisabled )
                                                {
                                                     break;
                                                }
                                           }
                                      }

                                      IProducer< Key, Value >::done_ = true;
                                 } );
     }

private:
     std::thread thread_;
     int loops_;
};

} // namespace qm

#endif // MQP_ENQUEUE_PRODUCER_H_