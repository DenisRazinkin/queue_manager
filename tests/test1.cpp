#include <iostream>

#include <map>

#include <boost/thread/thread.hpp>

#include <thread>
#include <atomic>

#include <boost/date_time/posix_time/posix_time.hpp>

#include <queue/block_concurrent_queue.hpp>
#include <queue/lock_free_queue.hpp>
#include <manager/mpsc_mqueue_manager.hpp>
#include <producer/base_producer.hpp>

#include <examples/consumer_thread.hpp>
#include <examples/producer.hpp>

std::atomic<int> producer_count(0);
std::atomic<int> consumer_count(0);

std::atomic<bool> done (false);
std::atomic<bool> produce (true);

int iterations = 1000;
const int producer_thread_count = 4;
const int consumer_thread_count = 1;

void producer(qm::IMultiQueueManager<std::string, int> *manager, int id, const std::string &key )
{
     for (int i = 0; i != iterations; ++i) {
          if ( !produce.load() ) return;
          producer_count++;
          //std::string msg = "producer: " + std::to_string( id ) + " value: " + std::to_string( producer_count ) + " \n";
          //std::cout << msg;
          qm::State state;

          //std::cout << "queue " << q << " enabled: " << q->Enabled() << " push value " << value << std::endl;
          while ( produce.load() && ( state = manager->Enqueue( key, producer_count ) ) != qm::State::Ok )
          {

               if ( state == qm::State::QueueDisabled )
               {
                    break;
               }

               continue;
          }
     }
}

void producerq( qm::IQueue<int> *q, int id, const std::string &key )
{
     //std::cout << "producer " << id << std::endl;
     for (int i = 0; i != iterations; ++i) {
          if ( !produce.load() ) return;
          producer_count++;
          //std::string msg = "producer: " + std::to_string( id ) + " value: " + std::to_string( producer_count ) + " \n";
          //std::cout << msg;
          qm::State state;

          //std::cout << "queue " << key << " enabled: " << q->Enabled() << " push value " << producer_count << std::endl;
          while ( produce.load() && ( state = q->Push( producer_count )) != qm::State::Ok )
          {

               if ( state == qm::State::QueueDisabled )
               {
                    break;
               }

               continue;
          }
     }
}


void consumer(qm::IQueue<int> *q, int id)
{
     while (!done || !q->Empty() ) {
          auto value = q->Pop();

          if ( value.has_value() )
          {
               //std::string msg = "consumer: " + std::to_string( id ) + " value: " + std::to_string( value.get() ) + " \n";
               //std::cout << msg;
               ++consumer_count;
          }
     }
}

void test1()
{
     qm::BlockConcurrentQueue<int> queue1( 100 );

     boost::thread_group producer_threads, consumer_threads;

     auto time = boost::posix_time::microsec_clock::local_time();

     for (int i = 0; i != producer_thread_count; ++i)
          producer_threads.create_thread(std::bind(&producerq, &queue1, i+1, "queue"));

     for (int i = 0; i != consumer_thread_count; ++i)
          consumer_threads.create_thread(std::bind(&consumer, &queue1, i+1));

     producer_threads.join_all();
     done = true;
     queue1.Stop();

     consumer_threads.join_all();

     auto time_end = boost::posix_time::microsec_clock::local_time();
     std::cout << time_end - time << std::endl;

     std::cout << "produced " << producer_count << " objects." << std::endl;
     std::cout << "consumed " << consumer_count << " objects." << std::endl;
}

void test2()
{
     qm::LockFreeQueue<int> queue( 100 );

     boost::thread_group producer_threads, consumer_threads;

     auto time = boost::posix_time::microsec_clock::local_time();
     //std::cout << boost::posix_time::milliseconds (time) << std::endl;
     //std::this_thread::sleep_for( std::chrono::milliseconds ( 100 ));

     for (int i = 0; i != producer_thread_count; ++i)
          producer_threads.create_thread(std::bind(&producerq, &queue, i+1, "queue"));

     for (int i = 0; i != consumer_thread_count; ++i)
          consumer_threads.create_thread(std::bind(&consumer, &queue, i+1));

     producer_threads.join_all();
     done = true;
     queue.Stop();

     consumer_threads.join_all();

     auto time_end = boost::posix_time::microsec_clock::local_time();
     std::cout << time_end - time << std::endl;

     std::cout << "produced " << producer_count << " objects." << std::endl;
     std::cout << "consumed " << consumer_count << " objects." << std::endl;
}

