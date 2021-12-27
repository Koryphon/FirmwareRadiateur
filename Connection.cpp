#include "Connection.h"
#include "Config.h"
#include "Debug.h"
#include "Heater.h"
#include "Network.h"
#include "Retryer.h"

/*------------------------------------------------------------------------------
 * Objects counting the number of reconnection attempts and restarting the ESP
 * when the number of attempts exceeds a threshold
 */
Retryer wifiRetryer(wifiRetryCount);
Retryer brokerMQTTRetryer(brokerMQTTRetryCount);
Retryer wifiReconnectCount(wifiRetryCount, 10000);
Retryer mqttReconnectCount(brokerMQTTRetryCount, 10000);

/*------------------------------------------------------------------------------
 * Identifier of the setpoint message, the mode message and the IP request
 * message
 */
extern String messageSetpoint;
extern String messageMode;
extern String messageRequest;
extern String heaterId;

/*------------------------------------------------------------------------------
 * handler for MQTT incoming messages
 */
extern void messageReceived(const String &topic, const String &payload);

/*------------------------------------------------------------------------------
 * Client for the WiFi connection
 */
WiFiClient Connection::sNet;

/*------------------------------------------------------------------------------
 * Client for the MQTT connection to the broker
 */
PubSubClient Connection::sClient(Connection::sNet);

/*------------------------------------------------------------------------------
 * IP address of the MQTT broker
 */
IPAddress Connection::sBrokerIP;

/*------------------------------------------------------------------------------
 * State of the connection
 */
Connection::State Connection::sState = INIT;

/*------------------------------------------------------------------------------
 * Sets up subscriptions
 */
void Connection::doSubscriptions() {
  sClient.subscribe(messageSetpoint.c_str());
  sClient.subscribe(messageMode.c_str());
  sClient.subscribe(messageRequest.c_str());
}

/*------------------------------------------------------------------------------
 * Callback for incoming messages
 */
void Connection::callback(char *inTopic, byte *inPayload,
                          unsigned int inLength) {
  if (inLength < 30) {
    char buf[31];
    uint32_t i;
    for (i = 0; i < inLength; i++) {
      buf[i] = inPayload[i];
    }
    buf[i] = '\0';
    messageReceived(String(inTopic), String(buf));
  }
}

/*------------------------------------------------------------------------------
 * Tests if the connection is OK, used by the heating policy
 * control automaton of the heater.
 */
bool Connection::isOnline() { return (sState == MQTT_OK); }

/*------------------------------------------------------------------------------
 * Updates heater status based on network connection status.
 * Attempts to reconnect as needed.
 */
void Connection::update() {
  switch (sState) {

  case INIT:
    /* Initial state after (re)boot. initialize the WiFi connection */
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);
    sState = WIFI_STBY;
    break;

  case WIFI_STBY:
    /* Do the initial connection to WiFi */
    LOGT;
    DEBUG_P("Connexion a ");
    DEBUG_P(ssid);
    DEBUG_P(" - ");
    if (WiFi.status() != WL_CONNECTED) {
      DEBUG_PLN("echec");
      wifiRetryer.retry();
    } else {
      DEBUG_PLN("connecte");
      wifiRetryer.reset();
      /* Start the multicast DNS */
      MDNS.begin(heaterId.c_str());
      sState = WIFI_OK;
    }
    break;

  case OFFLINE:
    /* Try to reconnect WiFi */
    WiFi.disconnect();
    DEBUG_P("Reconnexion a ");
    DEBUG_P(ssid);
    DEBUG_P(" - ");
    if (WiFi.reconnect()) {
      DEBUG_PLN("connecte");
      wifiRetryer.reset();
      wifiReconnectCount.retry();
      sState = WIFI_OK;
    } else {
      DEBUG_PLN("echec");
      wifiRetryer.retry();
    }
    break;

  case WIFI_OK:
    LOGT;
    if (WiFi.status() == WL_CONNECTED) {
      /*
       * Get the IP of the MQTT broker. If 0.0.0.0 is got, the broker was not
       * found
       */
      DEBUG_P("Broker MQTT ");
      DEBUG_P(brokerName);
      DEBUG_P(".local - ");
      sBrokerIP = MDNS.queryHost(brokerName);
      if (sBrokerIP != IPAddress(0, 0, 0, 0)) {
        /* Broker responded */
        DEBUG_PLN("trouve");
        sClient.setServer(sBrokerIP, 1883);
        sClient.setCallback(callback);
        sState = MDNS_OK;
      } else {
        DEBUG_PLN("absent");
      }
    } else {
      DEBUG_PLN("WiFi deconnecte");
      sState = OFFLINE;
    }
    break;

  case MDNS_OK:
    LOGT;
    if (WiFi.status() == WL_CONNECTED) {
      DEBUG_P("Connexion au broker MQTT ");
      DEBUG_P(brokerName);
      DEBUG_P(".local (");
      DEBUG_DO(sBrokerIP.printTo(Serial));
      DEBUG_P(") - ");
      if (!sClient.connect(heaterId.c_str())) { //, "public", "public")) {
        DEBUG_P("echec : ");
        DEBUG_PLN(sClient.state());
        brokerMQTTRetryer.retry();
      } else {
        DEBUG_PLN("connecte");
        brokerMQTTRetryer.reset();
        mqttReconnectCount.retry();
        doSubscriptions();
        sState = MQTT_OK;
      }
    } else {
      DEBUG_PLN("WiFi deconnecte");
      sState = OFFLINE;
    }
    break;

  case MQTT_OK:
    if (WiFi.status() != WL_CONNECTED) {
      LOGT;
      DEBUG_PLN("WiFi deconnecte");
      sState = OFFLINE;
    } else if (!sClient.connected()) {
      LOGT;
      DEBUG_P("Broker MQTT deconnecte : ");
      DEBUG_PLN(sClient.state());
      sState = MDNS_OK;
    }
    break;
  }
}

/*------------------------------------------------------------------------------
 */
void Connection::loop() { sClient.loop(); }

/*------------------------------------------------------------------------------
 */
void Connection::publish(const String &inTopic, const String &inPayload) {
  if (sClient.connected()) {
    sClient.publish(inTopic.c_str(), inPayload.c_str());
  }
}

/*------------------------------------------------------------------------------
 */
void Connection::publish(const String &inTopic, const char *inPayload) {
  if (sClient.connected()) {
    sClient.publish(inTopic.c_str(), inPayload);
  }
}