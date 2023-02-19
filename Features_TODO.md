# Features to be Implemented

---

## Firmware

- SD Card Logging
  - Add support for filename enumeration on startup
  - Ensure csv logging format matches the current logging format for CANlogGUI support
- Support rest of SLCAN protocol
  - Actually opening and closing ports
  - Baudrate selection
  - Standard ID support
- Standard ID support in general
- Support Sending CAN messages from Receiver &rarr; Transmitter &rarr; Bus
---

## Software

- Support CAN messages not in DBC
- Support Sending CAN messages
- Support more information in `Live Data` Tab
  - DLC
  - Cycle Time
  - Raw Representation
  - Receive Count
- Implement Vehicle Dashboard tab with general vehicle info
  - Cell Min/ Max Temp
  - Cell Min/ Max Voltage
  - Vehicle Speed
  - SOC
---

## Hardware

- Add proper USB-C Support (5.1 kOhms CC1 and CC2 pull-downs)
---