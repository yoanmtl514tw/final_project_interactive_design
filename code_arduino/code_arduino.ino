// --- HARDWARE CONFIGURATION ---
const int pin_pot = A0;
const int pin_button = 2;
const int pin_led_green1 = 3;
const int pin_led_green2 = 8;
const int pin_led_green3 = 9;
const int pin_led_red1 = 6;
const int pin_vibration = 7;

// --- GAME CONSTANTS ---
const int maxErrors = 3;
const int dialPositions = 20; // Number range: 0-20
const int debounceDelay = 50; // Button debounce time in ms
const int vibrationDuration = 1000; // Vibration time in ms
const int serialUpdateInterval = 100; // Min time between serial updates

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
  pinMode(pin_vibration, OUTPUT);

  generateCombination();
  
  // Wait a moment for serial connection to establish
  delay(1000);
  
  // Send initial state to Unity
  sendCurrentState();
}

void loop()
{
  int potValue = map(analogRead(pin_pot), 0, 1023, 0, dialPositions);
  
  // Send potentiometer position to Unity with rate limiting
  unsigned long currentMillis = millis();
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

  // NON-BLOCKING vibration motor logic
  if (potValue == currentTarget)
  {
    if (!vibrationActive)
    {
      digitalWrite(pin_vibration, HIGH);
      vibrationActive = true;
      vibrationStartTime = currentMillis;
    }
  }
  else
  {
    // Turn off vibration if we move away from target
    if (vibrationActive)
    {
      digitalWrite(pin_vibration, LOW);
      vibrationActive = false;
    }
  }
  
  // Check if vibration duration has elapsed
  if (vibrationActive && (currentMillis - vibrationStartTime >= vibrationDuration))
  {
    digitalWrite(pin_vibration, LOW);
    vibrationActive = false;
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
}

void checkInput(int input, int target)
{
  if (input == target)
  {
    turnOnGreenLED(currentStep);
    Serial.print("CORRECT:");
    Serial.println(currentStep);
    currentStep++;
    
    // Reset vibration state
    digitalWrite(pin_vibration, LOW);
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
  if (speed < 200) speed = 200; // Safety check
  
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
  code1 = random(10, 17);
  code2 = random(0, code1);
  code3 = random(code2 + 1, 17);
  
  // Send code to Unity
  Serial.print("CODE:");
  Serial.print(code1);
  Serial.print(",");
  Serial.print(code2);
  Serial.print(",");
  Serial.println(code3);
}

void sendCurrentState()
{
  // Send current combination
  Serial.print("CODE:");
  Serial.print(code1);
  Serial.print(",");
  Serial.print(code2);
  Serial.print(",");
  Serial.println(code3);
  
  // Send current step and errors
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
  digitalWrite(pin_vibration, LOW);
  
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