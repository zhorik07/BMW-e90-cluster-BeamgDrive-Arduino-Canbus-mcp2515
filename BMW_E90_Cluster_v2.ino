/*
  ================================================================
  BMW E90 Cluster — Arduino UNO + MCP2515 (кристалл 8MHz)
  Библиотека: MCP_CAN by Cory J. Fowler (Library Manager)
  ================================================================

  ПОДКЛЮЧЕНИЕ MCP2515 -> Arduino UNO:
    VCC -> 5V    GND -> GND
    CS  -> D10   SO  -> D12 (MISO)
    SI  -> D11 (MOSI)  SCK -> D13

  ================================================================
  НАСТРОЙКА SIMHUB:
  Controllers and Displays -> Add Arduino -> выбери COM порт
  Скорость: 115200
  Вкладка "Custom Serial" -> включи -> интервал 100мс
  Вставь строку отправки:

  $[SpeedKmh,0];[Rpms,0];[Gear];[TurnIndicatorLeft];[TurnIndicatorRight];[HighBeam];[LowBeam];[HandBrake];[FuelPercent,0];[WaterTemperature,0];[FuelConsumptionPerHour,0]\n

  ================================================================
  РУЧНЫЕ КОМАНДЫ (Serial Monitor, 115200, New Line):
    speed80     — скорость 80 км/ч
    rpm3000     — обороты 3000
    gear3       — передача 3 -> показывает M3 на дисплее
    gearP / gearR / gearN / gearD  — автомат
    left1/left0         — левый поворотник
    right1/right0       — правый поворотник
    hazzard1/hazzard0   — аварийка
    low1/low0           — ближний свет
    high1/high0         — дальний свет
    brake1/brake0       — ручник
    fuel75              — топливо 75%
    temp90              — температура 90C
    cons15              — расход 15 л/ч (стрелка мгновенного расхода)
    check1/check0       — Check Engine вкл/выкл
    abs1/abs0           — лампа ABS вкл/выкл
    time14:35           — время на дисплее
  ================================================================
*/

#include <SPI.h>
#include <mcp_can.h>

#define CAN_CS_PIN 10
MCP_CAN CAN(CAN_CS_PIN);

// ================================================================
//  СОСТОЯНИЕ
// ================================================================
struct {
  uint16_t speed        = 0;
  uint16_t rpm          = 800;
  uint8_t  waterTemp    = 90;
  uint16_t fuel         = 500;    // 0-1000 (% x 10)
  uint16_t fuelConsLph  = 0;      // л/ч x 10

  bool  lightLow        = false;
  bool  lightHigh       = false;
  bool  handbrake       = false;
  bool  checkEngine     = false;
  bool  absWarning      = false;

  uint8_t indicators    = 0;      // 0=выкл 1=лево 2=право 3=аварийка
  bool    blinkerState  = false;

  uint8_t gear          = 0;      // 0=N 1-8=мех 10=P 11=R 12=N 13=D

  uint8_t timeHour      = 12;
  uint8_t timeMinute    = 0;
  uint8_t timeSecond    = 0;
} S;

// ================================================================
//  СЧЁТЧИКИ
// ================================================================
uint8_t  ignCnt      = 0xE2;
uint16_t speedCnt    = 0;
uint16_t tickCnt     = 0;
uint8_t  absFrameCnt = 0xF0;
uint8_t  airbagCnt   = 0xC3;
uint8_t  absNibble   = 0;

uint32_t t50=0, t100=0, t200=0, t500=0, t650=0, t1s=0;

String rxStr  = "";
bool   rxDone = false;

// ================================================================
//  CAN отправка
// ================================================================
void tx(uint32_t id, uint8_t* d) {
  CAN.sendMsgBuf(id, 0, 8, d);
}

// ================================================================
//  0x130  Зажигание (100мс)
// ================================================================
void sendIgnition() {
  uint8_t d[8] = {0x45, 0x42, 0x69, 0x8F, ignCnt++, 0, 0, 0};
  tx(0x130, d);
}

