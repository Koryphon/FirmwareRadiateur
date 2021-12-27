#ifndef __CONNECTION_H__
#define __CONNECTION_H__

#include <Arduino.h>
#include <ESPmDNS.h>
#include <PubSubClient.h>
#include <WiFi.h>

/*------------------------------------------------------------------------------
 * Identifier of the setpoint message, the mode message and the IP request
 * message
 */
extern String messageSetpoint;
extern String messageMode;
extern String messageRequest;

class Connection {
public:
  typedef enum { INIT, WIFI_STBY, OFFLINE, WIFI_OK, MDNS_OK, MQTT_OK } State;

private:
  static WiFiClient sNet;
  static PubSubClient sClient;
  static IPAddress sBrokerIP;
  static State sState;

  Connection() {} /* prevent instanciation */

  static void doSubscriptions();
  static void callback(char *inTopic, byte *inPayload, unsigned int inLength);

public:
  static bool isOnline();
  static void update();
  static void loop();
  static void publish(const String &inTopic, const String &inPayload);
  static void publish(const String &inTopic, const char *inPayload);
};

#endif
