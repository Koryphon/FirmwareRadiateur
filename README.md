# FirmwareRadiateur

## Paramètres de connexion au WiFi

Le sketch inclut le fichier ```Network.h``` qui doit contenir les pareamètres de connexion au réseau WiFi. Il faut définir ces paramètres de connexion dans le fichier ```MyNetwork.h``` en se basant sur le canevas donnés dans ```NetworkTemplate.h``` et placer un lien symbolique :

```sh
ln -s MyNetwork.h Network.h
```

```Network.h``` et ```MyNetwork.h``` sont exclus du repository dans le ```.gitignore``` afin d'éviter de poussé le ssid et le mot de passe.