// ================================================================
//  0x0AA  Обороты (50мс)
// ================================================================
void sendRPM() {
  uint16_t v = (uint16_t)S.rpm * 4;
  uint8_t d[8] = {0x5F, 0x59, 0xFF, 0x00,
                  (uint8_t)(v), (uint8_t)(v>>8), 0x80, 0x99};
  tx(0x0AA, d);
}

// ================================================================
//  0x1A6  Скорость (100мс)
// ================================================================
void sendSpeed() {
  speedCnt += (uint32_t)S.speed * 136 / 100;
  tickCnt  += 400;
  uint8_t lo = speedCnt, hi = speedCnt >> 8;
  uint8_t d[8] = {lo, hi, lo, hi, lo, hi,
                  (uint8_t)tickCnt,
                  (uint8_t)(0xF0 | ((tickCnt>>8) & 0x0F))};
  tx(0x1A6, d);
}

// ================================================================
//  0x1D0  Температура + стрелка расхода (100мс)
//
//  Стрелка расхода = bytes[4:5] = накопительный счётчик впрыска.
//  Формула: л/ч -> мкл/100мс = л/ч * 27.78
//  Калибровка x28 (подобрана под E90 из veikkos проекта)
// ================================================================
void sendEngineTempAndInjection() {
  static uint8_t  alive    = 0;
  static uint32_t injAccum = 0;
  alive = (alive + 1) & 0x0F;

  uint8_t t = (uint8_t)constrain((int)S.waterTemp + 48, 0, 255);

  // fuelConsLph хранится x10, поэтому делим на 10
  // injAccum += (л/ч x 10) * 28 / 10 = л/ч * 28
  injAccum += (uint32_t)S.fuelConsLph * 28 / 10;

  uint8_t d[8] = {
    t, 0xFF,
    (uint8_t)(0x20 | alive),
    0xCD,
    (uint8_t)(injAccum & 0xFF),
    (uint8_t)(injAccum >> 8),
    0xCD, 0xA8
  };
  tx(0x1D0, d);
}

// ================================================================
//  0x349  Топливо — нелинейная таблица E90 (200мс)
// ================================================================
struct FP { float p; uint16_t L; uint16_t R; };
const FP FT[] = {
  {1.000f,9700,8400},{0.875f,8200,5400},{0.750f,6250,4600},
  {0.500f,3600,3350},{0.250f,1950,2200},{0.000f, 625, 950}
};
uint16_t fuelInterp(float p, bool left) {
  for (int i = 0; i < 5; i++) {
    if (p >= FT[i+1].p) {
      float a = (p - FT[i+1].p) / (FT[i].p - FT[i+1].p);
      uint16_t lo = left ? FT[i+1].L : FT[i+1].R;
      uint16_t hi = left ? FT[i].L   : FT[i].R;
      return (uint16_t)(lo + a*(hi-lo));
    }
  }
  return left ? FT[5].L : FT[5].R;
}
void sendFuel() {
  float p = constrain(S.fuel/1000.0f, 0.0f, 1.0f);
  uint16_t L = fuelInterp(p,true), R = fuelInterp(p,false);
  uint8_t d[8] = {(uint8_t)L,(uint8_t)(L>>8),(uint8_t)R,(uint8_t)(R>>8),0,0,0,0};
  tx(0x349, d);
}

// ================================================================
//  0x21A  Свет (200мс)
// ================================================================
// ================================================================
//  0x21A  Свет (200мс)
//  Подсветка и ближний — только когда зажигание включено (ignition=2)
// ================================================================
void sendLights() {
  uint8_t b = 0;
  if (S.lightLow) {
    b |= 0x01; // подсветка приборки
    b |= 0x04; // ближний свет
  }
  if (S.lightHigh) b |= 0x02;
  uint8_t d[8] = {b, 0x00, 0xF7, 0,0,0,0,0};
  tx(0x21A, d);
}

