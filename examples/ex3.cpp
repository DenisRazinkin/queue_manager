#include <boost/date_time/posix_time/posix_time.hpp>

#include <queue/lock_free_queue.hpp>
#include <manager/mpsc_mqueue_manager.hpp>
#include <producer/base_producer.hpp>

#include "consumer_thread.hpp"
#include "producer.hpp"

void LockFreeQueueWithProducerRegistration( int loops )
{
     auto workers = std::thread::hardware_concurrency() /2;
     qm::MPSCQueueManager<std::string, int> mpsc_manager;
     for ( std::size_t i = 0; i < workers; i++ )
     {
          mpsc_manager.AddQueue( std::to_string( i ), std::make_shared< qm::LockFreeQueue< int > >( 100 ) );
     }

     auto time = boost::posix_time::microsec_clock::local_time();

     for ( std::size_t i = 0; i < workers; i++ )
     {
          auto producer = std::make_shared<qm::SimpleLoopProducerThread<std::string, int> >( std::to_string( i ), loops );
          if ( mpsc_manager.RegisterProducer( std::to_string( i ), producer ) == qm::State::Ok )
          {
               producer->Produce();
          }

          auto producer2 = std::make_shared<qm::SimpleLoopProducerThread<std::string, int> >( std::to_string( i ), loops );
          if ( mpsc_manager.RegisterProducer( std::to_string( i ), producer2 ) == qm::State::Ok )
          {
               producer2->Produce();
          }
     }

     for ( std::size_t i = 0; i < workers; i++ )
     {
          auto consumer = std::make_shared< qm::QueueConsumerThreadWorker< std::string, int > > ( std::to_string( i ) );
          mpsc_manager.Subscribe( std::to_string( i ), consumer );
     }

     //wait for consumer work done
     while ( !mpsc_manager.AreAllQueuesEmpty() || !mpsc_manager.AreAllProducersDone() )
     {
          std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
     }

     auto time_end = boost::posix_time::microsec_clock::local_time();
     std::cout << time_end - time << std::endl;

     std::cout << "produced " << qm::counter_ << " objects." << std::endl;
     std::cout << "consumed " << qm::consumer_counter_ << " objects." << std::endl;
}

int main(int argc, char* argv[])
{
     int loops;
     if ( argc > 1 )
     {
          try
          {
               loops = std::stoi( argv[1] );
          }
          catch ( const std::logic_error &ex)
          {
               std::cout << "Invalid loops argument:" << argv[1];
               return -1;
          }
     }
     else
     {
          loops = 1000;
     }

     LockFreeQueueWithProducerRegistration( loops );

     return 0;
}