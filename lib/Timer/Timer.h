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
    /**
     * Constructor
     */
    Timer(unsigned long interval, void (*callback)(void));

    /**
     * Returns the currently set timer interval
     */
    unsigned long interval();

    /**
     * Sets the timer interval
     */
    void set_interval(unsigned long interval);

    /**
     * Sets the callback function
     */
    void set_callback(void (*callback)(void));

    /**
     * Starts the timer without delay
     */
    void start();

    /**
     * Starts the timer
     * Delays execution of callback if delay=true
     */
    void start(bool delay);

    /**
     * Runs the timer
     * NOTE: should be called in the main program loop
     */
    void run();

    /**
     * Stops the timer
     */
    void stop();

    /**
     * Checks the running state of the timer
     */
    bool running();
};

#endif //THERMOSTAT_TIMER_H
