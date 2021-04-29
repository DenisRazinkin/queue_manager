#include <future>

#include <gtest/gtest.h>

#include <manager/mpsc_mqueue_manager.hpp>
#include <queue/block_concurrent_queue.hpp>
#include <queue/lock_free_queue.hpp>
#include <consumer/base_consumer.hpp>

class QueueTestConsumer : public qm::IConsumer< int >
{
public:
     explicit QueueTestConsumer() = default;
     ~QueueTestConsumer() override = default;

     void Consume( const int &value ) override
     {
          std::string msg = "consumer_counter: " + std::to_string( consumer_counter_ ) +
                  " new value: " + std::to_string( value ) + "\n";
          std::cout << msg;
          consumer_counter_+=value;
     };

     int Result()
     {
          return consumer_counter_;
     }
private:
    int consumer_counter_ = 0;
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
}