void test3()
{
     qm::MPSCQueueManager<std::string, int> mpsc_manager;

     auto q1 = std::make_shared< qm::BlockConcurrentQueue< int > >( 100 );
     auto q2 = std::make_shared< qm::BlockConcurrentQueue< int > >( 100 );
     mpsc_manager.AddQueue( "first", q1 );
     mpsc_manager.AddQueue( "second", q2 );

     auto time = boost::posix_time::microsec_clock::local_time();

     boost::thread_group producer_threads;
     for (int i = 0; i != producer_thread_count; ++i)
     {
          std::string queue;
          if ( i % 2 == 0 )
          {
               queue = "first";
               producer_threads.create_thread( std::bind( &producerq, q1.get(), i + 1, queue ) );
          }
          else
          {
               queue = "second";
               producer_threads.create_thread( std::bind( &producerq, q2.get(), i + 1, queue ) );
          }
     }


     qm::ConsumerPtr<std::string, int> consumer1 = std::make_shared<qm::QueueConsumerThreadWorker<std::string, int>>();
     auto sub_result = mpsc_manager.Subscribe( consumer1, "first" );
     if ( sub_result != qm::State::Ok )
     {
          std::cout << "Failed to subscribe: " << qm::StateStr( sub_result ) << std::endl;
          return;
     }

     auto consumer2 = std::make_shared<qm::QueueConsumerThreadWorker<std::string, int>>();
     sub_result = mpsc_manager.Subscribe( consumer2, "second" );
     if ( sub_result != qm::State::Ok )
     {
          std::cout << "Failed to subscribe: " << qm::StateStr( sub_result ) << std::endl;
          return;
     }


     //std::this_thread::sleep_for( std::chrono::milliseconds( 1000 ) );
     //mpsc_manager.Unsubscribe( "first" );
     //mpsc_manager.Unsubscribe( "second" );
     //produce.store( false );

     //mpsc_manager.StopProcessing();

     producer_threads.join_all();
     done = true;

     //wait for consumer work done
     while ( !q2->Empty() || !q1->Empty() )
     {
          std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
     }


     auto time_end = boost::posix_time::microsec_clock::local_time();
     std::cout << time_end - time << std::endl;

     std::cout << "produced " << producer_count << " objects." << std::endl;
     std::cout << "consumed " << qm::consumer_counter_ << " objects." << std::endl;
}

void test4()
{
     qm::MPSCQueueManager<std::string, int> mpsc_manager;

     auto q1 = std::make_shared< qm::LockFreeQueue< int > >( 100 );
     auto q2 = std::make_shared< qm::LockFreeQueue< int > >( 100 );
     auto q3 = std::make_shared< qm::LockFreeQueue< int > >( 100 );
     auto q4 = std::make_shared< qm::LockFreeQueue< int > >( 100 );
     mpsc_manager.AddQueue( "first", q1 );
     mpsc_manager.AddQueue( "second", q2 );
     mpsc_manager.AddQueue( "third", q3 );
     mpsc_manager.AddQueue( "fourth", q4 );

     auto time = boost::posix_time::microsec_clock::local_time();

     boost::thread_group producer_threads;

     std::string queue = "first";
     producer_threads.create_thread( std::bind( &producerq, q1.get(), 1, queue ) );
     queue = "second";
     producer_threads.create_thread( std::bind( &producerq, q2.get(), 2, queue ) );
     queue = "third";
     producer_threads.create_thread( std::bind( &producerq, q3.get(), 3, queue ) );
     queue = "fourth";
     producer_threads.create_thread( std::bind( &producerq, q4.get(), 4, queue ) );



     qm::ConsumerPtr<std::string, int> consumer1 = std::make_shared<qm::QueueConsumerThreadWorker<std::string, int>>();
     auto sub_result = mpsc_manager.Subscribe( consumer1, "first" );
     if ( sub_result != qm::State::Ok )
     {
          std::cout << "Failed to subscribe: " << qm::StateStr( sub_result ) << std::endl;
          return;
     }

     auto consumer2 = std::make_shared<qm::QueueConsumerThreadWorker<std::string, int>>();
     sub_result = mpsc_manager.Subscribe( consumer2, "second" );
     if ( sub_result != qm::State::Ok )
     {
          std::cout << "Failed to subscribe: " << qm::StateStr( sub_result ) << std::endl;
          return;
     }

     auto consumer3 = std::make_shared<qm::QueueConsumerThreadWorker<std::string, int>>();
     sub_result = mpsc_manager.Subscribe( consumer3, "third" );
     if ( sub_result != qm::State::Ok )
     {
          std::cout << "Failed to subscribe: " << qm::StateStr( sub_result ) << std::endl;
          return;
     }

     auto consumer4 = std::make_shared<qm::QueueConsumerThreadWorker<std::string, int>>();
     sub_result = mpsc_manager.Subscribe( consumer4, "fourth" );
     if ( sub_result != qm::State::Ok )
     {
          std::cout << "Failed to subscribe: " << qm::StateStr( sub_result ) << std::endl;
          return;
     }


     //std::this_thread::sleep_for( std::chrono::milliseconds( 1000 ) );
     //mpsc_manager.Unsubscribe( "first" );
     //mpsc_manager.Unsubscribe( "second" );
     //produce.store( false );

     //mpsc_manager.StopProcessing();

     producer_threads.join_all();
     done = true;

     //wait for consumer work done
     while ( !q2->Empty() || !q1->Empty() || !q1->Empty() || !q4->Empty() )
     {
          std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
     }


     auto time_end = boost::posix_time::microsec_clock::local_time();
     std::cout << time_end - time << std::endl;

     std::cout << "produced " << producer_count << " objects." << std::endl;
     std::cout << "consumed " << qm::consumer_counter_ << " objects." << std::endl;
}

