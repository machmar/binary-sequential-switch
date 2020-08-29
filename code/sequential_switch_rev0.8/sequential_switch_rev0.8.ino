#define ratePot 20
#define attenuverterPot 21
#define jack111Detect 16
#define speedJack 15
#define goStop 14
#define muxINH 13
#define muxC 12
#define muxB 11
#define muxA 10
#define speedJackDetect 9
#define directionSwitch 8
#define jack001 7
#define jack010 6
#define jack100 5
#define led100 4
#define led010 3
#define led001 2

long pMil;
int stpCounterPos;
int stpCounterPosLast;

void mux(uint8_t channel, bool enable) {
  digitalWrite(muxA, bitRead(channel, 0));
  digitalWrite(muxB, bitRead(channel, 1));
  digitalWrite(muxC, bitRead(channel, 2));
  digitalWrite(muxINH, !enable);
  binLed(channel);
}

void binLed(uint8_t pos) {
  digitalWrite(led001, bitRead(pos, 0));
  digitalWrite(led010, bitRead(pos, 1));
  digitalWrite(led100, bitRead(pos, 2));
}

uint16_t fineAdj(uint16_t corse, int fine, uint8_t fineAmount, uint16_t maximum) {
  fine = fine / fineAmount;
  corse = corse + fine;
  if (corse > maximum && corse < 2000) corse = maximum;
  if (corse > 62000) corse = 0;
  return corse;
}

uint16_t inSwitch(uint16_t in1, uint16_t in2, bool select) {
  switch (select) {
    case 0:
      return in1;
      break;
    case 1:
      return in2;
      break;
  }
}

long attenuverter(long main, long att) {
  return (main * 1000 / 512) * (att - 511) / 2000;
}

void ledTest(uint8_t rate) {
  for (int i = 0; i < 11; i++) {
    if (i < 8) mux(i, true);
    if (i >= 8) mux(0, false);
    switch (i) {
      case 8:
        digitalWrite(led100, 1);
        digitalWrite(led010, 0);
        digitalWrite(led001, 0);
        break;
      case 9:
        digitalWrite(led100, 0);
        digitalWrite(led010, 1);
        digitalWrite(led001, 0);
        break;
      case 10:
        digitalWrite(led100, 0);
        digitalWrite(led010, 0);
        digitalWrite(led001, 1);
        break;
      default:
        digitalWrite(led100, 0);
        digitalWrite(led010, 0);
        digitalWrite(led001, 0);
    }
    delay(map(analogRead(ratePot), 0, 1023, 0, 500));
  }
}

void setup() {
  pinMode(led001, OUTPUT);
  pinMode(led010, OUTPUT);
  pinMode(led100, OUTPUT);
  pinMode(muxA, OUTPUT);
  pinMode(muxB, OUTPUT);
  pinMode(muxC, OUTPUT);
  pinMode(muxINH, OUTPUT);
  pinMode(jack001, INPUT);
  pinMode(jack010, INPUT);
  pinMode(jack100, INPUT);
  pinMode(directionSwitch, INPUT);
  pinMode(goStop, INPUT_PULLUP);
  pinMode(speedJackDetect, INPUT_PULLUP);
  pinMode(jack111Detect, INPUT_PULLUP);

  Serial.begin(9600);
}

void loop() {
  long cMil = millis();
  if (cMil - pMil > map(inSwitch(fineAdj(analogRead(ratePot), analogRead(attenuverterPot) - 511, 10, 1023), attenuverter(analogRead(speedJack), analogRead(attenuverterPot)) + 512, !digitalRead(speedJackDetect)), 0, 1023, 1000, 0)) {
    pMil = cMil;
    if (digitalRead(jack111Detect) == 1) {
      switch (digitalRead(directionSwitch)) {
        case 0:
          stpCounterPos --;
          break;
        case 1:
          stpCounterPos ++;
          break;
      }
    }
  }
  if (digitalRead(jack111Detect) == 0) {
    bitWrite(stpCounterPos, 0, digitalRead(jack001));
    bitWrite(stpCounterPos, 1, digitalRead(jack010));
    bitWrite(stpCounterPos, 2, digitalRead(jack100));
  }
  if (digitalRead(goStop) == 0) stpCounterPos = stpCounterPosLast;
  if (stpCounterPosLast != stpCounterPos) {
    if (stpCounterPos > 7) stpCounterPos = 0;
    if (stpCounterPos < 0) stpCounterPos = 7;
    mux(stpCounterPos, true);
  }
  if (digitalRead(goStop) == 1) stpCounterPosLast = stpCounterPos;


}
