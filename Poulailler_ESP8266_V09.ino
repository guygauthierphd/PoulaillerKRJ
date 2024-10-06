// Librairies à inclure

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "RTClib.h"         // Real time clock lib
#include <Wire.h>
#include "DHT.h"
#include <OneWire.h>

// Configuration du DHT22
#define DHTPIN 2 
#define DHTTYPE DHT22

RTC_DS3231 rtc; //RTC_PCF8523 rtc;

DateTime now;

//Adafruit_FeatherOLED oled = Adafruit_FeatherOLED();
DHT dht(DHTPIN, DHTTYPE);

char daysOfTheWeek[7][12] = {"Dimanche", "Lundi", "Mardi", "Mercredi", "Jeudi", "Vendredi", "Saturday"};

// Attibution des broches
const int Ouvrez = 14;  // Sortie - Commande l'ouverture de la porte
const int Fermez = 12;  // Sortie - Commande la fermeture de la porte (ancien 12)
const int Chauffe = 16; // Sortie - Commande de l'élément chauffant
const int Ouverte = 13; // Entrée - Détection porte ouverte
const int Fermee  = 15; // Entrée - Détection porte fermée (ancien 15)
const byte BROCHE_ONEWIRE = 0;

// État de la commande de porte
int etat = 0, new_etat = 0;
bool Open = false;
bool Close = false;

// Variables
float hum = 0.00;
float intTemp = 0.00;
int beep = 1;
char buf[16];
char bufheur[8];

float temperature;
int modePorte=0;
unsigned long Houvre=0;
unsigned long Hferme=0;
float seuil = 10.0;
float setpoint = -15.0;
unsigned long maintenant = 0;
int hAct = 0;
int mAct = 0;

enum DS18B20_RCODES {
  READ_OK,  // Lecture ok
  NO_SENSOR_FOUND,  // Pas de capteur
  INVALID_ADDRESS,  // Adresse reçue invalide
  INVALID_SENSOR  // Capteur invalide (pas un DS18B20)
};

const char* ssid = "Poulettes";  
const char* wifi_password = "Colette123"; 

const char* mqtt_server = "192.168.1.100";  // Adresse IP du Raspberry Pi
const char* your_topic0 = "Porte/arret";
const char* your_topic1 = "Porte/ouvre";
const char* your_topic2 = "Porte/ferme";
const char* your_topic3 = "Temperature/dehors";
const char* your_topic4 = "Ouverte";//Porte/etat";
const char* your_topic5 = "Temperature/interieure";
const char* your_topic6 = "Humidite";
const char* your_topic7 = "Automatique/Houvre";
const char* your_topic8 = "Automatique/Hferme";
const char* your_topic9 = "Automatique/Mode";
const char* your_topicA = "Automatique/seuil";
const char* your_topicB = "Automatique/consigneT";
const char* your_topicC = "Automatique/heureActuelle";
const char* your_topicD = "Horloge/Heure";
const char* your_topicE1 = "Horloge/Ouvre";
const char* your_topicE2 = "Horloge/Ferme";
const char* your_topicF = "Chauffage";

const char* clientID = "Poulailler_Portes";

OneWire ds(BROCHE_ONEWIRE);
WiFiClient wifiClient;
PubSubClient client(mqtt_server,1883,wifiClient);
IPAddress local_IP(192,168,1,109);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,0,0);
IPAddress primaryDNS(8,8,8,8);
IPAddress secondaryDNS(8,8,4,4);


// Routine appelée à chaque message reçu de la part du Broket MQTT
//
void ReceivedMessage(char* topic, byte* payload, unsigned int length){
  char mess[30];
  char bufH[8];

  memcpy(mess,payload,length);
  mess[length] = '\0';
  
  Serial.print(topic);
  Serial.print(" : ");
  for(int i=0;i<length;i++)
    Serial.print((char) payload[i]);
  Serial.println();

  // Demande d'arret de porte - manuellement
  if(strcmp(topic,your_topic0)==0){
    /*if ((char) payload[0]=='0'){
      if(digitalRead(Ouverte)) Open = true;
    }
    if ((char) payload[0]=='1'){
      Open = false;
    }*/
    Open = false;
    Close = false;
  }
  
  // Demande d'ouverture de porte - manuellement
  if(strcmp(topic,your_topic1)==0){
    if ((char) payload[0]=='0'){
      if(digitalRead(Ouverte)) Open = true;
    }
    if ((char) payload[0]=='1'){
      Open = false;
    }
  }

  // Demande de fermeture de porte - manuellement
  if(strcmp(topic,your_topic2)==0){
    if ((char) payload[0]=='0'){
      if(digitalRead(Fermee)) Close = true;
    }
    if ((char) payload[0]=='1'){
        Close = false;
    }
  }

  // Heure d'ouverture - automatiquement
  if(strcmp(topic,your_topic7)==0){
    Houvre = atol(mess)/1000;
    hAct = (int) Houvre/3600;
    mAct = (int) (Houvre-3600*hAct)/60;
    sprintf(bufH,"%02d:%02d",hAct,mAct);
    client.publish(your_topicE1,bufH);
  }

  // Heure de fermeture - automatiquement
  if(strcmp(topic,your_topic8)==0){
    Hferme = atol(mess)/1000;
    hAct = (int) Hferme/3600;
    mAct = (int) (Hferme-3600*hAct)/60;
    sprintf(bufH,"%02d:%02d",hAct,mAct);
    client.publish(your_topicE2,bufH);
  }

  // Mode de fonctionnement
  if(strcmp(topic,your_topic9)==0){
    modePorte = atoi(mess);
  }

  // Seuil en deça duquel la porte ne s'ouvre pas en mode automatique
  if(strcmp(topic,your_topicA)==0){
    seuil = atof(mess);
  }
  
  // Consigne de température du poulailler
  if(strcmp(topic,your_topicB)==0){
    setpoint = atof(mess);
  }

  // Heure d'ouverture - automatiquement
  if(strcmp(topic,your_topicC)==0){
    hAct = (int) atol(mess)/1000/3600;
    mAct = (int) (atol(mess)/1000-3600*hAct)/60;
    rtc.adjust(DateTime(2019, 2, 12, hAct, mAct, 0));
  }
}

