# FirmwareRadiateur

## Bibliothèques utilisées

Pour le DHT22, il s'agit de la bibliothèque Adafruit ```DHT Sensor Library```.
Pour MQTT, il s'agit de la bibliothèque MQTT de Joel Gaehwiler qui s'appelle tout simplement ```MQTT```.

Les deux sont à installer via le gestionnaire de bibliothèques de l'IDE Arduino)

## Paramètres de connexion au WiFi

Le sketch inclut le fichier ```Network.h``` qui doit contenir les paramètres de connexion au réseau WiFi. Il faut définir ces paramètres de connexion dans le fichier ```MyNetwork.h``` en se basant sur le canevas donnés dans ```NetworkTemplate.h``` et placer un lien symbolique :

```sh
ln -s MyNetwork.h Network.h
```

```Network.h``` et ```MyNetwork.h``` sont exclus du repository dans le ```.gitignore``` afin d'éviter de pousser le ssid et le mot de passe dans l'espace public.
