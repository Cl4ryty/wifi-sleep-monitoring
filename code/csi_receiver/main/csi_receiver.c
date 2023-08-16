#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_err.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

#include "lwip/sockets.h"

#include "utilities.h"
#include "using_eigen.h"
#include "fft.h"
#include "run_tf_inference.h"

// constants
#define SECONDS_TO_MICROSECONDS 1000000

#define GPIO_INPUT_PIN 0


/* This project uses configuration options that can be set via project configuration menu.

   Some values can be changed here, but it is easier to use menuconfig,
*/
// Wi-Fi settings
#define EXAMPLE_ESP_WIFI_SSID               CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS               CONFIG_ESP_WIFI_PASSWORD
#define EXAMPLE_ESP_WIFI_CHANNEL            CONFIG_ESP_WIFI_CHANNEL
#define EXAMPLE_MAX_STA_CONN                CONFIG_ESP_MAX_STA_CONN


#ifdef CONFIG_SENSE_LOG_TO_SD
    // defines for using the sd card
    #define MOUNT_POINT "/sdcard"

    #define SPI_DMA_CHAN SPI_DMA_CH_AUTO
    #define PIN_NUM_MISO CONFIG_SD_PIN_MISO
    #define PIN_NUM_MOSI CONFIG_SD_PIN_MOSI
    #define PIN_NUM_CLK  CONFIG_SD_PIN_CLK
    #define PIN_NUM_CS   CONFIG_SD_PIN_CS

    #define MAX_NAME_LEN 32

    sdmmc_host_t host;
    sdmmc_card_t *card;
    char *file_name;
    FILE *file;

    char *calibration_file_name;
    FILE *calibration_file;

    #define WRITE_FILE_AFTER_RECEIVED_CSI_NUMBER MAX_NUMBER_OF_SAMPLES_KEPT
#endif

// defines for determining how much data to keep for analysis → adjust according to how much RAM is available
#define CSI_RATE                    CONFIG_FRAMES_PER_SECOND // number of expected samples per second
#define TIME_RANGE_TO_KEEP          CONFIG_TIME_RANGE_TO_KEEP // in seconds
#define FFT_EVERY_X_SECONDS         CONFIG_FFT_EVERY_X_SECONDS


#define MRC_PCA_EVERY_X_SECONDS 30
#ifdef CONFIG_MRC_PCA_EVERY_X_SECONDS
    #define MRC_PCA_EVERY_X_SECONDS     CONFIG_MRC_PCA_EVERY_X_SECONDS
#endif

#define NUMBER_SUBCARRIERS 52 // we only use LLTF so get 64 subcarriers
#define MAX_NUMBER_OF_SAMPLES_KEPT  TIME_RANGE_TO_KEEP*CSI_RATE


uint8_t customBaseMAC[] = {0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0D};
uint8_t csi_sender_mac[] = {0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a};


static const char *TAG = "csi receiver"; 
static QueueHandle_t g_csi_info_queue = NULL;
static QueueHandle_t buffer_queue = NULL;
static QueueHandle_t gpio_evt_queue = NULL;

typedef struct
{
    size_t length;
    char *buffer;
}BufferObject;


// defines for UDP broadcast
#define HOST_IP_ADDR "192.168.4.255" // broadcast address for the standard AP-network
#define PORT 8081
#define BROADCAST_RATE 0.5 // rate in Hz -> every 2 seconds  -- probably_removable as logging to udp broadcast exists


#define BREATH_LOWER_FREQUENCY_BOUND    0.1
#define BREATH_UPPER_FREQUENCY_BOUND    0.5
#define HEART_LOWER_FREQUENCY_BOUND     0.75
#define HEART_UPPER_FREQUENCY_BOUND     2.0

// filters for sampling frequency of 50Hz
#define NUMBER_COEFFICIENTS 3
float breathing_bandpass_coefficients[2][NUMBER_COEFFICIENTS] = {{0.024521609249465892, 0.0, -0.024521609249465892}, {1.0, -1.9501864631153991, 0.9509567815010684}}; //  b and a
float heart_bandpass_coefficients[2][NUMBER_COEFFICIENTS] = {{0.07295965726826666, 0.0, -0.07295965726826666}, {1.0, -1.8321199891810072, 0.8540806854634666}};

int fft_count = 1024; // number of samples used for each fft, at a rate of 50 Hz this corresponds to 20.48 seconds of data
// these frequencies have been precomputed for a bin count of 1024 in order to reduce online computations
// if you want to use a different bin count you need to update these too or write code to calculate these on the go
float fft_frequencies[] = {0.0, 0.048828125, 0.09765625, 0.146484375, 0.1953125, 0.244140625, 0.29296875, 0.341796875, 0.390625, 0.439453125, 0.48828125, 0.537109375, 0.5859375, 0.634765625, 0.68359375, 0.732421875, 0.78125, 0.830078125, 0.87890625, 0.927734375, 0.9765625, 1.025390625, 1.07421875, 1.123046875, 1.171875, 1.220703125, 1.26953125, 1.318359375, 1.3671875, 1.416015625, 1.46484375, 1.513671875, 1.5625, 1.611328125, 1.66015625, 1.708984375, 1.7578125, 1.806640625, 1.85546875, 1.904296875, 1.953125, 2.001953125, 2.05078125, 2.099609375, 2.1484375, 2.197265625, 2.24609375, 2.294921875, 2.34375, 2.392578125, 2.44140625, 2.490234375, 2.5390625, 2.587890625, 2.63671875, 2.685546875, 2.734375, 2.783203125, 2.83203125, 2.880859375, 2.9296875, 2.978515625, 3.02734375, 3.076171875, 3.125, 3.173828125, 3.22265625, 3.271484375, 3.3203125, 3.369140625, 3.41796875, 3.466796875, 3.515625, 3.564453125, 3.61328125, 3.662109375, 3.7109375, 3.759765625, 3.80859375, 3.857421875, 3.90625, 3.955078125, 4.00390625, 4.052734375, 4.1015625, 4.150390625, 4.19921875, 4.248046875, 4.296875, 4.345703125, 4.39453125, 4.443359375, 4.4921875, 4.541015625, 4.58984375, 4.638671875, 4.6875, 4.736328125, 4.78515625, 4.833984375, 4.8828125, 4.931640625, 4.98046875, 5.029296875, 5.078125, 5.126953125, 5.17578125, 5.224609375, 5.2734375, 5.322265625, 5.37109375, 5.419921875, 5.46875, 5.517578125, 5.56640625, 5.615234375, 5.6640625, 5.712890625, 5.76171875, 5.810546875, 5.859375, 5.908203125, 5.95703125, 6.005859375, 6.0546875, 6.103515625, 6.15234375, 6.201171875, 6.25, 6.298828125, 6.34765625, 6.396484375, 6.4453125, 6.494140625, 6.54296875, 6.591796875, 6.640625, 6.689453125, 6.73828125, 6.787109375, 6.8359375, 6.884765625, 6.93359375, 6.982421875, 7.03125, 7.080078125, 7.12890625, 7.177734375, 7.2265625, 7.275390625, 7.32421875, 7.373046875, 7.421875, 7.470703125, 7.51953125, 7.568359375, 7.6171875, 7.666015625, 7.71484375, 7.763671875, 7.8125, 7.861328125, 7.91015625, 7.958984375, 8.0078125, 8.056640625, 8.10546875, 8.154296875, 8.203125, 8.251953125, 8.30078125, 8.349609375, 8.3984375, 8.447265625, 8.49609375, 8.544921875, 8.59375, 8.642578125, 8.69140625, 8.740234375, 8.7890625, 8.837890625, 8.88671875, 8.935546875, 8.984375, 9.033203125, 9.08203125, 9.130859375, 9.1796875, 9.228515625, 9.27734375, 9.326171875, 9.375, 9.423828125, 9.47265625, 9.521484375, 9.5703125, 9.619140625, 9.66796875, 9.716796875, 9.765625, 9.814453125, 9.86328125, 9.912109375, 9.9609375, 10.009765625, 10.05859375, 10.107421875, 10.15625, 10.205078125, 10.25390625, 10.302734375, 10.3515625, 10.400390625, 10.44921875, 10.498046875, 10.546875, 10.595703125, 10.64453125, 10.693359375, 10.7421875, 10.791015625, 10.83984375, 10.888671875, 10.9375, 10.986328125, 11.03515625, 11.083984375, 11.1328125, 11.181640625, 11.23046875, 11.279296875, 11.328125, 11.376953125, 11.42578125, 11.474609375, 11.5234375, 11.572265625, 11.62109375, 11.669921875, 11.71875, 11.767578125, 11.81640625, 11.865234375, 11.9140625, 11.962890625, 12.01171875, 12.060546875, 12.109375, 12.158203125, 12.20703125, 12.255859375, 12.3046875, 12.353515625, 12.40234375, 12.451171875, 12.5, 12.548828125, 12.59765625, 12.646484375, 12.6953125, 12.744140625, 12.79296875, 12.841796875, 12.890625, 12.939453125, 12.98828125, 13.037109375, 13.0859375, 13.134765625, 13.18359375, 13.232421875, 13.28125, 13.330078125, 13.37890625, 13.427734375, 13.4765625, 13.525390625, 13.57421875, 13.623046875, 13.671875, 13.720703125, 13.76953125, 13.818359375, 13.8671875, 13.916015625, 13.96484375, 14.013671875, 14.0625, 14.111328125, 14.16015625, 14.208984375, 14.2578125, 14.306640625, 14.35546875, 14.404296875, 14.453125, 14.501953125, 14.55078125, 14.599609375, 14.6484375, 14.697265625, 14.74609375, 14.794921875, 14.84375, 14.892578125, 14.94140625, 14.990234375, 15.0390625, 15.087890625, 15.13671875, 15.185546875, 15.234375, 15.283203125, 15.33203125, 15.380859375, 15.4296875, 15.478515625, 15.52734375, 15.576171875, 15.625, 15.673828125, 15.72265625, 15.771484375, 15.8203125, 15.869140625, 15.91796875, 15.966796875, 16.015625, 16.064453125, 16.11328125, 16.162109375, 16.2109375, 16.259765625, 16.30859375, 16.357421875, 16.40625, 16.455078125, 16.50390625, 16.552734375, 16.6015625, 16.650390625, 16.69921875, 16.748046875, 16.796875, 16.845703125, 16.89453125, 16.943359375, 16.9921875, 17.041015625, 17.08984375, 17.138671875, 17.1875, 17.236328125, 17.28515625, 17.333984375, 17.3828125, 17.431640625, 17.48046875, 17.529296875, 17.578125, 17.626953125, 17.67578125, 17.724609375, 17.7734375, 17.822265625, 17.87109375, 17.919921875, 17.96875, 18.017578125, 18.06640625, 18.115234375, 18.1640625, 18.212890625, 18.26171875, 18.310546875, 18.359375, 18.408203125, 18.45703125, 18.505859375, 18.5546875, 18.603515625, 18.65234375, 18.701171875, 18.75, 18.798828125, 18.84765625, 18.896484375, 18.9453125, 18.994140625, 19.04296875, 19.091796875, 19.140625, 19.189453125, 19.23828125, 19.287109375, 19.3359375, 19.384765625, 19.43359375, 19.482421875, 19.53125, 19.580078125, 19.62890625, 19.677734375, 19.7265625, 19.775390625, 19.82421875, 19.873046875, 19.921875, 19.970703125, 20.01953125, 20.068359375, 20.1171875, 20.166015625, 20.21484375, 20.263671875, 20.3125, 20.361328125, 20.41015625, 20.458984375, 20.5078125, 20.556640625, 20.60546875, 20.654296875, 20.703125, 20.751953125, 20.80078125, 20.849609375, 20.8984375, 20.947265625, 20.99609375, 21.044921875, 21.09375, 21.142578125, 21.19140625, 21.240234375, 21.2890625, 21.337890625, 21.38671875, 21.435546875, 21.484375, 21.533203125, 21.58203125, 21.630859375, 21.6796875, 21.728515625, 21.77734375, 21.826171875, 21.875, 21.923828125, 21.97265625, 22.021484375, 22.0703125, 22.119140625, 22.16796875, 22.216796875, 22.265625, 22.314453125, 22.36328125, 22.412109375, 22.4609375, 22.509765625, 22.55859375, 22.607421875, 22.65625, 22.705078125, 22.75390625, 22.802734375, 22.8515625, 22.900390625, 22.94921875, 22.998046875, 23.046875, 23.095703125, 23.14453125, 23.193359375, 23.2421875, 23.291015625, 23.33984375, 23.388671875, 23.4375, 23.486328125, 23.53515625, 23.583984375, 23.6328125, 23.681640625, 23.73046875, 23.779296875, 23.828125, 23.876953125, 23.92578125, 23.974609375, 24.0234375, 24.072265625, 24.12109375, 24.169921875, 24.21875, 24.267578125, 24.31640625, 24.365234375, 24.4140625, 24.462890625, 24.51171875, 24.560546875, 24.609375, 24.658203125, 24.70703125, 24.755859375, 24.8046875, 24.853515625, 24.90234375, 24.951171875, -25.0, -24.951171875, -24.90234375, -24.853515625, -24.8046875, -24.755859375, -24.70703125, -24.658203125, -24.609375, -24.560546875, -24.51171875, -24.462890625, -24.4140625, -24.365234375, -24.31640625, -24.267578125, -24.21875, -24.169921875, -24.12109375, -24.072265625, -24.0234375, -23.974609375, -23.92578125, -23.876953125, -23.828125, -23.779296875, -23.73046875, -23.681640625, -23.6328125, -23.583984375, -23.53515625, -23.486328125, -23.4375, -23.388671875, -23.33984375, -23.291015625, -23.2421875, -23.193359375, -23.14453125, -23.095703125, -23.046875, -22.998046875, -22.94921875, -22.900390625, -22.8515625, -22.802734375, -22.75390625, -22.705078125, -22.65625, -22.607421875, -22.55859375, -22.509765625, -22.4609375, -22.412109375, -22.36328125, -22.314453125, -22.265625, -22.216796875, -22.16796875, -22.119140625, -22.0703125, -22.021484375, -21.97265625, -21.923828125, -21.875, -21.826171875, -21.77734375, -21.728515625, -21.6796875, -21.630859375, -21.58203125, -21.533203125, -21.484375, -21.435546875, -21.38671875, -21.337890625, -21.2890625, -21.240234375, -21.19140625, -21.142578125, -21.09375, -21.044921875, -20.99609375, -20.947265625, -20.8984375, -20.849609375, -20.80078125, -20.751953125, -20.703125, -20.654296875, -20.60546875, -20.556640625, -20.5078125, -20.458984375, -20.41015625, -20.361328125, -20.3125, -20.263671875, -20.21484375, -20.166015625, -20.1171875, -20.068359375, -20.01953125, -19.970703125, -19.921875, -19.873046875, -19.82421875, -19.775390625, -19.7265625, -19.677734375, -19.62890625, -19.580078125, -19.53125, -19.482421875, -19.43359375, -19.384765625, -19.3359375, -19.287109375, -19.23828125, -19.189453125, -19.140625, -19.091796875, -19.04296875, -18.994140625, -18.9453125, -18.896484375, -18.84765625, -18.798828125, -18.75, -18.701171875, -18.65234375, -18.603515625, -18.5546875, -18.505859375, -18.45703125, -18.408203125, -18.359375, -18.310546875, -18.26171875, -18.212890625, -18.1640625, -18.115234375, -18.06640625, -18.017578125, -17.96875, -17.919921875, -17.87109375, -17.822265625, -17.7734375, -17.724609375, -17.67578125, -17.626953125, -17.578125, -17.529296875, -17.48046875, -17.431640625, -17.3828125, -17.333984375, -17.28515625, -17.236328125, -17.1875, -17.138671875, -17.08984375, -17.041015625, -16.9921875, -16.943359375, -16.89453125, -16.845703125, -16.796875, -16.748046875, -16.69921875, -16.650390625, -16.6015625, -16.552734375, -16.50390625, -16.455078125, -16.40625, -16.357421875, -16.30859375, -16.259765625, -16.2109375, -16.162109375, -16.11328125, -16.064453125, -16.015625, -15.966796875, -15.91796875, -15.869140625, -15.8203125, -15.771484375, -15.72265625, -15.673828125, -15.625, -15.576171875, -15.52734375, -15.478515625, -15.4296875, -15.380859375, -15.33203125, -15.283203125, -15.234375, -15.185546875, -15.13671875, -15.087890625, -15.0390625, -14.990234375, -14.94140625, -14.892578125, -14.84375, -14.794921875, -14.74609375, -14.697265625, -14.6484375, -14.599609375, -14.55078125, -14.501953125, -14.453125, -14.404296875, -14.35546875, -14.306640625, -14.2578125, -14.208984375, -14.16015625, -14.111328125, -14.0625, -14.013671875, -13.96484375, -13.916015625, -13.8671875, -13.818359375, -13.76953125, -13.720703125, -13.671875, -13.623046875, -13.57421875, -13.525390625, -13.4765625, -13.427734375, -13.37890625, -13.330078125, -13.28125, -13.232421875, -13.18359375, -13.134765625, -13.0859375, -13.037109375, -12.98828125, -12.939453125, -12.890625, -12.841796875, -12.79296875, -12.744140625, -12.6953125, -12.646484375, -12.59765625, -12.548828125, -12.5, -12.451171875, -12.40234375, -12.353515625, -12.3046875, -12.255859375, -12.20703125, -12.158203125, -12.109375, -12.060546875, -12.01171875, -11.962890625, -11.9140625, -11.865234375, -11.81640625, -11.767578125, -11.71875, -11.669921875, -11.62109375, -11.572265625, -11.5234375, -11.474609375, -11.42578125, -11.376953125, -11.328125, -11.279296875, -11.23046875, -11.181640625, -11.1328125, -11.083984375, -11.03515625, -10.986328125, -10.9375, -10.888671875, -10.83984375, -10.791015625, -10.7421875, -10.693359375, -10.64453125, -10.595703125, -10.546875, -10.498046875, -10.44921875, -10.400390625, -10.3515625, -10.302734375, -10.25390625, -10.205078125, -10.15625, -10.107421875, -10.05859375, -10.009765625, -9.9609375, -9.912109375, -9.86328125, -9.814453125, -9.765625, -9.716796875, -9.66796875, -9.619140625, -9.5703125, -9.521484375, -9.47265625, -9.423828125, -9.375, -9.326171875, -9.27734375, -9.228515625, -9.1796875, -9.130859375, -9.08203125, -9.033203125, -8.984375, -8.935546875, -8.88671875, -8.837890625, -8.7890625, -8.740234375, -8.69140625, -8.642578125, -8.59375, -8.544921875, -8.49609375, -8.447265625, -8.3984375, -8.349609375, -8.30078125, -8.251953125, -8.203125, -8.154296875, -8.10546875, -8.056640625, -8.0078125, -7.958984375, -7.91015625, -7.861328125, -7.8125, -7.763671875, -7.71484375, -7.666015625, -7.6171875, -7.568359375, -7.51953125, -7.470703125, -7.421875, -7.373046875, -7.32421875, -7.275390625, -7.2265625, -7.177734375, -7.12890625, -7.080078125, -7.03125, -6.982421875, -6.93359375, -6.884765625, -6.8359375, -6.787109375, -6.73828125, -6.689453125, -6.640625, -6.591796875, -6.54296875, -6.494140625, -6.4453125, -6.396484375, -6.34765625, -6.298828125, -6.25, -6.201171875, -6.15234375, -6.103515625, -6.0546875, -6.005859375, -5.95703125, -5.908203125, -5.859375, -5.810546875, -5.76171875, -5.712890625, -5.6640625, -5.615234375, -5.56640625, -5.517578125, -5.46875, -5.419921875, -5.37109375, -5.322265625, -5.2734375, -5.224609375, -5.17578125, -5.126953125, -5.078125, -5.029296875, -4.98046875, -4.931640625, -4.8828125, -4.833984375, -4.78515625, -4.736328125, -4.6875, -4.638671875, -4.58984375, -4.541015625, -4.4921875, -4.443359375, -4.39453125, -4.345703125, -4.296875, -4.248046875, -4.19921875, -4.150390625, -4.1015625, -4.052734375, -4.00390625, -3.955078125, -3.90625, -3.857421875, -3.80859375, -3.759765625, -3.7109375, -3.662109375, -3.61328125, -3.564453125, -3.515625, -3.466796875, -3.41796875, -3.369140625, -3.3203125, -3.271484375, -3.22265625, -3.173828125, -3.125, -3.076171875, -3.02734375, -2.978515625, -2.9296875, -2.880859375, -2.83203125, -2.783203125, -2.734375, -2.685546875, -2.63671875, -2.587890625, -2.5390625, -2.490234375, -2.44140625, -2.392578125, -2.34375, -2.294921875, -2.24609375, -2.197265625, -2.1484375, -2.099609375, -2.05078125, -2.001953125, -1.953125, -1.904296875, -1.85546875, -1.806640625, -1.7578125, -1.708984375, -1.66015625, -1.611328125, -1.5625, -1.513671875, -1.46484375, -1.416015625, -1.3671875, -1.318359375, -1.26953125, -1.220703125, -1.171875, -1.123046875, -1.07421875, -1.025390625, -0.9765625, -0.927734375, -0.87890625, -0.830078125, -0.78125, -0.732421875, -0.68359375, -0.634765625, -0.5859375, -0.537109375, -0.48828125, -0.439453125, -0.390625, -0.341796875, -0.29296875, -0.244140625, -0.1953125, -0.146484375, -0.09765625, -0.048828125};

