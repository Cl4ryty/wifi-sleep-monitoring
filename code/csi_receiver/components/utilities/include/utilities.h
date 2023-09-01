#include <stdio.h>
#include "esp_system.h"
#include "esp_log.h"
#include "math.h"
#include "float.h"
#include <string.h>


typedef struct
{
    int length;
    int *list;
    int current_last_element_position;
    int current_first_element_position;
}CircularIntList;

// used for easy definition of things to broadcast
typedef size_t (*functype)(char *buffer, size_t len);
typedef struct
{
    size_t len;
    size_t elements;
    functype *list;
} FunctionList;


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

typedef struct
{
    int last_sample_index;
    int array_size;
    unsigned *time_array;
    float *value_array;
    int current_number_of_elements;
    float current_sum;
    float current_mean;
} RunningMean;

typedef struct
{
    int last_sample_index;
    int newest_sample_index;
    int array_size;
    unsigned *time_array;
    float *value_array;
    int current_number_of_elements;
    float current_sum;
    float current_mean;
} DumbRunningMean;

typedef struct
{
    int index;
    bool is_up_intercept;
} Intercept;

typedef struct
{
    RunningMean MAC;
    Intercept *intercepts;
    int current_last_intercept;
    int current_first_intercept;
    RunningMean mean_peak_to_valley_amplitude;
    float *peak_to_valley_amplitudes;
    int peak_to_valley_last_index;
    CircularIntList peak_indices;
    CircularIntList valley_indices;
    int array_len;
    bool running_mean_initialized;
    bool first_intercept;
} MAC_struct;

typedef struct
{   
    float instantaneous_peak_rate;
    float instantaneous_valley_rate;
    DumbRunningMean mean_peak_rate_over_window;
    DumbRunningMean mean_valley_rate_over_window; // these two means are redundant because they can easily be calculated from the mean_stroke_length
    float fft_rate_over_window;
    float variance_of_peak_rate_in_window;
    float variance_of_valley_rate_in_window;

    DumbRunningMean mean_up_stroke_length;
    DumbRunningMean mean_down_stroke_length;
    float up_stroke_length_variance;
    float down_stroke_length_variance;
    float up_to_down_length_ratio;
    float fractional_up_stroke_time;

    DumbRunningMean mean_up_stroke_amplitude;
    DumbRunningMean mean_down_stroke_amplitude;
    float up_stroke_amplitude_variance;
    float down_stroke_amplitude_variance;
    float up_to_down_amplitude_ratio;
    float fractional_up_stroke_amplitude;
} Features;

typedef struct
{
    bool is_peak;
    unsigned time_difference_to_previous_poi;
    float amplitude_difference_to_previous_poi;
    int index;
    int index_difference_from_last_intercept;
} POI;

typedef struct
{
    POI *list;
    int first_element_index;
    int last_element_index;
    int length;
    int number_of_elements;
} POI_List;

typedef struct
{
    size_t length;
    size_t elements;
    int *list;
}ListInt;

typedef struct
{
    size_t length;
    size_t elements;
    char *list;
}ListChar;

typedef struct
{
    size_t length;
    size_t elements;
    float *list;
}ListFloat;

typedef struct
{
    unsigned window_size;
    float* window;
    unsigned number_of_samples;
    unsigned current_last_index;
    unsigned current_center_index;
}HampelFilter;




void *malloc_or_die(size_t size_to_allocate);

void float_arr_alloc(size_t x, size_t y, float(**aptr)[x][y]);

void int8_arr_alloc(size_t x, size_t y, int8_t(**aptr)[x][y]);

int get_previous_index(int current_index, int array_length);

int get_next_index(int current_index, int array_length);

void print_x(float *x, int x_length);

void circular_list_initialize(CircularIntList *l, int length);

void circular_list_append(CircularIntList *l, int input);

void create_function_list(FunctionList *l, size_t initialSize);

void append_to_function_list(FunctionList *l, functype function_to_append);

void create_list_char(ListChar *l, size_t initialSize);

void append_to_list_char(ListChar *l, char new_element);

void free_list_char(ListChar *l);

void create_list_int(ListInt *l, size_t initialSize);

void append_to_list_int(ListInt *l, int new_element);

void free_list_int(ListInt *l);

void create_list_float(ListFloat *l, size_t initialSize);

void append_to_list_float(ListFloat *l, float new_element);

void free_list_float(ListFloat *l);

void bandpass_filter_initialize(BandpassIIRFilter *f, float *b_coefficients, float *a_coefficients, int number_coefficients);

void bandpass_filter_apply(BandpassIIRFilter *f, float input);

void bandpass_filter_free(BandpassIIRFilter *f);

void running_mean_initialize(RunningMean *r, float input, int input_index, unsigned *time_array, float *value_array, int array_size);

void running_mean_append(RunningMean *r, float input, unsigned input_timestamp, unsigned window_size);

void dumb_running_mean_initialize(DumbRunningMean *r, int array_size);

void dumb_running_mean_append(DumbRunningMean *r, float input, unsigned input_timestamp, unsigned window_size);

void intercept_initialize(Intercept *i, int index, bool is_up_intercept);

void MAC_struct_check_if_indices_become_invalid(MAC_struct *m, int new_last_index);

void features_initialize(Features *f);

void poi_initialize(POI *poi, bool is_peak, unsigned time_difference, float amplitude_difference, int index, int index_difference_from_last_intercept);

void poi_list_initialize(POI_List *poi_list, int length);

void poi_list_reset(POI_List *poi_list);

void poi_list_append(POI_List *poi_list, POI *poi);

void calculate_variance_features(Features *f, POI_List *pois);

void get_best_thresholds(float *stis, char *ids, int length, float *t_presence, float *t_small_movement, float *t_large_movement, float *f_presence, float *f_small_movement, float *f_large_movement);

void initialize_hampel_filter(HampelFilter *f, unsigned window_size);

int float_compare(const void *x, const void *y);

float delayed_hampel_filter(HampelFilter *f, float input);