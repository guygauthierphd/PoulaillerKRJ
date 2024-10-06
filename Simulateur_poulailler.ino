const int Ouvrez = 14; // Commande l'ouverture de la porte
const int Fermez = 12; // Commande la fermeture de la porte
const int Ouverte = 13; // Détection porte ouverte
const int Fermee  = 15; // Détection porte fermée
int compteur = 0;
bool EtatO, EtatF;

void setup() {
  // put your setup code here, to run once:
  pinMode(Ouvrez,INPUT);
  pinMode(Fermez,INPUT);
  pinMode(Ouverte,OUTPUT);
  pinMode(Fermee,OUTPUT);

  // Initialement, porte fermée
  digitalWrite(Ouverte,HIGH);
  digitalWrite(Fermee,LOW);

  Serial.begin(115200);
}

void loop() {
  EtatO = digitalRead(Ouvrez);
  EtatF = digitalRead(Fermez);
  Serial.print(EtatO);
  Serial.print("\t");
  Serial.print(EtatF);
  Serial.print("\t");
  
  if (EtatO && !EtatF){
    compteur++;
  }
  if (!EtatO && EtatF){
    compteur--;
  }
  if (compteur<=0)
  {
    digitalWrite(Fermee,LOW);
    compteur = 0;
  }
  if (compteur>=100)
  {
    digitalWrite(Ouverte,LOW);
    compteur = 100;
  }
  if ((compteur>0) && (compteur<100))
  {
    digitalWrite(Fermee,HIGH);
    digitalWrite(Ouverte,HIGH);
  }
  delay(100);
  Serial.println(compteur);
}
