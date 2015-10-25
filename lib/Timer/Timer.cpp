//
// Created by Jason Snow on 10/23/15.
//

#include "Timer.h"

Timer::Timer(unsigned long interval, void (*callback)()) {
    this->_interval = interval;
    this->_callback = callback;
    this->_running = false;
}

unsigned long Timer::interval() {
    return this->_interval;
}

void Timer::set_interval(unsigned long interval) {
    this->_interval = interval;
}

void Timer::set_callback(void (*callback)()) {
    this->_callback = callback;
}

void Timer::start() {
    this->start(false);
}

void Timer::start(bool delay) {
    this->_timer = millis();
    this->_running = true;

    if (!delay) {
        this->_callback();
    }
}

void Timer::run() {
    if (this->_running) {
        if ((millis() - this->_timer) >= this->_interval) {
            this->_timer = millis();
            this->_callback();
        }
    }
}

void Timer::stop() {
    this->_running = false;
    this->_timer = 0;
}

bool Timer::running() {
    return this->_running;
}