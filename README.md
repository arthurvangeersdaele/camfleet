# Cam Fleet
La vidéo-surveillance de partout dans le monde pour seulement 10$.

## Introduction
Cam-Fleet (or 3DP Camera Fleet) est une solution de vidéo surveillance Open-Source. Cette solution permet d’intégrer un ESP32-CAM rapidement à la flotte de caméra déjà en place. Cette solution est utilisée dans le cadre de la surveillance des multiples imprimantes 3D. L'état de la solution est tel que vos images seront accessibles publiquement, n'utilisez pas cette solution pour surveiller des personnes sans leur consentement.

## Architecture de la solution
La solution se base sur un compte google et le google drive (de 15Go) associé. Ce compte google donne également accès à Google Script, élément clé du système. 

![](https://github.com/arthurvangeersdaele/camfleet/blob/main/architecture.png)

### 1) ESP32-CAM script
Le script à téléverser sur l’ESP-32 CAM permet d’envoyer des requêtes http au Google Script en place. Chaque seconde, il convertit l’image en URL et envoie l’information à Google Script grâce à la clé de déploiement du script.

### 2) Google Script
Google Script est une solution serveur hébergée directement chez Google. Dessus, on peut notamment définir les méthodes GET et POST nécessaires pour la communication HTTP. Ces scripts sont accessibles de partout grâce à leur clé de déploiement. Le Google Script possède une fonction POST, utilisée pour réceptionner des données, et une fonction GET pour les restituer. La fonction POST interagit avec le Google Drive associé au compte Google pour ajouter/supprimer des dossiers et des fichiers. La fonction GET  interagit avec le Google Drive associé au compte Google pour retourner l’URL drive des dernières images capturées. Un déclencheur est programmé pour supprimer les fichiers vieux de plus de 48h tous les matins à 1h00.

### 3) HTML Script
Le script HTML se sert de l’URL de déploiement du Google Script pour envoyer une requête GET et récupérer les dernières images capturées afin de les afficher dans un environnement utilisateur adéquat. 

## Comment ajouter un ESP-32 CAM (D0WD) à la flotte ?
Connectez-vous au Google Script du projet sur votre compte Google (ou celui que vous aurez créé pour le projet). Récupérez la clé de déploiement et l'url de déploiement. Lors du téléversement du fichier .ino sur l'ESP-32-CAM D0WD, mettez à jour la clé dans le script Arduino et définissez le nom de la caméra: "ESP32-CAM-xXxXx". Attention, le nom de la caméra doit impérativement commencer par "ESP32-CAM".

## Organisation des fichiers .jpg dans le Google Drive
Le Google Script va recevoir des informations de l'ESP32-CAM et : 
- Créer dans le dossier racine un dossier portant le même nom que l'ESP32-CAM, s'il n'existe pas déjà
- Créer un sous-dossier dont le nom est la date du jour, s'il n'existe pas déjà
- Sauvegarder dans ce dossier toutes les images réceptionnées depuis l'ESP32-CAM concerné.
  
## Délétion automatique des fichiers >48h
Afin de pouvoir fonctionner avec les 15Go disponibles gratuitement dans Google Drive, seuls les dernières 48h sont conservées. Un déclencheur est instancé dans Google Script et déclenche la délétion de tout sous-dossier vieux de plus de 48h pour chacun des dossiers caméra. 

## Fréquence de rafraichissement du flux
La fréquence de rafraichissement des images sur le site web dépend de 3 facteurs:
- la fréquence d'envoi d'image vers le Google Drive (harcodée à 1jpg/sec, mesurée à ~1jpg/20sec)
- la rapidité de d'éxécution du Google Script et de l'enregistrement dans le Drive (supposé)
- la qualité de la connexion Wi-Fi de l'ESP32-CAM (observé)

# TO DO 
- use ESP32-CAM calls to Google Script to execute commands on the ESP (index could be used to update a variable in the script)
- from that, implement LED flashing on demand
- create a manual for quick implementation of the solution
- ajouter les liens vers les composants
- faire un test avec d'autres version d'ESP32-CAM
- ajouter l'étape de fabrication des boitiers
- Alerter sur le fait que les images sont publiques car la clé de déploiement est en clair (bosser sur une version sécurisée ?)
