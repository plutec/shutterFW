# Firmware for KingArt Q4 Curtain
This device is special because it has a microcontroler (Nuvoton, MCU from now) for leds and relays. The communication with the ESP8285 is via Serial port with AT commands.

I made this firmware for two main reason:
- Tasmota doesn't manage right the MCU
- Tasmota doesn't work fine with Alexa

And here is the result of hours of frustration with Tasmota and reversing the behaviour of the device.

# Calculate up/down times
The MCU manages all the things related with timming, pressing and relays, and inform to the ESP via AT commands. To configure the timming do the next:
- Lower the blind totally.
- Press the middle button (stop) during 4 seconds. The 3 buttons start to blink.
- Press the up button. When the blind reach the top, press again the up button.

Now, the timming to up/down the blind are configured. Remind that this behaviour is managed by the MCU, not any code in this repository related with that.

# Compilation
Copy the file config_h to config.h, change your information (WiFi network and MQTT) and compile using platform.io

# First flash
This comment in Tasmota issues is very clear:

NOTE that the user was wrong, the GPIO0 for flash is SW, close to GND (both soldered). The pinout "RES1" is not required.

https://github.com/arendst/Tasmota/issues/5059#issuecomment-546736419

# Firmware update
When you first the first time, you can use this URL to do the next: http://<ip>:8080/update
