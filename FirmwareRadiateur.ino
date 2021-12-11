/*==============================================================================
  Firmware pour radiateur connecté

  V 2.2

  Jean-Luc Béchennec - décembre 2021

  Pour l'instant les modes hors-gel et éco ne sont pas exploités.
  ------------------------------------------------------------------------------
  Changelog :
  2.3     ajout du redémarrage du système lorsqu'un trop grand nombre de 
          tentatives de reconnexion ont été exécutées.
  2.2     ajout de OTA (Over The Air) permettant la mise à jour du firmware
          en WiFi.
  2.1     ajout du support de mDNS (aka Bonjour sur Mac). Le Broker MQTT est
          accessible via son nom Bonjour (<machine>.local) au lieu de son IP.
  2.0     version initiale. MQTT, support des modes stop et confort.
*/
#include <WiFi.h>
#include <ESPmDNS.h>
#include <MQTT.h>
#include <DHT.h>
#include <ArduinoOTA.h>

#include "PeriodicLED.h"
#include "PeriodicAction.h"
#include "Retryer.h"
#include "Heater.h"

/*------------------------------------------------------------------------------
*/
const String version = "2.2";

/*------------------------------------------------------------------------------
  Définition des broches

  1 - contrôle du radiateur

  Le contrôle du radiateur est réalisé via son fil pilote. Le fil pilote permet
  plusieurs fonctionnements :

  fil pilote en l'air       -> Confort
  2 alternances 220V        -> Eco
  1/2 alternances positive  -> Arrêt
  1/2 alternances négative  -> Hors Gel

  Le fil pilote est alimenté via 2 optotriacs, l'un passe les 1/2 alternances
  positives et l'autre les 1/2 alternances négatives. Les diodes infrarouge des
  optotriacs est contrôlé via les broches suivantes.
*/

const uint8_t pinAntifreeze = 23;
const uint8_t pinStop       = 19;

/*
  Les combinaisons sont donc :

  +------------+-------------+-------------+
  | pinHorsGel |   pinStop   |   Fonction  |
  +------------+-------------+-------------+
  |    LOW     |    LOW      |   Confort   |
  +------------+-------------+-------------+
  |    LOW     |    HIGH     |    Arrêt    |
  +------------+-------------+-------------+
  |    HIGH    |    LOW      |   Hors Gel  |
  +------------+-------------+-------------+
  |    HIGH    |    HIGH     |     Eco     |
  +------------+-------------+-------------+

  2 - broches du dip-switch permettant de régler un numéro de radiateur

  Sur 6 bits, les radiateurs sont numérotés de 0 à 63.
*/
const uint8_t pinAddr0 = 26;
const uint8_t pinAddr1 = 18;
const uint8_t pinAddr2 = 16;
const uint8_t pinAddr3 = 17;
const uint8_t pinAddr4 = 22;
const uint8_t pinAddr5 = 21;

/*
  3 - broche du capteur de température et humidité DHT22
*/
const uint8_t pinDHT22 = 4;

/*------------------------------------------------------------------------------
  Nombre d'essais de reconnexion pour le WiFi et le broker MQTT 
 */
const uint32_t wifiRetryCount = 10;
const uint32_t brokerMQTTRetryCount = 60;

/*------------------------------------------------------------------------------
  Paramètres de connexion au réseau WiFi de la maison
*/
#include "Network.h"

/*------------------------------------------------------------------------------
  Adresse IP du broker MQTT
*/
IPAddress brokerIP;

/*------------------------------------------------------------------------------
  Objet pour la connexion WiFi
*/
WiFiClient net;

/*------------------------------------------------------------------------------
  Objet pour la connexion au broker MQTT
*/
MQTTClient client;

/*------------------------------------------------------------------------------
  Objet pour la LED d'activité. Période de 1000 ms, pulse de 100 ms
*/
PeriodicLED activityLED(LED_BUILTIN, 1000, 100);

/*------------------------------------------------------------------------------
  Objet pour le contrôle du radiateur. Offset de 1000, période de 6000.
  Récupération de la température, comparaison avec la consigne,
  choix de mettre le radiateur en Arrêt ou Confort.
*/
PeriodicAction heaterControlAction(1000, 6000);

/*------------------------------------------------------------------------------
  Objet pour la publication des données. Offset de 4000, période de 6000.
  Publication de la température, publication de l'humidité, publication de
  la température ressentie, publication du ratio Confort / temps total
*/
PeriodicAction publishDataAction(4000, 6000);

