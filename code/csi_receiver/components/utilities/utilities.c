#include "utilities.h"
#include <stdio.h>
#include "esp_system.h"
#include "esp_log.h"
#include "math.h"
#include "float.h"
#include <string.h>

#define SECONDS_TO_MICROSECONDS 1000000

void *malloc_or_die(size_t size_to_allocate){
    void *p = malloc(size_to_allocate);
    if(p == NULL){
        ESP_LOGE("malloc or die", "failed to allocate memory");
        abort();
    }
    return p;
}

void float_arr_alloc(size_t x, size_t y, float(**aptr)[x][y])
{
  *aptr = malloc_or_die( sizeof(float[x][y]) ); // allocate a true 2D array
}

void int8_arr_alloc(size_t x, size_t y, int8_t(**aptr)[x][y])
{
  *aptr = malloc_or_die( sizeof(int8_t[x][y]) ); // allocate a true 2D array
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

void print_x(float *x, int x_length){
    char *buffer1 = malloc_or_die(8 * 1024* 4);
    size_t len1 = 0;
    len1 += sprintf(buffer1 + len1, "\"[%f", x[0]);
    for (int i = 1; i < x_length; i++) 
    {
        len1 += sprintf(buffer1 + len1, ",%f", x[i]);
    }
    len1 += sprintf(buffer1 + len1, "]\"\n");
    
    printf("%s", buffer1);
    free(buffer1);
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

void create_list_char(ListChar *l, size_t initialSize){
    l->list = malloc_or_die(initialSize*sizeof(char));
    l->length = initialSize;
    l->elements = 0;
}

void append_to_list_char(ListChar *l, char new_element){
    if(l->elements == l->length){
        l->length *= 1.2;
        l->list = realloc(l->list, l->length * sizeof(char));
    }
    l->list[l->elements++] = new_element;
}

void free_list_char(ListChar *l){
    free(l->list);
    l = NULL;
}

void create_list_int(ListInt *l, size_t initialSize){
    l->list = malloc_or_die(initialSize*sizeof(int));
    l->length = initialSize;
    l->elements = 0;
}

void append_to_list_int(ListInt *l, int new_element){
    if(l->elements == l->length){
        l->length *= 1.2;
        l->list = realloc(l->list, l->length * sizeof(int));
    }
    l->list[l->elements++] = new_element;
}

void free_list_int(ListInt *l){
    free(l->list);
    l = NULL;
}

void create_list_float(ListFloat *l, size_t initialSize){
    l->list = malloc_or_die(initialSize*sizeof(float));
    l->length = initialSize;
    l->elements = 0;
}

void append_to_list_float(ListFloat *l, float new_element){
    if(l->elements == l->length){
        l->length *= 1.2;
        l->list = realloc(l->list, l->length * sizeof(float));
    }
    l->list[l->elements++] = new_element;
}

void free_list_float(ListFloat *l){
    free(l->list);
    l = NULL;
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
    r->last_sample_index = -1;
    r->newest_sample_index = -1;
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
    if(r->current_number_of_elements == 0){
        r->last_sample_index = -1;
        r->newest_sample_index = -1;
    }

    // append the newest sample to the array
    int index = get_next_index(r->newest_sample_index, r->array_size);
    if(index == r->last_sample_index){
        r->current_sum -= r->value_array[r->last_sample_index];
        r->current_number_of_elements -= 1;
        r->last_sample_index = get_next_index(r->last_sample_index, r->array_size);
    }
    r->time_array[index] = input_timestamp;
    r->value_array[index] = input;
    r->newest_sample_index = index;

    if(r->last_sample_index==-1){
        r->last_sample_index=0;
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
    ESP_LOGD("calculate_variance_features", "start");
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
            peak_rate_variance += powf((float)(60.0 / ((double)pois->list[i].time_difference_to_previous_poi / (double)SECONDS_TO_MICROSECONDS)) - f->mean_peak_rate_over_window.current_mean, 2);
            up_stroke_length_variance += powf(pois->list[i].time_difference_to_previous_poi - f->mean_up_stroke_length.current_mean, 2);
            up_stroke_amplitude_variance += powf(pois->list[i].amplitude_difference_to_previous_poi - f->mean_up_stroke_amplitude.current_mean, 2);
            ESP_LOGD("calculate_variance_features", "peaks i %d, nr pois %d, rate var %f  mean %f, length var %f  mean %f,  amplitude variance %f  mean %f", i, number_of_pois, peak_rate_variance, f->mean_peak_rate_over_window.current_mean,  up_stroke_length_variance, f->mean_up_stroke_length.current_mean, up_stroke_amplitude_variance, f->mean_up_stroke_amplitude.current_mean);

        }
    }
    // check the last index
    int i = pois->last_element_index;
    if((check_even_indices && i%2==0) || (!check_even_indices && i%2!=0)){
        number_of_pois++;
        float rate = (float)(60.0 / ((double)pois->list[i].time_difference_to_previous_poi / (double)SECONDS_TO_MICROSECONDS));
        ESP_LOGD("r", "elements in list %d, index %d, rate %f, timedif %u", pois->number_of_elements, i, rate, pois->list[i].time_difference_to_previous_poi);
        peak_rate_variance += powf(rate - f->mean_peak_rate_over_window.current_mean, 2);
        up_stroke_length_variance += powf(pois->list[i].time_difference_to_previous_poi - f->mean_up_stroke_length.current_mean, 2);
        up_stroke_amplitude_variance += powf(pois->list[i].amplitude_difference_to_previous_poi - f->mean_up_stroke_amplitude.current_mean, 2);
        ESP_LOGD("calculate_variance_features", "peaks i %d, nr pois %d, rate var %f  mean %f, length var %f  mean %f,  amplitude variance %f  mean %f", i, number_of_pois, peak_rate_variance, f->mean_peak_rate_over_window.current_mean,  up_stroke_length_variance, f->mean_up_stroke_length.current_mean, up_stroke_amplitude_variance, f->mean_up_stroke_amplitude.current_mean);

    }

    peak_rate_variance /= number_of_pois;
    up_stroke_length_variance /= number_of_pois;
    up_stroke_amplitude_variance /= number_of_pois;
    if(number_of_pois==0){
        peak_rate_variance = -1;
        up_stroke_length_variance = -1;
        up_stroke_amplitude_variance = -1;
    }

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
            valley_rate_variance += powf((float) (60.0 / ((double)pois->list[i].time_difference_to_previous_poi / (double)SECONDS_TO_MICROSECONDS)) - f->mean_valley_rate_over_window.current_mean, 2);
            down_stroke_length_variance += powf(pois->list[i].time_difference_to_previous_poi - f->mean_down_stroke_length.current_mean, 2);
            down_stroke_amplitude_variance += powf(pois->list[i].amplitude_difference_to_previous_poi - f->mean_down_stroke_amplitude.current_mean, 2);
            ESP_LOGD("calculate_variance_features", "valleys i %d, nr pois %d, rate var %f  mean %f, length var %f  mean %f,  amplitude variance %f  mean %f", i, number_of_pois, valley_rate_variance, f->mean_valley_rate_over_window.current_mean, down_stroke_length_variance, f->mean_down_stroke_length.current_mean, down_stroke_amplitude_variance, f->mean_down_stroke_amplitude.current_mean);

        }
    }

    // check the last index
    i = pois->last_element_index;
    if((check_even_indices && i%2!=0) || (!check_even_indices && i%2==0)){
        number_of_pois++;
        float rate = (float)(60.0 / ((double)pois->list[i].time_difference_to_previous_poi / (double)SECONDS_TO_MICROSECONDS));
        ESP_LOGD("r", "elements in list %d, index %d, rate %f, timedif %u", pois->number_of_elements, i, rate, pois->list[i].time_difference_to_previous_poi);
        valley_rate_variance += powf(rate - f->mean_valley_rate_over_window.current_mean, 2);
        down_stroke_length_variance += powf(pois->list[i].time_difference_to_previous_poi - f->mean_down_stroke_length.current_mean, 2);
        down_stroke_amplitude_variance += powf(pois->list[i].amplitude_difference_to_previous_poi - f->mean_down_stroke_amplitude.current_mean, 2);
        ESP_LOGD("calculate_variance_features", "valleys i %d, nr pois %d, rate var %f  mean %f, length var %f  mean %f,  amplitude variance %f  mean %f", i, number_of_pois, valley_rate_variance, f->mean_valley_rate_over_window.current_mean, down_stroke_length_variance, f->mean_down_stroke_length.current_mean, down_stroke_amplitude_variance, f->mean_down_stroke_amplitude.current_mean);

    }

    valley_rate_variance /= number_of_pois;
    down_stroke_length_variance /= number_of_pois;
    down_stroke_amplitude_variance /= number_of_pois;

    if(number_of_pois==0){
        valley_rate_variance = -1;
        down_stroke_length_variance = -1;
        down_stroke_amplitude_variance = -1;
    }

    f->variance_of_valley_rate_in_window = valley_rate_variance;
    f->down_stroke_length_variance = down_stroke_length_variance;
    f->down_stroke_amplitude_variance = down_stroke_amplitude_variance;
    ESP_LOGD("calculate_variance_features", "end");
}