unsigned previous_fft_timestamp = 0; 
unsigned previous_mrc_pca_timestamp = 0; 


float (*amplitude_array)[MAX_NUMBER_OF_SAMPLES_KEPT][NUMBER_SUBCARRIERS];
unsigned timestamp_array[MAX_NUMBER_OF_SAMPLES_KEPT];
static int16_t current_first_element = -1;
static int16_t current_last_element = -1;

float filtered_breath[MAX_NUMBER_OF_SAMPLES_KEPT];
float filtered_heart[MAX_NUMBER_OF_SAMPLES_KEPT];

float amplitude_mean;
float previous_amplitude_hat[NUMBER_SUBCARRIERS];
float sti_array[MAX_NUMBER_OF_SAMPLES_KEPT];

FunctionList broadcast_functions;

BandpassIIRFilter breathing_filter;
BandpassIIRFilter heart_filter;

MAC_struct MAC_breath;
Features breath_features;
POI_List breath_pois;

MAC_struct MAC_heart;
Features heart_features;
POI_List heart_pois;

bool first_run = true;
float MRC_ratios_breath[NUMBER_SUBCARRIERS];
float MRC_ratios_heart[NUMBER_SUBCARRIERS];
float MRC_scalar_breath = 1;
float MRC_scalar_heart = 1;

char activity_id = 0;
bool running_calibration = false;

ListChar id_list;
float sti;
ListFloat sti_list;

float t_presence = -1;
float t_small_movement = -1;
float t_large_movement = -1;

float f_presence = -1;
float f_small_movement = -1;
float f_large_movement = -1;

int sleep_stage = -1;

bool presence_detected = false;
bool small_movement_detected = false;
bool large_movement_detected = false;

bool presence_detected_previously = false;
bool large_movement_detected_previously = false;
bool ran_mrc_in_the_beginning = false;

bool button_pressed = false;

#ifdef CONFIG_MRC_PCA_ON_TIMER
    bool do_not_run_mrc_on_timer = false;
#endif

#ifndef CONFIG_MRC_PCA_ON_TIMER
    bool do_not_run_mrc_on_timer = true;
#endif

nvs_handle_t my_handle;

#ifdef CONFIG_RUN_INFERENCE
void run_sleep_stage_classification(){
    // create the buffer with the features in the correct order as input to the model
    float model_input[42];
    // sti, detected motion / presence, breath features, heart features
    model_input[0] = sti;
    model_input[1] = presence_detected;
    model_input[2] = small_movement_detected;
    model_input[3] = large_movement_detected;
    
    // breath features
    model_input[4] = breath_features.instantaneous_peak_rate;
    model_input[5] = breath_features.instantaneous_valley_rate;
    model_input[6] = breath_features.mean_peak_rate_over_window.current_mean;
    model_input[7] = breath_features.mean_valley_rate_over_window.current_mean;
    model_input[8] = breath_features.fft_rate_over_window;
    model_input[9] = breath_features.variance_of_peak_rate_in_window;
    model_input[10] = breath_features.variance_of_valley_rate_in_window;

    model_input[11] = breath_features.mean_up_stroke_length.current_mean;
    model_input[12] = breath_features.mean_down_stroke_length.current_mean;
    model_input[13] = breath_features.up_stroke_length_variance;
    model_input[14] = breath_features.down_stroke_length_variance;
    model_input[15] = breath_features.up_to_down_length_ratio;
    model_input[16] = breath_features.fractional_up_stroke_time;

    model_input[17] = breath_features.mean_up_stroke_amplitude.current_mean;
    model_input[18] = breath_features.mean_down_stroke_amplitude.current_mean;
    model_input[19] = breath_features.up_stroke_amplitude_variance;
    model_input[20] = breath_features.down_stroke_amplitude_variance;
    model_input[21] = breath_features.up_to_down_amplitude_ratio;
    model_input[22] = breath_features.fractional_up_stroke_amplitude;

    // heart features
    model_input[23] = heart_features.instantaneous_peak_rate;
    model_input[24] = heart_features.instantaneous_valley_rate;
    model_input[25] = heart_features.mean_peak_rate_over_window.current_mean;
    model_input[26] = heart_features.mean_valley_rate_over_window.current_mean;
    model_input[27] = heart_features.fft_rate_over_window;
    model_input[28] = heart_features.variance_of_peak_rate_in_window;
    model_input[29] = heart_features.variance_of_valley_rate_in_window;

    model_input[30] = heart_features.mean_up_stroke_length.current_mean;
    model_input[31] = heart_features.mean_down_stroke_length.current_mean;
    model_input[32] = heart_features.up_stroke_length_variance;
    model_input[33] = heart_features.down_stroke_length_variance;
    model_input[34] = heart_features.up_to_down_length_ratio;
    model_input[35] = heart_features.fractional_up_stroke_time;

    model_input[36] = heart_features.mean_up_stroke_amplitude.current_mean;
    model_input[37] = heart_features.mean_down_stroke_amplitude.current_mean;
    model_input[38] = heart_features.up_stroke_amplitude_variance;
    model_input[39] = heart_features.down_stroke_amplitude_variance;
    model_input[40] = heart_features.up_to_down_amplitude_ratio;
    model_input[41] = heart_features.fractional_up_stroke_amplitude;

    // run inference
    sleep_stage = run_inference(&model_input);
    ESP_LOGI(TAG, "sleep stage classification returned %d", sleep_stage);
}

static void run_inference_task(){
    vTaskDelay((1000*CONFIG_START_INFERENCE_AFTER_X_SECONDS) / portTICK_PERIOD_MS);
    while(true){
        ESP_LOGI(TAG, "running inference");
        run_sleep_stage_classification();
        vTaskDelay((CONFIG_RUN_INFERENCE_EVERY_X_SECONDS*1000) / portTICK_PERIOD_MS);
    }
}
#endif


// -- probably_removable
size_t print_sti_to_buffer(char *buffer, size_t len){
    // sti values        
    // if(current_last_element >= current_first_element)
    // {
    //     len += sprintf(buffer + len, "[%f", sti_array[current_first_element]);
    //     for (int i = current_first_element+1; i <= current_last_element; i++) 
    //     {
    //         len += sprintf(buffer + len, ",%f", sti_array[i]);
    //     }
    //     len += sprintf(buffer + len, "]\n");
    // }
    // else
    // {
    //     len += sprintf(buffer + len, "[%f", sti_array[current_first_element]);
    //     for (int i = current_first_element+1; i < MAX_NUMBER_OF_SAMPLES_KEPT; i++) {
    //         len += sprintf(buffer + len, ",%f", sti_array[i]);
    //     }
    //     for (int i = 0+1; i <= current_last_element; i++) {
    //         len += sprintf(buffer + len, ",%f", sti_array[i]);
    //     }
    //     len += sprintf(buffer + len, "]\n");
    // }

    return len;
}

// -- probably_removable
size_t print_heart_rate_to_buffer(char *buffer, size_t len){ //TODO: currently only dummy data
    len += sprintf(buffer + len, "65");
    return len;
}

// -- probably_removable
size_t print_breathing_rate_to_buffer(char *buffer, size_t len){ //TODO: currently only dummy data
    len += sprintf(buffer + len, "18");
    return len;
}

// -- probably_removable
void setup_broadcast_messages(){
    // define what goes into the udp broadcast messages
    create_function_list(&broadcast_functions, 5);
    
    // heart rate
    append_to_function_list(&broadcast_functions, print_heart_rate_to_buffer);

    append_to_function_list(&broadcast_functions, print_breathing_rate_to_buffer);

    // sti
    append_to_function_list(&broadcast_functions, print_sti_to_buffer);

}

// -- probably_removable
static void udp_client_task(void *pvParameters)
{
    int addr_family = 0;
    int ip_protocol = 0;
    vTaskDelay(10000 / portTICK_PERIOD_MS);

    while (1) {

        struct sockaddr_in dest_addr;
        dest_addr.sin_addr.s_addr = inet_addr(HOST_IP_ADDR);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(PORT);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;


        int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            break;
        }

        // Set timeout
        struct timeval timeout;
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;
        setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);

        ESP_LOGI(TAG, "Socket created, sending to %s:%d", HOST_IP_ADDR, PORT);


        while (1) 
        {
            // char *buffer = malloc_or_die(8 * 1024);
            // size_t len = 0;
            // // add a message to the start
            // len += sprintf(buffer + len, "Message from ESP32;");


            // // use the broadcast functions to generate the payload
            // for(int i=0; i<broadcast_functions.elements-1; i++)
            // {
            //     len = (*broadcast_functions.list[i])(buffer, len);
            //     len += sprintf(buffer + len, ";"); // separate with semicolon
            // }
            // len = (*broadcast_functions.list[broadcast_functions.elements-1])(buffer, len);
            // create payload
            BufferObject *buffer_object = NULL;
            while (xQueueReceive(buffer_queue, &buffer_object, portMAX_DELAY)) {
            
                int err = sendto(sock, buffer_object->buffer, buffer_object->length, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
                if (err < 0) {
                    ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                    break;
                }
                // ESP_LOGI(TAG, "Message sent");
                free(buffer_object);
            }
       
            // vTaskDelay((1000/BROADCAST_RATE) / portTICK_PERIOD_MS);
        }

        if (sock != -1) 
        {
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }
    }
    vTaskDelete(NULL);
}

