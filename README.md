# FirmwareRadiateur

## Bibliothèques utilisées

Pour le DHT22, il s'agit de la bibliothèque Adafruit ```DHT Sensor Library```.
Pour MQTT, il s'agit de la bibliothèque MQTT de Joel Gaehwiler qui s'appelle tout simplement MQTT.

Les deux sont à installer via le gestionnaire de bibliothèques de l'IDE Arduino

## Paramètres de connexion au WiFi

Le sketch inclut le fichier ```Network.h``` qui doit contenir les paramètres de connexion au réseau WiFi, le nom du broker MQTT et le hash du mot de passe pour l'OTA. Il faut définir ces paramètres de connexion dans le fichier ```MyNetwork.h``` en se basant sur le canevas donné dans ```NetworkTemplate.h``` et placer un lien symbolique :

```sh
ln -s MyNetwork.h Network.h
```

```Network.h``` et ```MyNetwork.h``` sont exclus du repository dans le ```.gitignore``` afin d'éviter de poussé le ssid et le mot de passe.

### ssid

```ssid``` est le nom du point d'accès WiFi du réseau où le Broker MQTT et les radiateurs sont connectés.

### pass

```pass``` est le mot de passe du point d'accès WiFi.

### brokerName

```brokerName``` est le nom de la machine qui héberge le Broker (port 1883). Par exemple :

```
static const char brokerName[] = "Sirius";
```

indique que le Broker s'exécute sur la machine ```Sirius.local```.

### passHash

```passHash``` est le hash MD5 du mot de passe affecté à l'OTA. Sous Mac OS X, il suffit de taper :

```
md5 -s <mot de passe>
```

pour calculer le hash du mot de passe.

## Mise en réseau

Chaque radiateur s'identifie sur le réseau via mDNS (Multicast DNS, aka Bonjour sur Mac) sous le nom ```heater<num>.local``` où ```<num>``` est le numéro de radiateur, de 0 à 63, réglé sur le dip-switch de la carte.

## Flashage du firmware

Il est possible de flasher le firmware sur l'ensemble des radiateurs via le réseau WiFi en utilisant OTA. Comme il est assez fastidieux de le faire manuellement via l'IDE, un script, ```update.sh``` est fourni. Pour l'utiliser, il faut :

1. Exporter le binaire du firmware depuis l'IDE Arduino en choisissant ```Exporter les binaires compilées``` dans le menu ```Croquis```. Ceci engendre un fichier nommé ```FirmwareRadiateur.ino.mhetesp32minikit.bin``` dans le dossier du croquis. Ce nom ne doit pas être changé car il est inclus en dur dans ```update.sh```. 
2. Ouvrir un terminal dans le dossier du croquis.
3. Taper ```./update.sh <pass> <num>``` où ```<pass>``` est le mot de passe pour l'OTA et ```<num>``` le numéro de radiateur à mettre à jour. Ou bien ```./update.sh <pass> <num1> <num2>``` où ```<num1>``` est le début d'un intervalle de radiateurs à mettre à jour et où ```<num2>``` est la fin de l'intervalle. Un radiateur non trouvé n'interromp pas la procédure.
 
```update.sh``` utilise le script python ```espota.py``` qui est localisé dans ```~/Library/Arduino15/packages/esp32/hardware/esp32/2.0.0/tools/``` (la version peut être différente). Il vérifie sa présence et lui donne les droit en exécution (ce qui n'est pas le cas par défaut). Il vérifie également que le fichier binaire a été exporté (étape 1). Les arguments donnés à ```espota.py```sont :

1. ```--ip=heater<num>.local``` où ```<num>``` prend les valeurs successives dans l'intervalle spécifié.
2. ```--auth=<pass>```
3. ```--file=FirmwareRadiateur.ino.mhetesp32minikit.bin```
4. ```-r``` pour afficher la progression du téléversement.