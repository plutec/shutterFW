
# Firmware curtains, blinds, shutters, ...
This device is special because it has a microcontroler (Nuvoton, MCU from now) for leds and relays. The communication with the ESP8285 is via Serial port with AT commands.

I made this firmware for two main reason:
- Tasmota doesn't manage right the MCU
- Tasmota doesn't work fine with Alexa (in any curtain mode)

And here is the result of hours of frustration with Tasmota and reversing the behaviour of the device.

# Supported devices
Also, I reuse the same code for differents HW devices that I have:

- KingArt Q4 Curtain
- BlitzWolf SS4 (2 ways)
- Sonoff Dual R2 (2 ways)

# Calculate up/down times
## KingArt Q4
The MCU manages all the things related with timming, pressing and relays, and inform to the ESP via AT commands. To configure the timming do the next:
- Lower the blind totally.
- Press the middle button (stop) during 4 seconds. The 3 buttons start to blink.
- Press the up button. When the blind reach the top, press again the up button.

Now, the timming to up/down the blind are configured. Remind that this behaviour is managed by the MCU, not any code in this repository related with that.
## Other boards
In the configuration webpage, there is an integer that must configured with the time the shutter take to down or up (the maximum of both). This time will be use to calculate the percentages of open with Alexa and HomeAssistant.

# Compilation
Copy the file config_h to config.h, change your board information (also pinout if is required) and compile using platform.io.

This configuration is only valid for the pinout of the device and to select the device to use.

# First flash
## KingArt Q4
This comment in Tasmota issues is very clear:

NOTE that the user was wrong, the GPIO0 for flash is SW, close to GND (both soldered). The pinout "RES1" is not required.

https://github.com/arendst/Tasmota/issues/5059#issuecomment-546736419

## Other boards
TODO

### Sonoff Dual R2
- Relay UP -> GPIO 5
- Relay DOWN -> GPIO 12
- Button UP -> GPIO 0
- Button DOWN -> GPIO 9


### BW SS4
- Relay UP -> GPIO 12
- Relay DOWN -> GPIO 5
- Button UP -> GPIO 14 
- Button DOWN -> GPIO 2
- LED -> GPIO 5

### MS-108
- Relay Up -> GPIO 12
- Relay Down -> GPIO 14
- Button Up -> GPIO 4
- Button Down -> GPIO 5

# Firmware configuration
When you flash the device the first time. It creates an Access Point with name MIESPITO. You must connect to it with password "fibonacci" and navigate to http://192.168.4.1:8080

Configure the WiFi parameters and other also and restart the device to connect to your wifi.

After that, navigate to the IP that your router give to the device and configure it in: http://<ip>:8080

# TODO List
- [X] Integration of KingArt Q4 Curtain
- [X] Integration with BlitzWolf SS4
- [X] Integration of Sonoff Dual R2
- [X] Dynamic parameters instead hardcoded (time up/down, wifi, mqtt config, hostname, alexa name, ...)
- [ ] Integration with HomeAssistant
- [ ] Code clean (hehe)
- [ ] Add mDNS
- [ ] Set different levels of open (for instance: 20%, 40%, 50%, 70%)
- [ ] Control the device from the management webpage
- [ ] Remove all the content and the file config.h. Make it dynamic and configurable in web interface.