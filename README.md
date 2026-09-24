# flysky2espnow
A device to bridge flysky(AFHDS 2A protocol) and ESPNOW.

### A compact device which acts a bridge between flysky protocol and ESPNOW.
---
Connects via iBUS directly to flysky receiver and secondary esp device, supports both 2 way communication.

## Features
- Supports 2-way communication
- ESP32 C6 acts as the main microcontroller
- Handles both 3V3 based MCU and 5V based reciever via external voltage booster
- OLED for live stats
- USB-C charging through the XIAO ESP32 C6
- Compact form factor

---

# Schematics-
<img width="1168" height="596" alt="schematics" src="https://github.com/user-attachments/assets/1d8874f2-48cd-4bca-ae5a-8eb4082bc584" />

## How It Works

1. The ESP32 fetches flysky receiver data using iBUS.

2. Data is sent over ESPNOW to other ESP device.

3. OLED shows live data.


# BOM-
| Component Name | Description | Quantity |
| :--- | :--- | :---: |
| **Seeed Studio Xiao ESP32 C6** | Devboard for ESP32 C6 Chipset | 1 |
| **TPS61023** | Boost voltage to stable 5V | 1 |
| **Swtich** | Power | 1 |
| **OLED** | Live Stats | 1 |

---

**Made by hackerskill**
