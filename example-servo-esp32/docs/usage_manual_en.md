# ESP32 7-Axis Servo Motion Controller User Manual

This project is a high-performance **7-axis servo controller** designed specifically for motion cockpit control in Sim Racing and Flight Simulation. It provides high-precision motion synchronization when paired with leading motion software.

## 1. Overview
The web-based dashboard provides real-time interaction with the ESP32 firmware, allowing for precise position control of 7 servo axes, mechanical parameter configuration, and system-level diagnostics.

### Software Compatibility
The controller natively supports the following industry-standard motion software:
- **FlyPT Mover**
- **SimTools**
- **SimHub**

Ideal for 2DOF, 3DOF, 6DOF, and custom multi-axis motion platform applications.

## 2. Features

### Axis Control & Configuration (Axis 1 - Axis 7)
Each axis features an independent control panel with the following capabilities:
- **Status Indicator**: Shows if the axis is "ENABLED" and displays current position percentage.
- **Position Control**: Adjust axis position via slider or direct numerical input (in mm).
- **Stroke Setting**: Configure the maximum travel range (mm).
- **Lead Setting**: Set the screw lead or distance per revolution (mm/rev).
- **Motor Reverse**: Toggle the motor direction checkbox for quick orientation adjustment.
- **Homing Functions**:
    - **Home Button**: Triggers the automated homing sequence for the axis.
    - **Manual Home**: Calibrates the current position as the zero point manually.

### Global Controls
- **Connection Management**: Displays "Connected" status and allows for manual "Disconnect/Connect".
- **Auto Home**: Global toggle to enable automatic homing under specific conditions.
- **Speed Mode**: Select between High, Medium, or Low speed presets for global movement.
- **Home All**: One-click command to home all axes simultaneously.
- **Restart Controller**: Remotely trigger an ESP32 hardware reset.
- **Firmware Update**: Check for and upload new firmware versions.

### System Diagnostics
- **Console Output**: Real-time display of raw communication data (TX/RX) for protocol analysis and troubleshooting.
- **Clear Logs**: Instantly clear the console output window.
- **Language Support**: Switch between English (EN) and Chinese (CN).

## 3. Quick Start
1. Ensure the ESP32 is powered and connected to your network.
2. Access the dashboard via browser (e.g., `servo.marslife.top`).
3. Verify the "Connected" status in the top-left corner.
4. Configure "Stroke" and "Lead" parameters according to your mechanical setup.
5. Click "Home All" to initialize axis positions.
6. You are now ready to control the axes via the UI or compatible motion software.