// Routine d'abonnement aux topics du Broker MQTT
bool Connect(){
  if(client.connect(clientID)){
    client.subscribe(your_topic0);
    client.subscribe(your_topic1);
    client.subscribe(your_topic2);
    client.subscribe(your_topic7);
    client.subscribe(your_topic8);
    client.subscribe(your_topic9);
    client.subscribe(your_topicA);
    client.subscribe(your_topicB);
    client.subscribe(your_topicC);
    return true;
  }
  else{
    return false;
  }
}

// Routine de lecture de température du DS18B20
// Communication onewire...
byte getTemperature(float *temperature, byte reset_search) {
  byte data[9], addr[8];
  // data[] : Données lues depuis le scratchpad
  // addr[] : Adresse du module 1-Wire détecté
  
  /* Reset le bus 1-Wire ci nécessaire (requis pour la lecture du premier capteur) */
  if (reset_search) {
    ds.reset_search();
  }
 
  /* Recherche le prochain capteur 1-Wire disponible */
  if (!ds.search(addr)) {
    // Pas de capteur
    return NO_SENSOR_FOUND;
  }
  
  /* Vérifie que l'adresse a été correctement reçue */
  if (OneWire::crc8(addr, 7) != addr[7]) {
    // Adresse invalide
    return INVALID_ADDRESS;
  }
 
  /* Vérifie qu'il s'agit bien d'un DS18B20 */
  if (addr[0] != 0x28) {
    // Mauvais type de capteur
    return INVALID_SENSOR;
  }
 
  /* Reset le bus 1-Wire et sélectionne le capteur */
  ds.reset();
  ds.select(addr);
  
  /* Lance une prise de mesure de température et attend la fin de la mesure */
  ds.write(0x44, 1);
  delay(800);
  
  /* Reset le bus 1-Wire, sélectionne le capteur et envoie une demande de lecture du scratchpad */
  ds.reset();
  ds.select(addr);
  ds.write(0xBE);
 
 /* Lecture du scratchpad */
  for (byte i = 0; i < 9; i++) {
    data[i] = ds.read();
  }
   
  /* Calcul de la température en degré Celsius */
  *temperature = (int16_t) ((data[1] << 8) | data[0]) * 0.0625; 
  
  // Pas d'erreur
  return READ_OK;
}

// Programme d'initialisation du système
void setup () {
  pinMode(Ouverte,INPUT_PULLUP);
  pinMode(Fermee,INPUT_PULLUP);
  pinMode(Ouvrez,OUTPUT);
  pinMode(Fermez,OUTPUT);
  pinMode(Chauffe,OUTPUT);
  digitalWrite(Ouvrez,LOW);
  digitalWrite(Fermez,LOW);
  digitalWrite(Chauffe,LOW);

  Serial.begin(115200);
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  
  Serial.print("Connection a ");
  Serial.println(ssid);

  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)){
    Serial.println("ST failed to configure!!!");
  }

  WiFi.begin(ssid,wifi_password);

  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connecte au WiFi");
  Serial.print("Adresse IP: ");
  Serial.println(WiFi.localIP());
  dht.begin();
  
  delay(500);
  
  client.setCallback(ReceivedMessage);
  if (Connect()){
    Serial.println("Connecte au MQTT Broker !!!");
  }
  else{
    Serial.println("Echec de la connection au MQTT Broker !!!");
  }
  delay(1000);
  if(!digitalRead(Ouverte))
  {
    Serial.println("Ouverte");
    client.publish(your_topic4,"Porte ouverte");
  }
  else{
    if(!digitalRead(Fermee)) 
    {
      Serial.println("Fermée");
      client.publish(your_topic4,"Porte Fermée");
    }
    else
    {
      Serial.println("Arrêtée");
      client.publish(your_topic4,"Porte Arrêtée");
    }
  }
}

