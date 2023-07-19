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
} POI;

typedef struct
{
    POI *list;
    int first_element_index;
    int last_element_index;
    int length;
    int number_of_elements;
} POI_List;


void *malloc_or_die(size_t size_to_allocate){
    void *p = malloc(size_to_allocate);
    if(p == NULL){
        ESP_LOGE("malloc or die", "failed to allocate memory");
        abort();
    }
    return p;
}

void arr_alloc(size_t x, size_t y, float(**aptr)[x][y])
{
  *aptr = malloc_or_die( sizeof(float[x][y]) ); // allocate a true 2D array
}

int get_previous_index(int current_index, int array_length){
    int previous_index = current_index - 1;
    if(previous_index < 0){
        previous_index = array_length - 1;
    }
    return previous_index;
}

int get_next_index(int current_index, int array_length){
    int next_index = current_index + 1;
    if(next_index >= array_length){
        next_index = 0;
    }
    return next_index;
}

void circular_list_initialize(CircularIntList *l, int length){
    l->list = malloc_or_die(sizeof(int) * length);
    l->length = length;
    l->current_last_element_position = -1;
    l->current_first_element_position = -1;
}

void circular_list_append(CircularIntList *l, int input){
    if(l->current_last_element_position == -1){
        l->current_last_element_position = 0;
        l->current_first_element_position = 0;
    }else{
        l->current_last_element_position = get_next_index(l->current_last_element_position, l->length);
        if(l->current_last_element_position == l->current_first_element_position){
            l->current_first_element_position = get_next_index(l->current_first_element_position, l->length);
        }
    }
    l->list[l->current_last_element_position] = input;
}