/*------------------------------------------------------------------------------
  Objet pour la publication de l'IP. Offset de 5000, période de 6000.
*/
PeriodicAction publishIPAction(5000, 6000);

/*------------------------------------------------------------------------------
  Objet pour le contrôle de l'état de la connexion Wifi. Offset de 5500,
  période de 12000.
*/
PeriodicAction controlWiFiAction(5500, 12000);

/*------------------------------------------------------------------------------
  Objet pour le contrôle de l'état de la connexion MQTT. Offset de 1000,
  période de 1000
*/
PeriodicAction controlMQTTAction(1000, 1000);

/*------------------------------------------------------------------------------
  Objet pour la lecture du DHT22
*/
DHT dht(pinDHT22, DHT22);

/*------------------------------------------------------------------------------
  Objet pour le radiateur
*/
Heater heater(pinStop, pinAntifreeze);

/*------------------------------------------------------------------------------
  Température, humidité et température apparente (heatIndex)
  Voir https://fr.wikipedia.org/wiki/Indice_de_chaleur
*/
float temperature = 0.0;
float humidity = 0.0;
float heatIndex = 0.0;

/*------------------------------------------------------------------------------
  Température de consigne
*/
float setpointTemperature = 0.0;

/*------------------------------------------------------------------------------
  true si le radiateur est en mode automatique, false si il est en mode manuel
  Le démarrage de l'application s'effectue en mode manuel : le radiateur est mis
  en mode confort. Quand un message de température de consigne est reçu, le
  radiateur passe en mode automatique. Quand un message de mise en mode manuel
  est reçu, le radiateur passe en mode manuel
*/
bool automaticMode = false;

/*------------------------------------------------------------------------------
  true si la publication de l'IP a été demandée
*/
bool IPRequested = false;

/*------------------------------------------------------------------------------
  Identifiant du radiateur et des publications
*/
String heaterId;
String heaterTemperature;
String heaterHumidity;
String heaterHeatIndex;
String heaterComfortRatio;
String heaterState;
String heaterIP;

/*------------------------------------------------------------------------------
  Identifiant du message de consigne, du message de mode et du message de
  requête de l'IP
*/
String messageSetpoint;
String messageMode;
String messageRequest;

/*------------------------------------------------------------------------------
  Objets comptant le nombre d'essais de reconnexions et redémarrant l'ESP
  quand le nombre d'essais dépasse un seuil
*/
Retryer wifiRetryer(wifiRetryCount);
Retryer brokerMQTTRetryer(brokerMQTTRetryCount);


/*------------------------------------------------------------------------------
  Affiche un nombre entier positif sur le nombre minimum de caractères spécifiés
*/
void fmtPrint(uint32_t inVal, uint8_t inFieldSize)
{
  /* calcule le nombre de caractères nécessaire */
  uint32_t tmp = inVal / 10;
  uint8_t s = 1;
  while (tmp > 0) {
    s++;
    tmp /= 10;
  }
  for (uint8_t i = 0; i < (inFieldSize - s); i++) Serial.print('0');
  Serial.print(inVal);
}

/*------------------------------------------------------------------------------
  Affiche la date sous une forme HH:MM:SS:MS
*/
void logTime()
{
  uint32_t date = millis();
  uint32_t ms = date % 1000;
  uint32_t s = (date / 1000) % 60;
  uint32_t m = (date / 60000) % 60;
  uint32_t h = date / 3600000;
  fmtPrint(h, 2);
  Serial.print(':');
  fmtPrint(m, 2);
  Serial.print(':');
  fmtPrint(s, 2);
  Serial.print('.');
  fmtPrint(ms, 3);
  Serial.print(" : ");
}

/*------------------------------------------------------------------------------
  Lecture du numéro de radiateur
*/
uint8_t readHeaterNum()
{
  uint8_t num = 0;
  num |= digitalRead(pinAddr0);
  num |= digitalRead(pinAddr1) << 1;
  num |= digitalRead(pinAddr2) << 2;
  num |= digitalRead(pinAddr3) << 3;
  num |= digitalRead(pinAddr4) << 4;
  logTime();
  Serial.print("Numero radiateur : ");
  Serial.println(num);
  return num;
}

