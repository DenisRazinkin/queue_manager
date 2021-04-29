#pragma once

#ifndef MQP_QUEUE_STATE_H_
#define MQP_QUEUE_STATE_H_

namespace qm
{

/// @brief enum class describes queue state
enum class State
{
     Ok,                 ///< Queue is ok, nothing to do
     QueueFull,          ///< Queue is full, cannot add new objects
     QueueEmpty,         ///< Queue is empty, cannot pop object from queue
     QueueExists,        ///< Same queue exists for this key
     QueueBusy,          ///< Queue is busy ( by other producer or consumer )
     QueueAbsent,        ///< Queue is absent
     QueueDisabled,      ///< Queue is disabled, operation impossible
     ProducerNotFound    ///< Producer is not found for queue
};

/// @brief Text representation of queue state
/// @param s - queue state
/// @return - text description of queue state
inline std::string StateStr( State s )
{
     switch ( s )
     {
          case State::Ok:
               return "ok";
          case State::QueueFull:
               return "queue is full";
          case State::QueueEmpty:
               return "queue is empty";
          case State::QueueExists:
               return "queue exists";
          case State::QueueBusy:
               return "queue is busy";
          case State::QueueAbsent:
               return "queue is absent";
          case State::QueueDisabled:
               return "queue is disabled";
          case State::ProducerNotFound:
               return "producer is not found";
     }
}

} // qm

#endif // MQP_QUEUE_STATE_H_
