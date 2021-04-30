#include <future>

#include <gtest/gtest.h>

#include <manager/mpsc_mqueue_manager.hpp>
#include <queue/block_concurrent_queue.hpp>
#include <queue/lock_free_queue.hpp>
#include <consumer/base_consumer.hpp>
#include <producer/base_producer.hpp>

class QueueTestConsumer : public qm::IConsumer< int >
{
public:
     explicit QueueTestConsumer() = default;
     ~QueueTestConsumer() override = default;

     void Consume( const int &value ) override
     {
         // std::string msg = "consumer_counter: " + std::to_string( consumer_counter_ ) +
          //        " new value: " + std::to_string( value ) + "\n";
          //std::cout << msg;
          consumer_counter_+=value;
     };

     int Result()
     {
          return consumer_counter_;
     }
private:
    int consumer_counter_ = 0;
};

/// @brief Produce values from 1 to n
class SequenceValuesProducer : public qm::IProducer< std::string, int >
{
public:
     SequenceValuesProducer( const std::string &id, int n )
          : IProducer< std::string, int >( id ), produced_values_( 0 ), n_( n )
     {};

     ~SequenceValuesProducer() override
     {
          if ( thread_.joinable() ) thread_.join();
     };

     void WaitThreadDone() override
     {
          if ( thread_.joinable() ) thread_.join();
     }

     int Produced() const
     {
          return produced_values_;
     }

     void Produce() override
     {
          if ( IProducer< std::string, int >::queue_ == nullptr )
          {
               std::cout << "queue is null!";
               return;
          }

          auto producer = [ this ] ()
          {
               for ( int i = 1; i < n_ + 1; ++i )
               {
                    if ( !enabled_.load() ) return;
                    qm::State state;
                    while ( enabled_.load() && ( state = queue_->Push( i ) ) != qm::State::Ok )
                    {
                         if ( state == qm::State::QueueDisabled )
                         {
                              break;
                         }
                    }

                    if ( state == qm::State::Ok )
                    {
                         produced_values_++;
                    }
               }

               done_ = true;
          };

          thread_ = std::thread( producer );
     }

private:
     std::atomic< int > produced_values_;
     std::thread thread_;
     int n_;
};

class TestMpsc : public ::testing::Test
{
protected:
     void SetUp() override
     {
          manager = std::make_shared< qm::MPSCQueueManager<std::string, int> >();
     }
     void TearDown() override
     {
          manager.reset();
     }

     qm::ManagerPtr<std::string, int> manager;
};

int Accumulate( int n )
{
     int result = 0;
     for ( int i = 1; i < n + 1; i++ )
     {
          result += i;
     }

     return result;
}

TEST_F(TestMpsc, add_queue)
{
     auto queue = std::make_shared< qm::BlockConcurrentQueue< int > >( 100 );
     auto state = manager->AddQueue( "queue1", queue );
     ASSERT_EQ(state, qm::State::Ok);
     state = manager->AddQueue( "queue1", std::make_shared< qm::LockFreeQueue< int > >( 1 ) );
     ASSERT_EQ(state, qm::State::QueueExists);

     auto get_queue_res = manager->GetQueue( "queue1" );
     ASSERT_EQ( get_queue_res.s_, qm::State::Ok );
     ASSERT_EQ( get_queue_res.queue_, queue );
}

TEST_F(TestMpsc, remove_queue)
{
     auto queue = std::make_shared< qm::BlockConcurrentQueue< int > >( 100 );
     auto state = manager->AddQueue( "queue1", queue );
     ASSERT_EQ(state, qm::State::Ok);

     state = manager->RemoveQueue( std::string("queue1") );
     ASSERT_FALSE(queue->Enabled() );
     ASSERT_EQ(state, qm::State::Ok);

     state = manager->RemoveQueue( "queue1" );
     ASSERT_EQ(state, qm::State::QueueAbsent);

     auto get_queue_res = manager->GetQueue( "queue1" );
     ASSERT_EQ( get_queue_res.s_, qm::State::QueueAbsent );
     ASSERT_EQ( get_queue_res.queue_, nullptr );

     ASSERT_TRUE( manager->AreAllQueuesEmpty() );
}

TEST_F(TestMpsc, enqueue)
{
     auto queue = std::make_shared< qm::BlockConcurrentQueue< int > >( 100 );
     auto state = manager->AddQueue( "queue1", queue );
     ASSERT_EQ(state, qm::State::Ok);

     state = manager->Enqueue( "queue2", 1 );
     ASSERT_EQ(state, qm::State::QueueAbsent);

     state = manager->Enqueue( "queue1", 1 );
     ASSERT_EQ(state, qm::State::Ok);

     auto value = queue->Pop();
     ASSERT_TRUE( value.has_value() );
     ASSERT_EQ(value.value(), 1 );
}

