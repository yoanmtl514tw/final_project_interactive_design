// --- HARDWARE CONFIGURATION ---
const int pin_pot = A0;
const int pin_button = 2;
const int pin_led_green1 = 3;
const int pin_led_green2 = 8;
const int pin_led_green3 = 9;
const int pin_led_red1 = 6;
const int pin_vibration = 5;

// --- GAME CONSTANTS ---
const int maxErrors = 3;
const int dialPositions = 10; // 0-10 range

// CALIBRATION - Adjust POT_MAX until you can reach 10
const int POT_MIN = 0;
const int POT_MAX = 850; // Lower this if you can't reach 10

const int debounceDelay = 50;
const int vibrationDuration = 1000;
const int serialUpdateInterval = 100;

// --- GAME VARIABLES ---
int code1, code2, code3;
int currentStep = 1; 
int errors = 0;
bool buttonReleased = true;
bool vibrationActive = false;
int lastPositionSent = -1;

// --- TIMING VARIABLES ---
unsigned long vibrationStartTime = 0;
unsigned long lastSerialUpdate = 0;
unsigned long lastButtonPress = 0;

void setup()
{
  Serial.begin(9600);
  pinMode(pin_button, INPUT_PULLUP);
  pinMode(pin_led_green1, OUTPUT);
  pinMode(pin_led_green2, OUTPUT);
  digitalWrite(pin_led_green2, HIGH); 
  pinMode(pin_led_green3, OUTPUT);
  digitalWrite(pin_led_green3, HIGH); 
  pinMode(pin_led_red1, OUTPUT);

  generateCombination();
  delay(1000);
  sendCurrentState();
}

void loop()
{
  // Read and calibrate potentiometer
  int rawPot = analogRead(pin_pot);
  int constrainedValue = constrain(rawPot, POT_MIN, POT_MAX);
  int potValue = map(constrainedValue, POT_MIN, POT_MAX, 0, dialPositions);
  
  unsigned long currentMillis = millis();
  
  // Send potentiometer position to Unity with rate limiting
  if (potValue != lastPositionSent && (currentMillis - lastSerialUpdate >= serialUpdateInterval))
  {
    Serial.print("POT:");
    Serial.println(potValue);
    lastPositionSent = potValue;
    lastSerialUpdate = currentMillis;
  }
  
  int currentTarget = 0;
  if (currentStep == 1) currentTarget = code1;
  else if (currentStep == 2) currentTarget = code2;
  else if (currentStep == 3) currentTarget = code3;

  // CONSTANT vibration motor logic
  if (potValue == currentTarget)
  {
    if (!vibrationActive)
    {
      vibrationActive = true;
      vibrationStartTime = currentMillis;
      analogWrite(pin_vibration, 255);
      Serial.println("Vibration started!");
    }
  }
  else
  {
    if (vibrationActive)
    {
      analogWrite(pin_vibration, 0);
      vibrationActive = false;
      Serial.println("Vibration stopped");
    }
  }
  
  // Check if vibration duration has elapsed
  if (vibrationActive && (currentMillis - vibrationStartTime >= vibrationDuration))
  {
    analogWrite(pin_vibration, 0);
    vibrationActive = false;
    Serial.println("Vibration timeout");
  }

  // Button press detection with debouncing
  if (digitalRead(pin_button) == LOW && buttonReleased && (currentMillis - lastButtonPress >= debounceDelay))
  {
    buttonReleased = false; 
    lastButtonPress = currentMillis;
    checkInput(potValue, currentTarget);
  }
  
  if (digitalRead(pin_button) == HIGH)
  {
    buttonReleased = true;
  }

  // Debug output
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 500)
  {
    Serial.print("Raw: ");
    Serial.print(rawPot);
    Serial.print(" | Pot: "); 
    Serial.print(potValue);
    Serial.print(" | Target: "); 
    Serial.println(currentTarget);
    lastPrint = millis();
  }
}

void checkInput(int input, int target)
{
  if (input == target)
  {
    turnOnGreenLED(currentStep);
    Serial.print("CORRECT:");
    Serial.println(currentStep);
    currentStep++;
    
    analogWrite(pin_vibration, 0);
    vibrationActive = false;
    
    if (currentStep > 3)
    {
      Serial.println("VICTORY");
      victoryAnimation();
      resetGame();
    }
  }
  else
  {
    errors++;
    Serial.print("ERROR:");
    Serial.println(errors);
    Serial.print("You entered: ");
    Serial.print(input);
    Serial.print(" | Target was: ");
    Serial.println(target);
    blinkRedLED(errors);
    if (errors >= maxErrors)
    {
      Serial.println("GAMEOVER");
      resetGame();
    }
  }
}

void turnOnGreenLED(int stepNumber)
{
  if (stepNumber == 1) digitalWrite(pin_led_green1, HIGH);
  if (stepNumber == 2) digitalWrite(pin_led_green2, LOW);
  if (stepNumber == 3) digitalWrite(pin_led_green3, LOW);
}

void blinkRedLED(int numErrors) 
{
  int speed = 800 - (numErrors * 200); 
  if (speed < 200) speed = 200;
  
  for (int i = 0; i < 3; i++) 
  {
    digitalWrite(pin_led_red1, HIGH); 
    delay(speed / 2);
    digitalWrite(pin_led_red1, LOW); 
    delay(speed / 2);
  }
}

void generateCombination()
{
  randomSeed(analogRead(A5));
  code1 = random(5, dialPositions + 1);    // 5 to 10
  code2 = random(0, code1);                 // 0 to code1
  code3 = random(code2 + 1, dialPositions + 1); // code2+1 to 10
  
  Serial.print("CODE:");
  Serial.print(code1);
  Serial.print(",");
  Serial.print(code2);
  Serial.print(",");
  Serial.println(code3);
}

void sendCurrentState()
{
  Serial.print("CODE:");
  Serial.print(code1);
  Serial.print(",");
  Serial.print(code2);
  Serial.print(",");
  Serial.println(code3);
  
  Serial.print("STEP:");
  Serial.println(currentStep);
  Serial.print("ERRORS:");
  Serial.println(errors);
}

void resetGame()
{
  delay(1000);
  digitalWrite(pin_led_green1, LOW);
  digitalWrite(pin_led_green2, HIGH);
  digitalWrite(pin_led_green3, HIGH);
  digitalWrite(pin_led_red1, LOW);
  analogWrite(pin_vibration, 0);
  
  currentStep = 1;
  errors = 0;
  vibrationActive = false;
  
  generateCombination();
  Serial.println("RESET");
}

void victoryAnimation()
{
  for (int i = 0; i < 10; i++) 
  {
    digitalWrite(pin_led_green1, LOW); 
    digitalWrite(pin_led_green2, HIGH); 
    digitalWrite(pin_led_green3, HIGH);
    delay(100);
    digitalWrite(pin_led_green1, HIGH); 
    digitalWrite(pin_led_green2, LOW); 
    digitalWrite(pin_led_green3, LOW);
    delay(100);
  }
}