void setup() {
  // put your setup code here, to run once:
  pinMode(13,INPUT_PULLUP);
  pinMode(12,OUTPUT);
  pinMode(14,OUTPUT);
  
  digitalWrite(12,LOW);
  digitalWrite(14,LOW);
  
  delay(250);
  digitalWrite(12,HIGH);
  delay(1000);
  
  digitalWrite(14,HIGH);
  
}

void loop() {
  // put your main code here, to run repeatedly:

}
