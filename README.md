
# Firmware for curtains, blinds, shutters, ...
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
- MS-108

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
Using platform.io, you can select between diferent environments: **kingart** or **other** boards.

# First flash
## KingArt Q4
This comment in Tasmota issues is very clear:

NOTE that the user was wrong, the GPIO0 for flash is SW, close to GND (both soldered). The pinout "RES1" is not required.

https://github.com/arendst/Tasmota/issues/5059#issuecomment-546736419

## Other boards
After install the firmware for other boards, you need to configure the pinout for each one. As example, this is the pinout I am using in my devices, but you need to adapt to your soldered pins.

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
When you flash the device the first time. It creates an Access Point with name SHUTTERFW_XXXXXX. You must connect to it with password "fibonacci" and navigate to http://192.168.4.1:8080

Configure the WiFi parameters and other also and restart the device to connect to your wifi.

After that, navigate to the IP that your router give to the device and configure it in: http://\<IP\>:8080

# HomeAssistant Integration
To configure with HomeAssistant, you need to have configured:
- MQTT Server
- Hostname
- Click on "Enable HomeAssistant Integration" checkbox

After that, click on "See HomeAssistant configuration" link and put those configuration in your configuration.yaml file and restart your HomeAssistant server.

# TODO List
- [X] Integration of KingArt Q4 Curtain
- [X] Integration with BlitzWolf SS4
- [X] Integration of Sonoff Dual R2
- [X] Dynamic parameters instead hardcoded (time up/down, wifi, mqtt config, hostname, alexa name, ...)
- [X] Integration with HomeAssistant
- [X] Remove all the content and the file config.h. Make it dynamic and configurable in web interface.
- [X] Code clean (hehe)
- [X] Control the device from the management webpage
- [X] Set different levels of open (for instance: 30%, 50%, 70%, 90%)
- [ ] Add mDNS (Not yet)