static void udp_calibration_task(void *pvParameters)
{
    char rx_buffer[128];
    char *buffer = malloc_or_die(1024);
    size_t length = 0;
    char host_ip[] = HOST_IP_ADDR;
    int addr_family = 0;
    int ip_protocol = 0;

    while (1) {

        struct sockaddr_in dest_addr;
        dest_addr.sin_addr.s_addr = inet_addr(HOST_IP_ADDR);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(PORT);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;

        int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            break;
        }

        // Set timeout
        struct timeval timeout;
        timeout.tv_sec = 30;
        timeout.tv_usec = 0;
        int broadcast = 1;
        setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
        if (bind(sock, (const struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
            ESP_LOGE(TAG, "Error occurred during binding: errno %d", errno);
            break;
        }

        while (1) {

            struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
            socklen_t socklen = sizeof(source_addr);
            //int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&dest_addr, &socklen);

            int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);

            // Error occurred during receiving
            if (len < 0) {
                ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
                break;
            }
            // Data received
            else {
                rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
                ESP_LOGI(TAG, "Received %d bytes from %s:", len, host_ip);
                ESP_LOGI(TAG, "%s", rx_buffer);
                if (strncmp(rx_buffer, "-1", 2) == 0) {
                    ESP_LOGI(TAG, "Starting calibration");
                    create_list_char(&id_list, 15000);
                    create_list_float(&sti_list, 15000);
                    activity_id = 0;
                    running_calibration = true;                    
                }

                if (strncmp(rx_buffer, "-2", 2) == 0) {
                    ESP_LOGI(TAG, "Stopping calibration");
                    if(running_calibration){
                        running_calibration = false;
                        t_presence = -1;
                        t_small_movement = -1;
                        t_large_movement = -1;
                        f_presence = -1;
                        f_small_movement = -1;
                        f_large_movement = -1;
                        get_best_thresholds(sti_list.list, id_list.list, sti_list.elements, &t_presence, &t_small_movement, &t_large_movement, &f_presence, &f_small_movement, &f_large_movement);
                        ESP_LOGI(TAG, "got thresholds %f, %f, %f", t_presence, t_small_movement, t_large_movement);
                        free_list_char(&id_list);
                        free_list_float(&sti_list);

                        // send broadcast message with the new thresholds
                        length = 0;
                        length += sprintf(buffer + length, "thres_pres %f, thres_s_mov %f, thresh_l_mov %f", t_presence, t_small_movement, t_large_movement);
                        
                        BufferObject *q_data = malloc_or_die(len + sizeof(size_t));
                        q_data->length = length;
                        q_data->buffer = malloc_or_die(length);
                        memcpy(q_data->buffer, buffer, length);

                        if (!buffer_queue || xQueueSend(buffer_queue, &q_data, 0) == pdFALSE) {
                            ESP_LOGW(TAG, "buffer_queue full");
                            free(q_data);
                        }

                        // Write threshold values to NVS
                        uint32_t int_t_presence = 0;
                        uint32_t int_t_small_movement = 0;
                        uint32_t int_t_large_movement = 0;
                        memcpy(&int_t_presence, &t_presence, sizeof(uint32_t));
                        memcpy(&int_t_small_movement, &t_small_movement, sizeof(uint32_t));
                        memcpy(&int_t_large_movement, &t_large_movement, sizeof(uint32_t));

                        ESP_LOGI(TAG, "Updating threshold values in NVS ... ");
                        esp_err_t err = nvs_set_u32(my_handle, "thresh_pres", int_t_presence);
                        if(err != ESP_OK){
                            ESP_LOGW(TAG, "failed");
                        }
                        err = nvs_set_u32(my_handle, "thresh_s_mov", int_t_small_movement);
                        if(err != ESP_OK){
                            ESP_LOGW(TAG, "failed");
                        }                    err = nvs_set_u32(my_handle, "thresh_l_mov", int_t_large_movement);
                        if(err != ESP_OK){
                            ESP_LOGW(TAG, "failed");
                        }

                        // Commit written values
                        ESP_LOGI(TAG, "Committing updates in NVS ... ");
                        err = nvs_commit(my_handle);
                        if(err != ESP_OK){
                            ESP_LOGW(TAG, "failed");
                        }
                    }else{
                        ESP_LOGI(TAG, "Calibration was not running");
                        // send broadcast message with the new thresholds
                        length = 0;
                        length += sprintf(buffer + length, "Calibration was not running, keeping current thresholds: thres_pres %f, thres_s_mov %f, thresh_l_mov %f\n", t_presence, t_small_movement, t_large_movement);
                        
                        BufferObject *q_data = malloc_or_die(len + sizeof(size_t));
                        q_data->length = length;
                        q_data->buffer = malloc_or_die(length);
                        memcpy(q_data->buffer, buffer, length);

                        if (!buffer_queue || xQueueSend(buffer_queue, &q_data, 0) == pdFALSE) {
                            ESP_LOGW(TAG, "buffer_queue full");
                            free(q_data);
                        }
                    }
                }

                if (strncmp(rx_buffer, "0", 1) == 0) {
                    ESP_LOGI(TAG, "Start activity: transition");
                    activity_id = 0;
                }

                if (strncmp(rx_buffer, "1", 1) == 0) {
                    ESP_LOGI(TAG, "Start activity: no presence");
                    activity_id = 1;
                }

                if (strncmp(rx_buffer, "2", 1) == 0) {
                    ESP_LOGI(TAG, "Start activity: presence");
                    activity_id = 2;
                }

                if (strncmp(rx_buffer, "3", 1) == 0) {
                    ESP_LOGI(TAG, "Start activity: small movement");
                    activity_id = 3;
                }

                if (strncmp(rx_buffer, "4", 1) == 0) {
                    ESP_LOGI(TAG, "Start activity: large movement");
                    activity_id = 4;
                }
            }

            vTaskDelay(10 / portTICK_PERIOD_MS);
        }

        if (sock != -1) {
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }
    }
    vTaskDelete(NULL);
}

// -- probably_removable alhtough it might still be nice for debugging
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) 
    {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } 
    else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) 
    {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
}

// -- probably_removable
void analysis_init(void){
    bandpass_filter_initialize(&breathing_filter, breathing_bandpass_coefficients[0], breathing_bandpass_coefficients[1], NUMBER_COEFFICIENTS);
    bandpass_filter_initialize(&heart_filter, heart_bandpass_coefficients[0], heart_bandpass_coefficients[1], NUMBER_COEFFICIENTS);
}


float mrc_pca(float *MRC_ratios, float data[][NUMBER_SUBCARRIERS], int data_length, int data_current_last_index, int use_last_n_elements, float lower_frequency_bound, float upper_frequency_bound, float *bandpass_coefficients_b, float *bandpass_coefficients_a){
    // calculate psd

    // create the FFT config structure
    fft_config_t *real_fft_plan = fft_init(use_last_n_elements, FFT_REAL, FFT_FORWARD, NULL, NULL);

    // create array passed to the PCA
    float *x_pca = malloc_or_die(use_last_n_elements * sizeof(float));

    float *x_psd = malloc_or_die(use_last_n_elements/2 * sizeof(float));

    float MRC_scalar = 0;
    
    // for all subcarriers - calculate the MCR ratios
    for(int subcarrier = 0; subcarrier<NUMBER_SUBCARRIERS; subcarrier++){
        
        // calculate the mean
        int data_index = get_next_index(data_current_last_index, data_length);
        float mean = 0;
        int index=use_last_n_elements-1;
        while (index >= 0)
        {
            data_index = get_previous_index(data_index, data_length);
            mean += data[data_index][subcarrier]/use_last_n_elements; 
            index--;
        }

        data_index = get_next_index(data_current_last_index, data_length);

        // fill it from the end to the front
        index=use_last_n_elements-1;
        while (index >= 0)
        {
            data_index = get_previous_index(data_index, data_length);
            x_pca[index] = data[data_index][subcarrier]; 
            real_fft_plan->input[index] = data[data_index][subcarrier] - mean; 
            index--;
        }

        // Execute transformation
        fft_execute(real_fft_plan);

        // get the signal energy within the range of normal respiratory rates
        // get the signal energy outside of that range
        float signal_energy = 0;
        float noise_energy = 0;      
        for (int i = 0 ; i < use_last_n_elements/2 ; i++) {
            // calculate the amplitude
            x_psd[i] = powf(sqrtf(real_fft_plan->output[i*2] * real_fft_plan->output[i*2] + real_fft_plan->output[i*2+1] * real_fft_plan->output[i*2+1]), 2.0)/use_last_n_elements;
            
            if(fft_frequencies[i]>lower_frequency_bound && fft_frequencies[i]<upper_frequency_bound){
                signal_energy += x_psd[i]/use_last_n_elements;
            }else{
                noise_energy += x_psd[i]/use_last_n_elements;
            }
        }

        // calculate the ratio
        float SNR = signal_energy / noise_energy;
        if(noise_energy == 0){
            SNR = 0;
        }
        // store it in the ratio array
        MRC_ratios[subcarrier] = SNR;
        MRC_scalar += SNR;

        // apply bandpass filter to the signal multiplied with the SNR and then compute the first PC
        BandpassIIRFilter filter;
        bandpass_filter_initialize(&filter, bandpass_coefficients_b, bandpass_coefficients_a, NUMBER_COEFFICIENTS);
        for(int i=0; i<use_last_n_elements; i++){
            bandpass_filter_apply(&filter, x_pca[i]*SNR);
            x_pca[i] = filter.out;
        }

        // perform PCA
        float pc_positive = is_first_pc_positive(x_pca, use_last_n_elements);
        // add the sign of the first principal component to each MRC ratio
        MRC_ratios[subcarrier] *= pc_positive;
        
        // ESP_LOGI(TAG, "end of subcarrier for, value %f, signal %f, noise %f, pc_positive %f", MRC_ratios[subcarrier], signal_energy, noise_energy, pc_positive);
    }
    ESP_LOGI(TAG, "completed mrc-pca, MRC scalar %f", MRC_scalar);
    fft_destroy(real_fft_plan);
    free(x_pca);
    return MRC_scalar;
}

float fft_rate_estimation(float *data, int data_length, int data_current_last_index, int use_last_n_elements){
    // Create the FFT config structure
    fft_config_t *real_fft_plan = fft_init(use_last_n_elements, FFT_REAL, FFT_FORWARD, NULL, NULL);

    float *x_amp = malloc_or_die(use_last_n_elements/2 * sizeof(float));

    // calculate the mean
    int data_index = get_next_index(data_current_last_index, data_length);
    float mean = 0;
    int index=use_last_n_elements-1;
    while (index >= 0)
    {
        data_index = get_previous_index(data_index, data_length);
        mean += data[data_index]/use_last_n_elements; 
        index--;
    }

    data_index = get_next_index(data_current_last_index, data_length);

    // fill it from the end to the front
    index=use_last_n_elements-1;
    while (index >= 0)
    {
        data_index = get_previous_index(data_index, data_length);
        real_fft_plan->input[index] = data[data_index] - mean; 
        index--;
    }

    // Execute transformation
    fft_execute(real_fft_plan);
    
    for (int i = 0 ; i < use_last_n_elements/2 ; i++) {
       x_amp[i] = sqrtf(real_fft_plan->output[i*2] * real_fft_plan->output[i*2] + real_fft_plan->output[i*2+1] * real_fft_plan->output[i*2+1]);
    }

    // get the index of the maximum
    int maximum_index = 0;
    float current_maximum = -1;

    for(int i=1; i<use_last_n_elements/2; i++){
        if(x_amp[i]>current_maximum){
            current_maximum = x_amp[i];
            maximum_index = i;
        }
    }
    if(fabsf(real_fft_plan->output[1])>current_maximum){
        current_maximum = fabsf(real_fft_plan->output[1]);
        maximum_index = use_last_n_elements/2;
    }

    // get the frequency at the maximum index
    float max_frequency = fft_frequencies[maximum_index];

    fft_destroy(real_fft_plan);
    free(x_amp);
    return max_frequency*60;
}


