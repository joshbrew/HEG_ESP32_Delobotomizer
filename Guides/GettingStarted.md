## So you just got your device

### Quick Start
* Open this app demo via Chrome: https://hegalomania.netlify.app
* You can install it by clicking the settings icon (3 vertical dots) in the top right corner.
* Enable Serial by entering chrome://flags in a new tab and enabling the "Experimental Web Platform Features" flag and restarting Chrome when prompted.
* Plug in your HEG via USB, you should see it flashing then show solid red (if you don't see the LED, reset the device by unplugging it or feeling around for the reset button in the middle of the microcontroller hub) 
* Connect your device via "Set USB Device." Data should begin streaming in automatically. If not, reset the device. There are some known glitches on startup here.
* With the device on your head, you can squeeze and bend the headset until it is comfortable on your head. You don't need to wear it so tight that you feel like you are being poked by the LEDs or gripped by the headset, it should be comfortable.
* If the data looks funky or random, make sure the fabric on the headband is not covering the sensor, which should be in contact with your forehead.

![img](https://github.com/moothyknight/HEG_ESP32_Delobotomizer/blob/main/images/Capture.PNG)

#### Web Interface
* The device should boot up in WiFi mode by default. This is indicated by 3 quick flashes in succession upon reset. After that you will see 3 slower flashes. Resetting after the first slower flash will change the communication mode (2 for BLE, 3 for WiFi, 4 for BT Serial). Use BLE mode for mobile.
* Find the WiFi connection "My_HEG" and enter password 12345678
* Open a browser (Chrome preferably) and enter [http://192.168.4.1](http://192.168.4.1) or [http://esp32.local](http://esp32.local) if that doesn't work

### Quick Training Guide
* The ratio represents the relative blood-oxygen content in your brain. A higher ratio represents a higher relative blood-oxygen saturation in your brain tissue. The score is simply an exaggerated ratio with the baseline 0 point set at the initial ratio.
* Simply perform 6 breath per minute, deep cyclical breathing exercise for 5-15 minutes (no pauses, just do a chest-expanding, relaxing inhale and exhale wave pattern). Stop if you feel any strain.
* Once your breathing is stabilized, try to notice what thoughts or distractions come up and see if you notice any correlation with the change in score or ratio on-screen. You may quickly find that stress and focus can have a big effect on the patterns, it may take a few sessions to notice.
* Use the combination of stable breathing and awareness of your own thoughts to try to optimize the increase in your score. Your ratio will only increase so much before going back to baseline, which is your brain doing work then relaxing again. You may even feel fatigue when this occurs, which is a good indicator to stop.

