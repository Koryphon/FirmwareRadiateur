/*==============================================================================
 * Connected heater firmware
 *
 * V 2.7
 *
 * Jean-Luc Béchennec - December 2021
 *
 *------------------------------------------------------------------------------
 * Changelog :
 * - 2.7  added offset storage in Preferences to get the corrected temperature
 *        when offline
 * - 2.6  bug fix. In connection automaton, losing the MQTT connection
 *        would lead to state MDNS_OK instead of OTA_OK.
 * - 2.5  added timeout, new connection maintenance, eco and antifreeze
 *        mode management
 * - 2.4  added stop mode management for the
 *        global offloading policy.
 * - 2.3  added system reboot when too many reconnection attempts
 *        have been made.
 * - 2.2  added OTA (Over The Air) allowing the firmware update in
 *        WiFi.
 * - 2.1  added support for mDNS (aka Bonjour on Mac). The MQTT Broker is
 *        accessible via its Bonjour name (<machine>.local) instead of its
 *        IP.
 * - 2.0  initial version. MQTT, support of stop and comfort modes.
 */
#include <DHT.h>
#include <Preferences.h>

#include "Config.h"
#include "Connection.h"
#include "Debug.h"
#include "Heater.h"
#include "PeriodicAction.h"
#include "PeriodicLED.h"
#include "Timeout.h"

/*------------------------------------------------------------------------------
 */
const String version = "2.7";

/*------------------------------------------------------------------------------
 *  Settings for connecting to the home WiFi network
 */
#include "Network.h"

/*------------------------------------------------------------------------------
 * Object to manage preferences
 */
Preferences prefs;

/*------------------------------------------------------------------------------
 * Object for the activity LED. Period of 1000 ms, pulse of 100 ms
 */
PeriodicLED activityLED(LED_BUILTIN, 1000, 100);

/*------------------------------------------------------------------------------
 * Object for heater control. Offset of 1000, period of 6000.
 * Temperature retrieval, comparison with the setpoint,
 * Choice of heater in Stop or Comfort mode.
 */
PeriodicAction heaterControlAction(1000, 6000);

/*------------------------------------------------------------------------------
 * Object for data publication. Offset of 4000, period of 6000.
 * Publication of the temperature, publication of the humidity, publication of
 * the perceived temperature, publication of the ratio Comfort / total time
 */
PeriodicAction publishDataAction(4000, 6000);

/*------------------------------------------------------------------------------
 * Object for the publication of the IP. Offset of 5000, period of 6000.
 */
PeriodicAction publishIPAction(5000, 6000);

/*------------------------------------------------------------------------------
 * Object for monitoring the status of the Wifi and MQTT connection.
 */
PeriodicAction controlConnectionAction(1000, 1000);

/*------------------------------------------------------------------------------
 * Object for reading the DHT22
 */
DHT dht(pinDHT22, DHT22);

/*------------------------------------------------------------------------------
 * Object for the heater
 */
Heater heater(pinAddr, pinStop, pinAntifreeze);

/*------------------------------------------------------------------------------
 * Object for to handle a time out from the broker
 */
Timeout brokerTimeout(kMQTTBrokerTimeout);

/*------------------------------------------------------------------------------
 * Temperature, humidity and apparent temperature (heatIndex)
 * See https://fr.wikipedia.org/wiki/Indice_de_chaleur
 */
float temperature = 0.0;
float humidity = 0.0;
float heatIndex = 0.0;
float temperatureOffset = 0.0;

/*------------------------------------------------------------------------------
 * Setpoint temperature
 */
float setpointTemperature = 18.0;

/*------------------------------------------------------------------------------
 * Status requested for the heater, received from the broker.
 */
Heater::HeaterState functioningMode = Heater::ECO;

/*------------------------------------------------------------------------------
 * true if the publication of the IP has been requested
 */
bool IPRequested = false;

/*------------------------------------------------------------------------------
 * Identifier of the heater and publications
 */
String heaterId;
String heaterTemperature;
String heaterHumidity;
String heaterHeatIndex;
String heaterDuty;
String heaterState;
String heaterIP;

/*------------------------------------------------------------------------------
 * Identifier of the setpoint message, the mode message and the
 * request message
 */
String messageSetpoint;
String messageMode;
String messageRequest;
String messageOffset;

/*------------------------------------------------------------------------------
 * Publishes current values of temperature, humidity and heat index
 */
void publishData() {
  LOGT;
  if (Connection::isOnline()) {
    DEBUG_PLN("Publication des donnees !");
    Connection::publish(heaterTemperature, String(temperature));
    Connection::publish(heaterHumidity, String(humidity));
    Connection::publish(heaterHeatIndex, String(heatIndex));
    Connection::publish(heaterDuty, String(heater.duty()));
    Connection::publish(heaterState, heater.stringState());
  } else {
    DEBUG_PLN("Client deconnecte, pas de publication.");
  }
}

/*------------------------------------------------------------------------------
 * Publish IP address
 */
void publishIP() {
  if (IPRequested) {
    LOGT;
    if (Connection::isOnline()) {
      DEBUG_PLN("Publication de l'IP !");
      Connection::publish(heaterIP, WiFi.localIP().toString());
    } else {
      DEBUG_PLN("Client deconnecte, pas de publication d'IP.");
    }
    IPRequested = false;
  }
}

/*------------------------------------------------------------------------------
 * Controls the heater according to the mode and temperature set point
 */
