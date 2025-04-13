# 🔐 ESP32-Based Password & RFID Door Lock System

A secure door lock system built using ESP32, NFC cards, 4x4 keypad, and a 20x2 LCD display. It supports multiple authentication methods, admin/user management, buzzer alerts, and a lockdown mode to prevent brute force attacks.

## 📦 Features

| Feature | Description |
|----------|-----------|
|✅ Password Unlock | Users can unlock the door using a registered password via keypad |
|✅ RFID Unlock | Supports unlocking using pre-registered NFC cards|
|✅ LCD Interface | Displays status messages using a 20x2 LCD screen |
|✅ Admin/User Management | Add or remove users and update passwords through keypad and admin login |
|✅ Lockdown Mode | Locks out the system temporarily after 3 failed attempts |
|✅ Web-based Configuration | Configure users and system settings via an HTML interface |
|✅ Buzzer Alerts | Audio feedback for success, error, and alert states |
|🚧 Mobile App Integration | (Planned) Unlock door via mobile app using Wi-Fi |

## 🔐 Security Features

- Lockdown Mode: Activates after 3 failed password attempts, disabling inputs for 30 seconds.
- Setup Mode: First-time setup allows assigning the admin via a default password.
- Temporary Messages: Feedback is shown briefly on the LCD for user interactions.


## 📋 Status Table

| Module | Status | Notes|
|--------|--------|------|
| Password Unlock | ✅ Done | Fully functional|
| RFID Unlock | ✅ Done | RC522 integrated|
| Admin Management | ✅ Done | Admins can add/remove users|
| Web Configuration | ✅ Done | Includes HTML form handling|
| Lockdown Mode | ✅ Done | 3-strike mechanism with timeout|
| Persistence | ✅ Done | Could store state across reboots|
| Mobile App Control | 🚧 Planned | Integration planned for future version|
  
  <br>

> This project was built as part of a college major project for secure smart locking systems.
