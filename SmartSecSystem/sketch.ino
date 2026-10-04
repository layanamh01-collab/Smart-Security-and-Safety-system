#include <DHT.h>
//pins
#define PIR_PIN    13
#define DOOR_PIN   14
#define DHT_PIN     4
#define BUZZER_PIN 26
#define LED_RED    27
#define LED_GREEN  25
#define BTN_PIN    32
#define FIRE_TEMP  40.0
DHT dht(DHT_PIN, DHT22);

bool armed    = false;
bool lastBtn  = HIGH;
bool lastDoor = LOW;
bool lastPIR  = LOW;

unsigned long lastDoorAlert   = 0;
unsigned long lastMotionAlert = 0;
unsigned long lastFireAlert   = 0;
unsigned long lastTempPrint   = 0;

void flashRed() {
  digitalWrite(LED_RED, HIGH); delay(200);
  digitalWrite(LED_RED, LOW);  delay(200);
  digitalWrite(LED_RED, HIGH); delay(200);
  digitalWrite(LED_RED, LOW);
}

void beepThree() {
  for (int i = 0; i < 3; i++) {
    tone(BUZZER_PIN, 1000); delay(200);
    noTone(BUZZER_PIN);     delay(200);
  }
}

void clearAll() {
  noTone(BUZZER_PIN);
  digitalWrite(LED_RED,   LOW);
  digitalWrite(LED_GREEN, LOW);
}

void setup() {
  Serial.begin(115200);
  pinMode(PIR_PIN,    INPUT);
  pinMode(DOOR_PIN,   INPUT_PULLUP);
  pinMode(BTN_PIN,    INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_RED,    OUTPUT);
  pinMode(LED_GREEN,  OUTPUT);
  clearAll();
  dht.begin();
  Serial.println("System ready. Press button to arm.");
}

void loop() {
  unsigned long now = millis();
//button
  bool btn = digitalRead(BTN_PIN);
  if (btn == LOW && lastBtn == HIGH) {
    delay(300);
    armed = !armed;
    clearAll();
    digitalWrite(LED_GREEN, armed ? HIGH : LOW);
    Serial.println(armed ? ">> ARMED" : ">> DISARMED");
  }
  lastBtn = btn;

//temprature
  float temp = dht.readTemperature();
  bool  fire = (!isnan(temp) && temp >= FIRE_TEMP);

  if (fire) {
    tone(BUZZER_PIN, 2000);
    flashRed();
    if (now - lastFireAlert >= 10000) {
      lastFireAlert = now;
      Serial.print("!! FIRE ALERT: TEMP ");
      Serial.print(temp);
      Serial.println(" !!");
    }
  }

 if (!fire) {
    noTone(BUZZER_PIN);
    digitalWrite(LED_RED, LOW);
    if (now - lastTempPrint >= 10000) {
      lastTempPrint = now;
      if (!isnan(temp)) {
        Serial.print("[NORMAL TEMPERATURE: TEMP ");
        Serial.print(temp);
        Serial.println("]");
      }
    }
  }

//pir
  bool pir = digitalRead(PIR_PIN);
  if (armed && !fire) {
    if (pir == HIGH && lastPIR == LOW) {
      lastMotionAlert = now;
      flashRed(); beepThree();
      Serial.println("MOTION DETECTED");
    }
    if (pir == HIGH && (now - lastMotionAlert >= 20000)) {
      lastMotionAlert = now;
      flashRed(); beepThree();
      Serial.println("MOTION DETECTED");
    }
  }
  lastPIR = pir;

//door
  bool doorOpen = (digitalRead(DOOR_PIN) == HIGH);
  if (armed && !fire) {
    if (doorOpen && !lastDoor) {
      lastDoorAlert = now;
      flashRed(); beepThree();
      Serial.println("DOOR OPENED");
    }
    if (doorOpen && (now - lastDoorAlert >= 20000)) {
      lastDoorAlert = now;
      flashRed(); beepThree();
      Serial.println("DOOR OPENED");
    }
  }
  if (!doorOpen && lastDoor) {
    Serial.println("DOOR CLOSED");
    if (!fire) {
      clearAll();
      digitalWrite(LED_GREEN, armed ? HIGH : LOW);
    }
  }
  lastDoor = doorOpen;
}
