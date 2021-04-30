/// @brief Base template producer class
/// @author Denis Razinkin
#pragma once

#ifndef MQP_BASE_PRODUCER_H_
#define MQP_BASE_PRODUCER_H_

#include "common.h"

#include <iostream>
#include <thread>

#include "manager/base_mqueue_manager.hpp"

namespace qm
{

/// @brief Base template producer class
/// @tparam Key Type for queues map store.
/// @tparam Value Type for queue store
template< typename Key, typename Value >
class IProducer
{
public:
     /// @brief Constructor
     /// @param id Key id
     explicit IProducer( Key id );

     /// @brief Destructor
     virtual ~IProducer() = default;

     /// @brief Is producer enabled
     /// @return true/false
     /// @details Thread safe
     [[nodiscard]] inline bool Enabled() const;

     /// @brief Set is producer enabled
     /// @param enabled - true/false
     /// @details Thread safe
     inline void Enabled( bool enabled );

     /// @brief Is producer`s work done
     /// @return true/false
     /// @details Thread safe
     [[nodiscard]] inline bool Done() const;

public:
     /// @brief Producer work function
     /// Producer must check enabled_ value and breaks processing on disable signal
     /// Also, it should set done = true when work is done
     /// Example:
     /// Value obj = GetProducedValue();
     /// while ( IProducer< Key, Value >::enabled_.load() &&
     //           state = IProducer< Key, Value >::queue_->Push( obj ) ) != qm::State::Ok )
     //  {
     //       if ( state == qm::State::QueueDisabled )
     //       {
     //              break;
     //       }
     //  }
     //  IProducer< Key, Value >::done_ = true;
     virtual void Produce() = 0;

     /// @brief Waiting for producer's thread has done
     virtual void WaitThreadDone() = 0;

protected:
     Key id_;
     std::atomic< bool > done_;
     std::atomic< bool > enabled_;
     QueuePtr <Value> queue_;

private:
     void SetQueue( QueuePtr <Value> queue );
     friend class IMultiQueueManager< Key, Value >;
};

template< typename Key, typename Value >
IProducer< Key, Value >::IProducer( Key id ) : id_( id ), done_( false ), enabled_( true )
{}

template< typename Key, typename Value >
void IProducer< Key, Value >::Enabled( bool enabled )
{
     enabled_.store( enabled );
}

template< typename Key, typename Value >
bool IProducer< Key, Value >::Enabled() const
{
     return enabled_.load();
}

template< typename Key, typename Value >
bool IProducer< Key, Value >::Done() const
{
     return done_;
}

template< typename Key, typename Value >
void IProducer< Key, Value >::SetQueue( QueuePtr< Value > queue )
{
     queue_ = queue;
}

} // qm

#endif // MQP_BASE_PRODUCER_H_