void create_function_list(FunctionList *l, size_t initialSize){
    l->list = malloc_or_die(initialSize*sizeof(functype));
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

void bandpass_filter_initialize(BandpassIIRFilter *f, float *b_coefficients, float *a_coefficients, int number_coefficients){
    f->number_coefficients = number_coefficients;
    f->b_coefficients = b_coefficients;
    f->a_coefficients = a_coefficients;
    
    f->last_inputs = malloc_or_die(number_coefficients * sizeof(float));
    f->last_outputs = malloc_or_die(number_coefficients * sizeof(float));
    for(int i=0; i<number_coefficients; i++){
        f->last_inputs[i] = 0;
        f->last_outputs[i] = 0;
    }
    f->out = 0;
    f->received_inputs = 0;
    f->last_position = 0;
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
    int index = get_next_index(f->last_position, f->number_coefficients);
    f->last_position = index;
    f->last_outputs[index] = out;
    f->last_inputs[index] = input;
}

void running_mean_initialize(RunningMean *r, float input, int input_index, unsigned *time_array, float *value_array, int array_size){
    r->last_sample_index = input_index;
    r->time_array = time_array;
    r->value_array = value_array;
    r->current_number_of_elements = 1;
    r->current_sum = input;
    r->current_mean = input;
    r->array_size = array_size;
}

void running_mean_append(RunningMean *r, float input, unsigned input_timestamp, unsigned window_size){
    // check if the last sample fell out of the time frame and remove in necessary
    while(input_timestamp - r->time_array[r->last_sample_index] > window_size){
        r->current_sum -= r->value_array[r->last_sample_index];
        r->current_number_of_elements -= 1;
        r->last_sample_index = get_next_index(r->last_sample_index, r->array_size);
    }
    r->current_sum += input;
    r->current_number_of_elements += 1;
    r->current_mean = r->current_sum / r->current_number_of_elements;
}

void dumb_running_mean_initialize(DumbRunningMean *r, int array_size){
    r->last_sample_index = 0;
    r->newest_sample_index = 0;
    r->time_array = malloc_or_die(sizeof(unsigned) * array_size);
    r->value_array = malloc_or_die(sizeof(float) * array_size);
    r->current_number_of_elements = 0;
    r->current_sum = 0;
    r->current_mean = 0;
    r->array_size = array_size;
}

void dumb_running_mean_append(DumbRunningMean *r, float input, unsigned input_timestamp, unsigned window_size){
    // check if the last sample fell out of the time frame and remove in necessary
    while(r->current_number_of_elements > 0 && (input_timestamp - r->time_array[r->last_sample_index] > window_size)){
        r->current_sum -= r->value_array[r->last_sample_index];
        r->current_number_of_elements -= 1;
        r->last_sample_index = get_next_index(r->last_sample_index, r->array_size);
    }
    // append the newest sample to the array
    int index = get_next_index(r->newest_sample_index, r->array_size);
    r->time_array[index] = input_timestamp;
    r->value_array[index] = input;
    if(index == r->last_sample_index){
        r->last_sample_index = get_next_index(r->last_sample_index, r->array_size);
        r->current_number_of_elements -= 1;
    }

    r->current_sum += input;
    r->current_number_of_elements += 1;
    r->current_mean = r->current_sum / r->current_number_of_elements;
}

void intercept_initialize(Intercept *i, int index, bool is_up_intercept){
    i->index = index;
    i->is_up_intercept = is_up_intercept;
}

void MAC_struct_check_if_indices_become_invalid(MAC_struct *m, int new_last_index){
    // check if the current first peak/valley index becomes invalid with this index shift
    if(new_last_index == m->peak_indices.current_first_element_position){
        m->peak_indices.current_first_element_position = get_next_index(m->peak_indices.current_first_element_position, m->peak_indices.length);
    }

    if(new_last_index == m->valley_indices.current_first_element_position){
        m->valley_indices.current_first_element_position = get_next_index(m->valley_indices.current_first_element_position, m->valley_indices.length);
    }
}

void features_initialize(Features *f){

    f->instantaneous_peak_rate = -1;
    f->instantaneous_valley_rate = -1;
    f->fft_rate_over_window = -1;
    f->variance_of_peak_rate_in_window = -1;
    f->variance_of_valley_rate_in_window = -1;
    f->up_stroke_length_variance = -1;
    f->down_stroke_length_variance = -1;
    f->up_to_down_length_ratio = -1;
    f->fractional_up_stroke_time = -1;
    f->up_stroke_amplitude_variance = -1;
    f->down_stroke_amplitude_variance = -1;
    f->up_to_down_amplitude_ratio = -1;
    f->fractional_up_stroke_amplitude = -1;
}

void poi_initialize(POI *poi, bool is_peak, unsigned time_difference, float amplitude_difference, int index){
    poi->is_peak = is_peak;
    poi->time_difference_to_previous_poi = time_difference;
    poi->amplitude_difference_to_previous_poi = amplitude_difference;
    poi->index = index;
}

void poi_list_initialize(POI_List *poi_list, int length){
    poi_list->first_element_index = 0;
    poi_list->last_element_index = 0;
    poi_list->length = length;
    poi_list->number_of_elements = 0;
    poi_list->list = malloc_or_die(sizeof(POI) * length);
}

void poi_list_append(POI_List *poi_list, POI *poi){

    if(poi_list->number_of_elements == 0){
        memcpy(&poi_list->list[0], poi, sizeof(POI));
    }else{
        int index = get_next_index(poi_list->last_element_index, poi_list->length);
        memcpy(&poi_list->list[index], poi, sizeof(POI));
        poi_list->last_element_index = index;
        if(poi_list->first_element_index == index){
            poi_list->first_element_index = get_next_index(poi_list->first_element_index, poi_list->length);
        }

    }
    poi_list->number_of_elements++;
}

void calculate_variance_features(Features *f, POI_List *pois){
    // assumes that the means are already set
    // calculate all peak-related features
    ESP_LOGE("calculate_variance_features", "start");
    int number_of_pois = 0;
    float peak_rate_variance = 0;
    float up_stroke_length_variance = 0;
    float up_stroke_amplitude_variance = 0;

    bool starting_with_peaks = false;
    if(pois->list[pois->first_element_index].is_peak){
        starting_with_peaks = true;
    }
    bool check_even_indices = true;
    if((starting_with_peaks && ( (pois->first_element_index % 2) != 0) )  || ( !starting_with_peaks && ( (pois->first_element_index % 2) == 0)) ){
        check_even_indices = false;
    }
    for(int i=pois->first_element_index; i!=pois->last_element_index; i=get_next_index(i, pois->length)){
        if((check_even_indices && i%2==0) || (!check_even_indices && i%2!=0)){
            number_of_pois++;
            peak_rate_variance += powf(pois->list[i].time_difference_to_previous_poi - f->mean_peak_rate_over_window.current_mean, 2);
            up_stroke_length_variance += powf(pois->list[i].time_difference_to_previous_poi - f->mean_up_stroke_length.current_mean, 2);
            up_stroke_amplitude_variance += powf(pois->list[i].amplitude_difference_to_previous_poi - f->mean_up_stroke_amplitude.current_mean, 2);
        }
    }
    peak_rate_variance /= number_of_pois;
    up_stroke_length_variance /= number_of_pois;
    up_stroke_amplitude_variance /= number_of_pois;

    f->variance_of_peak_rate_in_window = peak_rate_variance;
    f->up_stroke_length_variance = up_stroke_length_variance;
    f->up_stroke_amplitude_variance = up_stroke_amplitude_variance;
    
    // calculate the valley features
    float valley_rate_variance = 0;
    float down_stroke_length_variance = 0;
    float down_stroke_amplitude_variance = 0;
    number_of_pois = 0;
    for(int i=pois->first_element_index; i!=pois->last_element_index; i=get_next_index(i, pois->length)){
        if((check_even_indices && i%2!=0) || (!check_even_indices && i%2==0)){
            number_of_pois++;
            valley_rate_variance += powf(pois->list[i].time_difference_to_previous_poi - f->mean_valley_rate_over_window.current_mean, 2);
            down_stroke_length_variance += powf(pois->list[i].time_difference_to_previous_poi - f->mean_down_stroke_length.current_mean, 2);
            down_stroke_amplitude_variance += powf(pois->list[i].amplitude_difference_to_previous_poi - f->mean_down_stroke_amplitude.current_mean, 2);
        }
    }
    valley_rate_variance /= number_of_pois;
    down_stroke_length_variance /= number_of_pois;
    down_stroke_amplitude_variance /= number_of_pois;

    f->variance_of_valley_rate_in_window = valley_rate_variance;
    f->down_stroke_length_variance = down_stroke_length_variance;
    f->down_stroke_amplitude_variance = down_stroke_amplitude_variance;
    ESP_LOGE("calculate_variance_features", "end");
}
