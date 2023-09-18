# csi_receiver
This is the ESP-IDF project for the receiver, the heart of the sleep monitoring system, which performs all of the processing and analysis. 
The following figure provides an overview of the system components. Most of the shown components can be configured using menuconfig.
![System overview](./../../writing/final_figures/System_overview.png)

This project uses the [esp32-fft](https://github.com/fakufaku/esp32-fft/tree/master), TFLite Micro with ESP-NN optimized kernels based on the [TFLite Micro examples for Espressif chipsets](https://github.com/espressif/tflite-micro-esp-examples/tree/master), which are included as components, and [Eigen for ESP-IDF](https://github.com/espressif/idf-extra-components/tree/master/eigen) which is added through the IDF Component Manager.

## Structure
The repository is structured as follows:
- `components` contains the components - esp32-fft, esp-nn, tflite, and utilities which includes reusable methods written for this project that might be of use in other project and were separated to keep the main code more clean
- `main` contains the main code of the receiver
    - `csi_receiver.c` contains the main code of the receiver
    - `model.cpp` contains the compiled model used for sleep staging. This is currently a dummy with an untrained model. Use the [model creation and training notebook](../notebooks/SleepStageModelCreationAndTraining.ipynb) to create, train, and test a model with collected data and then replace this file with the `model.cpp` produced by the notebook.
    - `run_tf_inference.cpp` contains the code for loading the model and running inference
    - `using_eigen.cpp` contains the function to check whether the first principal component is positive, which is used for the subcarrier fusion
- `test` contains unit tests for some of the functions in utility component
    - to run the tests, go into the test folder, then build, flash, and monitor (`idf.py build flash monitor`)

## How to use
1. This project uses the ESP-IDF, so make sure you have that installed, set up, and working correctly before continuing
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
     - there are various configuration option specific to this project under the menu `CSI Receiver Configuration`
     - you can also view those options and the corresponding description in the file [`Kconfig.projbuild`](main/Kconfig.projbuild)
     > [!NOTE]
     > When using multiple receivers at the same time and intending to use the live visualization, it is recommended to configure them to create differently named access points to avoid issues with receiving only the UDP broadcasts of the desired receiver. 

   - build and flash e.g. using `idf.py build flash`
     - if you have multiple ESPs connected to the computer, or if the correct device isn't found automatically, you should specify the port e.g. `idf.py -p /dev/ttyUSB1 build flash`
   - once flashing was successful, you can monitor the serial output (`idf.py monitor`)

