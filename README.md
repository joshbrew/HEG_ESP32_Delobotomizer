# HEG ESP32 - Delobotomizer Edition

Official repository for the HEGduino V2 Delobotomizer firmware, software, and designs.

### [WebApp: Brains@Play](https://app.brainsatplay.com)
- This will replace the original software and is MUCH more robust, we are still doing a lot of legwork to get all of the quality of life features we want into it but welcome to the new multi-bci community framework for the web! 
- Chrome only for desktop, works on Android and in VR. 
- Find the install button via the settings button in the top right of the browser.
### [Getting Started (click me)](https://github.com/moothyknight/HEG_ESP32_Delobotomizer/blob/main/Guides/GettingStarted.md)
### [Updating Your Device](https://github.com/moothyknight/HEG_ESP32_Delobotomizer/blob/main/Guides/Updating.md)
### [Whitepaper](https://github.com/moothyknight/HEG_ESP32_Delobotomizer/blob/main/Guides/Open%20Source%20HEG_FNIRS%20Whitepaper.pdf)
### [Old Progressive Web App (no longer updated)](https://hegalomania.netlify.app) 
- Original webapp demo, use via Chrome for Serial USB support. Use BLE mode if the USB device is not recognized in browser, this is a known issue. If that fails use the web server with the device in WiFi mode.


### If your browser won't identify the USB connection, try installing the VCP for Windows (or Mac) drivers here: https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers

## Firmware

The Firmware folder contains a .bin file that you can use via the on-board web interface to update the device. There are Arduino libraries and firmware code files for custom code, you will need the ESP32 module additionally for Arduino.

Simply connect to the wifi network named "My_HEG" when the device is powered up with password "12345678"

In your preferred browser, enter "192.168.4.1" into the address bar. You should see an interface show up. Click on the Update button to go to the update page.

Now follow the instructions on screen to browse for the .bin folder that you may download from this repo. Wait for it to finish uploading then reconnect to the HEG after that's taken care of if it does not do it automatically. Voila!

## Progressive Web App (WIP): https://hegalomania.netlify.app

Our new preferred method for using the HEG, a state of the art Progressive Web App! 

Open this link to use your HEG in a Chrome browser or Android for instant access to using your device. This is being updated to a React app with a more familiar UI and a more concise code base to expand from.

You may install it locally on desktop or mobile as well just like any app and create desktop shortcuts. For desktop open the settings bar in Chrome and click "Install HEG Alpha." The USB mode only works in chrome right now as it is a development feature. You can enable this feature by going to chrome://flags and enabling the "Experimental Web Platform features" flag. For mobile it should prompt you to install, if not find the install button in your chrome settings menu on mobile.

![Boids](images/Capture.PNG)

## Boot routine

The newer firmware contains a new boot-up routine for making it easier to identify streaming modes on the device. It will flash when powered up to indicate which mode it's in.

On boot:
* Two fast blinks: BLE mode
* Three fast blinks: WIFI mode
* Four fast blinks: Bluetooth Classic (Serial) mode

The button on the newer devices puts your device to sleep. There will be a switch added next to control device settings.

Since your device is covered in tape this will help you find the reset button if I didn't mark it for you:
![esp32](https://github.com/moothyknight/HEG_ESP32_Delobotomizer/blob/29a4eabf0b20d9b95add2a5981e5b34cf1502fad/images/esp32.jpg)

## Device Commands (accessible via app (Send Command button) or serial monitor)

* 't' - Turn sensor ON (automatic)
* 'f' - Turn sensor OFF, device will go to sleep after 10min of inactivity
* 'b' - BLE mode toggle. Toggles back to WiFi mode.
* 'B' - Bluetooth Serial toggle. Toggles back to WiFi mode.
* 'W' - Reset WiFi credentials (if it won't connect to your router)
* 'R' - Reboot device
* 'S' - Enter deep sleep mode e.g. to save battery
* 'u' - USB only output mode (no wireless signals)
* 'o' - Fast output mode (<20 byte packets for Android Web BLE)
* 'L' - External LED mode, configured for pins 12 and 14 to run LEDs and be sampled.
* 'N' - Toggle SPO2 output (experimental, not working properly yet)
* 'D' - Toggle Debug output (for using Arduino's graphing)
* 'e' - Toggle exposure settings: slow, fast, and default. default has the slowest output rate but samples faster than slow mode with more averaging.
* 'l' - Toggle LED protocols in this order: Ambient,IR,2 IR (AMBIENT, LED2, LED1+2); Red,IR,2 IR (LED3, LED2, LED1+2), Red = IR (LED1, LED2, AMB), or Default (LED3,LED2,AMBIENT)
* 'X' - DC filter toggle
* 'F' - Digital filter toggle
Note, use the first setting with the command 'l' if your red LED is underperforming. This will make the software compare Ambient and IR, and optional both IR LEDs for extra strength. For some reason the red output is not as consistent for some people.