static void csi_processing_task(void *arg)
{
    wifi_csi_info_t *info = NULL;

    char *buffer = malloc_or_die(8 * 1024);
    char *calibration_buffer = malloc_or_die(4 * 1024);
    static uint32_t count = 0;

#ifdef CONFIG_SENSE_LOG_TO_SD
    if(file==NULL)
    {
        ESP_LOGI(TAG, "Opening file initially");
        // Check if destination file exists before renaming
        file_name = malloc_or_die(MAX_NAME_LEN);
        sprintf(file_name, MOUNT_POINT"/csi.csv");

        calibration_file_name = malloc_or_die(MAX_NAME_LEN);
        sprintf(calibration_file_name, MOUNT_POINT"/cal.csv");

        struct stat st;
        int16_t i = 0;
        while (stat(file_name, &st) == 0) {
            // change the file name
            sprintf(calibration_file_name, MOUNT_POINT"/cal_%d.csv", i);
            sprintf(file_name, MOUNT_POINT"/csi_%d.csv", i++);
        }
        file = fopen(file_name, "a");
        ESP_LOGI(TAG, "Opening file initially cal name %s, file name %s", calibration_file_name, file_name);
        calibration_file = fopen(calibration_file_name, "a");
    }
    static uint32_t write_count = 0;
#endif

    while (xQueueReceive(g_csi_info_queue, &info, portMAX_DELAY)) {
        wifi_pkt_rx_ctrl_t *rx_ctrl = &info->rx_ctrl;

        float amplitude[NUMBER_SUBCARRIERS];
        // float phase[NUMBER_SUBCARRIERS];
        
        // calculate amplitude and phase
        float sum = 0;
        for (int i=0 ; i < 64; i++)
        {
            // skip null and pilot (unusable) subcarriers
            if(i==0 || i==7 || i==21 || i==29 || i==30 || i==31 || i==32 || i==33 || i==34 || i==35 || i==43 || i==57 ){
                continue;
            }
            amplitude[i] = sqrt(pow(info->buf[i*2 + 0], 2) + pow(info->buf[i*2 + 1], 2));
            sum = sum + amplitude[i];
            // phase[i] = atan2(info->buf[i*2 + 0],info->buf[i*2 + 1]);
        }

        amplitude_mean = sum / NUMBER_SUBCARRIERS;

        // calculate amplitude deviation
        float amplitude_deviation[NUMBER_SUBCARRIERS];
        sum = 0;
        for (int i=0; i<NUMBER_SUBCARRIERS; i++){
            amplitude_deviation[i] = amplitude[i] - amplitude_mean;
            sum = sum + pow(amplitude_deviation[i], 2);
        }
        float amplitude_sigma = sqrt((sum/NUMBER_SUBCARRIERS));

        float amplitude_hat[NUMBER_SUBCARRIERS];
        for (int i=0; i<NUMBER_SUBCARRIERS; i++){
            amplitude_hat[i] = amplitude_deviation[i] / amplitude_sigma;
        }

        if(count == 0){
            // cannot calculate sti in first iteration through the code, as there's no previous_amplitude_hat yet
            memcpy(previous_amplitude_hat, amplitude_hat, sizeof(amplitude_hat));
            sti = -1;
        }
        else{
            // calculate STI
            sum = 0;
            for (int i=0; i<NUMBER_SUBCARRIERS; i++){
                sum = sum + pow((previous_amplitude_hat[i] - amplitude_hat[i]), 2);
            }
            sti = sqrt(sum);
        }

        // compare sti value to thresholds to detect presence / movement
        presence_detected_previously = presence_detected;
        large_movement_detected_previously = large_movement_detected;
        if(sti > t_presence){
            presence_detected = true;
        }else{
            presence_detected = false;
        }
        if(sti > t_small_movement){
            small_movement_detected = true;
        }else{
            small_movement_detected = false;
        }
        if(sti > t_large_movement){
            large_movement_detected = true;
        }else{
            large_movement_detected = false;
        }
        
        // append amplitude to corresponding queue / array
        if(current_last_element+1 < MAX_NUMBER_OF_SAMPLES_KEPT)
        {
            current_last_element = current_last_element+1;           
        }
        else
        {
            current_last_element = 0;  
        }
        if(current_first_element == current_last_element){
            current_first_element = current_first_element+1;
        }
        if(current_first_element == -1 || current_first_element == MAX_NUMBER_OF_SAMPLES_KEPT){
            current_first_element = 0;
        }
        if(!first_run){
            MAC_struct_check_if_indices_become_invalid(&MAC_breath, current_last_element);
        }

        sti_array[current_last_element] = sti;
        memcpy((*amplitude_array)[current_last_element], amplitude, sizeof(amplitude));
        timestamp_array[current_last_element] = rx_ctrl->timestamp;


        // calculate fused amplitude
        float fused_amplitude_breath = 0;
        float fused_amplitude_heart = 0;
        for(int i=0; i<NUMBER_SUBCARRIERS; i++){
            fused_amplitude_breath += (amplitude[i] * (MRC_ratios_breath[i]/MRC_scalar_breath));
            fused_amplitude_heart += (amplitude[i] * (MRC_ratios_heart[i]/MRC_scalar_heart));
        }

        // bandpass filter the fused amplitude
        bandpass_filter_apply(&breathing_filter, fused_amplitude_breath);
        bandpass_filter_apply(&heart_filter, fused_amplitude_heart);
        // save the filtered amplitude
        filtered_breath[current_last_element] = breathing_filter.out;
        filtered_heart[current_last_element] = heart_filter.out;


        // MAC calculations
        // get T by using FFT on the filtered waveform
        float T_breath = 3; // period in seconds 3 corresponds to 20 bpm
        float T_heart = 1; // period in seconds 1 corresponds to 60 bpm

#ifdef CONFIG_MRC_PCA_ON_NEW_PRESENCE
        // perform MRC-PCA if new presence has been detected
        if(!presence_detected_previously && presence_detected && timestamp_array[current_last_element]-previous_mrc_pca_timestamp >= CONFIG_MRC_PCA_MINIMUM_INTERVAL*SECONDS_TO_MICROSECONDS){
            ESP_LOGI(TAG, "calculating MRC-PCA after new presence");
            MRC_scalar_breath = mrc_pca(MRC_ratios_breath, amplitude_array, MAX_NUMBER_OF_SAMPLES_KEPT, current_last_element, fft_count, BREATH_LOWER_FREQUENCY_BOUND, BREATH_UPPER_FREQUENCY_BOUND, breathing_bandpass_coefficients[0], breathing_bandpass_coefficients[1]);
            MRC_scalar_heart = mrc_pca(MRC_ratios_heart, amplitude_array, MAX_NUMBER_OF_SAMPLES_KEPT, current_last_element, fft_count, HEART_LOWER_FREQUENCY_BOUND, HEART_UPPER_FREQUENCY_BOUND, heart_bandpass_coefficients[0], heart_bandpass_coefficients[1]);
            previous_mrc_pca_timestamp = timestamp_array[current_last_element];
        }
#endif

#ifdef CONFIG_MRC_PCA_AFTER_MOVEMENT
        // perform MRC-PCA after large movement has been detected
        if(large_movement_detected_previously && !large_movement_detected && timestamp_array[current_last_element]-previous_mrc_pca_timestamp >= CONFIG_MRC_PCA_MINIMUM_INTERVAL*SECONDS_TO_MICROSECONDS){
            ESP_LOGI(TAG, "calculating MRC-PCA after large movement");
            MRC_scalar_breath = mrc_pca(MRC_ratios_breath, amplitude_array, MAX_NUMBER_OF_SAMPLES_KEPT, current_last_element, fft_count, BREATH_LOWER_FREQUENCY_BOUND, BREATH_UPPER_FREQUENCY_BOUND, breathing_bandpass_coefficients[0], breathing_bandpass_coefficients[1]);
            MRC_scalar_heart = mrc_pca(MRC_ratios_heart, amplitude_array, MAX_NUMBER_OF_SAMPLES_KEPT, current_last_element, fft_count, HEART_LOWER_FREQUENCY_BOUND, HEART_UPPER_FREQUENCY_BOUND, heart_bandpass_coefficients[0], heart_bandpass_coefficients[1]);
            previous_mrc_pca_timestamp = timestamp_array[current_last_element];
        }
#endif


        // perform MRC-PCA every X seconds
        if(((do_not_run_mrc_on_timer && !ran_mrc_in_the_beginning) || !do_not_run_mrc_on_timer) && timestamp_array[current_last_element]-previous_mrc_pca_timestamp >= MRC_PCA_EVERY_X_SECONDS*SECONDS_TO_MICROSECONDS){
            MRC_scalar_breath = mrc_pca(MRC_ratios_breath, amplitude_array, MAX_NUMBER_OF_SAMPLES_KEPT, current_last_element, fft_count, BREATH_LOWER_FREQUENCY_BOUND, BREATH_UPPER_FREQUENCY_BOUND, breathing_bandpass_coefficients[0], breathing_bandpass_coefficients[1]);
            MRC_scalar_heart = mrc_pca(MRC_ratios_heart, amplitude_array, MAX_NUMBER_OF_SAMPLES_KEPT, current_last_element, fft_count, HEART_LOWER_FREQUENCY_BOUND, HEART_UPPER_FREQUENCY_BOUND, heart_bandpass_coefficients[0], heart_bandpass_coefficients[1]);
            previous_mrc_pca_timestamp = timestamp_array[current_last_element];
            ran_mrc_in_the_beginning = true;
        }

        // get fft rate estimation every x seconds
        if(timestamp_array[current_last_element]-previous_fft_timestamp >= FFT_EVERY_X_SECONDS*SECONDS_TO_MICROSECONDS){
            T_breath = fft_rate_estimation(&filtered_breath, MAX_NUMBER_OF_SAMPLES_KEPT, current_last_element, fft_count);
            T_heart = fft_rate_estimation(&filtered_heart, MAX_NUMBER_OF_SAMPLES_KEPT, current_last_element, fft_count);
            breath_features.fft_rate_over_window = T_breath;
            heart_features.fft_rate_over_window = T_breath;

            previous_fft_timestamp = timestamp_array[current_last_element];
            ESP_LOGI(TAG, "fft rate estimated, breathing rate %f, heart rate %f, current timestamp %d", T_breath, T_heart, timestamp_array[current_last_element]);
        }

        // MAC for breathing
        int T_timestamp = 60/T_breath * SECONDS_TO_MICROSECONDS;

        bool breath_found_poi = false;
        POI *new_breath_poi;

        bool breath_found_up_intercept = false;
        bool breath_found_down_intercept = false;
        bool found_peak = false;
        bool found_valley = false;
        int peak_index_difference_from_last_intercept = 0;
        int valley_index_difference_from_last_intercept = 0;

        unsigned MAC_window_size = 2 * T_breath * SECONDS_TO_MICROSECONDS; // calculate from T -> timestamps are in microseconds
        unsigned window_size = TIME_RANGE_TO_KEEP * SECONDS_TO_MICROSECONDS;

        // update the MAC
        // use the filtered amplitude for the MAC
        if(first_run){
            MAC_breath.current_last_intercept = -1;
            MAC_breath.current_first_intercept = -1;
            MAC_breath.array_len = 20;
            MAC_breath.running_mean_initialized = false;
            MAC_breath.first_intercept = true;
            MAC_breath.peak_to_valley_amplitudes = malloc_or_die(sizeof(float)* MAX_NUMBER_OF_SAMPLES_KEPT);
            MAC_breath.peak_to_valley_last_index = -1;
            MAC_breath.intercepts = malloc_or_die(sizeof(Intercept) * MAC_breath.array_len);
            circular_list_initialize(&MAC_breath.peak_indices, MAC_breath.array_len);
            circular_list_initialize(&MAC_breath.valley_indices, MAC_breath.array_len);
            running_mean_initialize(&MAC_breath.MAC, breathing_filter.out, current_last_element, timestamp_array, filtered_breath, MAX_NUMBER_OF_SAMPLES_KEPT);

            poi_list_initialize(&breath_pois, 30);

            dumb_running_mean_initialize(&breath_features.mean_peak_rate_over_window, 30);
            dumb_running_mean_initialize(&breath_features.mean_up_stroke_length, 30);
            dumb_running_mean_initialize(&breath_features.mean_up_stroke_amplitude, 30);
            dumb_running_mean_initialize(&breath_features.mean_valley_rate_over_window, 30);
            dumb_running_mean_initialize(&breath_features.mean_down_stroke_length, 30);
            dumb_running_mean_initialize(&breath_features.mean_down_stroke_amplitude, 30);

        }else{
            running_mean_append(&MAC_breath.MAC, breathing_filter.out, rx_ctrl->timestamp, MAC_window_size);

            // check for intercepts
            int last_index = get_previous_index(current_last_element, MAX_NUMBER_OF_SAMPLES_KEPT);
            if(filtered_breath[last_index] <= MAC_breath.MAC.current_mean && filtered_breath[current_last_element] >= MAC_breath.MAC.current_mean && !(filtered_breath[last_index] == MAC_breath.MAC.current_mean && filtered_breath[current_last_element] == MAC_breath.MAC.current_mean)){
                // up intercept
                Intercept  up_intercept;
                intercept_initialize(&up_intercept, last_index, true);

                if(MAC_breath.first_intercept){
                    MAC_breath.current_last_intercept = 0;
                    MAC_breath.current_first_intercept = 0;
                    MAC_breath.intercepts[0] = up_intercept;
                    MAC_breath.first_intercept = false;
                }else{

                    // check whether previous intercept was of the same type - this should never happen normally, but only occurs because the last intercept and corresponding peak/valley failed the amplitude check and were not added
                    if(MAC_breath.intercepts[MAC_breath.current_last_intercept].is_up_intercept){
                        // disregard this intercept
                        ESP_LOGW(TAG, "same intercept type, up");
                    }
                    else{
                        Intercept previous_intercept = MAC_breath.intercepts[MAC_breath.current_last_intercept];

                        if(timestamp_array[up_intercept.index] - timestamp_array[previous_intercept.index] >= T_timestamp/20){
                            // find the minimum between this and the previous intercept
                            int i = previous_intercept.index;
                            float current_minimum = FLT_MAX;
                            int minimum_position = -1;
                            int index_dif = 0;
                            while(i != up_intercept.index){
                                
                                if(filtered_breath[i] <= current_minimum){
                                    current_minimum = filtered_breath[i];
                                    minimum_position = i;
                                    valley_index_difference_from_last_intercept = index_dif;
                                }
                                index_dif++;
                                i = get_next_index(i, MAX_NUMBER_OF_SAMPLES_KEPT);
                            }
                            float breath_cycle_amplitude = MAC_breath.mean_peak_to_valley_amplitude.current_mean;
                            if(breath_pois.number_of_elements > 0){
                                breath_cycle_amplitude = fabs(filtered_breath[minimum_position]-filtered_breath[MAC_breath.peak_indices.list[MAC_breath.peak_indices.current_last_element_position]]);
                            }

                            // check whether the amplitude is large enough
                            if(MAC_breath.mean_peak_to_valley_amplitude.current_number_of_elements > 20 && breath_cycle_amplitude <= MAC_breath.mean_peak_to_valley_amplitude.current_mean * 0.2){
                                // is too small, discard the newly found intercept and valley
                                ESP_LOGE(TAG, "ampliude not large enough");
                            }
                            else 
                            {
                                // it is large enough, so this is a valid intercept
                                // add this intercept
                                MAC_breath.current_last_intercept = get_next_index(MAC_breath.current_last_intercept, MAC_breath.array_len);
                                if(MAC_breath.current_last_intercept == MAC_breath.current_first_intercept){
                                    MAC_breath.current_first_intercept++;
                                }
                                MAC_breath.intercepts[MAC_breath.current_last_intercept] = up_intercept;

                                // store the found valley in the corresponding list
                                circular_list_append(&MAC_breath.valley_indices, minimum_position);
                                MAC_breath.peak_to_valley_last_index = get_next_index(MAC_breath.peak_to_valley_last_index, MAC_breath.array_len);
                                MAC_breath.peak_to_valley_amplitudes[MAC_breath.peak_to_valley_last_index] = breath_cycle_amplitude;
                                // update the peak-valley-mean
                                if(!MAC_breath.running_mean_initialized){
                                    running_mean_initialize(&MAC_breath.mean_peak_to_valley_amplitude, breath_cycle_amplitude, current_last_element, timestamp_array, MAC_breath.peak_to_valley_amplitudes, MAX_NUMBER_OF_SAMPLES_KEPT);
                                    MAC_breath.running_mean_initialized = true;
                                }else{
                                    running_mean_append(&MAC_breath.mean_peak_to_valley_amplitude, breath_cycle_amplitude, timestamp_array[MAC_breath.valley_indices.list[MAC_breath.valley_indices.current_last_element_position]], window_size);
                                }

                                POI new_poi;
                                // handle case of first poi
                                if(breath_pois.number_of_elements == 0){
                                    poi_initialize(&new_poi, false, timestamp_array[minimum_position], filtered_breath[minimum_position], minimum_position, valley_index_difference_from_last_intercept);
                                }else{
                                    poi_initialize(&new_poi, false, timestamp_array[minimum_position]-timestamp_array[MAC_breath.peak_indices.list[MAC_breath.peak_indices.current_last_element_position]], breath_cycle_amplitude, minimum_position, valley_index_difference_from_last_intercept);
                                }

                                // calculate valley-related features
                                // instantaneous rate, mean rate, mean down stroke length, mean down stoke amplitude, fractional amplitude, fractional time, up/down time ratio, up/down amplitude ratio
                                breath_features.instantaneous_valley_rate = 60.0 / ((float)new_poi.time_difference_to_previous_poi / (float)SECONDS_TO_MICROSECONDS);

                                dumb_running_mean_append(&breath_features.mean_valley_rate_over_window, breath_features.instantaneous_valley_rate, timestamp_array[minimum_position], window_size);
                                dumb_running_mean_append(&breath_features.mean_down_stroke_length, new_poi.time_difference_to_previous_poi, timestamp_array[minimum_position], window_size);
                                dumb_running_mean_append(&breath_features.mean_down_stroke_amplitude, new_poi.amplitude_difference_to_previous_poi, timestamp_array[minimum_position], window_size);

                                if(breath_pois.number_of_elements > 0){
                                    POI peak = breath_pois.list[breath_pois.last_element_index];
                                    breath_features.fractional_up_stroke_amplitude = peak.amplitude_difference_to_previous_poi / (peak.amplitude_difference_to_previous_poi + new_poi.amplitude_difference_to_previous_poi);
                                    breath_features.fractional_up_stroke_time = (float)peak.time_difference_to_previous_poi / (float)(peak.time_difference_to_previous_poi + new_poi.time_difference_to_previous_poi);
                                    breath_features.up_to_down_amplitude_ratio = peak.amplitude_difference_to_previous_poi / new_poi.amplitude_difference_to_previous_poi;
                                    breath_features.up_to_down_length_ratio = (float)peak.time_difference_to_previous_poi / (float)new_poi.time_difference_to_previous_poi;
                                }
                                poi_list_append(&breath_pois, &new_poi);
                                new_breath_poi = &new_poi;
                                breath_found_poi = true;
                                ESP_LOGI(TAG, "valley adding new poi, poi index %d, poi amp %f, poi peak %d, poi time %d", new_poi.index, new_poi.amplitude_difference_to_previous_poi, new_poi.is_peak, new_poi.time_difference_to_previous_poi);


                                if(breath_pois.number_of_elements > 1){
                                    calculate_variance_features(&breath_features, &breath_pois);
                                } 

                                breath_found_up_intercept = true;
                                found_valley = true;
                                ESP_LOGI(TAG, "up intercept added");
                            } 
                        }
                    }
                    MAC_breath.first_intercept = false;
                }
            }

            if(filtered_breath[last_index] >= MAC_breath.MAC.current_mean && filtered_breath[current_last_element] <= MAC_breath.MAC.current_mean && !(filtered_breath[last_index] == MAC_breath.MAC.current_mean && filtered_breath[current_last_element] == MAC_breath.MAC.current_mean)){
                // down intercept
                Intercept  down_intercept;
                intercept_initialize(&down_intercept, last_index, false);

                if(MAC_breath.first_intercept){
                    MAC_breath.current_last_intercept = 0;
                    MAC_breath.intercepts[0] = down_intercept;
                    MAC_breath.first_intercept = false;
                }else{

                    // check whether previous intercept was of the same type - this should never happen normally, but only occurs because the last intercept and corresponding peak/valley failed the amplitude check and were not added
                    if(!MAC_breath.intercepts[MAC_breath.current_last_intercept].is_up_intercept){
                        // disregard this intercept
                        ESP_LOGW(TAG, "same intercep type, down");
                    }
                    else{
                        Intercept previous_intercept = MAC_breath.intercepts[MAC_breath.current_last_intercept];

                        if(timestamp_array[down_intercept.index] - timestamp_array[previous_intercept.index] >= T_timestamp/20){
                            // find the maximum between this and the previous intercept
                            int i = previous_intercept.index;
                            float current_maximum = -FLT_MAX;
                            int maximum_position = -1;
                            int index_dif = 0;
                            while(i != down_intercept.index){
                                if(filtered_breath[i] >= current_maximum){
                                    current_maximum = filtered_breath[i];
                                    maximum_position = i;
                                    peak_index_difference_from_last_intercept = index_dif;
                                }
                                index_dif++;
                                i = get_next_index(i, MAX_NUMBER_OF_SAMPLES_KEPT);
                            }
                            float breath_cycle_amplitude = MAC_breath.mean_peak_to_valley_amplitude.current_mean;
                            if(breath_pois.number_of_elements > 0){
                                breath_cycle_amplitude = fabs(filtered_breath[maximum_position]-filtered_breath[MAC_breath.valley_indices.list[MAC_breath.valley_indices.current_last_element_position]]);
                            }


                            // check whether the amplitude is large enough - but only if enough amplitudes have been added yet, otherwise if the first amplitude is an outlier no further ones will be added
                            if(MAC_breath.mean_peak_to_valley_amplitude.current_number_of_elements > 20 && breath_cycle_amplitude <= MAC_breath.mean_peak_to_valley_amplitude.current_mean * 0.2){
                                // is too small, discard the newly found intercept and peak
                                ESP_LOGE(TAG, "amplitude not large enough");
                            }
                            else 
                            {   
                                // it is large enough, so this is a valid intercept
                                // add this intercept
                                MAC_breath.current_last_intercept = get_next_index(MAC_breath.current_last_intercept, MAC_breath.array_len);
                                if(MAC_breath.current_last_intercept == MAC_breath.current_first_intercept){
                                    MAC_breath.current_first_intercept++;
                                }
                                MAC_breath.intercepts[MAC_breath.current_last_intercept] = down_intercept;

                                MAC_breath.peak_to_valley_last_index = get_next_index(MAC_breath.peak_to_valley_last_index, MAC_breath.array_len);
                                MAC_breath.peak_to_valley_amplitudes[MAC_breath.peak_to_valley_last_index] = breath_cycle_amplitude;
                                // store the found peak in the corresponding list
                                circular_list_append(&MAC_breath.peak_indices, maximum_position);
                                // update the peak-valley-mean
                                if(!MAC_breath.running_mean_initialized){
                                    running_mean_initialize(&MAC_breath.mean_peak_to_valley_amplitude, breath_cycle_amplitude, current_last_element, timestamp_array, MAC_breath.peak_to_valley_amplitudes, MAX_NUMBER_OF_SAMPLES_KEPT);
                                    MAC_breath.running_mean_initialized = true;
                                }else{
                                    running_mean_append(&MAC_breath.mean_peak_to_valley_amplitude, breath_cycle_amplitude, timestamp_array[MAC_breath.peak_indices.list[MAC_breath.peak_indices.current_last_element_position]], window_size);
                                }

                                POI new_poi;
                                // handle case of first poi
                                if(breath_pois.number_of_elements == 0){
                                    poi_initialize(&new_poi, true, timestamp_array[maximum_position], filtered_breath[maximum_position], maximum_position, peak_index_difference_from_last_intercept);
                                }else{
                                    poi_initialize(&new_poi, true, timestamp_array[maximum_position]-timestamp_array[MAC_breath.valley_indices.list[MAC_breath.valley_indices.current_last_element_position]], breath_cycle_amplitude, maximum_position, peak_index_difference_from_last_intercept);
                                }

                                // calculate peak-related features
                                // instantaneous rate, mean rate, mean up stroke length, mean up stoke amplitude
                                breath_features.instantaneous_peak_rate = 60.0 / ((float)new_poi.time_difference_to_previous_poi / (float)SECONDS_TO_MICROSECONDS);

                                dumb_running_mean_append(&breath_features.mean_peak_rate_over_window, breath_features.instantaneous_peak_rate, timestamp_array[maximum_position], window_size);
                                dumb_running_mean_append(&breath_features.mean_up_stroke_length, new_poi.time_difference_to_previous_poi, timestamp_array[maximum_position], window_size);
                                dumb_running_mean_append(&breath_features.mean_up_stroke_amplitude, new_poi.amplitude_difference_to_previous_poi, timestamp_array[maximum_position], window_size);
                                
                                ESP_LOGI(TAG, "adding new poi, poi index %d, poi amp %f, poi peak %d, poi time %d", new_poi.index, new_poi.amplitude_difference_to_previous_poi, new_poi.is_peak, new_poi.time_difference_to_previous_poi);

                                poi_list_append(&breath_pois, &new_poi);
                                new_breath_poi = &new_poi;
                                breath_found_poi = true;

                                if(breath_pois.number_of_elements > 1){
                                    calculate_variance_features(&breath_features, &breath_pois);
                                } 

                                breath_found_down_intercept = true;
                                found_peak = true;
                                ESP_LOGI(TAG, "down intercept added");
                            } 
                        }
                    }
                    MAC_breath.first_intercept = false;
                }
            }
            // printf("breathing data,%d,%f,%f,%d,%d,%d,%d,%d,%d\n", timestamp_array[current_last_element], filtered_breath[current_last_element], MAC_breath.MAC.current_mean, breath_found_up_intercept, breath_found_down_intercept, found_valley, valley_index_difference_from_last_intercept, found_peak, peak_index_difference_from_last_intercept);
            
        }
        

        // MAC for heart
        T_timestamp = 60/T_heart * SECONDS_TO_MICROSECONDS;

        bool heart_found_poi = false;
        POI *new_heart_poi;

        bool heart_found_up_intercept = false;
        bool heart_found_down_intercept = false;
        found_peak = false;
        found_valley = false;
        peak_index_difference_from_last_intercept = 0;
        valley_index_difference_from_last_intercept = 0;

        MAC_window_size = 2 * T_heart * SECONDS_TO_MICROSECONDS; // calculate from T -> timestamps are in microseconds
        // update the MAC
        // use the filtered amplitude for the MAC
        if(first_run){
            MAC_heart.current_last_intercept = -1;
            MAC_heart.current_first_intercept = -1;
            MAC_heart.array_len = 20;
            MAC_heart.running_mean_initialized = false;
            MAC_heart.first_intercept = true;
            MAC_heart.peak_to_valley_amplitudes = malloc_or_die(sizeof(float)* MAX_NUMBER_OF_SAMPLES_KEPT);
            MAC_heart.peak_to_valley_last_index = -1;
            MAC_heart.intercepts = malloc_or_die(sizeof(Intercept) * MAC_heart.array_len);
            circular_list_initialize(&MAC_heart.peak_indices, MAC_heart.array_len);
            circular_list_initialize(&MAC_heart.valley_indices, MAC_heart.array_len);
            running_mean_initialize(&MAC_heart.MAC, heart_filter.out, current_last_element, timestamp_array, filtered_heart, MAX_NUMBER_OF_SAMPLES_KEPT);

            poi_list_initialize(&heart_pois, 30);

            dumb_running_mean_initialize(&heart_features.mean_peak_rate_over_window, 30);
            dumb_running_mean_initialize(&heart_features.mean_up_stroke_length, 30);
            dumb_running_mean_initialize(&heart_features.mean_up_stroke_amplitude, 30);
            dumb_running_mean_initialize(&heart_features.mean_valley_rate_over_window, 30);
            dumb_running_mean_initialize(&heart_features.mean_down_stroke_length, 30);
            dumb_running_mean_initialize(&heart_features.mean_down_stroke_amplitude, 30);

            first_run = false;
        }else{
            running_mean_append(&MAC_heart.MAC, heart_filter.out, rx_ctrl->timestamp, MAC_window_size);

            // check for intercepts
            int last_index = get_previous_index(current_last_element, MAX_NUMBER_OF_SAMPLES_KEPT);
            if(filtered_heart[last_index] <= MAC_heart.MAC.current_mean && filtered_heart[current_last_element] >= MAC_heart.MAC.current_mean && !(filtered_heart[last_index] == MAC_heart.MAC.current_mean && filtered_heart[current_last_element] == MAC_heart.MAC.current_mean)){
                // up intercept
                Intercept  up_intercept;
                intercept_initialize(&up_intercept, last_index, true);

                if(MAC_heart.first_intercept){
                    MAC_heart.current_last_intercept = 0;
                    MAC_heart.current_first_intercept = 0;
                    MAC_heart.intercepts[0] = up_intercept;
                    MAC_heart.first_intercept = false;
                }else{

                    // check whether previous intercept was of the same type - this should never happen normally, but only occurs because the last intercept and corresponding peak/valley failed the amplitude check and were not added
                    if(MAC_heart.intercepts[MAC_heart.current_last_intercept].is_up_intercept){
                        // disregard this intercept
                        ESP_LOGW(TAG, "same intercept type, up");
                    }
                    else{
                        Intercept previous_intercept = MAC_heart.intercepts[MAC_heart.current_last_intercept];

                        if(timestamp_array[up_intercept.index] - timestamp_array[previous_intercept.index] >= T_timestamp/20){
                            // find the minimum between this and the previous intercept
                            int i = previous_intercept.index;
                            float current_minimum = FLT_MAX;
                            int minimum_position = -1;
                            int index_dif = 0;
                            while(i != up_intercept.index){
                                
                                if(filtered_heart[i] <= current_minimum){
                                    current_minimum = filtered_heart[i];
                                    minimum_position = i;
                                    valley_index_difference_from_last_intercept = index_dif;
                                }
                                index_dif++;
                                i = get_next_index(i, MAX_NUMBER_OF_SAMPLES_KEPT);
                            }
                            float heart_cycle_amplitude = MAC_heart.mean_peak_to_valley_amplitude.current_mean;
                            if(heart_pois.number_of_elements > 0){
                                heart_cycle_amplitude = fabs(filtered_heart[minimum_position]-filtered_heart[MAC_heart.peak_indices.list[MAC_heart.peak_indices.current_last_element_position]]);
                            }

                            // check whether the amplitude is large enough
                            if(MAC_heart.mean_peak_to_valley_amplitude.current_number_of_elements > 20 && heart_cycle_amplitude <= MAC_heart.mean_peak_to_valley_amplitude.current_mean * 0.2){
                                // is too small, discard the newly found intercept and valley
                                ESP_LOGE(TAG, "ampliude not large enough");
                            }
                            else 
                            {
                                // it is large enough, so this is a valid intercept
                                // add this intercept
                                MAC_heart.current_last_intercept = get_next_index(MAC_heart.current_last_intercept, MAC_heart.array_len);
                                if(MAC_heart.current_last_intercept == MAC_heart.current_first_intercept){
                                    MAC_heart.current_first_intercept++;
                                }
                                MAC_heart.intercepts[MAC_heart.current_last_intercept] = up_intercept;

                                // store the found valley in the corresponding list
                                circular_list_append(&MAC_heart.valley_indices, minimum_position);
                                MAC_heart.peak_to_valley_last_index = get_next_index(MAC_heart.peak_to_valley_last_index, MAC_heart.array_len);
                                MAC_heart.peak_to_valley_amplitudes[MAC_heart.peak_to_valley_last_index] = heart_cycle_amplitude;
                                // update the peak-valley-mean
                                if(!MAC_heart.running_mean_initialized){
                                    running_mean_initialize(&MAC_heart.mean_peak_to_valley_amplitude, heart_cycle_amplitude, current_last_element, timestamp_array, MAC_heart.peak_to_valley_amplitudes, MAX_NUMBER_OF_SAMPLES_KEPT);
                                    MAC_heart.running_mean_initialized = true;
                                }else{
                                    running_mean_append(&MAC_heart.mean_peak_to_valley_amplitude, heart_cycle_amplitude, timestamp_array[MAC_heart.valley_indices.list[MAC_heart.valley_indices.current_last_element_position]], window_size);
                                }

                                POI new_poi;

                                // handle case of first poi
                                if(heart_pois.number_of_elements == 0){
                                    poi_initialize(&new_poi, false, timestamp_array[minimum_position], filtered_heart[minimum_position], minimum_position, valley_index_difference_from_last_intercept);
                                }else{
                                    poi_initialize(&new_poi, false, timestamp_array[minimum_position]-timestamp_array[MAC_heart.peak_indices.list[MAC_heart.peak_indices.current_last_element_position]], heart_cycle_amplitude, minimum_position, valley_index_difference_from_last_intercept);
                                }

                                // calculate valley-related features
                                // instantaneous rate, mean rate, mean down stroke length, mean down stoke amplitude, fractional amplitude, fractional time, up/down time ratio, up/down amplitude ratio
                                heart_features.instantaneous_valley_rate = 60.0 / ((float)new_poi.time_difference_to_previous_poi / (float)SECONDS_TO_MICROSECONDS);

                                dumb_running_mean_append(&heart_features.mean_valley_rate_over_window, heart_features.instantaneous_valley_rate, timestamp_array[minimum_position], window_size);
                                dumb_running_mean_append(&heart_features.mean_down_stroke_length, new_poi.time_difference_to_previous_poi, timestamp_array[minimum_position], window_size);
                                dumb_running_mean_append(&heart_features.mean_down_stroke_amplitude, new_poi.amplitude_difference_to_previous_poi, timestamp_array[minimum_position], window_size);
                                
                                if(heart_pois.number_of_elements > 0){
                                    POI peak = heart_pois.list[heart_pois.last_element_index];
                                    heart_features.fractional_up_stroke_amplitude = peak.amplitude_difference_to_previous_poi / (peak.amplitude_difference_to_previous_poi + new_poi.amplitude_difference_to_previous_poi);
                                    heart_features.fractional_up_stroke_time = (float)peak.time_difference_to_previous_poi / (float)(peak.time_difference_to_previous_poi + new_poi.time_difference_to_previous_poi);
                                    heart_features.up_to_down_amplitude_ratio = peak.amplitude_difference_to_previous_poi / new_poi.amplitude_difference_to_previous_poi;
                                    heart_features.up_to_down_length_ratio = (float)peak.time_difference_to_previous_poi / (float)new_poi.time_difference_to_previous_poi;
                                }

                                poi_list_append(&heart_pois, &new_poi);
                                new_heart_poi = &new_poi;
                                heart_found_poi = true;
                                ESP_LOGI(TAG, "valley adding new poi, poi index %d, poi amp %f, poi peak %d, poi time %d", new_poi.index, new_poi.amplitude_difference_to_previous_poi, new_poi.is_peak, new_poi.time_difference_to_previous_poi);


                                if(heart_pois.number_of_elements > 1){
                                    calculate_variance_features(&heart_features, &heart_pois);
                                } 

                                heart_found_up_intercept = true;
                                found_valley = true;
                                ESP_LOGI(TAG, "up intercept added");
                            } 
                        }
                    }
                    MAC_heart.first_intercept = false;
                }
            }

            if(filtered_heart[last_index] >= MAC_heart.MAC.current_mean && filtered_heart[current_last_element] <= MAC_heart.MAC.current_mean && !(filtered_heart[last_index] == MAC_heart.MAC.current_mean && filtered_heart[current_last_element] == MAC_heart.MAC.current_mean)){
                // down intercept
                Intercept  down_intercept;
                intercept_initialize(&down_intercept, last_index, false);

                if(MAC_heart.first_intercept){
                    MAC_heart.current_last_intercept = 0;
                    MAC_heart.intercepts[0] = down_intercept;
                    MAC_heart.first_intercept = false;
                }else{

                    // check whether previous intercept was of the same type - this should never happen normally, but only occurs because the last intercept and corresponding peak/valley failed the amplitude check and were not added
                    if(!MAC_heart.intercepts[MAC_heart.current_last_intercept].is_up_intercept){
                        // disregard this intercept
                        ESP_LOGW(TAG, "same intercep type, down");
                    }
                    else{
                        Intercept previous_intercept = MAC_heart.intercepts[MAC_heart.current_last_intercept];

                        if(timestamp_array[down_intercept.index] - timestamp_array[previous_intercept.index] >= T_timestamp/20){
                            // find the maximum between this and the previous intercept
                            int i = previous_intercept.index;
                            float current_maximum = -FLT_MAX;
                            int maximum_position = -1;
                            int index_dif = 0;
                            while(i != down_intercept.index){
                                if(filtered_heart[i] >= current_maximum){
                                    current_maximum = filtered_heart[i];
                                    maximum_position = i;
                                    peak_index_difference_from_last_intercept = index_dif;
                                }
                                index_dif++;
                                i = get_next_index(i, MAX_NUMBER_OF_SAMPLES_KEPT);
                            }
                            float heart_cycle_amplitude = MAC_heart.mean_peak_to_valley_amplitude.current_mean;
                            if(heart_pois.number_of_elements > 0){
                                heart_cycle_amplitude = fabs(filtered_heart[maximum_position]-filtered_heart[MAC_heart.valley_indices.list[MAC_heart.valley_indices.current_last_element_position]]);
                            }

                            // check whether the amplitude is large enough - but only if enough amplitudes have been added yet, otherwise if the first amplitude is an outlier no further ones will be added
                            if(MAC_heart.mean_peak_to_valley_amplitude.current_number_of_elements > 20 && heart_cycle_amplitude <= MAC_heart.mean_peak_to_valley_amplitude.current_mean * 0.2){
                                // is too small, discard the newly found intercept and peak
                                ESP_LOGE(TAG, "amplitude not large enough");
                            }
                            else 
                            {   
                                // it is large enough, so this is a valid intercept
                                // add this intercept
                                MAC_heart.current_last_intercept = get_next_index(MAC_heart.current_last_intercept, MAC_heart.array_len);
                                if(MAC_heart.current_last_intercept == MAC_heart.current_first_intercept){
                                    MAC_heart.current_first_intercept++;
                                }
                                MAC_heart.intercepts[MAC_heart.current_last_intercept] = down_intercept;

                                MAC_heart.peak_to_valley_last_index = get_next_index(MAC_heart.peak_to_valley_last_index, MAC_heart.array_len);
                                MAC_heart.peak_to_valley_amplitudes[MAC_heart.peak_to_valley_last_index] = heart_cycle_amplitude;
                                // store the found peak in the corresponding list
                                circular_list_append(&MAC_heart.peak_indices, maximum_position);
                                // update the peak-valley-mean
                                if(!MAC_heart.running_mean_initialized){
                                    running_mean_initialize(&MAC_heart.mean_peak_to_valley_amplitude, heart_cycle_amplitude, current_last_element, timestamp_array, MAC_heart.peak_to_valley_amplitudes, MAX_NUMBER_OF_SAMPLES_KEPT);
                                    MAC_heart.running_mean_initialized = true;
                                }else{
                                    running_mean_append(&MAC_heart.mean_peak_to_valley_amplitude, heart_cycle_amplitude, timestamp_array[MAC_heart.peak_indices.list[MAC_heart.peak_indices.current_last_element_position]], window_size);
                                }

                                POI new_poi;
                                // handle case of first poi
                                if(heart_pois.number_of_elements == 0){
                                    poi_initialize(&new_poi, true, timestamp_array[maximum_position], filtered_heart[maximum_position], maximum_position, peak_index_difference_from_last_intercept);
                                }else{
                                    poi_initialize(&new_poi, true, timestamp_array[maximum_position]-timestamp_array[MAC_heart.valley_indices.list[MAC_heart.valley_indices.current_last_element_position]], heart_cycle_amplitude, maximum_position, peak_index_difference_from_last_intercept);
                                }

                                // calculate peak-related features
                                // instantaneous rate, mean rate, mean up stroke length, mean up stoke amplitude
                                heart_features.instantaneous_peak_rate = 60.0 / ((float)new_poi.time_difference_to_previous_poi / (float)SECONDS_TO_MICROSECONDS);

                                dumb_running_mean_append(&heart_features.mean_peak_rate_over_window, heart_features.instantaneous_peak_rate, timestamp_array[maximum_position], window_size);
                                dumb_running_mean_append(&heart_features.mean_up_stroke_length, new_poi.time_difference_to_previous_poi, timestamp_array[maximum_position], window_size);
                                dumb_running_mean_append(&heart_features.mean_up_stroke_amplitude, new_poi.amplitude_difference_to_previous_poi, timestamp_array[maximum_position], window_size);
                                
                                ESP_LOGI(TAG, "adding new poi, poi index %d, poi amp %f, poi peak %d, poi time %d", new_poi.index, new_poi.amplitude_difference_to_previous_poi, new_poi.is_peak, new_poi.time_difference_to_previous_poi);

                                poi_list_append(&heart_pois, &new_poi);
                                new_heart_poi = &new_poi;
                                heart_found_poi = true;

                                if(heart_pois.number_of_elements > 1){
                                    calculate_variance_features(&heart_features, &heart_pois);
                                } 

                                heart_found_down_intercept = true;
                                found_peak = true;
                                ESP_LOGI(TAG, "down intercept added");
                            } 
                        }
                    }
                    MAC_heart.first_intercept = false;
                }
            }
            ESP_LOGD(TAG, "heart data,%d,%f,%f,%d,%d,%d,%d,%d,%d\n", timestamp_array[current_last_element], filtered_heart[current_last_element], MAC_heart.MAC.current_mean, heart_found_up_intercept, heart_found_down_intercept, found_valley, valley_index_difference_from_last_intercept, found_peak, peak_index_difference_from_last_intercept);
            
        }

        if(running_calibration){
            append_to_list_char(&id_list, activity_id);
            append_to_list_float(&sti_list, sti);
        }

#if !defined(CONFIG_SENSE_LOG_DIFFERENT_THINGS_TO_DIFFERENT_OUTPUTS) && (defined(CONFIG_SENSE_LOG_TO_SERIAL) || defined(CONFIG_SENSE_LOG_TO_SD) || defined(CONFIG_SENSE_LOG_TO_UDP))
        size_t len = 0;
        size_t calibration_len = 0;

        if (!count) {
            calibration_len += sprintf(calibration_buffer + calibration_len, "type,count,esp_timestamp,packet_timestamp,sti,class_id\n");

            ESP_LOGI(TAG, "================ CSI RECV ================");
            len += sprintf(buffer + len, "type");
#ifdef CONFIG_SENSE_PRINT_STI_SD
            len += sprintf(buffer + len, ",sti");
#endif

#ifdef CONFIG_SENSE_PRINT_FUSED_HEART_SD
            len += sprintf(buffer + len, ",fused_heart_amplitude");
#endif

#ifdef CONFIG_SENSE_PRINT_FUSED_BREATH_SD
            len += sprintf(buffer + len, ",fused_breath_amplitude");
#endif

#ifdef CONFIG_SENSE_PRINT_FILTERED_HEART_SD
            len += sprintf(buffer + len, ",filtered_heart");
#endif

#ifdef CONFIG_SENSE_PRINT_FILTERED_BREATH_SD
            len += sprintf(buffer + len, ",filtered_breath");
#endif

#ifdef CONFIG_SENSE_PRINT_HEART_FEATURES_SD
            len += sprintf(buffer + len, ",heart_features");
#endif

#ifdef CONFIG_SENSE_PRINT_BREATHING_FEATURES_SD
            len += sprintf(buffer + len, ",breath_features");
#endif

#ifdef CONFIG_SENSE_PRINT_HEART_POI_SD
            len += sprintf(buffer + len, ",heart_poi_found,new_heart_poi");
#endif

#ifdef CONFIG_SENSE_PRINT_BREATHING_POI_SD
            len += sprintf(buffer + len, ",breath_poi_found,new_breath_poi");
#endif

#ifdef CONFIG_SENSE_PRINT_DETECTION_SD
            len += sprintf(buffer + len, ",detected_presence,detected_small_movement,detected_large_movement");
#endif

#ifdef CONFIG_SENSE_PRINT_THRESHOLDS_SD
            len += sprintf(buffer + len, ",threshold_presence,f_presence,threshold_small_movement,f_small_movement,threshold_large_movement,f_large_movement");
#endif

#ifdef CONFIG_SENSE_PRINT_SLEEP_STAGE_CLASSIFICATION_SD
            len += sprintf(buffer + len, ",sleep_stage_classification");
#endif
            len += sprintf(buffer + len, ",button_pressed");
            len += sprintf(buffer + len, ",sequence,timestamp,source_mac,first_word_invalid,len,rssi,rate,sig_mode,mcs,bandwidth,smoothing,not_sounding,aggregation,stbc,fec_coding,sgi,noise_floor,ampdu_cnt,channel,secondary_channel,local_timestamp,ant,sig_len,rx_state");
#ifdef CONFIG_SENSE_PRINT_CSI_SD   
            len += sprintf(buffer + len, ",data");
#endif
            len += sprintf(buffer + len, "\n");
        }

        if(running_calibration){
            calibration_len += sprintf(calibration_buffer + calibration_len, "CALIBRATION,%d,%u,%u,%f,%d\n",
                    count, esp_log_timestamp(), rx_ctrl->timestamp, sti, activity_id);
        }
        
        len += sprintf(buffer + len, "CSI_DATA");
#ifdef CONFIG_SENSE_PRINT_STI_SD
            len += sprintf(buffer + len, ",%f", sti);
#endif

#ifdef CONFIG_SENSE_PRINT_FUSED_HEART_SD
            len += sprintf(buffer + len, ",%f", fused_amplitude_heart);
#endif

#ifdef CONFIG_SENSE_PRINT_FUSED_BREATH_SD
            len += sprintf(buffer + len, ",%f", fused_amplitude_breath);
#endif

#ifdef CONFIG_SENSE_PRINT_FILTERED_HEART_SD
            len += sprintf(buffer + len, ",%f", filtered_heart[current_last_element]);
#endif

#ifdef CONFIG_SENSE_PRINT_FILTERED_BREATH_SD
            len += sprintf(buffer + len, ",%f", filtered_breath[current_last_element]);
#endif

#ifdef CONFIG_SENSE_PRINT_HEART_FEATURES_SD
            len += sprintf(buffer + len, ",[%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f]", heart_features.instantaneous_peak_rate, heart_features.instantaneous_valley_rate, heart_features.mean_peak_rate_over_window.current_mean, heart_features.mean_valley_rate_over_window.current_mean, heart_features.fft_rate_over_window, heart_features.variance_of_peak_rate_in_window, heart_features.variance_of_valley_rate_in_window, heart_features.mean_up_stroke_length.current_mean, heart_features.mean_down_stroke_length.current_mean, heart_features.up_stroke_length_variance, heart_features.down_stroke_length_variance, heart_features.up_to_down_length_ratio, heart_features.fractional_up_stroke_time, heart_features.mean_up_stroke_amplitude.current_mean, heart_features.mean_down_stroke_amplitude.current_mean, heart_features.up_stroke_amplitude_variance, heart_features.down_stroke_amplitude_variance, heart_features.up_to_down_amplitude_ratio, heart_features.fractional_up_stroke_amplitude);
#endif

#ifdef CONFIG_SENSE_PRINT_BREATHING_FEATURES_SD
            len += sprintf(buffer + len, ",[%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f]", breath_features.instantaneous_peak_rate, breath_features.instantaneous_valley_rate, breath_features.mean_peak_rate_over_window.current_mean, breath_features.mean_valley_rate_over_window.current_mean, breath_features.fft_rate_over_window, breath_features.variance_of_peak_rate_in_window, breath_features.variance_of_valley_rate_in_window, breath_features.mean_up_stroke_length.current_mean, breath_features.mean_down_stroke_length.current_mean, breath_features.up_stroke_length_variance, breath_features.down_stroke_length_variance, breath_features.up_to_down_length_ratio, breath_features.fractional_up_stroke_time, breath_features.mean_up_stroke_amplitude.current_mean, breath_features.mean_down_stroke_amplitude.current_mean, breath_features.up_stroke_amplitude_variance, breath_features.down_stroke_amplitude_variance, breath_features.up_to_down_amplitude_ratio, breath_features.fractional_up_stroke_amplitude);
#endif

#ifdef CONFIG_SENSE_PRINT_HEART_POI_SD
            len += sprintf(buffer + len, ",%d", heart_found_poi);
            if(heart_found_poi){
                len += sprintf(buffer + len, ",[%d %u %f %d %d]", new_heart_poi->is_peak, new_heart_poi->time_difference_to_previous_poi, new_heart_poi->amplitude_difference_to_previous_poi, new_heart_poi->index, new_heart_poi->index_difference_from_last_intercept);
            }else{
                len += sprintf(buffer + len, ",[]");
            }
#endif

#ifdef CONFIG_SENSE_PRINT_BREATHING_POI_SD
            len += sprintf(buffer + len, ",%d", breath_found_poi);
            if(breath_found_poi){
                len += sprintf(buffer + len, ",[%d %u %f %d %d]", new_breath_poi->is_peak, new_breath_poi->time_difference_to_previous_poi, new_breath_poi->amplitude_difference_to_previous_poi, new_breath_poi->index, new_breath_poi->index_difference_from_last_intercept);
            }else{
                len += sprintf(buffer + len, ",[]");
            }
#endif

#ifdef CONFIG_SENSE_PRINT_DETECTION_SD
            len += sprintf(buffer + len, ",%d,%d,%d", presence_detected, small_movement_detected, large_movement_detected);
#endif

#ifdef CONFIG_SENSE_PRINT_THRESHOLDS_SD
            len += sprintf(buffer + len, ",%f,%f,%f,%f,%f,%f", t_presence, f_presence, t_small_movement, f_small_movement, t_large_movement, f_large_movement);
#endif

#ifdef CONFIG_SENSE_PRINT_SLEEP_STAGE_CLASSIFICATION_SD
            len += sprintf(buffer + len, ",%d", sleep_stage);
#endif

        len += sprintf(buffer + len, ",%d", button_pressed);
        len += sprintf(buffer + len, ",%d,%u," MACSTR ",%d,%d,%d,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%d,%u,%u,%u,%u,%u,%u,%u",
                    count, esp_log_timestamp(),
                    MAC2STR(info->mac), info->first_word_invalid, info->len, rx_ctrl->rssi, rx_ctrl->rate, rx_ctrl->sig_mode,
                    rx_ctrl->mcs, rx_ctrl->cwb, rx_ctrl->smoothing, rx_ctrl->not_sounding,
                    rx_ctrl->aggregation, rx_ctrl->stbc, rx_ctrl->fec_coding, rx_ctrl->sgi,
                    rx_ctrl->noise_floor, rx_ctrl->ampdu_cnt, rx_ctrl->channel, rx_ctrl->secondary_channel,
                    rx_ctrl->timestamp, rx_ctrl->ant, rx_ctrl->sig_len, rx_ctrl->rx_state);

#ifdef CONFIG_SENSE_PRINT_CSI_SD      
        len += sprintf(buffer + len, ",\"[%d", info->buf[0]);

        for (int i = 1; i < info->len; i++) {
            len += sprintf(buffer + len, ",%d", info->buf[i]);
        }

        len += sprintf(buffer + len, "]\"");
#endif
        len += sprintf(buffer + len, "\n");

#endif

#if defined(CONFIG_SENSE_LOG_TO_SERIAL) && !defined(CONFIG_SENSE_LOG_DIFFERENT_THINGS_TO_DIFFERENT_OUTPUTS)
        printf("%s", buffer);
        if(running_calibration || count==1){
            printf("%s", calibration_buffer);
        }
#endif

#if defined(CONFIG_SENSE_LOG_TO_UDP) && !defined(CONFIG_SENSE_LOG_DIFFERENT_THINGS_TO_DIFFERENT_OUTPUTS)
        
        BufferObject *q_data = malloc_or_die(len + sizeof(size_t));
        q_data->length = len;
        q_data->buffer = malloc_or_die(len);
        memcpy(q_data->buffer, buffer, len);

        if (!buffer_queue || xQueueSend(buffer_queue, &q_data, 0) == pdFALSE) {
            ESP_LOGW(TAG, "buffer_queue full");
            free(q_data);
        }
        
#endif
        
#if defined(CONFIG_SENSE_LOG_TO_SD) && !defined(CONFIG_SENSE_LOG_DIFFERENT_THINGS_TO_DIFFERENT_OUTPUTS)
        fprintf(file, "%s", buffer);
        if(running_calibration || count==1){
            fprintf(calibration_file, "%s", calibration_buffer);
        }

        write_count++;
        if (write_count > WRITE_FILE_AFTER_RECEIVED_CSI_NUMBER)
        {
            write_count = 0;
            ESP_LOGI(TAG, "================ Closing file ================");
            fflush(file);
            fclose(file);
            fflush(calibration_file);
            fclose(calibration_file);
            ESP_LOGI(TAG, "================  file closed ================");
            ESP_LOGI(TAG, "Opening file");
            file = fopen(file_name, "a");
            calibration_file = fopen(calibration_file_name, "a");
        }
#endif


#if defined(CONFIG_SENSE_LOG_DIFFERENT_THINGS_TO_DIFFERENT_OUTPUTS) && defined(CONFIG_SENSE_LOG_TO_SD)
        size_t len = 0;
        size_t calibration_len = 0;

        if (!count) {
            calibration_len += sprintf(calibration_buffer + calibration_len, "type,count,esp_timestamp,packet_timestamp,sti,class_id\n");

            ESP_LOGI(TAG, "================ CSI RECV ================");
            len += sprintf(buffer + len, "type");
#ifdef CONFIG_SENSE_PRINT_STI_SD
            len += sprintf(buffer + len, ",sti");
#endif

#ifdef CONFIG_SENSE_PRINT_FUSED_HEART_SD
            len += sprintf(buffer + len, ",fused_heart_amplitude");
#endif

#ifdef CONFIG_SENSE_PRINT_FUSED_BREATH_SD
            len += sprintf(buffer + len, ",fused_breath_amplitude");
#endif

#ifdef CONFIG_SENSE_PRINT_FILTERED_HEART_SD
            len += sprintf(buffer + len, ",filtered_heart");
#endif

#ifdef CONFIG_SENSE_PRINT_FILTERED_BREATH_SD
            len += sprintf(buffer + len, ",filtered_breath");
#endif

#ifdef CONFIG_SENSE_PRINT_HEART_FEATURES_SD
            len += sprintf(buffer + len, ",heart_features");
#endif

#ifdef CONFIG_SENSE_PRINT_BREATHING_FEATURES_SD
            len += sprintf(buffer + len, ",breath_features");
#endif

#ifdef CONFIG_SENSE_PRINT_HEART_POI_SD
            len += sprintf(buffer + len, ",heart_poi_found,new_heart_poi");
#endif

#ifdef CONFIG_SENSE_PRINT_BREATHING_POI_SD
            len += sprintf(buffer + len, ",breath_poi_found,new_breath_poi");
#endif

#ifdef CONFIG_SENSE_PRINT_DETECTION_SD
            len += sprintf(buffer + len, ",detected_presence,detected_small_movement,detected_large_movement");
#endif

#ifdef CONFIG_SENSE_PRINT_THRESHOLDS_SD
            len += sprintf(buffer + len, ",threshold_presence,f_presence,threshold_small_movement,f_small_movement,threshold_large_movement,f_large_movement");
#endif

#ifdef CONFIG_SENSE_PRINT_SLEEP_STAGE_CLASSIFICATION_SD
            len += sprintf(buffer + len, ",sleep_stage_classification");
#endif

            len += sprintf(buffer + len, ",button_pressed");

            len += sprintf(buffer + len, ",sequence,timestamp,source_mac,first_word_invalid,len,rssi,rate,sig_mode,mcs,bandwidth,smoothing,not_sounding,aggregation,stbc,fec_coding,sgi,noise_floor,ampdu_cnt,channel,secondary_channel,local_timestamp,ant,sig_len,rx_state");

#ifdef CONFIG_SENSE_PRINT_CSI_SD   
            len += sprintf(buffer + len, ",data");
#endif
            len += sprintf(buffer + len, "\n");
        }

        if(running_calibration){
            calibration_len += sprintf(calibration_buffer + calibration_len, "CALIBRATION,%d,%u,%u,%f,%d\n",
                    count, esp_log_timestamp(), rx_ctrl->timestamp, sti, activity_id);
        }
        
        len += sprintf(buffer + len, "CSI_DATA");
#ifdef CONFIG_SENSE_PRINT_STI_SD
            len += sprintf(buffer + len, ",%f", sti);
#endif

#ifdef CONFIG_SENSE_PRINT_FUSED_HEART_SD
            len += sprintf(buffer + len, ",%f", fused_amplitude_heart);
#endif

#ifdef CONFIG_SENSE_PRINT_FUSED_BREATH_SD
            len += sprintf(buffer + len, ",%f", fused_amplitude_breath);
#endif

#ifdef CONFIG_SENSE_PRINT_FILTERED_HEART_SD
            len += sprintf(buffer + len, ",%f", filtered_heart[current_last_element]);
#endif

#ifdef CONFIG_SENSE_PRINT_FILTERED_BREATH_SD
            len += sprintf(buffer + len, ",%f", filtered_breath[current_last_element]);
#endif

#ifdef CONFIG_SENSE_PRINT_HEART_FEATURES_SD
            len += sprintf(buffer + len, ",[%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f]", heart_features.instantaneous_peak_rate, heart_features.instantaneous_valley_rate, heart_features.mean_peak_rate_over_window.current_mean, heart_features.mean_valley_rate_over_window.current_mean, heart_features.fft_rate_over_window, heart_features.variance_of_peak_rate_in_window, heart_features.variance_of_valley_rate_in_window, heart_features.mean_up_stroke_length.current_mean, heart_features.mean_down_stroke_length.current_mean, heart_features.up_stroke_length_variance, heart_features.down_stroke_length_variance, heart_features.up_to_down_length_ratio, heart_features.fractional_up_stroke_time, heart_features.mean_up_stroke_amplitude.current_mean, heart_features.mean_down_stroke_amplitude.current_mean, heart_features.up_stroke_amplitude_variance, heart_features.down_stroke_amplitude_variance, heart_features.up_to_down_amplitude_ratio, heart_features.fractional_up_stroke_amplitude);
#endif

#ifdef CONFIG_SENSE_PRINT_BREATHING_FEATURES_SD
            len += sprintf(buffer + len, ",[%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f]", breath_features.instantaneous_peak_rate, breath_features.instantaneous_valley_rate, breath_features.mean_peak_rate_over_window.current_mean, breath_features.mean_valley_rate_over_window.current_mean, breath_features.fft_rate_over_window, breath_features.variance_of_peak_rate_in_window, breath_features.variance_of_valley_rate_in_window, breath_features.mean_up_stroke_length.current_mean, breath_features.mean_down_stroke_length.current_mean, breath_features.up_stroke_length_variance, breath_features.down_stroke_length_variance, breath_features.up_to_down_length_ratio, breath_features.fractional_up_stroke_time, breath_features.mean_up_stroke_amplitude.current_mean, breath_features.mean_down_stroke_amplitude.current_mean, breath_features.up_stroke_amplitude_variance, breath_features.down_stroke_amplitude_variance, breath_features.up_to_down_amplitude_ratio, breath_features.fractional_up_stroke_amplitude);
#endif

#ifdef CONFIG_SENSE_PRINT_HEART_POI_SD
            len += sprintf(buffer + len, ",%d", heart_found_poi);
            if(heart_found_poi){
                len += sprintf(buffer + len, ",[%d %u %f %d %d]", new_heart_poi->is_peak, new_heart_poi->time_difference_to_previous_poi, new_heart_poi->amplitude_difference_to_previous_poi, new_heart_poi->index, new_heart_poi->index_difference_from_last_intercept);
            }else{
                len += sprintf(buffer + len, ",[]");
            }
#endif

#ifdef CONFIG_SENSE_PRINT_BREATHING_POI_SD
            len += sprintf(buffer + len, ",%d", breath_found_poi);
            if(breath_found_poi){
                len += sprintf(buffer + len, ",[%d %u %f %d %d]", new_heart_poi->is_peak, new_heart_poi->time_difference_to_previous_poi, new_heart_poi->amplitude_difference_to_previous_poi, new_heart_poi->index, new_heart_poi->index_difference_from_last_intercept);
            }else{
                len += sprintf(buffer + len, ",[]");
            }
#endif

#ifdef CONFIG_SENSE_PRINT_DETECTION_SD
            len += sprintf(buffer + len, ",%d,%d,%d", presence_detected, small_movement_detected, large_movement_detected);
#endif

#ifdef CONFIG_SENSE_PRINT_THRESHOLDS_SD
            len += sprintf(buffer + len, ",%f,%f,%f,%f,%f,%f", t_presence, f_presence, t_small_movement, f_small_movement, t_large_movement, f_large_movement);
#endif

#ifdef CONFIG_SENSE_PRINT_SLEEP_STAGE_CLASSIFICATION_SD
            len += sprintf(buffer + len, ",%d", sleep_stage);
#endif

        len += sprintf(buffer + len, ",%d", button_pressed);
        len += sprintf(buffer + len, ",%d,%u," MACSTR ",%d,%d,%d,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%d,%u,%u,%u,%u,%u,%u,%u",
                    count, esp_log_timestamp(),
                    MAC2STR(info->mac), info->first_word_invalid, info->len, rx_ctrl->rssi, rx_ctrl->rate, rx_ctrl->sig_mode,
                    rx_ctrl->mcs, rx_ctrl->cwb, rx_ctrl->smoothing, rx_ctrl->not_sounding,
                    rx_ctrl->aggregation, rx_ctrl->stbc, rx_ctrl->fec_coding, rx_ctrl->sgi,
                    rx_ctrl->noise_floor, rx_ctrl->ampdu_cnt, rx_ctrl->channel, rx_ctrl->secondary_channel,
                    rx_ctrl->timestamp, rx_ctrl->ant, rx_ctrl->sig_len, rx_ctrl->rx_state);

#ifdef CONFIG_SENSE_PRINT_CSI_SD          
        len += sprintf(buffer + len, ",\"[%d", info->buf[0]);

        for (int i = 1; i < info->len; i++) {
            len += sprintf(buffer + len, ",%d", info->buf[i]);
        }

        len += sprintf(buffer + len, "]\"");
#endif
        len += sprintf(buffer + len, "\n");

        fprintf(file, "%s", buffer);
        if(running_calibration || count==1){
            fprintf(calibration_file, "%s", calibration_buffer);
        }

        write_count++;
        if (write_count > WRITE_FILE_AFTER_RECEIVED_CSI_NUMBER)
        {
            write_count = 0;
            ESP_LOGI(TAG, "================ Closing file ================");
            fflush(file);
            fclose(file);
            fflush(calibration_file);
            fclose(calibration_file);
            ESP_LOGI(TAG, "================  file closed ================");
            ESP_LOGI(TAG, "Opening file");
            file = fopen(file_name, "a");
            calibration_file = fopen(calibration_file_name, "a");
        }
#endif

#if defined(CONFIG_SENSE_LOG_DIFFERENT_THINGS_TO_DIFFERENT_OUTPUTS) && defined(CONFIG_SENSE_LOG_TO_SERIAL)
        size_t len1 = 0;
        size_t calibration_len1 = 0;

        if (!count) {
            calibration_len1 += sprintf(calibration_buffer + calibration_len1, "type,count,esp_timestamp,packet_timestamp,sti,class_id\n");

            ESP_LOGI(TAG, "================ CSI RECV ================");
            len1 += sprintf(buffer + len1, "type");
#ifdef CONFIG_SENSE_PRINT_STI_S
            len1 += sprintf(buffer + len1, ",sti");
#endif

#ifdef CONFIG_SENSE_PRINT_FUSED_HEART_S
            len1 += sprintf(buffer + len1, ",fused_heart_amplitude");
#endif

#ifdef CONFIG_SENSE_PRINT_FUSED_BREATH_S
            len1 += sprintf(buffer + len1, ",fused_breath_amplitude");
#endif

#ifdef CONFIG_SENSE_PRINT_FILTERED_HEART_S
            len1 += sprintf(buffer + len1, ",filtered_heart");
#endif

#ifdef CONFIG_SENSE_PRINT_FILTERED_BREATH_S
            len1 += sprintf(buffer + len1, ",filtered_breath");
#endif

#ifdef CONFIG_SENSE_PRINT_HEART_FEATURES_S
            len1 += sprintf(buffer + len1, ",heart_features");
#endif

#ifdef CONFIG_SENSE_PRINT_BREATHING_FEATURES_S
            len1 += sprintf(buffer + len1, ",breath_features");
#endif

#ifdef CONFIG_SENSE_PRINT_HEART_POI_S
            len1 += sprintf(buffer + len1, ",heart_poi_found,new_heart_poi");
#endif

#ifdef CONFIG_SENSE_PRINT_BREATHING_POI_S
            len1 += sprintf(buffer + len1, ",breath_poi_found,new_breath_poi");
#endif

#ifdef CONFIG_SENSE_PRINT_DETECTION_S
            len1 += sprintf(buffer + len1, ",detected_presence,detected_small_movement,detected_large_movement");
#endif

#ifdef CONFIG_SENSE_PRINT_THRESHOLDS_S
            len1 += sprintf(buffer + len1, ",threshold_presence,f_presence,threshold_small_movement,f_small_movement,threshold_large_movement,f_large_movement");
#endif

#ifdef CONFIG_SENSE_PRINT_SLEEP_STAGE_CLASSIFICATION_S
            len1 += sprintf(buffer + len1, ",sleep_stage_classification");
#endif

            len1 += sprintf(buffer + len1, ",sequence,timestamp,source_mac,first_word_invalid,len,rssi,rate,sig_mode,mcs,bandwidth,smoothing,not_sounding,aggregation,stbc,fec_coding,sgi,noise_floor,ampdu_cnt,channel,secondary_channel,local_timestamp,ant,sig_len,rx_state");
#ifdef CONFIG_SENSE_PRINT_CSI_S   
            len1 += sprintf(buffer + len1, ",data");
#endif
            len1 += sprintf(buffer + len1, "\n");
        
        }

        if(running_calibration){
            calibration_len1 += sprintf(calibration_buffer + calibration_len1, "CALIBRATION,%d,%u,%u,%f,%d\n",
                    count, esp_log_timestamp(), rx_ctrl->timestamp, sti, activity_id);
        }
        
        len1 += sprintf(buffer + len1, "CSI_DATA");
#ifdef CONFIG_SENSE_PRINT_STI_S
            len1 += sprintf(buffer + len1, ",%f", sti);
#endif

#ifdef CONFIG_SENSE_PRINT_FUSED_HEART_S
            len1 += sprintf(buffer + len1, ",%f", fused_amplitude_heart);
#endif

#ifdef CONFIG_SENSE_PRINT_FUSED_BREATH_S
            len1 += sprintf(buffer + len1, ",%f", fused_amplitude_breath);
#endif

#ifdef CONFIG_SENSE_PRINT_FILTERED_HEART_S
            len1 += sprintf(buffer + len1, ",%f", filtered_heart[current_last_element]);
#endif

#ifdef CONFIG_SENSE_PRINT_FILTERED_BREATH_S
            len1 += sprintf(buffer + len1, ",%f", filtered_breath[current_last_element]);
#endif

#ifdef CONFIG_SENSE_PRINT_HEART_FEATURES_S
            len1 += sprintf(buffer + len1, ",[%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f]", heart_features.instantaneous_peak_rate, heart_features.instantaneous_valley_rate, heart_features.mean_peak_rate_over_window.current_mean, heart_features.mean_valley_rate_over_window.current_mean, heart_features.fft_rate_over_window, heart_features.variance_of_peak_rate_in_window, heart_features.variance_of_valley_rate_in_window, heart_features.mean_up_stroke_length.current_mean, heart_features.mean_down_stroke_length.current_mean, heart_features.up_stroke_length_variance, heart_features.down_stroke_length_variance, heart_features.up_to_down_length_ratio, heart_features.fractional_up_stroke_time, heart_features.mean_up_stroke_amplitude.current_mean, heart_features.mean_down_stroke_amplitude.current_mean, heart_features.up_stroke_amplitude_variance, heart_features.down_stroke_amplitude_variance, heart_features.up_to_down_amplitude_ratio, heart_features.fractional_up_stroke_amplitude);
#endif

#ifdef CONFIG_SENSE_PRINT_BREATHING_FEATURES_S
            len1 += sprintf(buffer + len1, ",[%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f]", breath_features.instantaneous_peak_rate, breath_features.instantaneous_valley_rate, breath_features.mean_peak_rate_over_window.current_mean, breath_features.mean_valley_rate_over_window.current_mean, breath_features.fft_rate_over_window, breath_features.variance_of_peak_rate_in_window, breath_features.variance_of_valley_rate_in_window, breath_features.mean_up_stroke_length.current_mean, breath_features.mean_down_stroke_length.current_mean, breath_features.up_stroke_length_variance, breath_features.down_stroke_length_variance, breath_features.up_to_down_length_ratio, breath_features.fractional_up_stroke_time, breath_features.mean_up_stroke_amplitude.current_mean, breath_features.mean_down_stroke_amplitude.current_mean, breath_features.up_stroke_amplitude_variance, breath_features.down_stroke_amplitude_variance, breath_features.up_to_down_amplitude_ratio, breath_features.fractional_up_stroke_amplitude);
#endif

#ifdef CONFIG_SENSE_PRINT_HEART_POI_S
            len1 += sprintf(buffer + len1, ",%d", heart_found_poi);
            if(heart_found_poi){
                len1 += sprintf(buffer + len1, ",[%d %u %f %d %d]", new_heart_poi->is_peak, new_heart_poi->time_difference_to_previous_poi, new_heart_poi->amplitude_difference_to_previous_poi, new_heart_poi->index, new_heart_poi->index_difference_from_last_intercept);
            }else{
                len1 += sprintf(buffer + len1, ",[]");
            }
#endif

#ifdef CONFIG_SENSE_PRINT_BREATHING_POI_S
            len1 += sprintf(buffer + len1, ",%d", breath_found_poi);
            if(breath_found_poi){
                len1 += sprintf(buffer + len1, ",[%d %u %f %d %d]", new_heart_poi->is_peak, new_heart_poi->time_difference_to_previous_poi, new_heart_poi->amplitude_difference_to_previous_poi, new_heart_poi->index, new_heart_poi->index_difference_from_last_intercept);
            }else{
                len1 += sprintf(buffer + len1, ",[]");
            }
#endif

#ifdef CONFIG_SENSE_PRINT_DETECTION_S
            len1 += sprintf(buffer + len1, ",%d,%d,%d", presence_detected, small_movement_detected, large_movement_detected);
#endif

#ifdef CONFIG_SENSE_PRINT_THRESHOLDS_S
            len1 += sprintf(buffer + len1, ",%f,%f,%f,%f,%f,%f", t_presence, f_presence, t_small_movement, f_small_movement, t_large_movement, f_large_movement);
#endif

#ifdef CONFIG_SENSE_PRINT_SLEEP_STAGE_CLASSIFICATION_S
            len1 += sprintf(buffer + len1, ",%d", sleep_stage);
#endif

        len1 += sprintf(buffer + len1, ",%d,%u," MACSTR ",%d,%d,%d,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%d,%u,%u,%u,%u,%u,%u,%u",
                    count, esp_log_timestamp(),
                    MAC2STR(info->mac), info->first_word_invalid, info->len, rx_ctrl->rssi, rx_ctrl->rate, rx_ctrl->sig_mode,
                    rx_ctrl->mcs, rx_ctrl->cwb, rx_ctrl->smoothing, rx_ctrl->not_sounding,
                    rx_ctrl->aggregation, rx_ctrl->stbc, rx_ctrl->fec_coding, rx_ctrl->sgi,
                    rx_ctrl->noise_floor, rx_ctrl->ampdu_cnt, rx_ctrl->channel, rx_ctrl->secondary_channel,
                    rx_ctrl->timestamp, rx_ctrl->ant, rx_ctrl->sig_len, rx_ctrl->rx_state);

#ifdef CONFIG_SENSE_PRINT_CSI_S          
        len1 += sprintf(buffer + len1, ",\"[%d", info->buf[0]);

        for (int i = 1; i < info->len; i++) {
            len1 += sprintf(buffer + len1, ",%d", info->buf[i]);
        }

        len1 += sprintf(buffer + len1, "]\"");
#endif
        len1 += sprintf(buffer + len1, "\n");

        printf("%s", buffer);
        if(running_calibration || count==1){
            printf("%s", calibration_buffer);
        }
#endif

#if defined(CONFIG_SENSE_LOG_DIFFERENT_THINGS_TO_DIFFERENT_OUTPUTS) && defined(CONFIG_SENSE_LOG_TO_UDP)
        size_t len2 = 0;

        if (!count) {
            ESP_LOGI(TAG, "================ CSI RECV ================");
            len2 += sprintf(buffer + len2, "type");
#ifdef CONFIG_SENSE_PRINT_STI_U
            len2 += sprintf(buffer + len2, ",sti");
#endif

#ifdef CONFIG_SENSE_PRINT_FUSED_HEART_U
            len2 += sprintf(buffer + len2, ",fused_heart_amplitude");
#endif

#ifdef CONFIG_SENSE_PRINT_FUSED_BREATH_U
            len2 += sprintf(buffer + len2, ",fused_breath_amplitude");
#endif

#ifdef CONFIG_SENSE_PRINT_FILTERED_HEART_U
            len2 += sprintf(buffer + len2, ",filtered_heart");
#endif

#ifdef CONFIG_SENSE_PRINT_FILTERED_BREATH_U
            len2 += sprintf(buffer + len2, ",filtered_breath");
#endif

#ifdef CONFIG_SENSE_PRINT_HEART_FEATURES_U
            len2 += sprintf(buffer + len2, ",heart_features");
#endif

#ifdef CONFIG_SENSE_PRINT_BREATHING_FEATURES_U
            len2 += sprintf(buffer + len2, ",breath_features");
#endif

#ifdef CONFIG_SENSE_PRINT_HEART_POI_U
            len2 += sprintf(buffer + len2, ",heart_poi_found,new_heart_poi");
#endif

#ifdef CONFIG_SENSE_PRINT_BREATHING_POI_U
            len2 += sprintf(buffer + len2, ",breath_poi_found,new_breath_poi");
#endif

#ifdef CONFIG_SENSE_PRINT_DETECTION_U
            len2 += sprintf(buffer + len2, ",detected_presence,detected_small_movement,detected_large_movement");
#endif

#ifdef CONFIG_SENSE_PRINT_THRESHOLDS_U
            len2 += sprintf(buffer + len2, ",threshold_presence,f_presence,threshold_small_movement,f_small_movement,threshold_large_movement,f_large_movement");
#endif

#ifdef CONFIG_SENSE_PRINT_SLEEP_STAGE_CLASSIFICATION_U
            len2 += sprintf(buffer + len2, ",sleep_stage_classification");
#endif

            len2 += sprintf(buffer + len2, ",sequence,timestamp,source_mac,first_word_invalid,len,rssi,rate,sig_mode,mcs,bandwidth,smoothing,not_sounding,aggregation,stbc,fec_coding,sgi,noise_floor,ampdu_cnt,channel,secondary_channel,local_timestamp,ant,sig_len,rx_state");
#ifdef CONFIG_SENSE_PRINT_CSI_U   
            len2 += sprintf(buffer + len2, ",data");
#endif
            len2 += sprintf(buffer + len2, "\n");
        
        }
        
        len2 += sprintf(buffer + len2, "CSI_DATA");
#ifdef CONFIG_SENSE_PRINT_STI_U
            len2 += sprintf(buffer + len2, ",%f", sti);
#endif

#ifdef CONFIG_SENSE_PRINT_FUSED_HEART_U
            len2 += sprintf(buffer + len2, ",%f", fused_amplitude_heart);
#endif

#ifdef CONFIG_SENSE_PRINT_FUSED_BREATH_U
            len2 += sprintf(buffer + len2, ",%f", fused_amplitude_breath);
#endif

#ifdef CONFIG_SENSE_PRINT_FILTERED_HEART_U
            len2 += sprintf(buffer + len2, ",%f", filtered_heart[current_last_element]);
#endif

#ifdef CONFIG_SENSE_PRINT_FILTERED_BREATH_U
            len2 += sprintf(buffer + len2, ",%f", filtered_breath[current_last_element]);
#endif

#ifdef CONFIG_SENSE_PRINT_HEART_FEATURES_U
            len2 += sprintf(buffer + len2, ",[%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f]", heart_features.instantaneous_peak_rate, heart_features.instantaneous_valley_rate, heart_features.mean_peak_rate_over_window.current_mean, heart_features.mean_valley_rate_over_window.current_mean, heart_features.fft_rate_over_window, heart_features.variance_of_peak_rate_in_window, heart_features.variance_of_valley_rate_in_window, heart_features.mean_up_stroke_length.current_mean, heart_features.mean_down_stroke_length.current_mean, heart_features.up_stroke_length_variance, heart_features.down_stroke_length_variance, heart_features.up_to_down_length_ratio, heart_features.fractional_up_stroke_time, heart_features.mean_up_stroke_amplitude.current_mean, heart_features.mean_down_stroke_amplitude.current_mean, heart_features.up_stroke_amplitude_variance, heart_features.down_stroke_amplitude_variance, heart_features.up_to_down_amplitude_ratio, heart_features.fractional_up_stroke_amplitude);
#endif

#ifdef CONFIG_SENSE_PRINT_BREATHING_FEATURES_U
            len2 += sprintf(buffer + len2, ",[%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f]", breath_features.instantaneous_peak_rate, breath_features.instantaneous_valley_rate, breath_features.mean_peak_rate_over_window.current_mean, breath_features.mean_valley_rate_over_window.current_mean, breath_features.fft_rate_over_window, breath_features.variance_of_peak_rate_in_window, breath_features.variance_of_valley_rate_in_window, breath_features.mean_up_stroke_length.current_mean, breath_features.mean_down_stroke_length.current_mean, breath_features.up_stroke_length_variance, breath_features.down_stroke_length_variance, breath_features.up_to_down_length_ratio, breath_features.fractional_up_stroke_time, breath_features.mean_up_stroke_amplitude.current_mean, breath_features.mean_down_stroke_amplitude.current_mean, breath_features.up_stroke_amplitude_variance, breath_features.down_stroke_amplitude_variance, breath_features.up_to_down_amplitude_ratio, breath_features.fractional_up_stroke_amplitude);
#endif

#ifdef CONFIG_SENSE_PRINT_HEART_POI_U
            len2 += sprintf(buffer + len2, ",%d", heart_found_poi);
            if(heart_found_poi){
                len2 += sprintf(buffer + len2, ",[%d %u %f %d %d]", new_heart_poi->is_peak, new_heart_poi->time_difference_to_previous_poi, new_heart_poi->amplitude_difference_to_previous_poi, new_heart_poi->index, new_heart_poi->index_difference_from_last_intercept);
            }else{
                len2 += sprintf(buffer + len2, ",[]");
            }
#endif

#ifdef CONFIG_SENSE_PRINT_BREATHING_POI_U
            len2 += sprintf(buffer + len2, ",%d", breath_found_poi);
            if(breath_found_poi){
                len2 += sprintf(buffer + len2, ",[%d %u %f %d %d]", new_heart_poi->is_peak, new_heart_poi->time_difference_to_previous_poi, new_heart_poi->amplitude_difference_to_previous_poi, new_heart_poi->index, new_heart_poi->index_difference_from_last_intercept);
            }else{
                len2 += sprintf(buffer + len2, ",[]");
            }
#endif

#ifdef CONFIG_SENSE_PRINT_DETECTION_U
            len2 += sprintf(buffer + len2, ",%d,%d,%d", presence_detected, small_movement_detected, large_movement_detected);
#endif

#ifdef CONFIG_SENSE_PRINT_THRESHOLDS_U
            len2 += sprintf(buffer + len2, ",%f,%f,%f,%f,%f,%f", t_presence, f_presence, t_small_movement, f_small_movement, t_large_movement, f_large_movement);
#endif

#ifdef CONFIG_SENSE_PRINT_SLEEP_STAGE_CLASSIFICATION_U
            len2 += sprintf(buffer + len2, ",%d", sleep_stage);
#endif

        len2 += sprintf(buffer + len2, ",%d,%u," MACSTR ",%d,%d,%d,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%d,%u,%u,%u,%u,%u,%u,%u",
                    count, esp_log_timestamp(),
                    MAC2STR(info->mac), info->first_word_invalid, info->len, rx_ctrl->rssi, rx_ctrl->rate, rx_ctrl->sig_mode,
                    rx_ctrl->mcs, rx_ctrl->cwb, rx_ctrl->smoothing, rx_ctrl->not_sounding,
                    rx_ctrl->aggregation, rx_ctrl->stbc, rx_ctrl->fec_coding, rx_ctrl->sgi,
                    rx_ctrl->noise_floor, rx_ctrl->ampdu_cnt, rx_ctrl->channel, rx_ctrl->secondary_channel,
                    rx_ctrl->timestamp, rx_ctrl->ant, rx_ctrl->sig_len, rx_ctrl->rx_state);

#ifdef CONFIG_SENSE_PRINT_CSI_U          
        len2 += sprintf(buffer + len2, ",\"[%d", info->buf[0]);

        for (int i = 1; i < info->len; i++) {
            len2 += sprintf(buffer + len2, ",%d", info->buf[i]);
        }

        len2 += sprintf(buffer + len2, "]\"");
#endif
        len2 += sprintf(buffer + len2, "\n");

        BufferObject *q_data = malloc_or_die(len2 + sizeof(size_t));
        q_data->length = len2;
        q_data->buffer = malloc_or_die(len2);
        memcpy(q_data->buffer, buffer, len2);

        if (!buffer_queue || xQueueSend(buffer_queue, &q_data, 0) == pdFALSE) {
            ESP_LOGW(TAG, "buffer_queue full");
            free(q_data);
        }
#endif
        button_pressed = false;
        count++;
        free(info);
    }

    free(buffer);
#ifdef CONFIG_SENSE_LOG_TO_SD   
    fclose(file);
    ESP_LOGI(TAG, "File written");
    // All done, unmount partition and disable SDMMC or SPI peripheral
    esp_vfs_fat_sdcard_unmount(MOUNT_POINT, card);
    ESP_LOGI(TAG, "Card unmounted");
    //deinitialize the bus after all devices are removed
    spi_bus_free(host.slot);
#endif
    vTaskDelete(NULL);
}


