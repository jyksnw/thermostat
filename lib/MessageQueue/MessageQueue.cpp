//
// Created by Jason Snow on 10/26/15.
//

#include "MessageQueue.h"

int MessageQueue::push(Message m) {
    int tail = (_tail + 1) % QUEUE_SIZE;

    if (tail != _head) {
        _queue[_tail] = m;
        _tail = tail;

        _status = QUEUE_STATUS_OK;
        return _status;
    } else {
        _status = QUEUE_STATUS_FULL;
        return _status;
    }
}

Message MessageQueue::pop() {
    if (this->isEmpty()) {
        _status = QUEUE_STATUS_EMPTY;
        return {};
    }

    Message m = _queue[_head];
    _head = (_head + 1) % QUEUE_SIZE;

    return m;
}

Message MessageQueue::peak() {
    if (this->isEmpty()) {
        _status = QUEUE_STATUS_EMPTY;
        return {};
    }

    return _queue[_head];
}

bool MessageQueue::isEmpty() {
    return (_head == _tail);
}

int MessageQueue::status() {
    return _status;
}