// ================================================================
//  0x1F6  Поворотники
//  Arduino сам мигает каждые 500мс — приборка BMW так ожидает
//  SimHub управляет только тем КАКОЙ поворотник включён
// ================================================================
void sendIndicators() {
  static uint32_t lastBlink = 0;
  static bool blinkOn = false;
  uint32_t now = millis();

  if (S.indicators == 0) {
    // Выключены — сбрасываем состояние
    blinkOn = false;
    uint8_t d[8] = {0x80, 0xF0, 0,0,0,0,0,0};
    tx(0x1F6, d);
    return;
  }

  // Мигаем каждые 500мс
  if (now - lastBlink >= 500) {
    lastBlink = now;
    blinkOn = !blinkOn;
  }

  uint8_t b0;
  switch(S.indicators) {
    case 1: b0=0x91; break; // лево
    case 2: b0=0xA1; break; // право
    case 3: b0=0xB1; break; // аварийка
    default: b0=0x80;
  }
  uint8_t b1 = blinkOn ? 0xF2 : 0xF1;
  uint8_t d[8] = {b0, b1, 0,0,0,0,0,0};
  tx(0x1F6, d);
}

// ================================================================
//  0x34F  Ручник (200мс)
// ================================================================
void sendHandbrake() {
  uint8_t d[8] = {S.handbrake?(uint8_t)0xFE:(uint8_t)0xFD, 0xFF,0,0,0,0,0,0};
  tx(0x34F, d);
}

// ================================================================
//  0x1D2  Передача (100мс)
//
//  Механика: показываем как M1..M6 (byte4=0xF2 = режим M)
//  Это работает на ВСЕХ кластерах E90 включая бензиновые механику.
//  Автомат: P/R/N/D через byte0 + byte4=0xF0
// ================================================================
void sendGear() {
  static uint8_t cH = 0;
  uint8_t b0=0xB4, b1=0x0F, b3=0x0D, b4=0xF2;

  if (S.gear >= 10) {
    switch(S.gear) {
      case 10: b0=0xE1; break;
      case 11: b0=0xD2; break;
      case 12: b0=0xB4; break;
      case 13: b0=0x78; break;
    }
    b4 = 0xF0;
    b3 = (S.gear==10||S.gear==11) ? 0x0C : 0x0D;
  } else if (S.gear >= 1 && S.gear <= 8) {
    const uint8_t gb[] = {0x50,0x60,0x70,0x80,0x90,0xA0,0xB0,0xC0};
    b1 = gb[S.gear - 1];
    b3 = 0x0D;
    b4 = 0xF2;
  }
  b3 |= (cH << 4);
  cH = (cH + 1) % 16;

  uint8_t d[8] = {b0, b1, 0xFF, b3, b4, 0xFF, 0xFF, 0xFF};
  tx(0x1D2, d);
}

// ================================================================
//  0x39E  Время (1с)
// ================================================================
void sendTime() {
  uint8_t d[8] = {S.timeHour, S.timeMinute, S.timeSecond,
                  0x15, 0x3F, 0xE9, 0x07, 0xF2};
  tx(0x39E, d);
}

// ================================================================
//  0x12F  DME (100мс)
// ================================================================
void sendDME() {
  uint8_t d[8] = {0x3F,0x00,0x00,0x00,0x00,0x40,0x01,0x00};
  tx(0x12F, d);
}

// ================================================================
//  0x19E  ABS фрейм (200мс)
// ================================================================
void sendABS() {
  absNibble = (absNibble+1) % 3;
  uint8_t d[8] = {0x00,0xE0,(uint8_t)((absNibble<<4)|0x03),0xFC,0xCC,0,0,0};
  tx(0x19E, d);
}

// ================================================================
//  0x0C0  ABS счётчик (200мс)
// ================================================================
void sendABSCounter() {
  uint8_t d[8] = {absFrameCnt, 0xFF, 0,0,0,0,0,0};
  tx(0x0C0, d);
  if (++absFrameCnt == 0x00) absFrameCnt = 0xF0;
}

