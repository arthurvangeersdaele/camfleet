# CREDEM Cam Fleet
## Introduction
CREDEM 3DP Camera Fleet est une solution de vidéo surveillance Open-Source. Cette solution permet d’intégrer un ESP32-CAM rapidement à la flotte de caméra déjà en place. Cette 
solution est utilisée dans le cadre de la surveillance des multiples imprimantes 3D de la plateforme. 

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
