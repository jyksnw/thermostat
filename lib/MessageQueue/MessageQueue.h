//
// Created by Jason Snow on 10/26/15.
//

#ifndef THERMOSTAT_MESSAGEQUEUE_H
#define THERMOSTAT_MESSAGEQUEUE_H

#include <Arduino.h>

#define QUEUE_SIZE 5

#define QUEUE_STATUS_OK     0
#define QUEUE_STATUS_FULL  -1
#define QUEUE_STATUS_EMPTY -2

struct message {
    char topic[50];
    char message[25];
};

typedef struct message Message;

class MessageQueue {
private:
    Message _queue[QUEUE_SIZE];
    int _head = 0;
    int _tail = 0;
    int _status = QUEUE_STATUS_EMPTY;
public:
    /**
     * Adds the message to the queue
     */
    int push(Message m);

    /**
     * Gets and removes the first message in the queue
     */
    Message pop();

    /**
     * Get but doesn't remove the first message in the queue
     */
    Message peak();

    /**
     * Checks if the queue is empty
     */
    bool isEmpty();

    /**
     * Returns the current queue status
     */
    int status();
};


#endif //THERMOSTAT_MESSAGEQUEUE_H
