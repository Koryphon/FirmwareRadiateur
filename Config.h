/*==============================================================================
 * FirmwareRadiateur
 *
 * Application configuration constants.
 */

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <stdint.h>

/*------------------------------------------------------------------------------
 * The pins used on the ESP32.
 *
 * 1 - control of the heater.
 *
 * The control of the heater is done via its pilot wire.
 * The pilot wire allows several functions:
 *
 * pilot wire unconnected   -> Comfort
 * 2 alternations 220V      -> Eco
 * 1/2 pulse positive       -> Off
 * 1/2 pulse negative       -> Frost protection
 *
 * The pilot wire is supplied via 2 optotriacs, one passes the positive 1/2
 * pulses and the other one the negative 1/2 pulses. The infrared diodes of the
 * optotriacs are controlled via the following pins.
 */

static const uint8_t pinAntifreeze = 23;
static const uint8_t pinStop = 19;

/*
 * The combinations are therefore :
 *
 * +---------------+-------------+-------------+
 * | pinAntifreeze |   pinStop   |   Function  |
 * +---------------+-------------+-------------+
 * |      LOW      |    LOW      |   Comfort   |
 * +---------------+-------------+-------------+
 * |      LOW      |    HIGH     |    Stop     |
 * +---------------+-------------+-------------+
 * |      HIGH     |    LOW      | Antifreeze  |
 * +---------------+-------------+-------------+
 * |      HIGH     |    HIGH     |     Eco     |
 * +---------------+-------------+-------------+
 *
 * 2 - dip-switch pins for setting a heater number
 *
 * On 6 bits, the heaters are numbered from 0 to 63.
 */
static const uint8_t pinAddr0 = 26;
static const uint8_t pinAddr1 = 18;
static const uint8_t pinAddr2 = 16;
static const uint8_t pinAddr3 = 17;
static const uint8_t pinAddr4 = 22;
static const uint8_t pinAddr5 = 21;

extern const uint8_t pinAddr[];

/*
 * 3 - DHT22 temperature and humidity sensor pin
 */
static const uint8_t pinDHT22 = 4;

/*------------------------------------------------------------------------------
 * The number of consecutive retries of connection to the WiFi and MQTT broker
 * before reboot.
 */
static const uint32_t wifiRetryCount = 10;
static const uint32_t brokerMQTTRetryCount = 60;

/*------------------------------------------------------------------------------
 * Keep alive for the connection to the MQTT broker
 */
static const int kMQTTBrokerKeepAlive = 60; /* Not used with PubSubClient */

/*------------------------------------------------------------------------------
 * Timeout from the MQTT broker (in ms)
 */
static const uint32_t kMQTTBrokerTimeout = 1000ul * 60ul;

/*------------------------------------------------------------------------------
 * Default temperature when the node is operational but not receiving a
 * setpoint.
 */
static const float kDefaultTemperature = 18.0;

/*------------------------------------------------------------------------------
 * Preferences namespace name and keys
 */
static const char *const kPrefNamespaceName = "FirmRad";
static const char *const kTemperatureOffsetKey = "TOff";

#endif
