// Limit Switch
#define LIMIT_SWITCH_PIN 0

// Keypad
#include <Keypad.h>
const byte ROWS = 4; 
const byte COLS = 3; 
char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};
byte rowPins[ROWS] = {45, 47, 49, 51}; 
byte colPins[COLS] = {39, 41, 43}; 
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 

// LCD
#include <LiquidCrystal.h>
const int rs = 13, en = 12, d4 = 11, d5 = 10, d6 = 9, d7 = 8;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// LED Red and Green
#define GREEN 6
#define RED 5

// Servo
#include <Servo.h>
Servo myservo;  // create servo object to control a servo

// Limit Switch
int limitSwitch = 7;

// Buzzer
#define BUZZER 2

// Passcode
#define Password_Length 8 
char Data[Password_Length]; 
char Master[Password_Length] = "1234567"; 
char Last_Passcode[Password_Length] = "";  // Store last used passcode
char User1[Password_Length] = "7654321";  // Additional user passcode
char User2[Password_Length] = "1122334";  // Additional user passcode
byte data_count = 0;
bool Pass_is_good;
char customKey;
bool Status = 1;
char lastUser = ' '; // Track who used the pinpad last

// Change passcode
bool changePasscodeMode = false;
char tempPasscode1[Password_Length];
char tempPasscode2[Password_Length];
byte temp_count = 0;

unsigned long previousMillis = 0;
const long interval = 120000; // 2 minutes in milliseconds

void setup() {
  // Setup code
  pinMode(GREEN, OUTPUT);
  pinMode(RED, OUTPUT);
  digitalWrite(RED, LOW);
  digitalWrite(GREEN, HIGH);
  pinMode(BUZZER, OUTPUT);
  noTone(BUZZER);
  myservo.attach(3); 
  myservo.write(0);

  Serial.begin(9600);
  pinMode(LIMIT_SWITCH_PIN, INPUT);
  lcd.begin(16, 2);
  lcd.print("Monitoring...");
  pinMode(limitSwitch, INPUT_PULLUP);
}

void loop() {
  // Main code
  unsigned long currentMillis = millis();

  // Check if limit switch is open
  if (digitalRead(limitSwitch) == 1) { // Assuming 1 means open
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;

      while (digitalRead(limitSwitch) == 1) { // Alarm logic
        lcd.setCursor(0, 0); // Set cursor to top-left before printing
        lcd.print("Alert!");
        tone(BUZZER, 1000);
        digitalWrite(RED, HIGH);
        digitalWrite(GREEN, LOW);
        delay(200); // Brief delay for user to see the alert
        lcd.clear(); // Clear the alert message
        noTone(BUZZER);
        digitalWrite(RED, LOW);
        delay(200);
      }
    }
  } else {
    previousMillis = currentMillis; // Reset the timer if limit switch is closed
    noTone(BUZZER);
    digitalWrite(RED, LOW);
    digitalWrite(GREEN, HIGH);
    lcd.setCursor(0, 0); // Set cursor to top-left before printing
    lcd.print("Monitoring..."); // Reprint monitoring message
  }

  customKey = customKeypad.getKey();
  if (customKey) {
    if (changePasscodeMode) {
      handleChangePasscode(customKey);
    } else {
      handleRegularMode(customKey);
    }
  }

  if (data_count == Password_Length - 1) {
    lcd.clear();
    if (!strcmp(Data, Master)) {
      handleCorrectPasscode('M');
    } else if (!strcmp(Data, User1)) {
      handleCorrectPasscode('U');
    } else if (!strcmp(Data, User2)) {
      handleCorrectPasscode('V');
    } else {
      handleIncorrectPasscode();
    }
    clearData();  
  }
}

void handleRegularMode(char key) {
  if (key == '*') {
    if (data_count == 0) {
      changePasscodeMode = true;
      lcd.clear();
      lcd.print("Change Passcode");
      delay(1000);
      lcd.clear();
      lcd.print("Enter Current");

    } else {
      Data[data_count] = key;
      lcd.setCursor(data_count, 1);
      lcd.print(Data[data_count]);
      data_count++;
    }
  } else if (key == '#') {
    displayLastUser();
  } else {
    Data[data_count] = key;
    lcd.setCursor(data_count, 1);
    lcd.print(Data[data_count]);
    data_count++;
  }
}

void handleChangePasscode(char key) {
  if (temp_count == 0) {
    lcd.clear();
    lcd.print("Enter New");
    lcd.setCursor(0, 1);
  }
  tempPasscode1[temp_count] = key;
  lcd.print(tempPasscode1[temp_count]);
  temp_count++;

  if (temp_count == Password_Length - 1) {
    lcd.clear();
    lcd.print("Confirm New");
    lcd.setCursor(0, 1);
    temp_count = 0;
    while (temp_count < Password_Length - 1) {
      customKey = customKeypad.getKey();
      if (customKey) {
        tempPasscode2[temp_count] = customKey;
        lcd.print(tempPasscode2[temp_count]);
        temp_count++;
      }
    }

    if (!strcmp(tempPasscode1, tempPasscode2)) {
      strcpy(User1, tempPasscode1);
      lcd.clear();
      lcd.print("Passcode Updated");
      delay(1000);
      lcd.clear();
      lcd.print("Monitoring...");
    } else {
      lcd.clear();
      lcd.print("Mismatch");
      delay(1000);
      lcd.clear();
      lcd.print("Monitoring...");
    }

    clearTempData();
    changePasscodeMode = false;
  }
}

void handleCorrectPasscode(char user) {
  lcd.print("Correct");
  storeLastPasscode(Data);
  lastUser = user;
  delay(5000);
  if (Status == 1) {
    myservo.write(90);
    Status = 0;
  } else if (Status == 0) {
    myservo.write(0);
    Status = 1;
  }
}

void handleIncorrectPasscode() {
  lcd.print("Incorrect");
  tone(BUZZER, 1000);
  delay(500);
  noTone(BUZZER);
  delay(500);
  lcd.clear();
  lcd.print("Monitoring");
}

void storeLastPasscode(char *passcode) {
  strcpy(Last_Passcode, passcode);
}

void clearData() {
  while (data_count != 0) {
    Data[data_count--] = 0;
  }
}

void clearTempData() {
  for (int i = 0; i < Password_Length; i++) {
    tempPasscode1[i] = 0;
    tempPasscode2[i] = 0;
  }
  temp_count = 0;
}

void displayLastUser() {
  lcd.clear();
  lcd.print("Last User: ");
  if (lastUser == 'M') {
    lcd.print("Master");
  } else if (lastUser == 'U') {
    lcd.print("User1");
  } else if (lastUser == 'V') {
    lcd.print("User2");
  } else {
    lcd.print("None");
  }
  delay(2000);
  lcd.clear();
  lcd.print("Monitoring...");
}