// ================================================================
//  0x0D7  Airbag счётчик (200мс)
// ================================================================
void sendAirbagCounter() {
  uint8_t d[8] = {airbagCnt++, 0xFF, 0,0,0,0,0,0};
  tx(0x0D7, d);
}

// ================================================================
//  0x26E  Зажигание статус (200мс)
// ================================================================
void sendIgnitionStatus() {
  uint8_t d[8] = {0x40,0x40,0x7F,0x50,0xFF,0xFF,0xFF,0xFF};
  tx(0x26E, d);
}

// ================================================================
//  0x592  Лампы предупреждений (500мс)
//
//  Коды символов E90:
//  0x0023 = ABS
//  0x0024 = Airbag/SRS
//  0x000A = Рулевое
//  0x0035 = Check Engine
// ================================================================
void sendErrorLight(uint16_t code, bool on) {
  uint8_t d[8] = {0x40,
                  (uint8_t)(code&0xFF), (uint8_t)(code>>8),
                  on?(uint8_t)0x31:(uint8_t)0x30,
                  0xFF,0xFF,0xFF,0xFF};
  tx(0x592, d);
}

void sendWarningLights() {
  sendErrorLight(0x0024, false);   // Airbag — гасим
  delay(3);
  sendErrorLight(0x000A, false);   // Рулевое — гасим
  delay(3);
  sendErrorLight(0x0023, false);   // ABS — всегда гасим
  delay(3);
  sendErrorLight(0x0035, S.checkEngine);  // Check Engine
  delay(3);
  sendErrorLight(0x00A4, false);   // Жёлтый треугольник — гасим
  delay(3);
  sendErrorLight(0x00A5, false);   // Красный треугольник — гасим
}

// ================================================================
//  0x0C1  SOS (500мс)
// ================================================================
void suppressSOS() {
  uint8_t d[8] = {(uint8_t)random(255),0xFF,0,0,0,0,0,0};
  tx(0x0C1, d);
}

