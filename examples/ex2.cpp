#include <boost/date_time/posix_time/posix_time.hpp>

#include <manager/mpsc_mqueue_manager.hpp>
#include <producer/base_producer.hpp>
#include <queue/block_concurrent_queue.hpp>
#include <queue/lock_free_queue.hpp>

#include "consumer_counter.hpp"
#include "producer_thread_loop_example.h"

template< class QueueType >
void SimpleLoopProducerEqueue( unsigned int loops, unsigned int producer_multiple )
{
     if ( producer_multiple == 0 )
     {
          std::cout << "Invalid producers multiple.";
          return;
     }

     unsigned int workers = std::thread::hardware_concurrency();
     std::cout << "threads:" << workers << std::endl;

     auto mpsc_manager = std::make_shared< qm::MPSCQueueManager<std::string, int> >();
     for ( std::size_t i = 0; i < workers; i++ )
     {
          mpsc_manager->AddQueue( std::to_string( i ), std::make_shared< QueueType >( 100 ) );
     }

     auto time = boost::posix_time::microsec_clock::local_time();

     std::vector<qm::ProducerPtr<std::string, int> > producers;
     for ( std::size_t i = 0; i < workers * producer_multiple; i++ )
     {
          auto producer = std::make_shared<qm::example::EnqueueProducerThread<std::string, int> >( std::to_string( i / producer_multiple ), loops, mpsc_manager );
          producers.push_back( producer );
          producer->Produce();
     }

     for ( std::size_t i = 0; i < workers; i++ )
     {
          mpsc_manager->Subscribe( std::to_string( i ), std::make_shared< qm::example::ConsumerCounter< std::string, int > >( std::to_string( i )) );
     }

     while (  ! std::all_of( producers.begin(), producers.end(),
                             [] ( const auto& producer ) { return producer->Done(); } ) )
     {
          std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
     }

     //wait for consumer work done
     while ( !mpsc_manager->AreAllQueuesEmpty() )
     {
          std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
     }


     auto time_end = boost::posix_time::microsec_clock::local_time();
     std::cout << "Total time: " << time_end - time << std::endl;

     std::cout << "produced " << qm::example::produce_counter_ << " objects." << std::endl;
     std::cout << "consumed " << qm::example::consumer_counter_ << " objects." << std::endl;

     qm::example::produce_counter_ = 0;
     qm::example::consumer_counter_ = 0;
}

int main(int argc, char* argv[])
{
     unsigned int loops;
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

     std::cout << "Example 1.1: blocking queue with 1 unregistered producer and 1 consumer per 1 cpu core\n";
     SimpleLoopProducerEqueue< qm::BlockConcurrentQueue< int > >( loops, 10 );

     std::cout << "Example 1.2: lockfree queue with 1 unregistered producer and 1 consumer per 1 cpu core\n";
     SimpleLoopProducerEqueue< qm::LockFreeQueue< int > >( loops, 10 );

     return 0;
}