## Testing
use this to test the library via demo.ino
```bash
C:\Users\VWFMH1A\.platformio\penv\Scripts\platformio.exe ci --board esp32dev --project-option="lib_extra_dirs=examples" --lib="." --project-option="lib_ignore=AsyncTCP_RP2040W" examples/demo.ino
```
