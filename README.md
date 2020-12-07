# HEG ESP32 - Delobotomizer Edition

Official repository for the HEGduino V2 Delobotomizer firmware, software, and designs.

This repo is being updated shortly with more detailed instructions and a new whitepaper. For now, you can follow along from the [original repo](https://github.com/moothyknight/HEG_ESP32) on how to install the chrome extension. 

## Firmware

The Firmware folder contains a .bin file that you can use via the on-board web interface to update the device. There are Arduino libraries and firmware code files for custom code, you will need the ESP32 module additionally for Arduino.

Simply connect to the wifi network named "My_HEG" when the device is powered up with password "12345678"

In your preferred browser, enter "192.168.4.1" into the address bar. You should see an interface show up. Click on the Update button to go to the update page.

Now follow the instructions on screen to browse for the .bin folder that you may download from this repo. Wait for it to finish uploading then reconnect to the HEG after that's taken care of if it does not do it automatically. Voila!

## Progressive Web App (WIP): https://hegalomania.netlify.app

Our new preferred method for using the HEG, a state of the art Progressive Web App! Open this link to use your HEG in a Chrome browser or Android for instant access to using your device.

You may install it locally on desktop or mobile as well just like any app and create desktop shortcuts. For desktop open the settings bar in Chrome and click "Install HEG Alpha." The USB mode only works in chrome right now as it is a development feature. You can enable this feature by going to chrome://flags and enabling the "Experimental Web Platform features" flag. For mobile it should prompt you to install, if not find the install button in your chrome settings menu on mobile.