static void csi_callback(void *ctx, wifi_csi_info_t *info)
{
    int result = memcmp(info->mac, csi_sender_mac, sizeof(info->mac));

    //ESP_LOGI(TAG, "CSI_DATA,  received mac " MACSTR ", compared to " MACSTR ", result %d", MAC2STR(csi_sender_mac), MAC2STR(info->mac), result);
    // ESP_LOGI(TAG, "CSI_DATA2, " MACSTR ", result %d", 1, MAC2STR(temp), result);
    if(result==0){
        wifi_csi_info_t *q_data = malloc_or_die(sizeof(wifi_csi_info_t) + info->len);
        *q_data = *info;
        memcpy(q_data->buf, info->buf, info->len);

        if (!g_csi_info_queue || xQueueSend(g_csi_info_queue, &q_data, 0) == pdFALSE) {
            ESP_LOGW(TAG, "g_csi_info_queue full");
            free(q_data);
        }
    }
}

void wifi_init_softap(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
            .channel = EXAMPLE_ESP_WIFI_CHANNEL,
            .password = EXAMPLE_ESP_WIFI_PASS,
            .max_connection = EXAMPLE_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK,
            .pmf_cfg = {
                    .required = false,
            },
        },
    };
    if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    // disable modem sleep for more accurate Wi-Fi frame timestamps
    esp_wifi_set_ps(WIFI_PS_NONE);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS, EXAMPLE_ESP_WIFI_CHANNEL);

    ESP_ERROR_CHECK(esp_wifi_set_csi_rx_cb(csi_callback, NULL));

    // enable CSI collection
    ESP_ERROR_CHECK(esp_wifi_set_csi(true));

    // set the csi configuration -- this code only uses the LLTF subcarriers due to processing limitations, so only those are enabed
    wifi_csi_config_t csi_config = {
        .lltf_en = true,
        .htltf_en = false,
        .stbc_htltf2_en = false,
        .ltf_merge_en = false,
        .channel_filter_en = false,
        .manu_scale = false,
    };

    ESP_ERROR_CHECK(esp_wifi_set_csi_config(&csi_config));
}