/*------------------------------------------------------------------------------
  Publie les valeurs courantes de la température, de l'humidité et de la
  température apparente
*/
void publishData()
{
  logTime();
  if (client.connected()) {
    Serial.println("Publication des donnees !");
    client.publish(heaterTemperature.c_str(), String(temperature).c_str());
    client.publish(heaterHumidity.c_str(), String(humidity).c_str());
    client.publish(heaterHeatIndex.c_str(), String(heatIndex).c_str());
    client.publish(heaterComfortRatio.c_str(), String(heater.comfortRatio()).c_str());
    client.publish(heaterState.c_str(), heater.stringState());
  }
  else {
    Serial.println("Client deconnecte, pas de publication.");
  }
}

/*------------------------------------------------------------------------------
  Publie l'adresse IP
*/
void publishIP()
{
  if (IPRequested) {
    logTime();
    if (client.connected()) {
      Serial.println("Publication de l'IP !");
      client.publish(heaterIP.c_str(), WiFi.localIP().toString().c_str());
    }
    else {
      Serial.println("Client deconnecte, pas de publication d'IP.");
    }
    IPRequested = false;
  }
}

/*------------------------------------------------------------------------------
  Commande le radiateur en fonction du mode et de la temperature de consigne
*/
void controlHeater()
{
  /* lit la temperature et l'humidité, calcule la température apparente */
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (isnan(t) || isnan(h)) {
    logTime();
    Serial.println("DHT22 off");
    /* En cas de dysfonctionnement du capteur, on se met en confort */
    heater.setComfort();
  } else {
    temperature = t;
    humidity = h;
    heatIndex = dht.computeHeatIndex(temperature, humidity, false);
    /* Applique la consigne */
    if (setpointTemperature > temperature || !automaticMode) {
      heater.setComfort();
    }
    else {
      heater.setStop();
    }
  }
}

/*------------------------------------------------------------------------------
  Vérifie que la connection WiFi est ok et reconnecte au besoin
*/
void checkWiFi()
{
  logTime();
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Tente une reconnexion à ");
    Serial.println(ssid);
    WiFi.disconnect();
    if (WiFi.reconnect()) {
      wifiRetryer.reset();
    }
    else {
      wifiRetryer.retry();
    }
  }
  else {
    Serial.println("WiFi ok !");
  }
  /*
     TODO : compter le nombre de tentatives de reconnexion consécutives qui
     échouent et redémarrer l'ESP quand un seuil est dépassé.
  */
}

/*------------------------------------------------------------------------------
  Vérifie que la connection MQTT est ok et reconnecte au besoin
*/
void checkMQTT()
{
  if (!client.connected()) {
    reconnectBroker();
  }
  else {
    logTime();
    Serial.println("MQTT ok !");
  }
  /*
     TODO : compter le nombre de tentatives de reconnexion consécutives qui
     échouent et redémarrer l'ESP quand un seuil est dépassé.
  */
}

