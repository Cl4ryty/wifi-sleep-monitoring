# csi_sender
This is the ESP-IDF project for the sender, which sends raw 802.11n null-data frames to the receiver (MAC address 0E:0E:0E:0E:0E:0E). 

## How to use
1. This project uses the esp-idf, so make sure you have that installed, set up, and working correctly before continuing
   - see [Espressif's get-started guide](https://docs.espressif.com/projects/esp-idf/en/v5.1/esp32/get-started/) for installation options
   - you can run the example project from the guide to test whether everything works as expected
   - this project was created and tested using esp-idf version 5.1, so using that version is recommended although others might also work
2. Clone this repository e.g. using `git clone https://github.com/Cl4ryty/bachelor-thesis.git`
3. Configure and flash the sender
   - go into the csi_sender folder (`cd bachelor-thesis/code/csi_sender`)
   - set the target to the kind of ESP you are using e.g.
     - `idf.py set-target esp32` when using an ESP32
     - `idf.py set-target esp32s3` when using an ESP32-S3
   - configure any options you want in the menuconfig (`idf.py menuconfig`)
     - specific to this project, for the sender you can configure the rate that Wi-Fi frames are send - the default is 50Hz - if you change it, make sure to set the same value in the receiver settings!
   - build and flash e.g. using `idf.py build flash`
     - if you have multiple ESPs connected to the computer, or if the correct device isn't found automatically, you should specify the port e.g. `idf.py -p /dev/ttyUSB0 build flash`
   - to test whether the sender works correctly, monitor the output e.g. `idf.py monitor`. For each sent frame the ESP logs ```csi sender: csi``` so seeing these messages indicates that flashing was successful. 