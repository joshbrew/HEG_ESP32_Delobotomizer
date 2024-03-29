## So you just got your device

### Quick Start
* Open this new improved app https://app.brainsatplay.com or the old app demo via Chrome: https://hegalomania.netlify.app 
* You can install it by clicking the settings icon (3 vertical dots) in the top right corner. It will be automatically kept up to date as long as your device is connected to the internet.
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
* The ratio represents the relative blood-oxygen content in your brain. A higher ratio represents a higher relative blood-oxygen saturation in your brain tissue. The score is simply an exaggerated ratio with the baseline 0 point set at the initial ratio. The different visuals (accessible via the Feedback menu) react to the score. You can reset the session via the Reset button in the Data menu.
* Simply perform a 6 breath per minute (5 seconds in, 5 seconds out), deep cyclical breathing exercise for 5-15 minutes (no pauses, just do a chest-expanding, relaxing inhale and exhale wave pattern). Stop if you feel any strain.
* Once your breathing is stabilized, try to notice what thoughts or distractions come up and see if you notice any correlation with the change in score or ratio on-screen. You may quickly find that stress and focus can have a big effect on the patterns, it may take a few sessions to notice.
* Use the combination of stable breathing and awareness of your own thoughts to try to optimize the increase in your score. Your ratio will only increase so much before going back to baseline, which is your brain doing work then relaxing again. You may even feel fatigue when this occurs, which is a good indicator you're doing it right and to take a break.

### Important
* Keep the LEDs away from your eyes, you can even give yourself a headache from eye strain with the light passing through your skull if it's too close, so keep it higher on your forehead or to the side of your head.
* Speak to an expert before using this for headaches or migraines. Our development kit is not rated for medical use nor are any HEGs, so proceed with caution. 

