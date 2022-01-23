#ifndef __CONNECTION_H__
#define __CONNECTION_H__

#include <Arduino.h>
#include <ArduinoOTA.h>
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
  typedef enum {
    INIT,
    WIFI_STBY,
    OFFLINE,
    WIFI_OK,
    MDNS_OK,
    OTA_OK,
    MQTT_OK
  } State;

private:
  typedef void (*SubscriptionFunction)();
  typedef void (*MessageHandlingFunction)(const String &, const String &);

  static WiFiClient sNet;
  static PubSubClient sClient;
  static IPAddress sBrokerIP;
  static State sState;
  static String sName;
  static SubscriptionFunction sSubs;
  static MessageHandlingFunction sHandler;

  Connection() {} /* prevent instanciation */

  static void doSubscriptions();
  static void callback(char *inTopic, byte *inPayload, unsigned int inLength);
  static void startOTA();
  static void progressOTA(unsigned int progress, unsigned int total);
  static void endOTA();
  static void errorOTA(ota_error_t error);
  static void initOTA();

public:
  static void begin(String &inName, SubscriptionFunction inSubFunction = NULL, MessageHandlingFunction inHandler = NULL);
  static bool isOnline();
  static void update();
  static void loop();
  static void publish(const String &inTopic, const String &inPayload);
  static void publish(const String &inTopic, const char *inPayload);
  static void subscribe(const String &inTopic);
};

#endif
