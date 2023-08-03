# bachelor-thesis
**This is a work in progress, parts may not yet function correctly and might change drastically until the final version**

## How to use
1. This project uses the esp-idf, so make sure you have that installed, set up, and working correctly before continuing
   - see [Espressif's get-started guide](https://docs.espressif.com/projects/esp-idf/en/v5.1/esp32/get-started/) for installation options
   - you can run the example project to test whether everything works as expected
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
     - if you have multiple ESPs connected to the computer, or if the correct device isn't found automatically, you should specify the port e.g. `idf.py -p build flash`

2. Configure and flash the receiver
   - go into the csi_receiver folder (`cd ../csi_receiver/`)
   - set the target to the kind of ESP you are using e.g.
     - `idf.py set-target esp32` when using an ESP32
     - `idf.py set-target esp32s3` when using an ESP32-S3
   - configure any options you want in the menuconfig (`idf.py menuconfig`)
     - specific to this project, you can configure the follow options for the receiver:
       -  Wi-Fi configuration: ssid, password, Wi-Fi channel, and the maximum number of connected station the AP accepts
       -  Logging settings: enabling logging to SD card, to serial, and choosing which information to log including logging different things to different outputs
   - build and flash e.g. using `idf.py build flash`
     - if you have multiple ESPs connected to the computer, or if the correct device isn't found automatically, you should specify the port e.g. `idf.py -p build flash`
   - once flashing was successful, you can monitor the serial output (`idf.py monitor`)
4. Run live visualization
   - the live visualization can plot features logged to serial or UDP
   - which of the logged features are plotted can be selected through command line arguments (see the beginnning of the file for available options)
   - to run visualization of features logged to serial run the following command from within the `csi_receiver` folder: `idf.py monitor | python ../live_visualization/visualization.py`
   - when plotting the features logged to UDP just running the visualization script is sufficient, however, the device running the script needs to be connected to the receiver's Wi-Fi network to receive the udp