// ================================================================
//  ПАРСЕР
// ================================================================
void parseSerial() {
  if (!rxDone) return;
  rxStr.trim();

  if (rxStr.startsWith("$")) {
    // Протокол (19 полей):
    // $speed;rpm;oilTemp;fuel*10;gear;waterTemp;ignition;engineOn;blinkers;handbrake;abs;tc;fuelCons;year;month;day;hour;min;sec
    String s = rxStr.substring(1);
    String tok[19];
    int n = 0;
    while (s.length() > 0 && n < 19) {
      int idx = s.indexOf(';');
      if (idx < 0) { tok[n++]=s; break; }
      tok[n++] = s.substring(0, idx);
      s = s.substring(idx+1);
    }

    if (n>0)  S.speed        = tok[0].toInt();
    if (n>1)  S.rpm          = tok[1].toInt();
    // tok[2] = oilTemp (не используем)
    if (n>3)  S.fuel         = tok[3].toInt(); // уже x10 из SimHub
    if (n>4) {
      String g = tok[4];
      if      (g=="P"||g=="p") S.gear=10;
      else if (g=="R"||g=="r") S.gear=11;
      else if (g=="N"||g=="n") S.gear=12;
      else if (g=="D"||g=="d") S.gear=13;
      else                     S.gear=g.toInt();
    }
    if (n>5)  S.waterTemp    = tok[5].toInt();
    // tok[6] = ignition (2=on), tok[7] = engineRunning
    if (n>6)  S.lightLow     = (tok[6].toInt() >= 2); // зажигание >= 2 = подсветка вкл
    if (n>8) {
      // поворотники: 0=выкл 1=лево 2=право 3=аварийка
      int blink = tok[8].toInt();
      S.indicators = blink; // уже в нужном формате!
    }
    if (n>9)  S.handbrake    = (tok[9].toInt() == 1);
    if (n>10) S.absWarning   = (tok[10].toInt() == 1);
    // tok[11] = TC (не используем пока)
    if (n>12) S.fuelConsLph  = (uint16_t)(tok[12].toFloat() * 10);
    // Время из игры
    if (n>16) S.timeHour     = tok[16].toInt();
    if (n>17) S.timeMinute   = tok[17].toInt();
    if (n>18) S.timeSecond   = tok[18].toInt();

  } else {
    if      (rxStr.startsWith("speed"))    S.speed       = rxStr.substring(5).toInt();
    else if (rxStr.startsWith("rpm"))      S.rpm         = rxStr.substring(3).toInt();
    else if (rxStr.startsWith("fuel"))     S.fuel        = rxStr.substring(4).toInt()*10;
    else if (rxStr.startsWith("temp"))     S.waterTemp   = rxStr.substring(4).toInt();
    else if (rxStr.startsWith("cons"))     S.fuelConsLph = rxStr.substring(4).toInt()*10;
    else if (rxStr.startsWith("brake"))    S.handbrake   = (rxStr.charAt(5)=='1');
    else if (rxStr.startsWith("low"))      S.lightLow    = (rxStr.charAt(3)=='1');
    else if (rxStr.startsWith("high"))     S.lightHigh   = (rxStr.charAt(4)=='1');
    else if (rxStr.startsWith("left"))     S.indicators  = (rxStr.charAt(4)=='1')?1:0;
    else if (rxStr.startsWith("right"))    S.indicators  = (rxStr.charAt(5)=='1')?2:0;
    else if (rxStr.startsWith("hazzard")) S.indicators  = (rxStr.charAt(7)=='1')?3:0;
    else if (rxStr.startsWith("check"))    S.checkEngine = (rxStr.charAt(5)=='1');
    else if (rxStr.startsWith("abs"))      S.absWarning  = (rxStr.charAt(3)=='1');
    else if (rxStr.startsWith("gear")) {
      String g=rxStr.substring(4);
      if      (g=="P") S.gear=10;
      else if (g=="R") S.gear=11;
      else if (g=="N") S.gear=12;
      else if (g=="D") S.gear=13;
      else             S.gear=g.toInt();
    }
    else if (rxStr.startsWith("time")) {
      String t=rxStr.substring(4);
      int c=t.indexOf(':');
      if (c>=0) { S.timeHour=t.substring(0,c).toInt(); S.timeMinute=t.substring(c+1).toInt(); }
    }
  }
  rxStr=""; rxDone=false;
}

void serialEvent() {
  while (Serial.available()) {
    char c=(char)Serial.read();
    if (c=='\n') rxDone=true;
    else rxStr+=c;
  }
}

// ================================================================
//  SETUP
// ================================================================
void setup() {
  Serial.begin(115200);
  rxStr.reserve(80);
  randomSeed(analogRead(A0));

  while (CAN_OK != CAN.begin(MCP_STDEXT, CAN_100KBPS, MCP_8MHZ)) {
    Serial.println("CAN init failed...");
    delay(200);
  }
  CAN.setMode(MCP_NORMAL);
  Serial.println("=== BMW E90 Cluster READY ===");
  uint32_t now=millis();
  t50=t100=t200=t500=t650=t1s=now;
}

// ================================================================
//  LOOP
// ================================================================
void loop() {
  parseSerial();
  uint32_t now=millis();

  if (now-t50  >= 50)  { t50=now;  sendRPM(); }

  if (now-t100 >= 100) { t100=now;
    sendIgnition();
    sendSpeed();
    sendGear();
    sendDME();
    sendEngineTempAndInjection();
  }

  if (now-t200 >= 200) { t200=now;
    sendFuel();
    sendABS();
    sendABSCounter();
    sendAirbagCounter();
    sendHandbrake();
    sendLights();
    sendIgnitionStatus();
    sendIndicators(); // таймер мигания внутри функции
  }

  if (now-t500 >= 500) { t500=now;
    sendWarningLights();
    suppressSOS();
  }

  if (now-t1s  >= 1000) { t1s=now;
    if (++S.timeSecond>=60) { S.timeSecond=0;
      if (++S.timeMinute>=60) { S.timeMinute=0; S.timeHour=(S.timeHour+1)%24; }
    }
    sendTime();
  }
}
