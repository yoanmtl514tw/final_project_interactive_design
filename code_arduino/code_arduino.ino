//CONSTANTE
const int potentiometer_pin = A0;
const int led_pin_1 = 3;
const int tolerance_nt = 2;
const int vibrator_pin = 4;
const int vibrator_time = 2000;

//VARIABLE GLOBALE
int number_one = 0;
int number_two = 0;
int number_three = 0;
int potentiometer_value_scaled = -1;

int game_stage = 1; //1 = find number one, 2 = find number 2, 3 = find number three and 4 = game finish


void setup()
{
  Serial.begin(9600);
  
  //configuration pin
  pinMode(led_pin_1,OUTPUT);
  pinMode(vibrator_pin, OUTPUT);

  //Obtaining random number
  randomSeed(analogRead(A0));
  random_number_generate();

    // 3. Afficher les résultats pour vérification
  display_target();

  

}

void loop()
{

  // Lecture et conversion du potentiomètre (0-1023 -> 0-100)
  int raw_value = analogRead(potentiometer_pin);
  potentiometer_value_scaled = map(raw_value, 0, 1023, 0, 100);
  
  // Logique principale : on vérifie la valeur en fonction de l'étape actuelle
  if (game_stage == 1)
  {
    check_target(number_one, 2);
  }
  else if (game_stage == 2)
  {
    check_target(number_two, 3);
  }
  else if (game_stage == 3)
  {
    check_target(number_three, 4);
  }
  else if (game_stage == 4)
  {
    digitalWrite(led_pin_1, HIGH); 
  }

  // Affichage du suivi dans le moniteur série
  // (Utile pour le débug, mais peut être commenté ou supprimé plus tard)
  Serial.print("Potentiomètre: ");
  Serial.print(potentiometer_value_scaled);
  Serial.print(" | Étape : ");
  Serial.println(game_stage);

  delay(100); // Petite pause pour la stabilité
}


void random_number_generate()
{
  //between 50 and 100
  number_one = random(50,101);

  //Less than number one
  number_two = random(0,number_one);

  //More than number two
  number_three = random(number_two +1,101);
}


void display_target()
{
  Serial.println("--- CIBLES DU JEU (Mode Debug) ---");
  Serial.print("CHFFRE 1 : ");
  Serial.println(number_one);
  Serial.print("CHFFRE 2 : ");
  Serial.println(number_two);
  Serial.print("CHFFRE 3 : ");
  Serial.println(number_three);
  Serial.println("----------------------------------");
}

void check_target(int target_value, int next_stage) {
  // Vérifie si la valeur du potentiomètre est dans la zone de tolérance
  if (potentiometer_value_scaled >= target_value - tolerance_nt && 
      potentiometer_value_scaled <= target_value + tolerance_nt) {
      
    // CIBLE ATTEINTE
    
    // Si c'est une nouvelle étape (n'est pas encore l'étape FINALE 4)
    if (game_stage != next_stage) { 
        // 1. Démarrer la vibration
        digitalWrite(vibrator_pin, HIGH);
        
        // 2. Allumer la LED (feedback visuel immédiat)
        digitalWrite(led_pin_1, HIGH);
        
        // 3. Annoncer le succès
        Serial.print(">>> CIBLE TROUVÉE : ");
        Serial.print(target_value);
        Serial.println(" --- VIBRATION DÉCLENCHÉE ! ---");
        
        // 4. Attendre la durée de la vibration (2 secondes)
        delay(vibrator_time);
        
        // 5. Arrêter la vibration
        digitalWrite(vibrator_pin, LOW);

        // La LED reste allumée jusqu'à ce que l'utilisateur commence à chercher la prochaine cible.
        
        // 6. Passer à l'étape suivante
        game_stage = next_stage; 
        
        if (game_stage <= 3) {
          Serial.print("ÉTAPE ");
          Serial.print(game_stage);
          Serial.println(" : Trouvez la nouvelle valeur !");
        } else {
          Serial.println("\n*** BRAVO ! VOUS AVEZ TROUVÉ LES 3 VALEURS ! ***");
        }
        
        // 7. Petite attente supplémentaire avant de reprendre le loop()
        // Cela empêche l'étape de s'activer plusieurs fois rapidement.
        delay(500); 

    } // Fin de la condition 'if (game_stage != next_stage)'

  } else {
    // Si la cible n'est pas atteinte OU si le jeu est terminé (game_stage = 4), la LED est éteinte
    if (game_stage < 4) {
      digitalWrite(led_pin_1, LOW);
    }
  }
}

