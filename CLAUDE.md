# BitInk Color ESP32 Project

## Development Commands

### ESP-IDF Setup and Build
```bash
# Source ESP-IDF environment
get_idf

# Build project
idf.py build

# Flash to device and monitor
idf.py flash
```

### Serial Monitor Script
Since `idf.py monitor` requires TTY, use this Python script to monitor ESP32 output:

```python
import serial
import time
import sys

try:
    ser = serial.Serial('/dev/cu.usbserial-110', 115200, timeout=1)
    print('Connected to ESP32, resetting device...')
    
    # Reset the device by toggling DTR/RTS
    ser.dtr = False
    ser.rts = True
    time.sleep(0.1)
    ser.rts = False
    time.sleep(0.5)
    
    print('Monitoring for 10 seconds...')
    start_time = time.time()
    
    while time.time() - start_time < 10:
        if ser.in_waiting > 0:
            data = ser.readline().decode('utf-8', errors='ignore').strip()
            if data:
                print(data)
        time.sleep(0.1)
    
    ser.close()
    print('\nMonitoring complete.')
except Exception as e:
    print(f'Error: {e}')
```

### Quick Monitor Command
```bash
get_idf && python3 -c "[paste the above Python script here]"
```

## Project Info
- **Hardware**: ESP32-S3 with 2MB PSRAM
- **Display**: 1024x576 color e-paper display
- **Features**: Battery monitoring, LVGL graphics, USB charging detection
- **Serial Port**: /dev/cu.usbserial-110 (may vary)