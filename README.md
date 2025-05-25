# DIY Compressor Pressure Monitor 🎛️

A beginner-friendly, configurable pressure monitoring system using Kypruino UNO+ board (or Arduino Uno R3) with real-time readings, 5-minute statistics, visual feedback and alarms with settings menu.

---

## ❓ Why Build Your Own Pressure Monitor?

Whether you’re a weekend DIYer, a small workshop owner or running a commercial service—knowing your system’s pressure at a glance pays off:

- 🏠 **Home & Hobby Use**  
  - **Pneumatic tools** (nail guns, impact wrenches, spray guns): keep exactly 6.3 bar under load to avoid mis-fires, paint runs or tool wear  
  - **Airbrush artists**: maintain steady 1–2 bar for flawless finishes and less overspray  
  - **Aquarium & irrigation**: monitor water line pressure to protect pumps and avoid leaks  

- ⚙️ **Professional & Industrial**  
  - **Auto-service shops**: verify consistent tire-inflation pressure or pneumatic lifts, reduce customer comebacks  
  - **HVAC & heating systems**: track antifreeze (tосол) or hot-water loop pressure to prevent bursts and optimize energy use  
  - **Oil & gas applications**: safeguard low-pressure piping, detect slow leaks early and avoid costly downtime  

- 🔒 **Safety & Savings**  
  - Sound alarms on under- or over-pressure keep your equipment—and you—safe  
  - 5-minute stats reveal hidden dips in supply that cause poor tool performance or wasted energy  
  - Color LEDs give instant “OK/not-OK” feedback from across the garage or plant  

Build it once, customize it forever! This DIY monitor adapts to any sensor and any medium—air, water, oil or gas—so you’ll always know that your system is running at just the right pressure.  

---

## 📋 Features

- 📺 **Screen based interface** including settings menu
- 🕑 **2 Hz instant reading** in bar  
- 📊 **5-minute stats**: minimum, average & maximum  
- 🔔 **Custom alarm thresholds** with buzzer alerts  
- 🌈 **Color LEDs**: blue → yellow → red for 0 → 4 → 8 bar  
- 🔄 **Optimized OLED updates**—only redraw changed regions  
- 💾 **EEPROM-friendly**—writes thresholds only on change  

---

## 📝 Bill of Materials

| Item                                    | Qty | Notes        |
|-----------------------------------------|:---:|--------------|
| Enclosure                               | 1   | [URL]        |
| USB-C power supply                      | 1   | [URL]        |
| Kypruino UNO+ v0.6 (or Arduino Uno R3)  | 1   | [URL]        |
| Analog pressure sensor (0–8 bar)        | 1   | [URL]        |
| Juliet cable (3-pin, used 4-pin)        | 1   | [URL]        |
| Compressor adapters                     | 1   | [URL]        |
| Solder header pins                      | —   | [URL]        |
| 128×32 I²C OLED display                 | 1   | [URL]        |

---

## 🛠️ Hardware Setup (for Kypruino UNO+)

1. **Power & Ground**  
   - USB-C power cable → USB-C socket

2. **Pressure Sensor**  
   - VCC → 5 V  
   - GND → GND  
   - OUT → A2  

3. **OLED (I²C)**  
   - just plug into a special screen socket of your Kypruino UNO+

> If you are using a plain Uno R3, you have to wirie external buttons, LEDs & buzzer (scheme will be added later).

---

## 💻 Software & Configuration

1. **Clone & open** the Arduino sketch.  
2. **Adjust sensor parameters** if you are using a different pressure sensor (usually provided with a sensor in its documentation):  
   ```cpp
   const float V_MIN    = …;   // sensor's voltage output at 0 PSI pressure 
   const float V_MAX    = …;   // sensor's voltage output at max pressure
   const float PSI_MIN  = …;   // base pressure of the sensor (usually 0 PSI)
   const float PSI_MAX  = …;   // maximum pressure of the sensor (in PSI)

3. **Upload** the sketch to your board.  
4. **Set alert thresholds:**  
   - Press any of the Kypruino's on-board buttons → enter config mode  
   - Use B and D buttons as +/– to adjust maximum threshold (in 0.1 bar steps)
   - Use C and A buttons as +/– to adjust minimum threshold (in 0.1 bar steps)  
   - Wait 5 s without presses → exit & auto-save to EEPROM  

> **Quick disable:** use on-board switches to mute the buzzer or disable LEDs.

---

## ⚙️ Optimizations implemented in the code

- **Partial screen redraws for perfomance and UI smoothness:** only changed regions are sent over I²C  
- **EEPROM-friendly:** writes thresholds only once upon exit from config mode and only if it changed

---

## 🚀 Possible Upgrades

- 🔊 **External buzzer:** connect a louder piezo to the same pin and disable the on-board buzzer  
- 🌟 **Extended NeoPixel strip:** solder and drive an external LED strip for bolder visual feedback  
- 📈 **Data logging:** add an SD card module or serial output for long-term pressure trend analysis (in future updates of this project we may add cloud data sharing support)  

---

## 🖼️ Visual Reference

See the `/images` folder for setup photos, wiring diagrams & deployment shots.

---

## 📜 License

This project is licensed under the MIT License.
