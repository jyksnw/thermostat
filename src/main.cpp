//
// Created by Jason Snow on 10/23/15.
//
// TODO: Wifi setup using a SoftAP and EEPROM
// TODO: add in an error and/or state publication (so the server can known about things like can't get temp, etc...)

#include <Arduino.h>

/* Third-party libraries */
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

/* User created libraries */
#include <Timer.h>
#include <MessageQueue.h>

// Prototypes
void setup_wifi();
void reconnect();
void get_temperature();
void set_temperature_threshold(char* threshold_value);
void set_alarm_threshold();
void check_alarm();
void alarm_callback(const uint8_t *device_address);
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void build_topic(char* topic);
void build_message(int value);
void build_message(float value);
void build_message(char* value);
void publish_message();
void publish_message(char* topic, char* message);
void broadcast_self();

// Hardware pins
uint8_t one_wire_bus = 2;

// Timer setup
Timer _get_temperature(SECONDS(5), get_temperature);
Timer _check_alarm(SECONDS(2), check_alarm);
Timer _mqtt_reconnect(SECONDS(5), reconnect);

// Temperature variables
float _current_temp = 0.0;
double _temp_threshold = 0.0;
uint8_t _alarm_state = LOW;
char _alarm_on[] = "ON";
char _alarm_off[] = "OFF";

// Thermostat
OneWire one_wire(one_wire_bus);
DallasTemperature device(&one_wire);
DeviceAddress _device_address;
int _thermostat_id = 2472823;   // TODO: This should be a value stored in the EEPROM


// TODO: instead of hard coding these values they should be pulled from the EEPROM
// TODO: setup an initial connection routine that setups a SoftAP and allows user to set
//      network configuration (SSID, Password, etc...)
// Update these with values suitable for your network.
const char* ssid = "*****";
const char* password = "*******";

// MQTT related stuff
WiFiClient wifi_client;
PubSubClient client(wifi_client);
const char* mqtt_server = "thermostat-hub";
const int mqtt_port = 1883;

// MQTT topic variables
char _alarm_topic[] = "/alarm";
char _current_temp_topic[] = "/currentTemp";
char _threshold_temp_topic[] = "/setTemp";
char _thermostat_topic_prefix[] = "thermostat/";
char _thermostat_presence_topic[] = "presence/thermostat";
char _thermostat_hub_presence_topic[] = "presence/thermostat-hub";

// Variables to hold the current MQTT topic and message
char _topic[50];
char _message[25];
MessageQueue _unsent_messages;

void setup() {
    Serial.begin(9600);

    setup_wifi();
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(mqtt_callback);
    reconnect();

    device.begin();
    if (device.getAddress(_device_address, 0)) {
        _get_temperature.start();
        device.setAlarmHandler(&alarm_callback);
        _check_alarm.start();
    } else {
        // TODO: this is a fatal error. Maybe hang at this line or publish the error to the MQTT broker
        Serial.println("Couldn't connect to thermostat");
    }
}

void loop() {
    if (!client.connected()) {
        _mqtt_reconnect.start();
    }
    client.loop();

    _mqtt_reconnect.run();
    _get_temperature.run();
    _check_alarm.run();
}

void setup_wifi() {
    delay(10);

    Serial.printf("Connecting to wiFi network %s\n", ssid);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print('.');
    }

    Serial.print('\n');
    Serial.println("WiFi Connected");
}

void reconnect() {
    if (client.connect("thermostat")) {
        Serial.println("MQTT Connected");
        _mqtt_reconnect.stop();

        broadcast_self();
        client.subscribe(_thermostat_hub_presence_topic);

        build_topic(_threshold_temp_topic);
        client.subscribe(_topic);

        while (!_unsent_messages.isEmpty()) {
            Message m = _unsent_messages.pop();

            Serial.printf("Republishing topic %s message %s\n", m.topic, m.message);
            publish_message(m.topic, m.message);
        }
    }
}

