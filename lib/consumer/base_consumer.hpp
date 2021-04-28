/// @brief Base template class for consume
/// @author Denis Razinkin
#pragma once

#ifndef MQP_BASE_CONSUMER_H_
#define MQP_BASE_CONSUMER_H_

#include "common.h"

#include <iostream>

namespace qm
{

template< typename Value >
class IConsumer
{
public:
     /// @brief Constructor
     explicit IConsumer() = default;

     /// @brief Destructor
     virtual ~IConsumer() = default;

     /// @brief Is queue enabled
     /// @return true/false
     [[nodiscard]] inline bool Enabled() const;

     /// @brief Disable or enable queue
     /// @param enabled True - enbaled, false - disabled
     inline void Enabled( bool enabled );

public:
     /// @brief Pure virtual func for object processing
     /// @param obj Object
     virtual void Consume( const Value &obj ) = 0;

private:
     std::atomic< bool > enabled_ = true;
};

template< typename Value >
bool IConsumer< Value >::Enabled() const
{
     return enabled_.load();
}

template<  typename Value >
void IConsumer< Value >::Enabled( bool enabled )
{
     enabled_.store( enabled );
}

} // qm

#endif // MQP_BASE_CONSUMER_H_
