idf_component_register(
    SRCS "run_tf_inference.cpp" "model.cpp" "using_eigen.cpp" "csi_receiver.c"
    PRIV_REQUIRES tflite-lib spi_flash
    REQUIRES esp_wifi nvs_flash fatfs vfs sdmmc utilities fft
    INCLUDE_DIRS ".")
set_target_properties(${COMPONENT_LIB} PROPERTIES COMPILE_FLAGS -Wno-format)
