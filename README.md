# ESP32 Walkie-Talkie (ESP-NOW + I2S)

## Overview

This project implements a real-time digital walkie-talkie using ESP32 boards. It uses ESP-NOW for direct peer-to-peer communication and I2S for audio input/output, enabling low-latency voice transmission without requiring a WiFi network or internet connection.

## Features

* Real-time audio streaming
* ESP-NOW based communication (no router required)
* Push-to-Talk (PTT) control
* I2S microphone and speaker support
* Broadcast transmission (one-to-many communication)

## Hardware Requirements

* 2x ESP32 boards
* I2S microphone (e.g., INMP441)
* I2S amplifier/speaker module (e.g., MAX98357A)
* Push button
* Connecting wires

## Wiring

### Microphone (I2S)

| Signal | ESP32 GPIO |
| ------ | ---------- |
| WS     | 4          |
| SCK    | 5          |
| SD     | 6          |

### Speaker (I2S)

| Signal | ESP32 GPIO |
| ------ | ---------- |
| DIN    | 7          |
| BCLK   | 15         |
| LRC    | 16         |

### Control

| Component  | ESP32 GPIO |
| ---------- | ---------- |
| PTT Button | 39         |

## Usage

1. Upload the same code to both ESP32 devices.
2. Power the devices.
3. Press and hold the PTT button to transmit audio.
4. Release the button to switch back to receive mode.

## Notes

* Ensure both devices are within ESP-NOW range.
* Stable power supply is recommended for consistent audio performance.
* Audio quality may vary depending on the microphone and speaker modules used.

## License

This project is provided as open-source for educational and experimental use.
