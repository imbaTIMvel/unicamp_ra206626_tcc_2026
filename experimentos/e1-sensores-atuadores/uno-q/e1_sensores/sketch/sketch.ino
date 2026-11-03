const int PIN_LDR = A0;
const int PIN_LED = 9;

void setup() {
  Serial.begin(115200);
  pinMode(PIN_LED, OUTPUT);
  analogReadResolution(12);
}

void loop() {
  int leitura = analogRead(PIN_LDR);
  analogWrite(PIN_LED, map(leitura, 0, 4095, 0, 255));
  Serial.println(leitura);
  delay(50);

  unsigned long t0 = micros();
  for (int k = 0; k < 1000; k++) {
    analogWrite(PIN_LED, analogRead(PIN_LDR) >> 4);
  }
  unsigned long t1 = micros();
  Serial.print("por_iter_ns="); Serial.println((t1 - t0));
}