void controlHeater() {
  /* reads temperature and humidity, computes heat index */
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (isnan(t) || isnan(h)) {
    LOGT;
    DEBUG_PLN("DHT22 off");
    /* In case of sensor malfunction, we check the connection */
    if (Connection::isOnline() && brokerTimeout.isNotTimedout()) {
      /* If online, we check the functioning mode */
      if (functioningMode != Heater::AUTO) {
        heater.setMode(functioningMode);
      } else {
        /* Auto cannot be applied because the sensor is off, fallback to eco */
        heater.setEco();
      }
    } else {
      /* Nothing working, fallback to echo */
      heater.setEco();
    }
  } else {
    LOGT;
    temperature = t + temperatureOffset;
    humidity = h;
    DEBUG_P("DHT22 ok : t = ");
    DEBUG_P(t);
    DEBUG_P(", tc = ");
    DEBUG_P(temperature);
    DEBUG_P(", h = ");
    DEBUG_PLN(humidity);
    heatIndex = dht.computeHeatIndex(temperature, humidity, false);
    heater.setRoomTemperature(temperature);

    if (Connection::isOnline() && brokerTimeout.isNotTimedout()) {
      /* sensor and connection ok, apply the command */
      heater.setSetpoint(setpointTemperature);
      heater.setMode(functioningMode);
    } else {
      /* sensor ok but connection lost, set temperature to default one */
      heater.setSetpoint(kDefaultTemperature);
      heater.setAuto();
    }
  }
  heater.loop();
}

/*------------------------------------------------------------------------------
 * Handler for receiving messages from the broker
 */
void messageReceived(const String &topic, const String &payload) {
  LOGT;
  DEBUG_P("incoming: ");
  DEBUG_P(topic);
  DEBUG_P(" - ");
  DEBUG_PLN(payload);

  /* reset the timeout */
  brokerTimeout.timestamp();

  if (topic == messageSetpoint) {
    float t = payload.toFloat();
    if (t != 0.0) {
      LOGT;
      DEBUG_P("Temperature de consigne = ");
      DEBUG_PLN(t);
      setpointTemperature = t;
    }
  } else if (topic == messageMode) {
    if (payload == "stop") {
      LOGT;
      DEBUG_PLN("Mode stop");
      functioningMode = Heater::STOP;
    } else if (payload == "auto") {
      LOGT;
      DEBUG_PLN("Mode auto");
      functioningMode = Heater::AUTO;
    } else if (payload == "anti") {
      LOGT;
      DEBUG_PLN("Mode antifreeze");
      functioningMode = Heater::ANTI;
    } else if (payload == "eco") {
      LOGT;
      DEBUG_PLN("Mode eco");
      functioningMode = Heater::ECO;
    } else {
      LOGT;
      DEBUG_PLN("Mode ?");
    }
  } else if (topic == messageRequest) {
    if (payload == "IP") {
      LOGT;
      DEBUG_PLN("Requete de l'IP");
      IPRequested = true;
    }
  } else if (topic == messageOffset) {
    float o = payload.toFloat();
    LOGT;
    DEBUG_P("Offset temperature = ");
    DEBUG_P(o);
    if (o != temperatureOffset) {
      DEBUG_P(", Mise a jour de l'offset");
      temperatureOffset = o;
      prefs.begin(kPrefNamespaceName, false); /* Open in RW mode */
      prefs.putFloat(kTemperatureOffsetKey, temperatureOffset);
      prefs.end();
    }
    DEBUG_PLN();
  }
}

/*------------------------------------------------------------------------------
 * All subscriptions
 */
void performSubscriptions() {
  LOGT;
  DEBUG_PLN("Souscriptions");
  Connection::subscribe(messageSetpoint);
  Connection::subscribe(messageMode);
  Connection::subscribe(messageRequest);
  Connection::subscribe(messageOffset);
}

/*------------------------------------------------------------------------------
 * setup
 */
void setup() {

  /* the LED on the board is used to signal the operation */
  pinMode(LED_BUILTIN, OUTPUT);
  /* For debugging */
  Serial.begin(115200);
  /* Version */
  Serial.println("--------------------------------");
  Serial.print("Firmware version ");
  Serial.println(version);
  Serial.println("--------------------------------");

  /* The heater */
  heater.begin();

  /* calculates the heater identifier and the topics of the published data */
  heaterId = String("heater") + heater.num();
  heaterTemperature = heaterId + "/temperature";
  heaterHumidity = heaterId + "/humidity";
  heaterHeatIndex = heaterId + "/heatIndex";
  heaterDuty = heaterId + "/duty";
  heaterState = heaterId + "/state";
  heaterIP = heaterId + "/IP";

  messageSetpoint = heaterId + "/setpoint";
  messageMode = heaterId + "/mode";
  messageRequest = heaterId + "/request";
  messageOffset = heaterId + "/offset";

  /* Starts the activity LED */
  activityLED.begin(LOW);
  /* Start of the heater control action */
  heaterControlAction.begin(controlHeater);
  /* Starts the data publishing action */
  publishDataAction.begin(publishData);
  /* Starts the IP publishing action */
  publishIPAction.begin(publishIP);
  /* Starts the WiFi and MQTT connection control action */
  controlConnectionAction.begin(Connection::update);

  /* Get the offset from the preferences */
  prefs.begin(kPrefNamespaceName, true); /* Open in RO mode */
  temperatureOffset = prefs.getFloat(kTemperatureOffsetKey, 0.0);
  LOGT;
  DEBUG_P("Pref TOff : ");
  DEBUG_PLN(temperatureOffset);
  prefs.end();

  /* Start the DHT22 */
  dht.begin();

  /* Connection initialization */
  Connection::begin(heaterId, performSubscriptions);

  /* Mark the initial time for the TimeObject.s */
  TimeObject::setup();
}

/*------------------------------------------------------------------------------
  loop
*/
void loop() {
  /* MQTT Client */
  Connection::loop();
  /* Periodic actions */
  TimeObject::loop();
  /* OTA */
  ArduinoOTA.handle();
}