void get_best_thresholds(float *stis, char *ids, int length, float *t_presence, float *t_small_movement, float *t_large_movement, float *f_presence, float *f_small_movement, float *f_large_movement){
    // keep track of the best values for each threshold
    float max_f1_presence = -1.0;
    float best_t_presence = -1.0;

    float max_f1_small_movement = -1.0;
    float best_t_small_movement = -1.0;

    float max_f1_large_movement = -1.0;
    float best_t_large_movement = -1.0;

    for(float t=0.0; t<10.0; t+=0.05){
        // keep track of true/false positives and negatives for all thresholds
        int tp_presence = 0;
        int fp_presence = 0;
        int tn_presence = 0;
        int fn_presence = 0;

        int tp_small_movement = 0;
        int fp_small_movement = 0;
        int tn_small_movement = 0;
        int fn_small_movement = 0;

        int tp_large_movement = 0;
        int fp_large_movement = 0;
        int tn_large_movement = 0;
        int fn_large_movement = 0;

        int presence = 2;
        int small_movement = 3;
        int large_movement = 4;


        for(int i=0; i<length; i++){
            if(ids[i]==0){
                continue;
            }

            if(stis[i]>t && ids[i]>=presence){
                tp_presence++;
            }
            if(stis[i]>t && ids[i]<presence){
                fp_presence++;
            }
            if(stis[i]<=t && ids[i]<presence){
                tn_presence++;
            }
            if(stis[i]<=t && ids[i]>=presence){
                fn_presence++;
            }

            if(stis[i]>t && ids[i]>=small_movement){
                tp_small_movement++;
            }
            if(stis[i]>t && ids[i]<small_movement){
                fp_small_movement++;
            }
            if(stis[i]<=t && ids[i]<small_movement){
                tn_small_movement++;
            }
            if(stis[i]<=t && ids[i]>=small_movement){
                fn_small_movement++;
            }

            if(stis[i]>t && ids[i]>=large_movement){
                tp_large_movement++;
            }
            if(stis[i]>t && ids[i]<large_movement){
                fp_large_movement++;
            }
            if(stis[i]<=t && ids[i]<large_movement){
                tn_large_movement++;
            }
            if(stis[i]<=t && ids[i]>=large_movement){
                fn_large_movement++;
            }
        }

        // calculate f scores
        float f_presence = (float)(2*tp_presence) / (float)(2*tp_presence + fp_presence + fn_presence);
        float f_small_movement = (float)(2*tp_small_movement) / (float)(2*tp_small_movement + fp_small_movement + fn_small_movement);
        float f_large_movement = (float)(2*tp_large_movement) / (float)(2*tp_large_movement + fp_large_movement + fn_large_movement);


        // prevent nans -> 0/0
        // check if there the activity even exists
        if(tp_presence + fn_presence == 0){
            f_presence = -2;
        }
        if(tp_small_movement + fn_small_movement == 0){
            f_small_movement = -2;
        }
        if(tp_large_movement + fn_large_movement == 0){
            f_large_movement = -2;
        }

        //ESP_LOGI("values", "tp %d, fp %d, tn %d, fn %d, f %f, current max %f, current best t %f, current t %f", tp_presence, fp_presence, tn_presence, fn_presence, f_presence, max_f1_presence, best_t_presence, t);

        // check whether this is the new best score and t
        if(f_presence >= max_f1_presence){
            max_f1_presence = f_presence;
            best_t_presence = t;
        }

        if(f_small_movement >= max_f1_small_movement){
            max_f1_small_movement = f_small_movement;
            best_t_small_movement = t;
        }

        if(f_large_movement >= max_f1_large_movement){
            max_f1_large_movement = f_large_movement;
            best_t_large_movement = t;
        }
    }
    ESP_LOGI("get_best_thresholds", "presence t %f, f score %f", best_t_presence, max_f1_presence);
    ESP_LOGI("get_best_thresholds", "small_movement t %f, f score %f", best_t_small_movement, max_f1_small_movement);
    ESP_LOGI("get_best_thresholds", "large_movement t %f, f score %f", best_t_large_movement, max_f1_large_movement);

    *t_presence = best_t_presence;
    *t_small_movement = best_t_small_movement;
    *t_large_movement = best_t_large_movement;

    *f_presence = max_f1_presence;
    *f_small_movement = max_f1_small_movement;
    *f_large_movement = max_f1_large_movement;
}