#ifdef CONFIG_SENSE_LOG_TO_SD
    void init_sdcard(void)
{
    esp_err_t ret;
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };

    const char mount_point[] = MOUNT_POINT;
    ESP_LOGI(TAG, "Initializing SD card");
    ESP_LOGI(TAG, "Using SPI peripheral");

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };
    ret = spi_bus_initialize(host.slot, &bus_cfg, SPI_DMA_CHAN);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize bus.");
        return;
    }

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_CS;
    slot_config.host_id = host.slot;

    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                     "If you want the card to be formatted, set the EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                     "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return;
    }
    ESP_LOGI(TAG, "Filesystem mounted");

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);
}
#endif

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

static void gpio_task_example(void* arg)
{
    uint32_t io_num;
    for(;;) {
        if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            printf("GPIO[%"PRIu32"] intr, val: %d\n", io_num, gpio_get_level(io_num));
            button_pressed = true;
        }
    }
}

void app_main(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // initialize gpio for interrupt
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    io_conf.pin_bit_mask = (1ULL << GPIO_INPUT_PIN);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(GPIO_INPUT_PIN, gpio_isr_handler, (void*) GPIO_INPUT_PIN);

    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    //start gpio task
    xTaskCreate(gpio_task_example, "gpio_task_example", 2048, NULL, 10, NULL);


    // open nvs to restore threshold values
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } else {
        // Read the values
        ESP_LOGI(TAG, "Reading threshold values from NVS ... ");

        float default_value = -1.0;
        uint32_t int_t_presence = 0;
        uint32_t int_t_small_movement = 0;
        uint32_t int_t_large_movement = 0;
        memcpy(&int_t_presence, &default_value, sizeof(uint32_t));
        memcpy(&int_t_small_movement, &default_value, sizeof(uint32_t));
        memcpy(&int_t_large_movement, &default_value, sizeof(uint32_t));

        err = nvs_get_u32(my_handle, "thresh_pres", &int_t_presence);
        switch (err) {
            case ESP_OK:
                memcpy(&t_presence, &int_t_presence, sizeof(float));
                ESP_LOGI(TAG, "t presence = %F", t_presence);
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                memcpy(&t_presence, &int_t_presence, sizeof(float));
                ESP_LOGI(TAG, "The value is not initialized yet! Using default: %f", t_presence);
                break;
            default :
                ESP_LOGE(TAG, "Error (%s) reading!", esp_err_to_name(err));
        }

        err = nvs_get_u32(my_handle, "thresh_s_mov", &int_t_small_movement);
        switch (err) {
            case ESP_OK:
                memcpy(&t_small_movement, &int_t_small_movement, sizeof(float));
                ESP_LOGI(TAG, "t small_movement = %F", t_small_movement);
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                memcpy(&t_small_movement, &int_t_small_movement, sizeof(float));
                ESP_LOGI(TAG, "The value is not initialized yet! Using default: %f", t_small_movement);
                break;
            default :
                ESP_LOGE(TAG, "Error (%s) reading!", esp_err_to_name(err));
        }

        err = nvs_get_u32(my_handle, "thresh_l_mov", &int_t_large_movement);
        switch (err) {
            case ESP_OK:
                memcpy(&t_large_movement, &int_t_large_movement, sizeof(float));
                ESP_LOGI(TAG, "t large_movement = %F", t_large_movement);
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                memcpy(&t_large_movement, &int_t_large_movement, sizeof(float));
                ESP_LOGI(TAG, "The value is not initialized yet! Using default: %f", t_large_movement);
                break;
            default :
                ESP_LOGE(TAG, "Error (%s) reading!", esp_err_to_name(err));
        }
    }


