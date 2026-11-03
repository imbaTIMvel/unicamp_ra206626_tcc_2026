#include <Wire.h>

const uint8_t ENDERECO = 0x68;
const int N_ITER = 1000;
const int N_BLOCOS = 10;

bool lerEixos(int16_t *v) {
  Wire.beginTransmission(ENDERECO);
  Wire.write(0x3B);
  Wire.endTransmission();
  if (Wire.requestFrom(ENDERECO, (uint8_t)6) != 6) return false;
  for (int i = 0; i < 3; i++) {
    uint8_t hi = Wire.read();
    uint8_t lo = Wire.read();
    v[i] = (int16_t)((hi << 8) | lo);
  }
  return true;
}

void setup() {
  Serial.begin(115200);
  while (!Serial) { }
  delay(500);

  Wire.begin();
  Wire.setClock(400000);

  Wire.beginTransmission(ENDERECO);
  Wire.write(0x75);
  Wire.endTransmission();
  Wire.requestFrom(ENDERECO, (uint8_t)1);
  Serial.print("WHO_AM_I=0x"); Serial.println(Wire.read(), HEX);

  Wire.beginTransmission(ENDERECO);
  Wire.write(0x6B); Wire.write(0x00);
  Wire.endTransmission();
  delay(100);

  int16_t v[3];
  Serial.println("=== FUNCIONAL: 20 leituras, mexa no modulo ===");
  for (int i = 0; i < 20; i++) {
    bool ok = lerEixos(v);
    Serial.print(i); Serial.print(',');
    Serial.print(ok ? "OK" : "FALHA"); Serial.print(',');
    Serial.print(v[0]); Serial.print(',');
    Serial.print(v[1]); Serial.print(',');
    Serial.println(v[2]);
    delay(250);
  }

  Serial.println("=== LATENCIA: 10 blocos de 1000 iteracoes ===");
  for (int b = 0; b < N_BLOCOS; b++) {
    unsigned long t0 = micros();
    for (int k = 0; k < N_ITER; k++) lerEixos(v);
    unsigned long dt = micros() - t0;
    Serial.print(b); Serial.print(",por_iter_ns="); Serial.println(dt);
  }
  Serial.println("=== FIM ===");
}

void loop() { }