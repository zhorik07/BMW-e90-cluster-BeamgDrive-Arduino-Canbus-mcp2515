# 🏎️ BMW E90 Cluster to BeamNG.drive 🏁
### Проект по оживлению реальной приборной панели BMW E90 через Arduino и CAN-шину.
### Project for connecting a real BMW E90 instrument cluster to BeamNG.drive via Arduino and CAN bus.

---

## 🇷🇺 Инструкция на русском

Всем привет! Этот репозиторий — подробный гайд «от А до Я» по подключению приборки BMW E90 к игре **BeamNG.drive**. 

> **Цель проекта:** сделать максимально простой и понятный гайд, чтобы даже новичок мог собрать свой симрейсинг-кокпит без головной боли.

### 🛠 Что понадобится (Железо)

1. **Arduino UNO** — «мозги» проекта.
   <img width="400" src="https://github.com/user-attachments/assets/7158eb8e-e9a7-48a0-8a68-2824ac90451c" />
2. **CAN BUS Модуль (MCP2515)** — наш переводчик для общения с приборкой.
   <img width="300" src="https://github.com/user-attachments/assets/47d8c46a-92cb-45c9-8b2b-903fb16e7274" />
3. **Блок питания (12В, минимум 2А)** — приборка прожорливая, USB её не потянет.
   <img width="400" src="https://github.com/user-attachments/assets/4de35bf2-7811-4cef-ab32-aaddd08a6b0f" />
4. **Провода Dupont** (Мама-Папа, Папа-Папа).

### ⚠️ Секреты стабильной работы
* **Общая Земля (GND):** Обязательно соедини минус блока питания, GND Ардуины и GND приборки в одну точку. Иначе данные превратятся в «мусор».
* **Витая пара (CAN-косичка):** Провода CAN-High и CAN-Low нужно свить между собой (как косичку). Это критически важно для защиты от помех.

### 🔌 Схема подключения
За основу взята проверенная схема от [Adam-Sidor](https://github.com/Adam-Sidor/CAN_Cluster). 

<img width="800" src="https://github.com/user-attachments/assets/f008cdcb-656e-4742-a96f-c7bc1a77f513" />

| Пин приборки | Функция | Куда подключаем |
| :--- | :--- | :--- |
| **Пин 9** | +12V | Плюс блока питания |
| **Пин 18** | GND | Минус БП + GND Ардуины |
| **Пин 1** | CAN-High | Пин H на MCP2515 |
| **Пин 2** | CAN-Low | Пин L на MCP2515 |

### 💻 Программная часть
1. Установите **Arduino IDE**.
2. Установите библиотеку **MCP_CAN by Cory J. Fowler** через Library Manager.
3. Залейте скетч `BMW_E90_Cluster_v2.ino` на Ардуину.
4. После подачи питания приборка должна «ожить» (подсветка и самодиагностика).
   <img width="400" src="https://github.com/user-attachments/assets/81f96a73-bc2b-4fc1-82c7-15524ec6997b" />

> **Если горит красный подъемник:** проверьте все соединения и убедитесь, что CAN-H и CAN-L не перепутаны местами.

### 🎮 Настройка SimHub
1. Откройте **Custom Serial Devices**.
2. Настройте: **Baudrate 115200**, частота **20 Hz** (или 10 Hz для стабильности).
3. В разделе **Update Message** вставьте формулу из файла `Update_Message_Simhub.txt`.
<img width="800" src="https://github.com/user-attachments/assets/c3668ddd-f77b-4f29-a8f1-bfa285c7fa2e" />

---

## 🇺🇸 English Instructions

Welcome! This is a step-by-step guide on how to connect a BMW E90 instrument cluster to **BeamNG.drive**.

### 🛠 Hardware Requirements
* **Arduino UNO**
* **CAN BUS Module (MCP2515)**
* **Power Supply (12V, 2A min)**
* **Dupont Wires**

### ⚠️ Pro Tips for Stability
* **Common Ground (GND):** Connect the power supply minus, Arduino GND, and Cluster GND together.
* **Twisted Pair:** Twist the CAN-High and CAN-Low wires together to prevent signal interference.

### 🔌 Wiring Diagram
| Cluster Pin | Function | Connection |
| :--- | :--- | :--- |
| **Pin 9** | +12V | Power Supply (+) |
| **Pin 18** | GND | PS (-) + Arduino GND |
| **Pin 1** | CAN-High | MCP2515 Pin H |
| **Pin 2** | CAN-Low | MCP2515 Pin L |

### 🚀 Features / Что работает:
* ✅ **Speedometer & Tachometer** (Smooth movement)
* ✅ **Fuel Level**
* ✅ **Instant Consumption** (The needle under the tachometer)
* ✅ **Turn Signals**
* ✅ **Dynamic Backlight** (Syncs with engine status)

---

## 🤝 Support the Project
My code isn't perfect. If you are good with C++ or CAN protocols, feel free to contribute via Pull Requests! Let's make the ultimate guide together.