TEST_F(TestMpsc, subscribe_stop_start)
{
     auto queue = std::make_shared< qm::BlockConcurrentQueue< int > >( 100 );
     auto state = manager->AddQueue( "queue1", queue );

     state = manager->Enqueue( "queue1", 1 );
     ASSERT_EQ(state, qm::State::Ok);

     auto consumer = std::make_shared<QueueTestConsumer>();
     state = manager->Subscribe( "queue1", consumer );
     ASSERT_EQ(state, qm::State::Ok);

     manager->StopProcessing();
     ASSERT_EQ( consumer->Result(), 1 );

     state = manager->Enqueue( "queue1", 1 );
     ASSERT_EQ(state, qm::State::QueueDisabled);

     manager->StartProcessing();
     state = manager->Enqueue( "queue1", 1 );
     ASSERT_EQ(state, qm::State::Ok);
     manager->StopProcessing();
     ASSERT_EQ( consumer->Result(), 2 );
}

TEST_F(TestMpsc, subscribe_unsubscribe)
{
     auto queue = std::make_shared< qm::BlockConcurrentQueue< int > >( 100 );
     auto state = manager->AddQueue( "queue1", queue );

     auto consumer = std::make_shared<QueueTestConsumer>();
     state = manager->Subscribe( "queue1", consumer );
     ASSERT_EQ(state, qm::State::Ok);

     auto consumer2 = std::make_shared<QueueTestConsumer>();
     state = manager->Subscribe( "queue1", consumer2 );
     ASSERT_EQ(state, qm::State::QueueBusy);

     state = manager->Unsubscribe( "queue2" );
     ASSERT_EQ(state, qm::State::QueueAbsent);

     state = manager->Unsubscribe( "queue1" );
     ASSERT_EQ(state, qm::State::Ok);

     state = manager->Subscribe( "queue1", consumer2 );
     ASSERT_EQ(state, qm::State::Ok);

     state = manager->Enqueue( "queue1", 1 );
     ASSERT_EQ(state, qm::State::Ok);
     manager->StopProcessing();
     ASSERT_EQ( consumer2->Result(), 1 );
}


TEST_F(TestMpsc, register_producer)
{
     auto queue = std::make_shared< qm::BlockConcurrentQueue< int > >( 100 );
     auto state = manager->AddQueue( "queue1", queue );

     auto consumer = std::make_shared<QueueTestConsumer>();
     state = manager->Subscribe( "queue1", consumer );
     ASSERT_EQ( state, qm::State::Ok );

     const int values_count = 1000;
     auto producer = std::make_shared<SequenceValuesProducer>( "queue1", values_count );
     state = manager->RegisterProducer( "queue2", producer );
     ASSERT_EQ( state, qm::State::QueueAbsent );

     state = manager->RegisterProducer( "queue1", producer );
     ASSERT_EQ( state, qm::State::Ok );

     producer->Produce();
     producer->WaitThreadDone();

     manager->StopProcessing();
     ASSERT_EQ( consumer->Result(), Accumulate( values_count ) );
}

TEST_F(TestMpsc, register_unregister)
{
     auto queue = std::make_shared< qm::BlockConcurrentQueue< int > >( 100 );
     auto state = manager->AddQueue( "queue1", queue );

     auto consumer = std::make_shared<QueueTestConsumer>();
     state = manager->Subscribe( "queue1", consumer );
     ASSERT_EQ( state, qm::State::Ok );

     const int values_count = 10000;
     auto producer = std::make_shared<SequenceValuesProducer>( "queue1", values_count );

     state = manager->RegisterProducer( "queue1", producer );
     ASSERT_EQ( state, qm::State::Ok );

     producer->Produce();
     std::this_thread::sleep_for( std::chrono::nanoseconds( 1 ) );

     state = manager->UnregisterProducer( "queue1", producer );
     ASSERT_EQ( state, qm::State::Ok );

     manager->StopProcessing();
     ASSERT_EQ( consumer->Result(), Accumulate( producer->Produced() ) );
}

TEST_F(TestMpsc, register_subscribe_unsibscribe)
{
     auto queue = std::make_shared< qm::BlockConcurrentQueue< int > >( 100 );
     auto state = manager->AddQueue( "queue1", queue );

     auto consumer = std::make_shared<QueueTestConsumer>();
     state = manager->Subscribe( "queue1", consumer );
     ASSERT_EQ( state, qm::State::Ok );

     const int values_count = 10000;
     auto producer = std::make_shared<SequenceValuesProducer>( "queue1", values_count );

     state = manager->RegisterProducer( "queue1", producer );
     ASSERT_EQ( state, qm::State::Ok );

     producer->Produce();
     std::this_thread::sleep_for( std::chrono::nanoseconds( 1 ) );

     state = manager->Unsubscribe( "queue1" );
     ASSERT_EQ(state, qm::State::Ok);

     auto consumer2 = std::make_shared<QueueTestConsumer>();
     state = manager->Subscribe( "queue1", consumer2 );
     ASSERT_EQ( state, qm::State::Ok );

     manager->StopProcessing();
     ASSERT_EQ( consumer->Result() + consumer2->Result(), Accumulate( producer->Produced() ) );
}