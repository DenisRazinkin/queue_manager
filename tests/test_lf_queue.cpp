#include <future>

#include <gtest/gtest.h>

#include <queue/lock_free_queue.hpp>

TEST(LockFreeQueue, push_pop)
{
     qm::LockFreeQueue<int> queue( 10 );

     int a = 1;
     int b = 1;

     auto state = queue.Push( a );
     ASSERT_EQ( state, qm::State::Ok );
     auto value = queue.Pop();
     ASSERT_TRUE( value.has_value() );
     ASSERT_NE( value, std::nullopt );
     ASSERT_EQ( value.value(), a );

     state = queue.Push( a );
     ASSERT_EQ( state, qm::State::Ok );

     state = queue.Push( b );
     ASSERT_EQ( state, qm::State::Ok );

     value = queue.Pop();
     ASSERT_TRUE( value.has_value() );
     ASSERT_NE( value, std::nullopt );
     ASSERT_EQ( value.value(), b );

     value = queue.Pop();
     ASSERT_TRUE( value.has_value() );
     ASSERT_NE( value, std::nullopt );
     ASSERT_EQ( value.value(), b );

     auto pop_future = std::async( std::launch::async, [&queue] ()
     {
          auto value = queue.Pop();
          ASSERT_FALSE( value.has_value() );
     });

     queue.Enabled( false );
     ASSERT_FALSE( queue.Enabled() );
}

TEST(LockFreeQueue, full_queue)
{
     std::vector<int> values = { 1, 2, 3 };
     qm::LockFreeQueue<int> queue( values.size() );
     ASSERT_TRUE( queue.Empty() );

     for ( const int &value : values )
     {
          auto state = queue.Push( value );
          ASSERT_EQ( state, qm::State::Ok );
     }

     auto state = queue.TryPush( 4 );
     ASSERT_EQ( state, qm::State::QueueFull );


}

TEST(LockFreeQueue, enable_disable_queue)
{
     std::vector<int> values = { 1, 2, 3 };
     qm::LockFreeQueue<int> queue( values.size() );
     ASSERT_TRUE( queue.Empty() );

     for ( const int &value : values )
     {
          auto state = queue.Push( value );
          ASSERT_EQ( state, qm::State::Ok );
     }

     queue.Enabled( false );
     ASSERT_FALSE( queue.Enabled() );

     queue.Pop();
     auto state = queue.Push( 4 );
     ASSERT_EQ( state, qm::State::QueueDisabled );

     queue.Enabled( true );
     state = queue.Push( 4 );
     ASSERT_EQ( state, qm::State::Ok );

     queue.Stop();
     ASSERT_FALSE( queue.Enabled() );
     state = queue.Push( 5 );
     ASSERT_EQ( state, qm::State::QueueDisabled );
}