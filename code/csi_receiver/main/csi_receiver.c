#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_err.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "esp_vfs_fat.h"
#include "driver/sdspi_host.h"
#include "driver/spi_common.h"
#include "sdmmc_cmd.h"
#include "sdkconfig.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "rom/ets_sys.h"


#include "lwip/sockets.h"
#include <lwip/netdb.h>

#include "math.h"

/* The examples use WiFi configuration that you can set via project configuration menu.

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define EXAMPLE_ESP_WIFI_SSID      CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS      CONFIG_ESP_WIFI_PASSWORD
#define EXAMPLE_ESP_WIFI_CHANNEL   CONFIG_ESP_WIFI_CHANNEL
#define EXAMPLE_MAX_STA_CONN       CONFIG_ESP_MAX_STA_CONN

static const char *TAG = "wifi softAP";  // TODO change to something more appropriate
static QueueHandle_t g_csi_info_queue    = NULL;

// defines for UDP broadcast
#define HOST_IP_ADDR "192.168.4.255" // broadcastp address for the standard AP-network
#define PORT 8081


// defines for determining how much data to keep for analysis → adjust according to how much RAM is available
#define NUMBER_SUBCARRIERS 64 // we only use LLTF so get 64 subcarriers -TODO: change to usable subcarriers only to save RAM

#define CSI_RATE 50 // number of expected samples per second
#define BROADCAST_RATE 0.5 // rate in Hz -> every 2 seconds
#define TIME_RANGE_TO_KEEP 2 // in seconds

#define MAX_NUMBER_OF_SAMPLES_KEPT  TIME_RANGE_TO_KEEP*CSI_RATE

// uncomment the next line to enable logging the raw CSI to the serial output of the ESP
// #define PRINT_CSI_TO_SERIAL

// uncomment the next line to enable logging the raw CSI to SD-card
// #define WRITE_TO_SD_CARD

#ifdef WRITE_TO_SD_CARD
    // defines for using the sd card
    #define MOUNT_POINT "/sdcard"

    #define SPI_DMA_CHAN    1
    #define PIN_NUM_MISO 2
    #define PIN_NUM_MOSI 15
    #define PIN_NUM_CLK  14
    #define PIN_NUM_CS   13

    #define MAX_NAME_LEN 32

    sdmmc_host_t host;
    sdmmc_card_t *card;
    char *file_name;
    int16_t file_count = 0;
    FILE *file;

    #define WRITE_FILE_AFTER_RECEIVED_CSI_NUMBER MAX_NUMBER_OF_SAMPLES_KEPT
#endif

// filters for sampling frequency of 50Hz
#define NUMBER_COEFFICIENTS 3
float breathing_bandpass_coefficients[2][NUMBER_COEFFICIENTS] = {{0.024521609249465892, 0.0, -0.024521609249465892}, {1.0, -1.9501864631153991, 0.9509567815010684}}; //  b and a
float heart_bandpass_coefficients[2][NUMBER_COEFFICIENTS] = {{0.07295965726826666, 0.0, -0.07295965726826666}, {1.0, -1.8321199891810072, 0.8540806854634666}};



float (*amplitude_array)[MAX_NUMBER_OF_SAMPLES_KEPT][NUMBER_SUBCARRIERS];
unsigned timestamp_array[MAX_NUMBER_OF_SAMPLES_KEPT];
static int16_t current_first_element = -1;
static int16_t current_last_element = -1;

float amplitude_mean;
float previous_amplitude_hat[NUMBER_SUBCARRIERS];
float sti_array[MAX_NUMBER_OF_SAMPLES_KEPT];


// used for easy definition of things to broadcast
typedef size_t (*functype)(char *buffer, size_t len);
typedef struct
{
    size_t len;
    size_t elements;
    functype *list;
} FunctionList;

FunctionList broadcast_functions;

typedef struct
{
    float *a_coefficients;
    float *b_coefficients;
    int number_coefficients;
    float *last_inputs;
    float *last_outputs;
    int last_position;
    float out;
    int received_inputs;
} BandpassIIRFilter;

BandpassIIRFilter breathing_filter;
BandpassIIRFilter heart_filter;


void bandpass_filter_initialize(BandpassIIRFilter *f, float *b_coefficients, float *a_coefficients, int number_coefficients){
    ESP_LOGI(TAG, "got here");
    printf("t");
    printf("hello");
    f->number_coefficients = number_coefficients;
    f->b_coefficients = b_coefficients;
    f->a_coefficients = a_coefficients;
    
    f->last_inputs = malloc(number_coefficients * sizeof(float));
    f->last_outputs = malloc(number_coefficients * sizeof(float));
    for(int i=0; i<number_coefficients; i++){
        f->last_inputs[i] = 0;
        f->last_outputs[i] = 0;
    }
    f->out = 0;
    f->received_inputs = 0;
    f->last_position = 0;
}


// TODO
void bandpass_filter_free(BandpassIIRFilter *f){
    free(f->last_inputs);
    free(f->last_outputs);
}

void bandpass_filter_apply(BandpassIIRFilter *f, float input){
    
    if(f->received_inputs+1 > f->number_coefficients)
    {
        f->received_inputs++;   
    }
    
    float out = f->b_coefficients[0] * input;

    for(int i=1; i<f->number_coefficients; i++)
    {
        int index = f->last_position - i + 1;
        if(index < 0){
            index = f->number_coefficients - 1;
        }
        out += f->b_coefficients[i] * f->last_inputs[index];
        out -= f->a_coefficients[i] * f->last_outputs[index];
    }

    f->out = out;
    int index = f->last_position+1;
    if(index >= f->number_coefficients){
        index = 0;
    }
    f->last_position = index;
    f->last_outputs[index] = out;
    f->last_inputs[index] = input;
}


void create_function_list(FunctionList *l, size_t initialSize){
    l->list = malloc(initialSize*sizeof(functype));
    l->len = initialSize;
    l->elements = 0;
}

void append_to_function_list(FunctionList *l, functype function_to_append){
    if(l->elements == l->len){
        l->len *= 2;
        l->list = realloc(l->list, l->len * sizeof(functype));
    }
    l->list[l->elements++] = function_to_append;
}

void arr_alloc(size_t x, size_t y, float(**aptr)[x][y])
{
  *aptr = malloc( sizeof(float[x][y]) ); // allocate a true 2D array
  assert(*aptr != NULL);
}



size_t print_sti_to_buffer(char *buffer, size_t len){
    // sti values        
    if(current_last_element >= current_first_element)
    {
        len += sprintf(buffer + len, "[%f", sti_array[current_first_element]);
        for (int i = current_first_element+1; i <= current_last_element; i++) 
        {
            len += sprintf(buffer + len, ",%f", sti_array[i]);
        }
        len += sprintf(buffer + len, "]\n");
    }
    else
    {
        len += sprintf(buffer + len, "[%f", sti_array[current_first_element]);
        for (int i = current_first_element+1; i < MAX_NUMBER_OF_SAMPLES_KEPT; i++) {
            len += sprintf(buffer + len, ",%f", sti_array[i]);
        }
        for (int i = 0+1; i <= current_last_element; i++) {
            len += sprintf(buffer + len, ",%f", sti_array[i]);
        }
        len += sprintf(buffer + len, "]\n");
    }

    return len;
}

size_t print_heart_rate_to_buffer(char *buffer, size_t len){ //TODO: currently only dummy data
    len += sprintf(buffer + len, "65");
    return len;
}

size_t print_breathing_rate_to_buffer(char *buffer, size_t len){ //TODO: currently only dummy data
    len += sprintf(buffer + len, "18");
    return len;
}



void setup_broadcast_messages(){
    // define what goes into the udp broadcast messages
    create_function_list(&broadcast_functions, 5);
    
    // heart rate
    append_to_function_list(&broadcast_functions, print_heart_rate_to_buffer);

    append_to_function_list(&broadcast_functions, print_breathing_rate_to_buffer);

    // sti
    append_to_function_list(&broadcast_functions, print_sti_to_buffer);

}

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
            char *buffer = malloc(8 * 1024);
            size_t len = 0;
            // create payload

            // add a message to the start
            len += sprintf(buffer + len, "Message from ESP32;");


            // use the broadcast functions to generate the payload
            for(int i=0; i<broadcast_functions.elements-1; i++)
            {
                ESP_LOGI(TAG, "function");
                len = (*broadcast_functions.list[i])(buffer, len);
                len += sprintf(buffer + len, ";"); // separate with semicolon
            }
            len = (*broadcast_functions.list[broadcast_functions.elements-1])(buffer, len);


            // TODO amplitude array

            int err = sendto(sock, buffer, len, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
            if (err < 0) {
                ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                break;
            }
            ESP_LOGI(TAG, "Message sent");
            free(buffer);
       
            vTaskDelay((1000/BROADCAST_RATE) / portTICK_PERIOD_MS);
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

void analysis_init(void){
    bandpass_filter_initialize(&breathing_filter, breathing_bandpass_coefficients[0], breathing_bandpass_coefficients[1], NUMBER_COEFFICIENTS);
    bandpass_filter_initialize(&heart_filter, heart_bandpass_coefficients[0], heart_bandpass_coefficients[1], NUMBER_COEFFICIENTS);
}

static void csi_data_print_task(void *arg)
{
    wifi_csi_info_t *info = NULL;
    char *buffer = malloc(8 * 1024);
    static uint32_t count = 0;

#ifdef WRITE_TO_SD_CARD
    if(file==NULL)
    {
        ESP_LOGI(TAG, "Opening file initially");
        // Check if destination file exists before renaming
        
        file_name = malloc(MAX_NAME_LEN);
        sprintf(file_name, MOUNT_POINT"/csi.csv");
        struct stat st;
        int16_t i = 0;
        while (stat(file_name, &st) == 0) {
            // change the file name
            sprintf(file_name, MOUNT_POINT"/csi_%d.csv", i++);
        }
        file = fopen(file_name, "a");
    }

    static uint32_t write_count = 0;
#endif

    while (xQueueReceive(g_csi_info_queue, &info, portMAX_DELAY)) {
        size_t len = 0;
        wifi_pkt_rx_ctrl_t *rx_ctrl = &info->rx_ctrl;

        float amplitude[NUMBER_SUBCARRIERS];
        // float phase[NUMBER_SUBCARRIERS];
        
        // calculate amplitude and phase
        float sum = 0;
        for (int i=0 ; i < NUMBER_SUBCARRIERS; i++)
        {
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

        float sti;
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
            // to-do: store the STI value, check if it is larger than a certain threshold and output the result of the presence detection
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
        
        sti_array[current_last_element] = sti;
        memcpy((*amplitude_array)[current_last_element], amplitude, sizeof(amplitude));
        timestamp_array[current_last_element] = rx_ctrl->timestamp;

        // test the band pass filters on one selected subcarrier amplitude
        bandpass_filter_apply(&breathing_filter, amplitude[42]);
        printf("Bandpass_test,%f,%f\n", amplitude[42], breathing_filter.out);



        if(current_last_element == MAX_NUMBER_OF_SAMPLES_KEPT-1){
            ESP_LOGI(TAG, "full amplitude queue");
            printf("current_first_element %d, current last element %d \n", current_first_element, current_last_element);
            char *buffer1 = malloc(8 * 1024);
            size_t len1 = 0;
            if(current_last_element >= current_first_element)
            {
                len1 += sprintf(buffer1 + len1, "\"[%f", sti_array[current_first_element]);
                for (int i = current_first_element+1; i <= current_last_element; i++) 
                {
                    len1 += sprintf(buffer1 + len1, ",%f", sti_array[i]);
                }
                len1 += sprintf(buffer1 + len1, "]\"\n");
            }
            else
            {
                len1 += sprintf(buffer1 + len1, "\"[%f", sti_array[current_first_element]);
                for (int i = current_first_element+1; i < MAX_NUMBER_OF_SAMPLES_KEPT; i++) {
                    len1 += sprintf(buffer1 + len1, ",%f", sti_array[i]);
                }
                for (int i = 0+1; i <= current_last_element; i++) {
                    len1 += sprintf(buffer1 + len1, ",%f", sti_array[i]);
                }
                len1 += sprintf(buffer1 + len1, "]\"\n");
            }
            printf("%s", buffer1);
            free(buffer1);
            
        }



        // printing to serial
#ifdef PRINT_CSI_TO_SERIAL || WRITE_TO_SD_CARD
        if (!count) {
            ESP_LOGI(TAG, "================ CSI RECV ================");
            len += sprintf(buffer + len, "type,sequence,timestamp,source_mac,first_word_invalid,len,rssi,rate,sig_mode,mcs,bandwidth,smoothing,not_sounding,aggregation,stbc,fec_coding,sgi,noise_floor,ampdu_cnt,channel,secondary_channel,local_timestamp,ant,sig_len,rx_state,data\n");
        }


        len += sprintf(buffer + len, "CSI_DATA,%d,%u," MACSTR ",%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%u,%d,%d,%d,%d,%d,",
                       count++, esp_log_timestamp(),
                       MAC2STR(info->mac), info->first_word_invalid, info->len, rx_ctrl->rssi, rx_ctrl->rate, rx_ctrl->sig_mode,
                       rx_ctrl->mcs, rx_ctrl->cwb, rx_ctrl->smoothing, rx_ctrl->not_sounding,
                       rx_ctrl->aggregation, rx_ctrl->stbc, rx_ctrl->fec_coding, rx_ctrl->sgi,
                       rx_ctrl->noise_floor, rx_ctrl->ampdu_cnt, rx_ctrl->channel, rx_ctrl->secondary_channel,
                       rx_ctrl->timestamp, rx_ctrl->ant, rx_ctrl->sig_len, rx_ctrl->rx_state);

        
        len += sprintf(buffer + len, "\"[%d", info->buf[0]);

        for (int i = 1; i < info->len; i++) {
            len += sprintf(buffer + len, ",%d", info->buf[i]);
        }

        len += sprintf(buffer + len, "]\"\n");
#endif

#ifdef PRINT_CSI_TO_SERIAL
        printf("%s", buffer);
#endif
        
#ifdef WRITE_TO_SD_CARD
        ESP_LOGI(TAG, "================ printing to file ================");
        fprintf(file, "%s", buffer);
        

        write_count++;
        if (write_count > WRITE_FILE_AFTER_RECEIVED_CSI_NUMBER) // TODO:  change this to an appropriate number (depending on the CSI rate)
        {
            write_count = 0;
            ESP_LOGI(TAG, "================ Closing file ================");
            fflush(file);
            fclose(file);
            ESP_LOGI(TAG, "================  file closed ================");
            ESP_LOGI(TAG, "Opening file");
            file = fopen(file_name, "a");
        }
#endif
        free(info);
    }

    free(buffer);
#ifdef WRITE_TO_SD_CARD   
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
    wifi_csi_info_t *q_data = malloc(sizeof(wifi_csi_info_t) + info->len);
    *q_data = *info;
    memcpy(q_data->buf, info->buf, info->len);


    if (!g_csi_info_queue || xQueueSend(g_csi_info_queue, &q_data, 0) == pdFALSE) {
        ESP_LOGW(TAG, "g_csi_info_queue full");
        free(q_data);
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

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS, EXAMPLE_ESP_WIFI_CHANNEL);

    // set to promiscuous mode to use as sniffer and get more CSI from unconnected devices
    //ESP_ERROR_CHECK(esp_wifi_set_promiscuous(true));
    ESP_ERROR_CHECK(esp_wifi_set_csi_rx_cb(csi_callback, NULL));


    // enable CSI collection
    esp_err_t ret = esp_wifi_set_csi(true);
    ESP_LOGI(TAG, "csi returned%d",
             ret);


}


#ifdef WRITE_TO_SD_CARD
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



void app_main(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

#ifdef WRITE_TO_SD_CARD
    init_sdcard();
#endif

    ESP_LOGI(TAG, "ESP_WIFI_MODE_AP");
    wifi_init_softap();
    esp_wifi_set_protocol(WIFI_IF_AP, WIFI_PROTOCOL_11N);
    esp_wifi_config_80211_tx_rate(WIFI_IF_AP, WIFI_PHY_RATE_MCS7_SGI);
    esp_wifi_set_bandwidth(WIFI_IF_AP, WIFI_BW_HT40);

    arr_alloc(MAX_NUMBER_OF_SAMPLES_KEPT, NUMBER_SUBCARRIERS, &amplitude_array);

    setup_broadcast_messages();
    analysis_init();

    g_csi_info_queue = xQueueCreate(256, sizeof(void *));
    xTaskCreate(csi_data_print_task, "csi_data_print", 4 * 1024, NULL, 0, NULL);
    xTaskCreate(udp_client_task, "udp_client_task", 4 * 1024, NULL, 0, NULL);
}