/*------------------------------------------------------------------------------
  Connexion au réseau WiFi
*/
void connectWiFi() {
  Serial.print("Connexion a ");
  Serial.print(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println(" connecte");
}

/*------------------------------------------------------------------------------
  Met en place les abonnements
*/
void doSubscriptions()
{
  client.subscribe(messageSetpoint.c_str());
  client.subscribe(messageMode.c_str());
  client.subscribe(messageRequest.c_str());
}

/*------------------------------------------------------------------------------
  Connexion au broker MQTT
*/
void connectBroker()
{
  logTime();
  Serial.print("Connexion au serveur MQTT");
  while (!client.connect(heaterId.c_str(), "public", "public")) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println(" connecte");
  doSubscriptions();
}

/*------------------------------------------------------------------------------
  Reconnexion au broker MQTT en cas de coupure
*/
void reconnectBroker()
{
  logTime();
  Serial.println("Reconnexion au serveur MQTT");
  if (client.connect(heaterId.c_str(), "public", "public")) {
    Serial.println(" connecte");
    doSubscriptions();
    brokerMQTTRetryer.reset();
  }
  else {
    brokerMQTTRetryer.retry();
  }
}

/*------------------------------------------------------------------------------
  Handler de réception des messages en provenance du broker
*/
void messageReceived(String &topic, String &payload)
{
  logTime();
  Serial.println("incoming: " + topic + " - " + payload);

  if (topic == messageSetpoint) {
    float t = payload.toFloat();
    if (t != 0.0) {
      logTime();
      Serial.print("Temperature de consigne = ");
      Serial.println(t);
      setpointTemperature = t;
      automaticMode = true;
    }
  }
  else if (topic == messageMode) {
    if (payload == "manual") {
      logTime();
      Serial.println("Mode manuel");
      automaticMode = false;
    }
    else if (payload == "auto") {
      logTime();
      Serial.println("Mode auto");
      automaticMode = true;
    }
  }
  else if (topic == messageRequest) {
    if (payload == "IP") {
      Serial.println("Requete de l'IP");
      IPRequested = true;
    }
  }
}

/*------------------------------------------------------------------------------
  OTA
*/
void startOTA()
{
  const int command = ArduinoOTA.getCommand();
  if (command == U_FLASH)
  {
    Serial.println("Mise a jour du firmware");
  }
  else {
    Serial.print("Commande non supportee : ");
    Serial.println(command);
  }
}

void progressOTA(unsigned int progress, unsigned int total)
{
  static int lastProgress = -1;
  if (progress != lastProgress) {
    Serial.print("En cours : ");
    Serial.print(100 * progress / total);
    Serial.print("%\r");
  }
}

void endOTA()
{
  Serial.println();
  Serial.println("Fini");
}

void errorOTA(ota_error_t error)
{
  Serial.print("Erreur[");
  Serial.print(error);
  Serial.print("] : ");
  switch (error) {
    case OTA_AUTH_ERROR:    Serial.println("L'authentification a échoué"); break;
    case OTA_BEGIN_ERROR:   Serial.println("Échec au début");              break;
    case OTA_CONNECT_ERROR: Serial.println("Échec à la connexion");        break;
    case OTA_RECEIVE_ERROR: Serial.println("Échec à la réception");        break;
    case OTA_END_ERROR:     Serial.println("Échec à la fermeture");        break;
  }
}

void initOTA()
{
  ArduinoOTA.setHostname(heaterId.c_str());
  ArduinoOTA.setPasswordHash(passHash);
  ArduinoOTA.onStart(startOTA);
  ArduinoOTA.onProgress(progressOTA);
  ArduinoOTA.onEnd(endOTA);
  ArduinoOTA.onError(errorOTA);
  ArduinoOTA.begin();
}

/*------------------------------------------------------------------------------
  setup
*/
void setup() {

  /* la LED de la carte est utilisée pour signaler le fonctionnement */
  pinMode(LED_BUILTIN, OUTPUT);
  /* Pour le debug */
  Serial.begin(115200);
  /* Version */
  Serial.println("--------------------------------");
  Serial.print("Firmware version ");
  Serial.println(version);
  Serial.println("--------------------------------");

  /* calcule l'identifiant du radiateur et
    les identifiants des données publiées */
  heaterId = String("heater") + readHeaterNum();
  heaterTemperature = heaterId + "/temperature";
  heaterHumidity = heaterId + "/humidity";
  heaterHeatIndex = heaterId + "/heatIndex";
  heaterComfortRatio = heaterId + "/comfortRatio";
  heaterState = heaterId + "/state";
  heaterIP = heaterId + "/IP";

  messageSetpoint = heaterId + "/setpoint";
  messageMode = heaterId + "/mode";
  messageRequest = heaterId + "/request";

  /* Init du radiateur */
  heater.begin();
  /* Démarre la LED d'activité */
  activityLED.begin(LOW);
  /* Démarrage de l'action de contrôle du radiateur */
  heaterControlAction.begin(controlHeater);
  /* Démarre l'action de publication des données */
  publishDataAction.begin(publishData);
  /* Démarre l'action de publication des données */
  publishIPAction.begin(publishIP);
  /* Démarre l'action de contrôle de la connexion Wifi */
  controlWiFiAction.begin(checkWiFi);
  /* Démarre l'action de contrôle de la connexion au broker MQTT */
  controlMQTTAction.begin(checkMQTT);

  /* Démarre le DHT22 */
  dht.begin();

  /* Démarre le WiFi */
  WiFi.begin(ssid, pass);
  connectWiFi();

  /* Récupère l'adresse IP du Broker */
  MDNS.begin(heaterId.c_str());
  brokerIP = MDNS.queryHost(brokerName);

  /* Démarre le client MQTT */
  client.begin(brokerIP, net);
  client.onMessage(messageReceived);

  connectBroker();

  /* Initialisation de l'OTA */
  initOTA();

  /* Marque l'instant initial pour les TimeObject */
  TimeObject::setup();
}

/*------------------------------------------------------------------------------
  loop
*/
void loop() {
  /* Client MQTT */
  client.loop();
  /* Actions périodiques */
  TimeObject::loop();
  /* OTA */
  ArduinoOTA.handle();
}
