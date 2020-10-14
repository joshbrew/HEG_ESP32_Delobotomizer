# HEG ESP32 - Delobotomizer Edition

Official repository for the HEGduino V2 Delobotomizer firmware, software, and designs.

This repo is being updated shortly with more detailed instructions and a new whitepaper. For now, you can follow along from the [original repo](https://github.com/moothyknight/HEG_ESP32) on how to install the chrome extension. 

## Firmware

The Firmware folder contains a .bin file that you can use via the on-board web interface to update the device. There are Arduino libraries and firmware code files for custom code, you will need the ESP32 module additionally for Arduino.

Simply connect to the wifi network named "My_HEG" when the device is powered up with password "12345678"

In your preferred browser, enter "192.168.4.1" into the address bar. You should see an interface show up. Click on the Update button to go to the update page.

Now follow the instructions on screen to browse for the .bin folder that you may download from this repo. Wait for it to finish uploading then reconnect to the HEG after that's taken care of if it does not do it automatically. Voila!

## Chrome Extension

This is my preferred way to interact with the HEG as it functions like a desktop app rather than dealing with the web interface (which does not work on older laptops), and allows Serial/Bluetooth Serial connectivity via the COM port browser in the top right corner of the UI. These are faster than WiFi transmission rates as well which is soft-limited at 20sps (while the device still runs at full speed but only reports a limited number of samples over WiFi)

First download this repository to get the ChromeExtension folder.

Now open Chrome and type in "chrome://extensions" into the address bar.

Turn on Developer Mode in the top right hand corner and then click the "Load Unpacked" option that shows up.

Browse for the aforementioned ChromeExtension folder and select it. You should see an icon show up.

Now enter "chrome://apps" into the address bar and this will let you open the Chrome extension. If you right click the app icon on the page or in your OS taskbar, you can create a desktop shortcut or pin the app to the taskbar so it is accessible just like any other desktop software!





