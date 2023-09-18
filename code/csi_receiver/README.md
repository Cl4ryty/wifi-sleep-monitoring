# csi_receiver
This is the ESP-IDF project for the receiver, the heart of the sleep monitoring system, which performs all of the processing and analysis. 
The following figure provides an overview of the system components. Most of the shown components can be configured using menuconfig.
![System overview](./../../writing/final_figures/System_overview.png)

This project uses the [esp32-fft](https://github.com/fakufaku/esp32-fft/tree/master), TFLite Micro with ESP-NN optimized kernels based on the [TFLite Micro examples for Espressif chipsets](https://github.com/espressif/tflite-micro-esp-examples/tree/master), which are included as components, and [Eigen for ESP-IDF](https://github.com/espressif/idf-extra-components/tree/master/eigen) which is added through the IDF Component Manager.

## Structure
The repository is structured as follows:
- `components` contains the components - esp32-fft, esp-nn, tflite, and utilities which includes reusable methods written for this project that might be of use in other project and were separated to keep the main code more clean
- `main` contains the main code of the receiver
    - `live_visualization` contains the python script for live visualization of the results of the sleep monitoring system
    - `notebooks` contains various Jupyter notebooks e.g. for precalculating parameters and creating and training the NN
    - `csi_sender` contains the ESP-IDF project code for the transmitter
    - `csi_receiver` contains the ESP-IDF project code for the receiver which performs all the processing and analysis of the CSI and is the heart of the sleep monitoring system
- `test` contains unit tests for some of the functions in utility component

## How to use
1. This project uses the esp-idf, so make sure you have that installed, set up, and working correctly before continuing
   - see [Espressif's get-started guide](https://docs.espressif.com/projects/esp-idf/en/v5.1/esp32/get-started/) for installation options
   - you can run the example project to test whether everything works as expected
   - this project was created and tested using esp-idf version 5.1, so using that version is recommended although others might also work
2. Clone this repository e.g. using `git clone https://github.com/Cl4ryty/bachelor-thesis.git`
3. Configure and flash the receiver
   - go into the csi_receiver folder (`cd ../csi_receiver/`)
   - set the target to the kind of ESP you are using e.g.
     - `idf.py set-target esp32` when using an ESP32
     - `idf.py set-target esp32s3` when using an ESP32-S3
   - configure any options you want in the menuconfig (`idf.py menuconfig`)
     - specific to this project, you can configure the follow options for the receiver:
       -  Wi-Fi configuration: ssid, password, Wi-Fi channel, and the maximum number of connected station the AP accepts
       -  Logging settings: enabling logging to SD card, to serial, and choosing which information to log including logging different things to different outputs
   - build and flash e.g. using `idf.py build flash`
     - if you have multiple ESPs connected to the computer, or if the correct device isn't found automatically, you should specify the port e.g. `idf.py -p /dev/ttyUSB1 build flash`
   - once flashing was successful, you can monitor the serial output (`idf.py monitor`)
4. Run live visualization
   - the live visualization can plot features logged to serial or UDP
   - which of the logged features are plotted can be selected through command line arguments (see the beginnning of the file for available options)
   - to run visualization of features logged to serial run the following command from within the `csi_receiver` folder: `idf.py monitor | python ../live_visualization/visualization.py`
   - when plotting the features logged to UDP just running the visualization script is sufficient, however, the device running the script needs to be connected to the receiver's Wi-Fi network to receive the UDP broadcasts
