#include <boost/date_time/posix_time/posix_time.hpp>

#include <queue/block_concurrent_queue.hpp>
#include <queue/lock_free_queue.hpp>
#include <manager/mpsc_mqueue_manager.hpp>
#include <producer/base_producer.hpp>

#include "consumer_thread.hpp"
#include "producer.hpp"

void ConcurrentQeueuWithManagerEnqueue( int loops )
{
     auto workers = std::thread::hardware_concurrency() /2;
     auto mpsc_manager = std::make_shared< qm::MPSCQueueManager<std::string, int> >();
     for ( std::size_t i = 0; i < workers; i++ )
     {
          mpsc_manager->AddQueue( std::to_string( i ), std::make_shared< qm::BlockConcurrentQueue< int > >( 100 ) );
     }

     auto time = boost::posix_time::microsec_clock::local_time();

     std::vector<qm::ProducerPtr<std::string, int> > producers;
     for ( std::size_t i = 0; i < workers; i++ )
     {
          auto producer = std::make_shared<qm::EnqueueProducerThread<std::string, int> >( std::to_string( i ), loops, mpsc_manager );
          producers.push_back( producer );
          producer->Produce();

          auto producer2 = std::make_shared<qm::EnqueueProducerThread<std::string, int> >( std::to_string( i ), loops, mpsc_manager );
          producers.push_back( producer2 );
          producer2->Produce();
     }

     for ( std::size_t i = 0; i < workers; i++ )
     {
          mpsc_manager->Subscribe( std::to_string( i ), std::make_shared< qm::QueueConsumerThreadWorker< int > >() );
     }

     while (  ! std::all_of( producers.begin(), producers.end(),
                             [] ( const auto& producer ) { return producer->Done(); } ) )
     {
          std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
     }

     //wait for consumer work done
     while ( !mpsc_manager->AreAllQueuesEmpty() || !mpsc_manager->AreAllProducersDone() )
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

     ConcurrentQeueuWithManagerEnqueue( loops );

     return 0;
}