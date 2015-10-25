//
// Created by Jason Snow on 10/23/15.
//

#ifndef THERMOSTAT_TIMER_H
#define THERMOSTAT_TIMER_H

#include <Arduino.h>

#define MICRO_SECONDS(x) (x)
#define SECONDS(x) (x * 1000UL)
#define MINUTES(x) (x * 60000UL)

class Timer {
private:
    unsigned long _interval;
    unsigned long _timer;
    void (*_callback)(void);
    bool _running;
public:
    Timer(unsigned long interval, void (*callback)(void));
    unsigned long interval();
    void set_interval(unsigned long interval);
    void set_callback(void (*callback)(void));
    void start();
    void start(bool delay);
    void run();
    void stop();
    bool running();
};

#endif //THERMOSTAT_TIMER_H
