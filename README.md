# CREDEM Cam Fleet
## Introduction
CREDEM 3DP Camera Fleet est une solution de vidéo surveillance Open-Source. Cette solution permet d’intégrer un ESP32-CAM rapidement à la flotte de caméra déjà en place. Cette 
solution est utilisée dans le cadre de la surveillance des multiples imprimantes 3D de la plateforme. 

## Architecture de la solution
La solution se base sur un compte google et le google drive (de 15Go) associé. Ce compte google donne également accès à Google Script, élément clé du système. 

### 1) ESP32-CAM script
Le script à téléverser sur l’ESP-32 CAM permet d’envoyer des requêtes http au Google Script en place. Chaque seconde, il convertit l’image en URL et envoie l’information à Google Script grâce à la clé de déploiement du script.µ

### 2) Google Script
Google Script est une solution serveur hébergée directement chez Google. Dessus, on peut notamment définir les méthodes GET et POST nécessaires pour la communication HTTP. Ces scripts sont accessibles de partout grâce à leur clé de déploiement. Le Google Script possède une fonction POST, utilisée pour réceptionner des données, et une fonction GET pour les restituer. La fonction POST interagit avec le Google Drive associé au compte Google pour ajouter/supprimer des dossiers et des fichiers. La fonction GET  interagit avec le Google Drive associé au compte Google pour retourner l’URL drive des dernières images capturées. 

### 3) HTML Script
Le script HTML se sert de l’URL de déploiement du Google Script pour envoyer une requête GET et récupérer les dernières images capturées afin de les afficher dans un environnement utilisateur adéquat. 

