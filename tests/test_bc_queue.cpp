#include <future>

#include <gtest/gtest.h>

#include <queue/block_concurrent_queue.hpp>

TEST(BlockConcurrentQueue, push_pop)
{
     qm::BlockConcurrentQueue<int> queue( 10 );

     int a = 1;
     int b = 1;

     auto state = queue.Push( a );
     ASSERT_TRUE( state == qm::State::Ok );
     auto value = queue.Pop();
     ASSERT_TRUE( value.has_value() );
     ASSERT_TRUE( value != std::nullopt );
     ASSERT_TRUE( value.value() == a );

     state = queue.Push( a );
     ASSERT_TRUE( state == qm::State::Ok );

     state = queue.Push( b );
     ASSERT_TRUE( state == qm::State::Ok );

     value = queue.Pop();
     ASSERT_TRUE( value.has_value() );
     ASSERT_TRUE( value != std::nullopt );
     ASSERT_TRUE( value.value() == b );

     value = queue.Pop();
     ASSERT_TRUE( value.has_value() );
     ASSERT_TRUE( value != std::nullopt );
     ASSERT_TRUE( value.value() == b );

     auto pop_future = std::async( std::launch::async, [&queue] ()
     {
          auto value = queue.Pop();
          ASSERT_FALSE( value.has_value() );
     });

     queue.Enabled( false );
     ASSERT_FALSE( queue.Enabled() );
}

TEST(BlockConcurrentQueue, full_queue)
{
     std::vector<int> values = { 1, 2, 3 };
     qm::BlockConcurrentQueue<int> queue( values.size() );
     ASSERT_TRUE( queue.Empty() );

     for ( const int &value : values )
     {
          queue.Push( value );
     }
     ASSERT_TRUE( queue.Size() == values.size() );

     auto state = queue.TryPush( 4 );
     ASSERT_TRUE( state == qm::State::QueueFull );

     auto push_future = std::async( std::launch::async, [&queue] ()
          {
               auto state = queue.Push( 4 );
               ASSERT_TRUE( state == qm::State::QueueFull );
          });

     queue.Enabled( false );
     ASSERT_FALSE( queue.Enabled() );
}