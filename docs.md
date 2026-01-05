# ESP32 Smartwatch Prototype — Functional Requirements & Timeline

A smartwatch prototype built with **ESP32** and an **Android companion app**, focusing on core smartwatch behaviors: BLE communication, UI responsiveness, Pomodoro timer, step tracking, and Spotify remote control.
---

## 📌 Project Overview

**Team Size:** 2  
- **Embedded Engineer:** ESP32 firmware, FreeRTOS, LVGL, BLE, power  
- **Mobile Engineer:** Android app, BLE Central, Spotify SDK, Health APIs  

**Primary Goal:**  
Deliver a **stable, demo-ready MVP within 8 weeks**, with reliable BLE communication and real user-facing features.

---

## 🎯 Functional Requirements

### 1. System-Level Requirements
| ID | Requirement |
|----|------------|
| SYS-1 | The system shall consist of an ESP32 smartwatch and an Android companion app |
| SYS-2 | The watch and phone shall communicate using Bluetooth Low Energy (BLE) |
| SYS-3 | The system shall recover automatically from BLE disconnections |
| SYS-4 | Heavy computation (steps, Spotify, weather) shall be handled by the phone |
| SYS-5 | The watch shall remain usable when the phone screen is off |

---

### 2. Watch (Embedded) Functional Requirements

#### Boot & OS
| ID | Requirement |
|----|------------|
| W-1 | The watch shall boot into FreeRTOS |
| W-2 | The watch shall initialize LVGL and render a UI within 3 seconds |
| W-3 | The system shall use multiple FreeRTOS tasks with defined priorities |
| W-4 | The UI task shall have the highest priority |

#### User Interface
| ID | Requirement |
|----|------------|
| W-UI-1 | The watch shall support touch input |
| W-UI-2 | Touch latency shall be less than 100 ms |
| W-UI-3 | Users shall be able to swipe between screens |
| W-UI-4 | Screen transitions shall not block BLE communication |

#### BLE
| ID | Requirement |
|----|------------|
| W-BLE-1 | The watch shall act as a BLE Peripheral |
| W-BLE-2 | The watch shall expose GATT (Generic Attribute Profile) characteristics for commands and status |
| W-BLE-3 | The watch shall reconnect automatically after disconnection |
| W-BLE-4 | Command round-trip shall complete without data corruption |

#### Pomodoro Timer
| ID | Requirement |
|----|------------|
| W-POM-1 | The watch shall support start / pause / reset of Pomodoro |
| W-POM-2 | The timer shall continue running without phone interaction |
| W-POM-3 | The watch shall trigger vibration when a Pomodoro completes |

#### Vibration
| ID | Requirement |
|----|------------|
| W-VIB-1 | The vibration motor shall be driven using PWM |
| W-VIB-2 | Vibration duration and intensity shall be configurable |

#### Data Display
| ID | Requirement |
|----|------------|
| W-DATA-1 | The watch shall display step count received from the phone |
| W-DATA-2 | The watch shall display Spotify playback status |

---

### 3. Mobile (Android) Functional Requirements

#### BLE
| ID | Requirement |
|----|------------|
| M-BLE-1 | The phone shall act as a BLE Central |
| M-BLE-2 | The app shall automatically reconnect to the watch |
| M-BLE-3 | BLE shall work in foreground and background |
| M-BLE-4 | The app shall survive watch reboot |

#### Step Tracking
| ID | Requirement |
|----|------------|
| M-STEP-1 | Step count shall be retrieved via Health / Sensor API |
| M-STEP-2 | Step data shall be periodically sent to the watch |
| M-STEP-3 | Step calculation shall not run on the watch |

#### Spotify
| ID | Requirement |
|----|------------|
| M-SPOT-1 | The app shall integrate Spotify SDK |
| M-SPOT-2 | The watch shall control play / pause / skip |
| M-SPOT-3 | Playback shall run on the phone, not the watch |

#### Background Operation
| ID | Requirement |
|----|------------|
| M-BG-1 | BLE communication shall persist when app is backgrounded |
| M-BG-2 | Notifications shall not block BLE or UI responsiveness |

---

## 📅 Timeline & Milestones

### Weeks 1–2: Bluetooth Protocol & Hardware Bring-up

| Week | Area | Goal | Success Criteria |
|----|------|------|----------------|
| 1–2 | Embedded | ESP32 boots FreeRTOS & LVGL | UI renders on boot |
| 1–2 | Embedded | BLE Peripheral implemented | Stable BLE connection |
| 1–2 | Embedded | PWM vibration control | Phone triggers vibration |
| 1–2 | Mobile | BLE Central implemented | Connect / disconnect works |
| 1–2 | Mobile | Command round-trip | 100+ commands without crash |
| 1–2 | Mobile | Pipeline | Write pipeline to verified build |

---

### Week 3: OS Structure & UI Framework

| Week | Area | Goal | Success Criteria |
|----|------|------|----------------|
| 3 | Embedded | FreeRTOS task architecture | No UI freeze |
| 3 | Embedded | LVGL screen navigation | Smooth swipe transitions |
| 3 | Mobile | Background BLE service | Auto-reconnect works |
| 3 | Mobile | UI responsiveness | No disconnects in background |

---

### Week 4: First Real Features

| Week | Area | Goal | Success Criteria |
|----|------|------|----------------|
| 4 | Embedded | App template architecture | Clean app creation |
| 4 | Embedded | Pomodoro timer | Full cycle completes |
| 4 | Embedded | Step display | Correct values shown |
| 4 | Mobile | Spotify SDK | Play / pause / skip |
| 4 | Mobile | Step data pipeline | Steps visible on watch |

---

## 📦 Deliverables by End of Week 4
- Stable BLE communication
- Touch-based UI framework
- Fully functional Pomodoro timer
- Spotify remote control
- Step count displayed on watch

---

## 🔜 Weeks 5–8 (Planned)
- Notification system
- Weather integration
- Power optimization
- UX polish
- Final demo & documentation

---

## 🛠 Tech Stack Summary

**Watch:** ESP32, ESP-IDF, FreeRTOS, LVGL, BLE, PWM  
**Mobile:** Android, BLE Central, Spotify SDK, Health API, Background Services
