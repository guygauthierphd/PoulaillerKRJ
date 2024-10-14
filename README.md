# PoulaillerKRJ
Poulailler de Karine MQTT/WIFI

Le poulailler est commandé à distance pour ouvrir ou fermer la porte et contrôler la température.
Le microcontrôleur utilisé est un ESP8266 qui communique en WiFi avec un Raspberry Pi via le protocole MQTT.
L'interface Web est faite via NodeRed et permet de commander l'ouverture et la fermeture de la porte du poulailler en mode manuel 
ou en mode automatique.

En mode automatique, la porte s'ouvre à une heure configurée par l'utilisateur et si la température extérieure excède un seuil 
qui est aussi choisi par l'utilisateur. La porte se ferme à une seconde heure configurée par l'usager. Notez qu'il faudra 
tout de même vérifier que les poules sont toutes dans le poulailler.

Une sortie 120 Vac est prévue pour y brancher un élément chauffant pour le chauffage du poulailler. La consigne est ajustée par 
l'usager.
