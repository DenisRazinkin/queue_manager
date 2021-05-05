#include <benchmark/benchmark.h>

#include <manager/mpsc_mqueue_manager.hpp>
#include <producer/base_producer.hpp>
#include <queue/block_concurrent_queue.hpp>
#include <queue/lock_free_queue.hpp>

#include "examples/consumer_counter.h"
#include "examples/producer_thread_loop_example.h"

template< class QueueType >
void SimpleLoopProducerRegistration( unsigned int workers, unsigned int loops, unsigned int producer_multiple )
{
     qm::example::produce_counter_ = 0;
     qm::example::consumer_counter_ = 0;
     if ( producer_multiple == 0 )
     {
          std::cout << "Invalid producers multiple.";
          return;
     }

     auto mpsc_manager = qm::MPSCQueueManager<std::string, int>();
     for ( std::size_t i = 0; i < workers; i++ )
     {
          mpsc_manager.AddQueue( std::to_string( i ), std::make_shared< QueueType >( 100 ) );
     }

     for ( std::size_t i = 0; i < workers * producer_multiple; i++ )
     {
          auto producer = std::make_shared< qm::example::SimpleLoopProducerThread<std::string, int> >( std::to_string( i / producer_multiple ), loops );
          if ( mpsc_manager.RegisterProducer( std::to_string( i / producer_multiple ), producer ) == qm::State::Ok )
          {
               producer->Produce();
          }
     }

     for ( std::size_t i = 0; i < workers; i++ )
     {
          auto consumer = std::make_shared< qm::example::ConsumerCounter< std::string, int > > ( std::to_string( i ) );
          mpsc_manager.Subscribe( std::to_string( i ), consumer );
     }

     //wait for consumer work done
     while ( !mpsc_manager.AreAllQueuesEmpty() || !mpsc_manager.AreAllProducersDone() )
     {
          std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
     }
}

template< class Queue>
static void TestQueue(benchmark::State& state) {

     for (auto _ : state)
          SimpleLoopProducerRegistration< Queue >( state.range(0),
                  state.range(1 ), state.range(2 ) );
}
BENCHMARK_TEMPLATE(TestQueue, qm::BlockConcurrentQueue< int > )->Unit(benchmark::kMillisecond)
               ->Args( { std::thread::hardware_concurrency(), 1000, 1} )
               ->Args( { std::thread::hardware_concurrency(), 1000, 4} )
               ->Args( { std::thread::hardware_concurrency(), 1000, 16} )
               ->Args( { std::thread::hardware_concurrency(), 100000, 1} )
               ->Args( { std::thread::hardware_concurrency(), 100000, 4} )
               ->Args( { std::thread::hardware_concurrency(), 100000, 16} )
               ->Args( { std::thread::hardware_concurrency() * 8, 1000, 1} )
               ->Args( { std::thread::hardware_concurrency() * 8, 1000, 4} )
               ->Args( { std::thread::hardware_concurrency() * 8, 1000, 16} )
               ->Args( { std::thread::hardware_concurrency() * 8, 100000, 1} )
               ->Args( { std::thread::hardware_concurrency() * 8, 100000, 4} );
BENCHMARK_TEMPLATE(TestQueue, qm::LockFreeQueue< int > )->Unit(benchmark::kMillisecond)
               ->Args( { std::thread::hardware_concurrency(), 1000, 1} )
               ->Args( { std::thread::hardware_concurrency(), 1000, 4} )
               ->Args( { std::thread::hardware_concurrency(), 1000, 16} )
               ->Args( { std::thread::hardware_concurrency(), 100000, 1} )
               ->Args( { std::thread::hardware_concurrency(), 100000, 4} )
               ->Args( { std::thread::hardware_concurrency(), 100000, 16} )
               ->Args( { std::thread::hardware_concurrency() * 8, 1000, 1} )
               ->Args( { std::thread::hardware_concurrency() * 8, 1000, 4} )
               ->Args( { std::thread::hardware_concurrency() * 8, 1000, 16} )
               ->Args( { std::thread::hardware_concurrency() * 8, 100000, 1} )
               ->Args( { std::thread::hardware_concurrency() * 8, 100000, 4} );

template< class QueueType >
void EnqueueProducerNoRegistration( unsigned int workers, unsigned int loops, unsigned int producer_multiple )
{
     qm::example::produce_counter_ = 0;
     qm::example::consumer_counter_ = 0;
     if ( producer_multiple == 0 )
     {
          std::cout << "Invalid producers multiple.";
          return;
     }

     auto mpsc_manager = std::make_shared< qm::MPSCQueueManager< std::string, int > >();
     for ( std::size_t i = 0; i < workers; i++ )
     {
          mpsc_manager->AddQueue( std::to_string( i ), std::make_shared< QueueType >( 100 ) );
     }

     std::vector<qm::ProducerPtr<std::string, int> > producers;
     for ( std::size_t i = 0; i < workers * producer_multiple; i++ )
     {
          auto producer = std::make_shared<qm::example::EnqueueProducerThread<std::string, int> >( std::to_string( i / producer_multiple ), loops, mpsc_manager );
          producers.push_back( producer );
          producer->Produce();
     }

     for ( std::size_t i = 0; i < workers; i++ )
     {
          auto consumer = std::make_shared< qm::example::ConsumerCounter< std::string, int > > ( std::to_string( i ) );
          mpsc_manager->Subscribe( std::to_string( i ), consumer );
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
}

template< class Queue>
static void TestQueueNoRegistration(benchmark::State& state) {

     for (auto _ : state)
          EnqueueProducerNoRegistration< Queue >( state.range(0),
                                                   state.range(1 ), state.range(2 ) );
}
BENCHMARK_TEMPLATE(TestQueueNoRegistration, qm::BlockConcurrentQueue< int > )->Unit(benchmark::kMillisecond)
->Args( { std::thread::hardware_concurrency(), 1000, 1} )
->Args( { std::thread::hardware_concurrency(), 1000, 4} )
->Args( { std::thread::hardware_concurrency(), 1000, 16} )
->Args( { std::thread::hardware_concurrency(), 100000, 1} )
->Args( { std::thread::hardware_concurrency(), 100000, 4} )
->Args( { std::thread::hardware_concurrency() * 8, 1000, 1} )
->Args( { std::thread::hardware_concurrency() * 8, 1000, 4} );

BENCHMARK_TEMPLATE(TestQueueNoRegistration, qm::LockFreeQueue< int > )->Unit(benchmark::kMillisecond)
->Args( { std::thread::hardware_concurrency(), 1000, 1} )
->Args( { std::thread::hardware_concurrency(), 1000, 4} )
->Args( { std::thread::hardware_concurrency(), 1000, 16} )
->Args( { std::thread::hardware_concurrency(), 100000, 1} )
->Args( { std::thread::hardware_concurrency(), 100000, 4} )
->Args( { std::thread::hardware_concurrency() * 8, 1000, 1} )
->Args( { std::thread::hardware_concurrency() * 8, 1000, 4} );

BENCHMARK_MAIN();