void test5(int loops)
{


     auto workers = std::thread::hardware_concurrency() /2;
     qm::MPSCQueueManager<std::string, int> mpsc_manager;
     for ( int i = 0; i < workers; i++ )
     {
          mpsc_manager.AddQueue( std::to_string( i ), std::make_shared< qm::BlockConcurrentQueue< int > >( 100 ) );
     }

     auto time = boost::posix_time::microsec_clock::local_time();


     for ( int i = 0; i < workers; i++ )
     {
          auto producer = std::make_shared<qm::SimpleLoopProducerThread<std::string, int> >( std::to_string( i ), loops );

          if ( mpsc_manager.RegisterProducer( producer, std::to_string( i ) ) == qm::State::Ok )
          {
               producer->Produce();
          }

          auto producer2 = std::make_shared<qm::SimpleLoopProducerThread<std::string, int> >( std::to_string( i ), loops );

          if ( mpsc_manager.RegisterProducer( producer2, std::to_string( i ) ) == qm::State::Ok )
          {
               producer2->Produce();
          }
     }

     for ( int i = 0; i < workers; i++ )
     {
          auto consumer = std::make_shared< qm::QueueConsumerThreadWorker< std::string, int > > ();
          //mpsc_manager.Subscribe( std::make_shared<qm::QueueConsumerThreadWorker<std::string, int>>(), std::to_string( i ) );
          mpsc_manager.Subscribe( consumer, std::to_string( i ) );
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

void test6(int loops)
{


     auto workers = std::thread::hardware_concurrency() /2;
     auto mpsc_manager = std::make_shared< qm::MPSCQueueManager<std::string, int> >();
     for ( int i = 0; i < workers; i++ )
     {
          mpsc_manager->AddQueue( std::to_string( i ), std::make_shared< qm::BlockConcurrentQueue< int > >( 100 ) );
     }

     auto time = boost::posix_time::microsec_clock::local_time();

     std::vector<qm::ProducerPtr<std::string, int> > producers;
     for ( int i = 0; i < workers; i++ )
     {
          auto producer = std::make_shared<qm::EnqueueProducerThread<std::string, int> >( std::to_string( i ), loops, mpsc_manager );
          producers.push_back( producer );
          producer->Produce();

          auto producer2 = std::make_shared<qm::EnqueueProducerThread<std::string, int> >( std::to_string( i ), loops, mpsc_manager );
          producers.push_back( producer2 );
          producer2->Produce();


     }

     for ( int i = 0; i < workers; i++ )
     {
          mpsc_manager->Subscribe( std::make_shared<qm::QueueConsumerThreadWorker<std::string, int>>(), std::to_string( i ) );
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
     if ( argc > 1 )
     {
          iterations = std::stoi( argv[1] );
     }
     else
     {
          iterations = 1000;
     }


     /*std::cout << "TEST1\n";
     test1();
     producer_count = 0;
     consumer_count = 0;
     done = false;

     std::cout << "TEST2\n";
     test2();
     producer_count = 0;
     consumer_count = 0;
     done = false;

     std::cout << "TEST3 4 p 2 c\n";
     test3();

     producer_count = 0;
     consumer_count = 0;
     done = false;

     std::cout << "TEST4 4 lockfree queues, 4 producers, 4 consumers\n";
     qm::consumer_counter_ = 0;
     test4();

     producer_count = 0;
     consumer_count = 0;
     done = false;*/

     std::cout << "TEST5 harware concurrency producers/consumers\n";
     qm::consumer_counter_ = 0;
     qm::counter_ = 0;
     test5( iterations );

     producer_count = 0;
     consumer_count = 0;
     done = false;


     std::cout << "TEST6 harware concurrency producers/consumers, enqueue method\n";
     qm::consumer_counter_ = 0;
     qm::counter_ = 0;
     test6( iterations );

     producer_count = 0;
     consumer_count = 0;
     done = false;

     return 0;
}