#ifdef CONFIG_SENSE_LOG_TO_SD
    init_sdcard();
#endif

    esp_base_mac_addr_set(customBaseMAC);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_AP");
    wifi_init_softap();
    esp_wifi_set_protocol(WIFI_IF_AP, WIFI_PROTOCOL_11N);
    esp_wifi_config_80211_tx_rate(WIFI_IF_AP, WIFI_PHY_RATE_MCS7_SGI);
    esp_wifi_set_bandwidth(WIFI_IF_AP, WIFI_BW_HT20);

    float_arr_alloc(MAX_NUMBER_OF_SAMPLES_KEPT, NUMBER_SUBCARRIERS, &amplitude_array);

    // setup_broadcast_messages();
    analysis_init();
    ESP_LOGI(TAG, "before model setup");

#ifdef CONFIG_RUN_INFERENCE
    model_setup();
    ESP_LOGI(TAG, "completed model setup");

    // run inference with dummy data so that potential errors show up directly after starting
    float input_array[] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
    run_inference(&input_array);
    ESP_LOGI(TAG, "ran dummy inference");
#endif

    g_csi_info_queue = xQueueCreate(256, sizeof(void *));
    buffer_queue = xQueueCreate(2048, sizeof(void *));
    xTaskCreate(csi_processing_task, "csi_data_print", 8 * 1024, NULL, 0, NULL);
    xTaskCreate(udp_client_task, "udp_client_task", 4 * 1024, NULL, 0, NULL);
    xTaskCreate(udp_calibration_task, "udp_calibration_task", 4 * 1024, NULL, 0, NULL);
#ifdef CONFIG_RUN_INFERENCE
    xTaskCreate(run_inference_task, "run_inference_task", 4*1024, NULL, 0, NULL);
#endif
    ESP_LOGI(TAG, "completed main");
}