void get_temperature() {
    Serial.print("Getting temperature: ");
    float current_temp;

    device.requestTemperatures();
    current_temp = device.getTempF(_device_address);

    Serial.println(current_temp);

    if (current_temp != _current_temp) {
        _current_temp = current_temp;

        build_topic(_current_temp_topic);
        build_message(_current_temp);
        publish_message();
    }
}

void set_temperature_threshold(char* threshold_value) {
    double temp_threshold = atof(threshold_value);

    if (temp_threshold) {
        if (temp_threshold != _temp_threshold) {
            Serial.print("Setting threshold value to ");
            Serial.println(temp_threshold);
            _temp_threshold = temp_threshold;
            set_alarm_threshold();
        }
    }
}

void set_alarm_threshold() {
    float current_alarm_threshold = device.getLowAlarmTemp(_device_address);

    if (current_alarm_threshold != device.toCelsius(_temp_threshold)) {
        device.setLowAlarmTemp(_device_address, device.toCelsius(_temp_threshold));
        check_alarm();
    }
}

void check_alarm() {
    device.processAlarms();

    if (!device.hasAlarm()) {
        alarm_callback(_device_address);
    }
}

void alarm_callback(const uint8_t * device_address) {
    uint8_t alarm_state;

    if (_current_temp < _temp_threshold) {
        alarm_state = HIGH;
    } else {
        alarm_state = LOW;
    }

    if (alarm_state != _alarm_state) {
        _alarm_state = alarm_state;

        build_topic(_alarm_topic);
        build_message((_alarm_state) ? _alarm_on : _alarm_off);
        publish_message();
    }
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
    char packet[length + 1];
    memcpy(packet, payload, length);
    packet[length] = '\0';

    // Cast topic to string in order to take advantage of string related functions.
    String t(topic);

    if (t.endsWith(_threshold_temp_topic)) {
        set_temperature_threshold(packet);
    } else if (t.endsWith(_thermostat_hub_presence_topic)) {
        // Rebroadcast so the thermostat-hub can subscribe to this device
        broadcast_self();
    }
}

void build_topic(char* topic) {
    snprintf (_topic, sizeof(_topic), "%s%d%s", _thermostat_topic_prefix, _thermostat_id, topic);
}

void build_message(int value) {
    snprintf(_message, sizeof(_message), "%d", value);
}

void build_message(char* value) {
    snprintf(_message, sizeof(_message), "%s", value);
}

void build_message(float value) {
    // ESP8266 doesn't have a way to format floating point numbers directly
    // a work around is to conver the float to a String.
    String strVal(value);
    snprintf(_message, sizeof(_message), "%s", strVal.c_str());
}

void publish_message() {
    publish_message(_topic, _message);
}

void publish_message(char* topic, char* message) {
    if (client.connected()) {
        client.publish(topic, message);
        delay(MICRO_SECONDS(100));
    } else {
        Message m;
        strcpy_P(m.topic, _topic);
        strcpy_P(m.message, _message);

        Serial.printf("Saving topic %s with message %s to to be resent.\n", m.topic, m.message);

        int rc = _unsent_messages.push(m);
        while ((rc != QUEUE_STATUS_OK) && (rc == QUEUE_STATUS_FULL)) {
            // We really only want to keep the most recent stuff around
            // at this point the data at the front of the queue is stale so just toss it out.

            _unsent_messages.pop();
            rc = _unsent_messages.push(m);
        }

        // Setup the MQTT reconnect sequance
        if (!_mqtt_reconnect.running()) {
            _mqtt_reconnect.start();
        }
    }
}

void broadcast_self() {
    build_message(_thermostat_id);
    client.publish(_thermostat_presence_topic, _message);

    build_topic(_alarm_topic);
    build_message(_alarm_state);
    publish_message();

    build_topic(_current_temp_topic);
    build_message(_current_temp);
    publish_message();
}