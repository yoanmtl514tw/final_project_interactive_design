// --- CONFIGURATION DU MATÉRIEL ---
const int pin_pot = A0;
const int pin_bp = 2;
const int pin_led_g1 = 3;
const int pin_led_g2 = 8;
const int pin_led_g3 = 9;
const int pin_led_r1 = 6;
const int pin_vibr = 7;

// --- VARIABLES DE JEU ---
int code1, code2, code3;
int etape = 1; 
int erreurs = 0;
const int maxErreurs = 3;
bool boutonRelache = true;
bool vibreurFait = false;

void setup()
{
  Serial.begin(9600);
  pinMode(pin_bp, INPUT_PULLUP);
  pinMode(pin_led_g1, OUTPUT);
  pinMode(pin_led_g2, OUTPUT);
  digitalWrite(pin_led_g2, HIGH); 
  pinMode(pin_led_g3, OUTPUT);
  digitalWrite(pin_led_g3, HIGH); 
  pinMode(pin_led_r1, OUTPUT);
  pinMode(pin_vibr, OUTPUT);

  genererCombinaison();
  Serial.println("--- TEST COFFRE-FORT ARDUINO ---");
  Serial.print("Code a trouver: ");
  Serial.print(code1); Serial.print(" - ");
  Serial.print(code2); Serial.print(" - ");
  Serial.println(code3);
}

void loop()
{
  int val_pot = map(analogRead(pin_pot), 0, 1023, 0, 20);
  
  int cibleActuelle = 0;
  if (etape == 1) cibleActuelle = code1;
  else if (etape == 2) cibleActuelle = code2;
  else if (etape == 3) cibleActuelle = code3;

  //logique du vibreur
  if (val_pot == cibleActuelle)
  {
    if (!vibreurFait)
    {
      Serial.println("Vibration en cours...");
      digitalWrite(pin_vibr, HIGH);
      delay(1000);                                          //1SECONDES
      digitalWrite(pin_vibr, LOW);
      vibreurFait = true;
    }
  }
  else
  {
    vibreurFait = false;
  }

  if (digitalRead(pin_bp) == LOW && boutonRelache)
  {
    boutonRelache = false; 
    verifierSaisie(val_pot, cibleActuelle);
  }
  
  if (digitalRead(pin_bp) == HIGH)
  {
    boutonRelache = true;
  }

  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 200)
  {
    Serial.print("Pot: "); 
    Serial.print(val_pot);
    Serial.print(" | Etape: "); 
    Serial.print(etape);
    Serial.print(" | Erreurs: "); 
    Serial.println(erreurs);
    lastPrint = millis();
  }
}

void verifierSaisie(int saisie, int cible)
{
  if (saisie == cible)
  {
    Serial.println("BRAVO ! Chiffre correct.");
    allumerLedVerte(etape);
    etape++;
    vibreurFait = false;
    
    if (etape > 3)
    {
      Serial.println("COFFRE OUVERT !");
      victoireAnim();
      resetJeu();
    }
  }
  else
  {
    erreurs++;
    clignoterRouge(erreurs);
    if (erreurs >= maxErreurs)
    {
      Serial.println("GAME OVER !");
      resetJeu();
    }
  }
}

void allumerLedVerte(int num)
{
  if (num == 1) digitalWrite(pin_led_g1, HIGH);
  if (num == 2) digitalWrite(pin_led_g2, LOW);
  if (num == 3) digitalWrite(pin_led_g3, LOW);
}

void clignoterRouge(int nErreurs) {
  int vitesse = 800 - (nErreurs * 200); 
  for (int i = 0; i < 3; i++) {
    digitalWrite(pin_led_r1, HIGH); delay(vitesse / 2);
    digitalWrite(pin_led_r1, LOW); delay(vitesse / 2);
  }
}

void genererCombinaison()
{
  randomSeed(analogRead(A5));
  code1 = random(10, 17);
  code2 = random(0, code1);
  code3 = random(code2 + 1, 17);
}

void resetJeu()
{
  delay(1000);
  digitalWrite(pin_led_g1, LOW);
  digitalWrite(pin_led_g2, HIGH);
  digitalWrite(pin_led_g3, HIGH);
  digitalWrite(pin_led_r1, LOW);
  etape = 1;
  erreurs = 0;
  vibreurFait = false;
  genererCombinaison();
  Serial.println("Nouveau code : " + String(code1) + "-" + String(code2) + "-" + String(code3));
}

void victoireAnim()
{
  for (int i = 0; i < 10; i++) {
    digitalWrite(pin_led_g1, LOW); digitalWrite(pin_led_g2, HIGH); digitalWrite(pin_led_g3, HIGH);
    delay(100);
    digitalWrite(pin_led_g1, HIGH); digitalWrite(pin_led_g2, LOW); digitalWrite(pin_led_g3, LOW);
    delay(100);
  }
}