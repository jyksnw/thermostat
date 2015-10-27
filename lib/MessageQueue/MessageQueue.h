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
    int push(Message m);
    Message pop();
    Message peak();
    bool isEmpty();
    int status();
};


#endif //THERMOSTAT_MESSAGEQUEUE_H