// Programme principal 
void loop () {
  now = rtc.now();
  maintenant = now.hour()*3600+now.minute()*60+now.second();

  // Si mode automatique...
  if(modePorte==1){
    
    // Si porte non ouverte et pas trop froid et la bonne heure, ouvrir la porte
    if((Houvre<maintenant) && (Houvre>(maintenant-6)) && !Open){
      if(seuil<temperature){
        if(digitalRead(Ouverte)) Open = true;
        Close = false;
        Serial.println("Ouverture automatique");
      }
    }
    // Si porte non fermée et la bonne heure, fermer la porte
    if((Hferme<maintenant) && (Hferme>(maintenant-6)) && !Close){
        if(digitalRead(Fermee)) Close = true;
        Open = false;
        Serial.println("Fermeture automatique");
    }
  }
  
  if ((beep--) <=0){
    hum = dht.readHumidity();
    intTemp = dht.readTemperature();
    if (isnan(hum) || isnan(intTemp)) {
      Serial.println("Failed to read from DHT sensor!");
    //return;
    }
    else{
      Serial.print("Humidity: ");
      Serial.print(hum);
      Serial.print(" %\t");
      snprintf(buf, sizeof(buf), "%5.2f", hum);
      client.publish(your_topic6,buf);
      Serial.print("Temperature: ");
      Serial.print(intTemp);
      Serial.print(" *C \t");
      snprintf(buf, sizeof(buf), "%5.2f", intTemp);
      client.publish(your_topic5,buf);
      //affiche_interieur(hum, intTemp);
    }
    if (getTemperature(&temperature, true) != READ_OK) {
    Serial.println(F("Erreur de lecture du capteur"));
    }else{
      Serial.print(temperature);
      Serial.println(" *C");
      snprintf(buf, sizeof(buf), "%5.2f", temperature); //(randNumber/10.0));
      client.publish(your_topic3,buf);
    }
    sprintf(bufheur,"%02d:%02d",now.hour(),now.minute());
    client.publish(your_topicD,bufheur);

  if (intTemp<setpoint){  
    client.publish(your_topicF,"En marche");
  }
  else{
    client.publish(your_topicF,"Arrêté");
  }
    beep = 100;  // Lecture des données 1 fois tous les 100 cycles.
  }

  /* SEQUENCE DE COMMANDE DE LA PORTE */
  
  if(digitalRead(Ouverte) && Open)
  {
    new_etat = 1; // La porte s'ouvre
    //client.publish(your_topic4,"Porte en ouverture");
  }
  if(digitalRead(Fermee) && Close)
  {
    new_etat = 2;  // La porte se ferme
    //client.publish(your_topic4,"Porte en fermeture");
  }
  if(!digitalRead(Ouverte) && !Close)
  {
    new_etat = 3; // Porte ouverte
    //client.publish(your_topic4,"Porte ouverte");
    Open = false;
  }
  if(!digitalRead(Fermee) && !Open)
  {
    new_etat = 4; // Porte fermee
    //client.publish(your_topic4,"Porte fermée");
    Close = false;
  }
  if(digitalRead(Ouverte) && !Open && digitalRead(Fermee) && !Close){
    new_etat = 5; // Porte fermee
    //client.publish(your_topic4,"Porte arrêtée");
  }
  
  if (etat != new_etat){
    switch(new_etat)
    {
      case(1):
        Serial.println("Ouverture");
        client.publish(your_topic4,"Porte en ouverture");
        digitalWrite(Ouvrez,HIGH);
        digitalWrite(Fermez,LOW);
        break;
      case(2):
        Serial.println("Fermeture");
        client.publish(your_topic4,"Porte en fermeture");
        digitalWrite(Ouvrez,LOW);
        digitalWrite(Fermez,HIGH);
        break;
      case(3):
        Serial.println("Ouverte");
        client.publish(your_topic4,"Porte ouverte");
        digitalWrite(Ouvrez,LOW);
        break;
      case(4):
        Serial.println("Fermee");
        client.publish(your_topic4,"Porte fermée");
        digitalWrite(Fermez,LOW);
        break;
      case(5):
        Serial.println("Arrêtée");
        client.publish(your_topic4,"Porte arrêtée");
        digitalWrite(Fermez,LOW);
        digitalWrite(Ouvrez,LOW);
        break;
      default:
        Serial.println("Erreur");
        break;
    }
    etat = new_etat;
  }
  
  /* COMMANDE DU CHAUFFAGE DU POULAILLER */
  
  if (intTemp<setpoint){
    digitalWrite(Chauffe,HIGH);
  }
  else{
    digitalWrite(Chauffe,LOW);
  }

  // Fin du cycle, affiche et vérifie persistence de la connection avec le Broker
  
 if(!client.connected()){
    Connect();
  }
  client.loop();
  delay(50);
}
