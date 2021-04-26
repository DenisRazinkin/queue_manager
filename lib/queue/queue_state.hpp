#pragma once

#ifndef MQP_QUEUE_STATE_H_
#define MQP_QUEUE_STATE_H_

namespace qm
{

enum class State
{
    Ok,
    QueueFull,
    QueueEmpty,
    QueueBusy,
    QueueAbsent,
    QueueDisabled
};

std::string StateStr(State s) {
    switch (s) {
        case State::QueueFull:
            return "queue is full";
        case State::QueueEmpty:
            return "queue is empty";
        case State::QueueBusy:
            return "queue is busy";
        case State::QueueAbsent:
            return "queue is absent";

        default:
            return "unknown state";
    }
}

} // qm

#endif // MQP_QUEUE_STATE_H_
