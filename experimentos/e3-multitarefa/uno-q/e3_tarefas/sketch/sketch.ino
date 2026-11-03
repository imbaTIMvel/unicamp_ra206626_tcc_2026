const int PIN_LED = 9;
const unsigned long PERIODO_US = 10000;
const int N = 500;

unsigned long desvios[N];

void trabalhoInutil() {
  volatile uint32_t x = 1;
  for (int i = 0; i < 2000; i++) x = x * 1103515245 + 12345;
}

int cmp(const void *a, const void *b) {
  unsigned long x = *(unsigned long*)a, y = *(unsigned long*)b;
  return (x > y) - (x < y);
}

void medir(const char *rotulo, bool comCarga) {
  unsigned long prox = micros() + PERIODO_US;
  for (int i = 0; i < N; i++) {
    while ((long)(micros() - prox) < 0) {
      if (comCarga) trabalhoInutil();
    }
    unsigned long real = micros();
    digitalWrite(PIN_LED, !digitalRead(PIN_LED));
    desvios[i] = real - prox;
    prox += PERIODO_US;
  }
  qsort(desvios, N, sizeof(unsigned long), cmp);
  Serial.print(rotulo);
  Serial.print(": mediana="); Serial.print(desvios[N/2]);
  Serial.print(" us  p95="); Serial.print(desvios[(int)(N*0.95)]);
  Serial.print(" us  max="); Serial.println(desvios[N-1]);
}

void setup() {
  Serial.begin(115200);
  delay(5000);
  pinMode(PIN_LED, OUTPUT);

  Serial.println("=== A: ocioso ===");
  medir("ocioso", false);

  Serial.println("=== C: carga no MESMO nucleo ===");
  medir("carga_core0", true);

  Serial.println("=== B: carga em nucleo separado — NAO APLICAVEL ===");
  Serial.println("o nucleo Arduino nao expoe criacao de threads");
  Serial.println("=== FIM ===");
}

void loop() { }