# 🏎️ BMW E90 Cluster → BeamNG.drive

> **Оживи реальную приборную панель BMW E90 и подключи её к игре BeamNG.drive через Arduino и CAN-шину.**

![Status](https://img.shields.io/badge/status-working-brightgreen)
![Platform](https://img.shields.io/badge/platform-Arduino%20UNO-blue)
![Game](https://img.shields.io/badge/game-BeamNG.drive-orange)

---

## 🇷🇺 Русский | [🇬🇧 English below](#-english-version)

---

## 📋 Что работает

| Функция | Статус |
|---|---|
| Спидометр | ✅ Плавно, без рывков |
| Тахометр | ✅ Работает |
| Уровень топлива | ✅ Нелинейная калибровка E90 |
| Мгновенный расход | ✅ Стрелка под тахометром |
| Поворотники | ✅ Лево / Право / Аварийка |
| Подсветка приборки | ✅ Включается при запуске двигателя |
| Время на дисплее | ✅ Берётся с компьютера |

---

## 🛠️ Что понадобится

### Железо

| Компонент | Комментарий |
|---|---|
| **Arduino UNO** | Основа проекта |
| **CAN-модуль MCP2515** | Кристалл **8 MHz** — обязательно! |
| **Блок питания 12В, от 2А** | Приборка потребляет много, рекомендую 5А |
| **Провода Dupont** | Мама-Папа, Папа-Папа |
| **Паяльник и термоусадка** | Для надёжных соединений (рекомендуется) |

### Программы

- [Arduino IDE](https://www.arduino.cc/en/software)
- [SimHub](https://www.simhubdash.com/)
- Библиотека **MCP_CAN by Cory J. Fowler** (через Library Manager в Arduino IDE)

---

## ⚡ Важные моменты перед сборкой

> ⚠️ **Общая земля (GND)** — обязательно соедини минус от блока питания 12В с GND на Arduino, GND на приборке и GND на CAN-модуле. Без этого данные превратятся в мусор и ничего не заработает.

> 💡 **Витая пара** — провода CAN-High и CAN-Low лучше свить между собой. Это снижает электромагнитные помехи и делает соединение стабильнее.

---

## 🔌 Схема подключения

Схема взята из проекта [Adam-Sidor/CAN_Cluster](https://github.com/Adam-Sidor/CAN_Cluster?tab=readme-ov-file) — спасибо автору!

> Главное правило: провода должны **надёжно сидеть** в разъёмах. Даже слегка болтающийся контакт может всё сломать.

<img width="1394" height="697" alt="image" src="https://github.com/user-attachments/assets/41a31809-7277-4b87-857e-a6cb3736e6c6" />

---

## 💾 Программная часть

### 1. Arduino

1. Скачай и установи **Arduino IDE**
2. Открой Library Manager (`Sketch → Include Library → Manage Libraries`)
3. Найди и установи **MCP_CAN by Cory J. Fowler**
4. Открой файл `BMW_E90_Cluster_v2.ino` и загрузи его на Arduino

**Проверка:** подключи питание 12В к приборке — она должна засветиться и стрелки должны выполнить приветственный sweep. Если горит только красный значок автомобиля на домкрате — проверь все провода и соединения.

### 2. SimHub

1. Скачай и открой **SimHub**
2. Перейди в раздел `Custom Serial Devices`
3. Нажми `Add new serial device`
4. Настрой параметры:
   - **Serial port:** твой COM порт (Arduino)
   - **Baudrate:** `115200`
   - **Enable RTS:** ✅
5. В разделе **Update messages** нажми `Edit` и вставь формулу из файла `UpdateMessage_SimHub.txt`
6. Выставь частоту **10 Hz**
7. Включи устройство (тумблер `Enabled`)

**Проверка:** запусти BeamNG.drive, сядь за руль — под строкой формулы должны побежать числа. Это значит, что игра передаёт данные и приборка оживёт.

---

## 🎮 Ручное управление (без SimHub)

Открой Serial Monitor в Arduino IDE (скорость 115200, New Line) и вводи команды:

```
speed80       → скорость 80 км/ч
rpm3000       → обороты 3000
gear3         → передача 3 (показывает M3)
gearP / gearR / gearN / gearD  → автомат
left1 / left0    → левый поворотник
right1 / right0  → правый поворотник
hazzard1 / hazzard0  → аварийка
low1 / low0   → ближний свет
high1 / high0 → дальний свет
brake1 / brake0  → ручник
fuel75        → топливо 75%
temp90        → температура 90°C
check1 / check0  → Check Engine
time14:35     → время на дисплее
```

---

## 🤝 Помощь проекту

Код не идеален — если ты хорошо разбираешься в C++ или CAN-протоколах, буду рад любому фидбеку и Pull Request. Давай сделаем идеальный гайд вместе!

📬 Контакты
По вопросам, предложениям и просто пообщаться:

📧 Email: khizhniakgeorgij07@gmail.com
✈️ Telegram: @kkhizhniakkk


---
---

## 🇬🇧 English Version

> **Bring a real BMW E90 instrument cluster to life and connect it to BeamNG.drive using Arduino and CAN bus.**

---

## 📋 What works

| Feature | Status |
|---|---|
| Speedometer | ✅ Smooth, no jumps |
| Tachometer | ✅ Working |
| Fuel level | ✅ Non-linear E90 calibration |
| Instant fuel consumption | ✅ Needle below tachometer |
| Turn signals | ✅ Left / Right / Hazard |
| Instrument backlight | ✅ Turns on when engine starts |
| Clock display | ✅ Synced from PC |

---

## 🛠️ What you need

### Hardware

| Component | Notes |
|---|---|
| **Arduino UNO** | The brain of the project |
| **MCP2515 CAN module** | Must have **8 MHz** crystal |
| **12V power supply, 2A minimum** | Cluster draws a lot — 5A recommended |
| **Dupont wires** | Male-Female, Male-Male |
| **Soldering iron & heat shrink** | For reliable connections (recommended) |

### Software

- [Arduino IDE](https://www.arduino.cc/en/software)
- [SimHub](https://www.simhubdash.com/)
- Library: **MCP_CAN by Cory J. Fowler** (via Library Manager in Arduino IDE)

---

## ⚡ Important before you start

> ⚠️ **Common GND** — you MUST connect the negative (GND) from the 12V power supply to GND on the Arduino, GND on the cluster, and GND on the CAN module. Without this, data will be garbage and nothing will work.

> 💡 **Twisted pair** — twist the CAN-High and CAN-Low wires together. This reduces electromagnetic interference and improves stability.

---

## 🔌 Wiring

Wiring diagram is borrowed from [Adam-Sidor/CAN_Cluster](https://github.com/Adam-Sidor/CAN_Cluster?tab=readme-ov-file) — credits to the author!

> Key rule: all wires must sit **firmly** in their connectors. Even a slightly loose contact can break everything.

<img width="1394" height="697" alt="image" src="https://github.com/user-attachments/assets/eeac772b-c74f-44cb-a684-2be96cacb73d" />

---

## 💾 Software setup

### 1. Arduino

1. Download and install **Arduino IDE**
2. Open Library Manager (`Sketch → Include Library → Manage Libraries`)
3. Find and install **MCP_CAN by Cory J. Fowler**
4. Open `BMW_E90_Cluster_v2.ino` and upload it to your Arduino

**Test:** connect 12V power to the cluster — it should light up and needles should do a welcome sweep. If only the red car-on-jack icon is showing — check all your wires and connections.

### 2. SimHub

1. Download and open **SimHub**
2. Go to `Custom Serial Devices`
3. Click `Add new serial device`
4. Configure:
   - **Serial port:** your Arduino COM port
   - **Baudrate:** `115200`
   - **Enable RTS:** ✅
5. In **Update messages** click `Edit` and paste the formula from `UpdateMessage_SimHub.txt`
6. Set frequency to **10 Hz**
7. Enable the device (toggle `Enabled`)

**Test:** launch BeamNG.drive and get behind the wheel — you should see numbers running below the formula field. This means the game is sending data and the cluster will come alive.

---

## 🎮 Manual control (without SimHub)

Open Serial Monitor in Arduino IDE (115200 baud, New Line) and type commands:

```
speed80       → set speed to 80 km/h
rpm3000       → set RPM to 3000
gear3         → gear 3 (shows M3 on display)
gearP / gearR / gearN / gearD  → automatic gearbox
left1 / left0    → left turn signal
right1 / right0  → right turn signal
hazzard1 / hazzard0  → hazard lights
low1 / low0   → low beam
high1 / high0 → high beam
brake1 / brake0  → handbrake
fuel75        → fuel level 75%
temp90        → engine temp 90°C
check1 / check0  → Check Engine light
time14:35     → set clock display
```

---

## 🤝 Contributing

The code isn't perfect — if you know C++ or CAN protocols well, feedback and Pull Requests are very welcome. Let's build the best guide together!

📬 Contacts
Questions, suggestions or just want to chat:

📧 Email: khizhniakgeorgij07@gmail.com
✈️ Telegram: @kkhizhniakkk
