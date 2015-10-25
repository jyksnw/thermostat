//
// Created by Jason Snow on 10/23/15.
//
// TODO: Wifi setup using a SoftAP and EEPROM
// TODO: before each publish and subscribe check if client is connected
// TODO: add in a error and/or state publication (so the server can known about things like can't get temp, etc...)
// TODO: add in a temp broadcast type as of right now everything is broadcast as F (optional just let the server handle this)
// TODO: there exists a situation where the thermostat can be turned on before the hub. Maybe have a ping and/or rebroadcast to fix this
//

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <Timer.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

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

// Update these with values suitable for your network.

const char* ssid = "****";
const char* password = "***********";
const char* mqtt_server = "thermostat-hub";

WiFiClient wifi_client;
PubSubClient client(wifi_client);
int _thermostat_id = 2472823;
char _alarm_topic[] = "/alarm";
char _current_temp_topic[] = "/currentTemp";
char _threshold_temp_topic[] = "/setTemp";
char _topic[50];
char _message[25];

void setup() {
    Serial.begin(9600);

    setup_wifi();
    client.setServer(mqtt_server, 1883);
    client.setCallback(mqtt_callback);
    reconnect();

    device.begin();
    if (device.getAddress(_device_address, 0)) {
        _get_temperature.start();
        device.setAlarmHandler(&alarm_callback);
        _check_alarm.start();
    } else {
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

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }
}

void reconnect() {
    Serial.println("Attempting to connect to MQTT broker");
    if (client.connect("thermostat")) {
        Serial.println("Connected");
        _mqtt_reconnect.stop();

        build_message(_thermostat_id);
        client.publish("presence/thermostat", _message);

        build_topic(_threshold_temp_topic);
        client.subscribe(_topic);
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

        client.publish(_topic, _message);
    }
}

void set_temperature_threshold(char* threshold_value) {
    Serial.print("Received new threshold ");
    Serial.println(threshold_value);

    double temp_threshold = atof(threshold_value);

    Serial.print("Converted thershold_value to ");
    Serial.println(temp_threshold);

    if (temp_threshold) {
        if (temp_threshold != _temp_threshold) {
            _temp_threshold = temp_threshold;
            set_alarm_threshold();
        }
    }
}

void set_alarm_threshold() {
    float current_alarm_threshold = device.getLowAlarmTemp(_device_address);

    if (current_alarm_threshold != device.toCelsius(_temp_threshold)) {
        Serial.println("Updating alarm threshold");
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

        client.publish(_topic, _message);
        // TODO: this could be a good place to turn on a led or sound a buzzer
    }
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
    char packet[length + 1];
    memcpy(packet, payload, length);
    packet[length] = '\0';

    set_temperature_threshold(packet);
}

void build_topic(char* topic) {
    snprintf (_topic, sizeof(_topic), "%s%d%s", "thermostat/", _thermostat_id, topic);
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