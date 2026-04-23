// lab07_pwm_servo_dimmer.ino
#include <Servo.h>

const int PIN_LED = 9;    // pin PWM para LED
const int PIN_SERVO = 10; // pin PWM para servo
const int PIN_POT = A0;   // entrada analógica del potenciómetro

Servo miServo;
bool modoAuto = true;
String bufferCmd = "";

// Prototipos
void procesarComando(String cmd);

void setup() {
    Serial.begin(115200);
    pinMode(PIN_LED, OUTPUT);
    miServo.attach(PIN_SERVO);
    miServo.write(90); // posición inicial
    Serial.println("Lab 07 - PWM + Servo + ADC");
    Serial.println("Comandos: D:0-255 | S:0-180 | AUTO | POT");
}

void loop() {
    // Leer comandos seriales
    while (Serial.available()) {
        char c = Serial.read();
        if (c == '\n') { 
            procesarComando(bufferCmd); 
            bufferCmd = ""; 
        } else if (c != '\r') {
            bufferCmd += c;
        }
    }

    if (modoAuto) {
        // Demo automática: rampa de brillo y movimiento de servo
        static int val = 0;
        static int dir = 1;
        static unsigned long ultimo = 0;
        if (millis() - ultimo > 10) {
            ultimo = millis();
            val += dir * 2;
            if (val > 255) { val = 255; dir = -1; }
            if (val <= 0) { val = 0; dir = 1; }
            analogWrite(PIN_LED, val);
            int grados = map(val, 0, 255, 0, 180);
            miServo.write(grados);
        }
    } else {
        // Modo manual: potenciómetro controla servo y LED
        int lecturaPot = analogRead(PIN_POT); // 0-1023
        int grados = map(lecturaPot, 0, 1023, 0, 180);
        int brillo = map(lecturaPot, 0, 1023, 0, 255);
        miServo.write(grados);
        analogWrite(PIN_LED, brillo);

        static unsigned long ultimoReport = 0;
        if (millis() - ultimoReport > 500) {
            ultimoReport = millis();
            float voltaje = lecturaPot * (5.0 / 1023.0);
            Serial.print("ADC="); Serial.print(lecturaPot);
            Serial.print(" V="); Serial.print(voltaje, 2);
            Serial.print(" Servo="); Serial.print(grados);
            Serial.print("deg LED="); Serial.println(brillo);
        }
    }
}

void procesarComando(String cmd) {
    cmd.trim();
    if (cmd == "AUTO") { 
        modoAuto = true; 
        Serial.println("[OK] Modo automático"); 
    }
    else if (cmd == "POT") { 
        modoAuto = false; 
        Serial.println("[OK] Modo potenciómetro"); 
    }
    else if (cmd.startsWith("D:")) {
        int v = cmd.substring(2).toInt();
        v = constrain(v, 0, 255);
        analogWrite(PIN_LED, v);
        modoAuto = false;
        Serial.print("[OK] LED duty="); Serial.println(v);
    }
    else if (cmd.startsWith("S:")) {
        int g = cmd.substring(2).toInt();
        g = constrain(g, 0, 180);
        miServo.write(g);
        modoAuto = false;
        Serial.print("[OK] Servo="); Serial.print(g); Serial.println(" grados");
    }
}
