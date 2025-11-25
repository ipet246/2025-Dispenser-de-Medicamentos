#include <Wire.h>
#include <RTClib.h>
#include <Servo.h>

RTC_DS3231 rtc;
Servo servo;

// LED rojo y verde
int ledRojo = 4;
int ledVerde = 5;

// Buzzer
int buzzerPin = 8;

// Hora programada
int horaProg = -1;
int minutoProg = -1;

bool hayDosisPendiente = false;

void setup() {
  Serial.begin(9600);

  servo.attach(12);
  servo.write(0);   // inicia cerrado

  pinMode(ledRojo, OUTPUT);
  pinMode(ledVerde, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  digitalWrite(ledRojo, HIGH);   // rojo esperando
  digitalWrite(ledVerde, LOW);
  digitalWrite(buzzerPin, LOW);

  if (!rtc.begin()) {
    Serial.println("ERROR: No se encuentra RTC");
    while (1);
  }

  Serial.println("Dispensador listo.");
}

void loop() {

  // ------------------ COMANDOS ------------------
  if (Serial.available()) {
    String msg = Serial.readStringUntil('\n');
    msg.trim();

    // ---- tomada ----
    if (msg.equalsIgnoreCase("tomada")) {
      digitalWrite(ledRojo, LOW);
      digitalWrite(ledVerde, HIGH);
      digitalWrite(buzzerPin, LOW);
      Serial.println("OK: tomada");

      // No movemos el servo; queda donde estaba
      return;
    }

    // ---- verhora ----
    if (msg.equalsIgnoreCase("verhora")) {
      DateTime now = rtc.now();
      char buffer[6];
      sprintf(buffer, "%02d:%02d", now.hour(), now.minute());
      Serial.println(buffer);
      return;
    }

    // ---- hora HH:MM ----
    if (msg.startsWith("hora ")) {
      String h = msg.substring(5);
      int sep = h.indexOf(':');

      if (sep > 0) {
        horaProg = h.substring(0, sep).toInt();
        minutoProg = h.substring(sep + 1).toInt();
        hayDosisPendiente = true;

        digitalWrite(ledRojo, HIGH);
        digitalWrite(ledVerde, LOW);
        digitalWrite(buzzerPin, LOW);

        Serial.println("Hora programada: " + h);
      } else {
        Serial.println("ERROR: formato = hora HH:MM");
      }
      return;
    }
  }

  // ------------------ LÓGICA ------------------
  if (hayDosisPendiente) {
    DateTime now = rtc.now();

    if (now.hour() == horaProg && now.minute() == minutoProg) {

      // DISPENSAR = mover servo a 180°
      servo.write(180);

      // LED rojo + buzzer hasta que ponga "tomada"
      digitalWrite(ledRojo, HIGH);
      digitalWrite(ledVerde, LOW);
      digitalWrite(buzzerPin, HIGH);

      hayDosisPendiente = false;

      Serial.println("DISPENSADO");
    }
  }
}