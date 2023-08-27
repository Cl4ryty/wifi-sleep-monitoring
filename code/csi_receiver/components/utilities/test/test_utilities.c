#include "unity.h"
#include "utilities.h"

#define SECONDS_TO_MICROSECONDS 1000000

TEST_CASE("test dumb running mean simple functionality", "[mean]")
{
        DumbRunningMean mean;
        int array_size = 5;
        dumb_running_mean_initialize(&mean, array_size);

        float input = 5.0;
        unsigned timestamp = 0;
        unsigned window_size = 10;
        
        dumb_running_mean_append(&mean, input, timestamp, window_size);
        TEST_ASSERT_EQUAL_FLOAT(5.0, mean.current_mean);

        input = 5.0;
        timestamp++;
        dumb_running_mean_append(&mean, input, timestamp, window_size);
        TEST_ASSERT_EQUAL_FLOAT(5.0, mean.current_mean);

        input = 20.0;
        timestamp++;
        dumb_running_mean_append(&mean, input, timestamp, window_size);
        TEST_ASSERT_EQUAL_FLOAT(10.0, mean.current_mean);

        input = 10.0;
        timestamp++;
        dumb_running_mean_append(&mean, input, timestamp, window_size);
        TEST_ASSERT_EQUAL_FLOAT(10.0, mean.current_mean);

        input = 5.0;
        timestamp++;
        dumb_running_mean_append(&mean, input, timestamp, window_size);
        TEST_ASSERT_EQUAL_FLOAT(9.0, mean.current_mean);

        input = 10.0;
        timestamp++;
        dumb_running_mean_append(&mean, input, timestamp, window_size);
        TEST_ASSERT_EQUAL_FLOAT(10.0, mean.current_mean);

        input = 10.0;
        timestamp++;
        dumb_running_mean_append(&mean, input, timestamp, window_size);
        TEST_ASSERT_EQUAL_FLOAT(11.0, mean.current_mean);

        // testing samples falling out of window size

        dumb_running_mean_initialize(&mean, array_size);

        input = 5.0;
        timestamp = 0;
        dumb_running_mean_append(&mean, input, timestamp, window_size);
        TEST_ASSERT_EQUAL_FLOAT(5.0, mean.current_mean);

        input = 5.0;
        timestamp = 5;
        dumb_running_mean_append(&mean, input, timestamp, window_size);
        TEST_ASSERT_EQUAL_FLOAT(5.0, mean.current_mean);

        input = 20.0;
        timestamp = 6;
        dumb_running_mean_append(&mean, input, timestamp, window_size);
        TEST_ASSERT_EQUAL_FLOAT(10.0, mean.current_mean);

        input = 10.0;
        timestamp = 10;
        dumb_running_mean_append(&mean, input, timestamp, window_size);
        TEST_ASSERT_EQUAL_FLOAT(10.0, mean.current_mean);

        input = 5.0;
        timestamp = 11;
        dumb_running_mean_append(&mean, input, timestamp, window_size);
        TEST_ASSERT_EQUAL_FLOAT(10.0, mean.current_mean);

        input = 20.0;
        timestamp = 15;
        dumb_running_mean_append(&mean, input, timestamp, window_size);
        TEST_ASSERT_EQUAL_FLOAT(12.0, mean.current_mean);

        input = 10.0;
        timestamp++;
        dumb_running_mean_append(&mean, input, timestamp, window_size);
        TEST_ASSERT_EQUAL_FLOAT(13.0, mean.current_mean);


}

TEST_CASE("test dumb running mean negative numbers", "[mean]")
{
        DumbRunningMean mean;
        int array_size = 5;
        dumb_running_mean_initialize(&mean, array_size);

        float input = -5.0;
        unsigned timestamp = 0;
        unsigned window_size = 10;
        
        dumb_running_mean_append(&mean, input, timestamp, window_size);
        TEST_ASSERT_EQUAL_FLOAT(-5.0, mean.current_mean);

        input = -5.0;
        timestamp++;
        dumb_running_mean_append(&mean, input, timestamp, window_size);
        TEST_ASSERT_EQUAL_FLOAT(-5.0, mean.current_mean);

        input = -20.0;
        timestamp++;
        dumb_running_mean_append(&mean, input, timestamp, window_size);
        TEST_ASSERT_EQUAL_FLOAT(-10.0, mean.current_mean);

        input = -10.0;
        timestamp++;
        dumb_running_mean_append(&mean, input, timestamp, window_size);
        TEST_ASSERT_EQUAL_FLOAT(-10.0, mean.current_mean);

        input = -5.0;
        timestamp++;
        dumb_running_mean_append(&mean, input, timestamp, window_size);
        TEST_ASSERT_EQUAL_FLOAT(-9.0, mean.current_mean);

        input = -10.0;
        timestamp++;
        dumb_running_mean_append(&mean, input, timestamp, window_size);
        TEST_ASSERT_EQUAL_FLOAT(-10.0, mean.current_mean);

        input = -10.0;
        timestamp++;
        dumb_running_mean_append(&mean, input, timestamp, window_size);
        TEST_ASSERT_EQUAL_FLOAT(-11.0, mean.current_mean);

        // testing samples falling out of window size

        dumb_running_mean_initialize(&mean, array_size);

        input = -5.0;
        timestamp = 0;
        dumb_running_mean_append(&mean, input, timestamp, window_size);
        TEST_ASSERT_EQUAL_FLOAT(-5.0, mean.current_mean);

        input = -5.0;
        timestamp = 5;
        dumb_running_mean_append(&mean, input, timestamp, window_size);
        TEST_ASSERT_EQUAL_FLOAT(-5.0, mean.current_mean);

        input = -20.0;
        timestamp = 6;
        dumb_running_mean_append(&mean, input, timestamp, window_size);
        TEST_ASSERT_EQUAL_FLOAT(-10.0, mean.current_mean);

        input = -10.0;
        timestamp = 10;
        dumb_running_mean_append(&mean, input, timestamp, window_size);
        TEST_ASSERT_EQUAL_FLOAT(-10.0, mean.current_mean);

        input = -5.0;
        timestamp = 11;
        dumb_running_mean_append(&mean, input, timestamp, window_size);
        TEST_ASSERT_EQUAL_FLOAT(-10.0, mean.current_mean);

        input = -20.0;
        timestamp = 15;
        dumb_running_mean_append(&mean, input, timestamp, window_size);
        TEST_ASSERT_EQUAL_FLOAT(-12.0, mean.current_mean);

        input = -10.0;
        timestamp++;
        dumb_running_mean_append(&mean, input, timestamp, window_size);
        TEST_ASSERT_EQUAL_FLOAT(-13.0, mean.current_mean);
}

TEST_CASE("test dumb running mean input timestamp larget than window", "[mean]")
{
        DumbRunningMean mean;
        int array_size = 5;
        dumb_running_mean_initialize(&mean, array_size);

        float input = 5.0;
        unsigned timestamp = 0;
        unsigned window_size = 10;
        
        dumb_running_mean_append(&mean, input, timestamp, window_size);
        TEST_ASSERT_EQUAL_FLOAT(5.0, mean.current_mean);

        input = 5.0;
        timestamp++;
        dumb_running_mean_append(&mean, input, timestamp, window_size);
        TEST_ASSERT_EQUAL_FLOAT(5.0, mean.current_mean);

        input = 20.0;
        timestamp++;
        dumb_running_mean_append(&mean, input, timestamp, window_size);
        TEST_ASSERT_EQUAL_FLOAT(10.0, mean.current_mean);

        input = 15.0;
        dumb_running_mean_append(&mean, input, 12, window_size);
        TEST_ASSERT_EQUAL_FLOAT(17.5, mean.current_mean);


        dumb_running_mean_initialize(&mean, array_size);

        input = 5.0;
        timestamp = 0;
        dumb_running_mean_append(&mean, input, timestamp, window_size);
        TEST_ASSERT_EQUAL_FLOAT(5.0, mean.current_mean);

        input = 5.0;
        timestamp = 5;
        dumb_running_mean_append(&mean, input, timestamp, window_size);
        TEST_ASSERT_EQUAL_FLOAT(5.0, mean.current_mean);

        input = 20.0;
        timestamp = 6;
        dumb_running_mean_append(&mean, input, timestamp, window_size);
        TEST_ASSERT_EQUAL_FLOAT(10.0, mean.current_mean);

        input = 15.0;
        timestamp = 10;
        dumb_running_mean_append(&mean, input, 20, window_size);
        TEST_ASSERT_EQUAL_FLOAT(15.0, mean.current_mean);

        // input = 5.0;
        // timestamp = 11;
        // dumb_running_mean_append(&mean, input, timestamp, window_size);
        // TEST_ASSERT_EQUAL(10.0, mean.current_mean);

        // input = 20.0;
        // timestamp = 15;
        // dumb_running_mean_append(&mean, input, timestamp, window_size);
        // TEST_ASSERT_EQUAL(12.0, mean.current_mean);

        // input = 10.0;
        // timestamp++;
        // dumb_running_mean_append(&mean, input, timestamp, window_size);
        // TEST_ASSERT_EQUAL(13.0, mean.current_mean);


}

TEST_CASE("test calculate_variance_features zero var", "[variance]")
{
        Features features;
        features_initialize(&features);

        POI_List pois;
        poi_list_initialize(&pois, 20);

        POI poi;
        poi_initialize(&poi, true, 1000000, 1, 0, 1);
        poi_list_append(&pois, &poi);
        for(int i=1; i<9; i++){
                POI poi;
                if(i%2==0){
                        poi_initialize(&poi, true, 1000000, 1, i, 1);
                }else{
                        poi_initialize(&poi, false, 1000000, -1, i, 1);
                }
                poi_list_append(&pois, &poi);
        }

        features.mean_peak_rate_over_window.current_mean = 60.0;
        features.mean_valley_rate_over_window.current_mean = 60.0;
        features.mean_up_stroke_length.current_mean = 1000000.0;
        features.mean_down_stroke_length.current_mean = 1000000.0;
        features.mean_up_stroke_amplitude.current_mean = 1.0;
        features.mean_down_stroke_amplitude.current_mean = -1.0;

        calculate_variance_features(&features, &pois);
        TEST_ASSERT_EQUAL(0.0, features.variance_of_peak_rate_in_window);
        TEST_ASSERT_EQUAL(0.0, features.variance_of_valley_rate_in_window);
        TEST_ASSERT_EQUAL(0.0, features.up_stroke_length_variance);
        TEST_ASSERT_EQUAL(0.0, features.down_stroke_length_variance);
        TEST_ASSERT_EQUAL(0.0, features.up_stroke_amplitude_variance);
        TEST_ASSERT_EQUAL(0.0, features.down_stroke_amplitude_variance);

        features_initialize(&features);

        poi_list_initialize(&pois, 20);

        poi_initialize(&poi, false, 1000000, -1, 0, 1);
        poi_list_append(&pois, &poi);
        for(int i=1; i<9; i++){
                POI poi;
                if(i%2!=0){
                        poi_initialize(&poi, false, 1000000, 1, i, 1);
                }else{
                        poi_initialize(&poi, true, 1000000, -1, i, 1);
                }
                poi_list_append(&pois, &poi);
        }

        features.mean_peak_rate_over_window.current_mean = 60.0;
        features.mean_valley_rate_over_window.current_mean = 60.0;
        features.mean_up_stroke_length.current_mean = 1000000.0;
        features.mean_down_stroke_length.current_mean = 1000000.0;
        features.mean_up_stroke_amplitude.current_mean = 1.0;
        features.mean_down_stroke_amplitude.current_mean = -1.0;

        calculate_variance_features(&features, &pois);
        TEST_ASSERT_EQUAL(0.0, features.variance_of_peak_rate_in_window);
        TEST_ASSERT_EQUAL(0.0, features.variance_of_valley_rate_in_window);
        TEST_ASSERT_EQUAL(0.0, features.up_stroke_length_variance);
        TEST_ASSERT_EQUAL(0.0, features.down_stroke_length_variance);
        TEST_ASSERT_EQUAL(0.0, features.up_stroke_amplitude_variance);
        TEST_ASSERT_EQUAL(0.0, features.down_stroke_amplitude_variance);

}

TEST_CASE("test calculate_variance_features amplitude var starting with peak", "[variance]")
{
        Features features;
        features_initialize(&features);

        POI_List pois;
        poi_list_initialize(&pois, 20);

        POI poi;
        poi_initialize(&poi, true, 1000000, 1, 0, 1);
        poi_list_append(&pois, &poi);
        for(int i=1; i<7; i++){
                POI poi;
                if(i%2==0){
                        poi_initialize(&poi, true, 1000000, 1, i, 1);
                }else{
                        poi_initialize(&poi, false, 1000000, -1, i, 1);
                }
                poi_list_append(&pois, &poi);
        }
        poi_initialize(&poi, false, 1000000, -11, 7, 1);
        poi_list_append(&pois, &poi);
        poi_initialize(&poi, true, 1000000, 11, 8, 1);
        poi_list_append(&pois, &poi);
        poi_initialize(&poi, false, 1000000, -1, 9, 1);
        poi_list_append(&pois, &poi);

        features.mean_peak_rate_over_window.current_mean = 60.0;
        features.mean_valley_rate_over_window.current_mean = 60.0;
        features.mean_up_stroke_length.current_mean = 1000000.0;
        features.mean_down_stroke_length.current_mean = 1000000.0;
        features.mean_up_stroke_amplitude.current_mean = 1.0;
        features.mean_down_stroke_amplitude.current_mean = -1.0;

        calculate_variance_features(&features, &pois);
        TEST_ASSERT_EQUAL_FLOAT(0.0, features.variance_of_peak_rate_in_window);
        TEST_ASSERT_EQUAL_FLOAT(0.0, features.variance_of_valley_rate_in_window);
        TEST_ASSERT_EQUAL_FLOAT(0.0, features.up_stroke_length_variance);
        TEST_ASSERT_EQUAL_FLOAT(0.0, features.down_stroke_length_variance);
        TEST_ASSERT_EQUAL_FLOAT(20.0, features.up_stroke_amplitude_variance);
        TEST_ASSERT_EQUAL_FLOAT(20.0, features.down_stroke_amplitude_variance);

        

}

TEST_CASE("test calculate_variance_features amplitude var starting with valley", "[variance]")
{
        Features features;
        features_initialize(&features);

        POI_List pois;
        poi_list_initialize(&pois, 20);

        POI poi;
        poi_initialize(&poi, false, 1000000, 1, 0, 1);
        poi_list_append(&pois, &poi);
        for(int i=1; i<7; i++){
                POI poi;
                if(i%2==0){
                        poi_initialize(&poi, false, 1000000, 1, i, 1);
                }else{
                        poi_initialize(&poi, true, 1000000, -1, i, 1);
                }
                poi_list_append(&pois, &poi);
        }
        poi_initialize(&poi, true, 1000000, -11, 7, 1);
        poi_list_append(&pois, &poi);
        poi_initialize(&poi, false, 1000000, 11, 8, 1);
        poi_list_append(&pois, &poi);
        poi_initialize(&poi, true, 1000000, -1, 9, 1);
        poi_list_append(&pois, &poi);

        features.mean_peak_rate_over_window.current_mean = 60.0;
        features.mean_valley_rate_over_window.current_mean = 60.0;
        features.mean_up_stroke_length.current_mean = 1000000.0;
        features.mean_down_stroke_length.current_mean = 1000000.0;
        features.mean_up_stroke_amplitude.current_mean = -1.0;
        features.mean_down_stroke_amplitude.current_mean = 1.0;

        calculate_variance_features(&features, &pois);
        TEST_ASSERT_EQUAL_FLOAT(0.0, features.variance_of_peak_rate_in_window);
        TEST_ASSERT_EQUAL_FLOAT(0.0, features.variance_of_valley_rate_in_window);
        TEST_ASSERT_EQUAL_FLOAT(0.0, features.up_stroke_length_variance);
        TEST_ASSERT_EQUAL_FLOAT(0.0, features.down_stroke_length_variance);
        TEST_ASSERT_EQUAL_FLOAT(20.0, features.up_stroke_amplitude_variance);
        TEST_ASSERT_EQUAL_FLOAT(20.0, features.down_stroke_amplitude_variance);
}

TEST_CASE("test features", "[features]")
{

        unsigned window_size = 30 * SECONDS_TO_MICROSECONDS;
        Features features;
        features_initialize(&features);

        POI_List pois;
        POI new_poi;
        poi_list_initialize(&pois, 20);
        dumb_running_mean_initialize(&features.mean_peak_rate_over_window, 30);
        dumb_running_mean_initialize(&features.mean_up_stroke_length, 30);
        dumb_running_mean_initialize(&features.mean_up_stroke_amplitude, 30);
        dumb_running_mean_initialize(&features.mean_valley_rate_over_window, 30);
        dumb_running_mean_initialize(&features.mean_down_stroke_length, 30);
        dumb_running_mean_initialize(&features.mean_down_stroke_amplitude, 30);

        POI poi;
        poi_initialize(&poi, 1, 31080133, 9.693646, 1125, 1);
        poi_list_append(&pois, &poi);
        ESP_LOGI("added", "poi");

        unsigned timestamp = 31080133;
        features.instantaneous_peak_rate = 60.0 / (float)(poi.time_difference_to_previous_poi / SECONDS_TO_MICROSECONDS);
        ESP_LOGI("p rate", "poi  %f", features.instantaneous_peak_rate);
        dumb_running_mean_append(&features.mean_peak_rate_over_window, features.instantaneous_peak_rate, timestamp, window_size);
        dumb_running_mean_append(&features.mean_up_stroke_length, poi.time_difference_to_previous_poi, timestamp, window_size);
        dumb_running_mean_append(&features.mean_up_stroke_amplitude, poi.amplitude_difference_to_previous_poi, timestamp, window_size);
        if(pois.number_of_elements > 1){
                calculate_variance_features(&features, &pois);
        } 
                                


        POI poi1;
        poi_initialize(&poi1, 0, 10744651, 10.558222, 36, 1);
        poi_list_append(&pois, &poi1);
        ESP_LOGI("added", "poi1");
        timestamp += 10744651;
        new_poi = poi1;
        features.instantaneous_valley_rate = 60.0 / (float)(poi1.time_difference_to_previous_poi / SECONDS_TO_MICROSECONDS);
        ESP_LOGI("v rate", "poi1  %f", features.instantaneous_valley_rate);
        dumb_running_mean_append(&features.mean_valley_rate_over_window, features.instantaneous_valley_rate, timestamp, window_size);
        dumb_running_mean_append(&features.mean_down_stroke_length, new_poi.time_difference_to_previous_poi, timestamp, window_size);
        dumb_running_mean_append(&features.mean_down_stroke_amplitude, new_poi.amplitude_difference_to_previous_poi, timestamp, window_size);
        if(pois.number_of_elements > 0){
                POI peak = pois.list[pois.last_element_index];
                features.fractional_up_stroke_amplitude = peak.amplitude_difference_to_previous_poi / (peak.amplitude_difference_to_previous_poi + new_poi.amplitude_difference_to_previous_poi);
                features.fractional_up_stroke_time = (float)peak.time_difference_to_previous_poi / (float)(peak.time_difference_to_previous_poi + new_poi.time_difference_to_previous_poi);
                features.up_to_down_amplitude_ratio = peak.amplitude_difference_to_previous_poi / new_poi.amplitude_difference_to_previous_poi;
                features.up_to_down_length_ratio = (float)peak.time_difference_to_previous_poi / (float)new_poi.time_difference_to_previous_poi;
        }
        if(pois.number_of_elements > 1){
                calculate_variance_features(&features, &pois);
        } 


        POI poi2;
        poi_initialize(&poi2, 1, 5228664, 1.002261, 236, 1);
        poi_list_append(&pois, &poi2);
        ESP_LOGI("added", "poi2");
        timestamp += 5228664;
        features.instantaneous_peak_rate = 60.0 / ((float)poi2.time_difference_to_previous_poi / (float)SECONDS_TO_MICROSECONDS);
        ESP_LOGI("numbers", "%u", poi2.time_difference_to_previous_poi / SECONDS_TO_MICROSECONDS);
        ESP_LOGI("p rate", "poi2  %f", features.instantaneous_peak_rate);
        dumb_running_mean_append(&features.mean_peak_rate_over_window, features.instantaneous_peak_rate, timestamp, window_size);
        dumb_running_mean_append(&features.mean_up_stroke_length, poi2.time_difference_to_previous_poi, timestamp, window_size);
        dumb_running_mean_append(&features.mean_up_stroke_amplitude, poi2.amplitude_difference_to_previous_poi, timestamp, window_size);
        if(pois.number_of_elements > 1){
                calculate_variance_features(&features, &pois);
        } 


        POI poi3;
        poi_initialize(&poi3, 0, 1766116, 0.261949, 304, 1);
        poi_list_append(&pois, &poi3);
        ESP_LOGI("added", "poi3");
        timestamp += 1766116;
        new_poi = poi3;
        features.instantaneous_valley_rate = 60.0 / ((float)new_poi.time_difference_to_previous_poi / (float)SECONDS_TO_MICROSECONDS);
        ESP_LOGI("v rate", "poi3  %f", features.instantaneous_valley_rate);
        dumb_running_mean_append(&features.mean_valley_rate_over_window, features.instantaneous_valley_rate, timestamp, window_size);
        dumb_running_mean_append(&features.mean_down_stroke_length, new_poi.time_difference_to_previous_poi, timestamp, window_size);
        dumb_running_mean_append(&features.mean_down_stroke_amplitude, new_poi.amplitude_difference_to_previous_poi, timestamp, window_size);
        if(pois.number_of_elements > 0){
                POI peak = pois.list[pois.last_element_index];
                features.fractional_up_stroke_amplitude = peak.amplitude_difference_to_previous_poi / (peak.amplitude_difference_to_previous_poi + new_poi.amplitude_difference_to_previous_poi);
                features.fractional_up_stroke_time = (float)peak.time_difference_to_previous_poi / (float)(peak.time_difference_to_previous_poi + new_poi.time_difference_to_previous_poi);
                features.up_to_down_amplitude_ratio = peak.amplitude_difference_to_previous_poi / new_poi.amplitude_difference_to_previous_poi;
                features.up_to_down_length_ratio = (float)peak.time_difference_to_previous_poi / (float)new_poi.time_difference_to_previous_poi;
        }
        if(pois.number_of_elements > 1){
                calculate_variance_features(&features, &pois);
        } 


        POI poi4;
        poi_initialize(&poi4, 1, 120532267, 2.935997, 368, 1);
        poi_list_append(&pois, &poi4);
        ESP_LOGI("added", "poi4");
        timestamp += 120532267;
        features.instantaneous_peak_rate = (float) (60.0 / ((double)poi4.time_difference_to_previous_poi / (double)SECONDS_TO_MICROSECONDS));
        ESP_LOGI("rate", "poi4  %f, mean rate %f, mean l %f, mean amp %f", features.instantaneous_peak_rate, features.mean_peak_rate_over_window.current_mean, features.mean_up_stroke_length.current_mean, features.mean_up_stroke_amplitude.current_mean);
        dumb_running_mean_append(&features.mean_peak_rate_over_window, features.instantaneous_peak_rate, timestamp, window_size);
        dumb_running_mean_append(&features.mean_up_stroke_length, poi4.time_difference_to_previous_poi, timestamp, window_size);
        dumb_running_mean_append(&features.mean_up_stroke_amplitude, poi4.amplitude_difference_to_previous_poi, timestamp, window_size);
        ESP_LOGI("rate", "poi4  %f, mean rate %f, mean l %f, mean amp %f", features.instantaneous_peak_rate, features.mean_peak_rate_over_window.current_mean, features.mean_up_stroke_length.current_mean, features.mean_up_stroke_amplitude.current_mean);
        if(pois.number_of_elements > 1){
                calculate_variance_features(&features, &pois);
        } 

        TEST_ASSERT_EQUAL(FP_NORMAL, fpclassify(features.variance_of_peak_rate_in_window));
        TEST_ASSERT_EQUAL(FP_NORMAL, fpclassify(features.variance_of_valley_rate_in_window));
        TEST_ASSERT_EQUAL(FP_NORMAL, fpclassify(features.up_stroke_length_variance));
        TEST_ASSERT_EQUAL(FP_NORMAL, fpclassify(features.down_stroke_length_variance));
        TEST_ASSERT_EQUAL(FP_NORMAL, fpclassify(features.up_stroke_amplitude_variance));
        TEST_ASSERT_EQUAL(FP_NORMAL, fpclassify(features.down_stroke_amplitude_variance));
}

TEST_CASE("print type sizes", "[types]")
{
        ESP_LOGI("type size", "char %d", sizeof(char));
        ESP_LOGI("type size", "int %d", sizeof(int));
        ESP_LOGI("type size", "float %d", sizeof(float));
        ESP_LOGI("type size", "double %d", sizeof(double));
        ESP_LOGI("type size", "bool %d", sizeof(bool));
        ESP_LOGI("type size", "unsigned %d", sizeof(unsigned));

}

TEST_CASE("ts", "[ts]")
{
        float stis[] = {0.806263, 0.451672, 0.599478, 0.543439, 0.531586, 0.653764, 0.62558, 0.564304, 0.873193, 0.747747, 0.583633, 0.810991, 0.611437, 0.878018, 0.669445, 0.508852, 9.65649, 0.656971, 0.592151, 4.992373, 0.624266, 0.658438, 0.71594, 0.652882, 0.84899, 0.513698, 0.68102, 0.557011, 5.694009, 0.80317, 0.625136, 0.593034, 0.659109, 0.651721, 4.754633, 0.638352, 0.551015, 0.548937, 0.869014, 0.831583, 0.745463, 0.640867, 0.806627, 0.858757, 0.688504, 6.679452, 4.0612, 0.775959, 0.748288, 0.704434, 0.660318, 0.815525, 0.588584, 0.564577, 0.775792, 0.859632, 0.522204, 0.64915, 0.75289, 0.63391, 0.632962, 0.583405, 0.566383, 0.641823, 0.677834, 0.63962, 0.661781, 0.750593, 0.658507, 0.803529, 0.630324, 0.612147, 0.619074, 0.626662, 0.565045, 0.639149, 0.629093, 0.681557, 0.685013, 0.731293, 6.99397, 0.707826, 0.601193, 0.929582, 6.612237, 0.638049, 0.724414, 0.61136, 0.554514, 0.725634, 0.643579, 0.649607, 0.565453, 0.663003, 0.679372, 0.657154, 0.772162, 0.785795, 0.618947, 0.561943, 0.716296, 0.831177, 0.648827, 0.799259, 0.660132, 6.17398, 0.666492, 0.591752, 0.609101, 2.782297, 0.615222, 0.730171, 0.701503, 0.818153, 0.629785, 0.526181, 1.252057, 0.789855, 0.611379, 0.564227, 0.627276, 0.539335, 0.591603, 0.681254, 0.668772, 0.710175, 0.699837, 0.722933, 0.759823, 0.692981, 0.628645, 0.678058, 0.404172, 0.872422, 0.645065, 0.680261, 0.577161, 0.609764, 0.482844, 0.855163, 0.762743, 0.876933, 0.587415, 0.720483, 0.675049, 2.858884, 0.527082, 0.562973, 0.607226, 0.664066, 0.971826, 0.612054, 0.624428, 0.633025, 0.6353, 0.743549, 0.787556, 0.657714, 0.628014, 0.608486, 0.564241, 0.698145, 0.644879, 0.710467, 0.592768, 0.618602, 0.786188, 0.562599, 0.783823, 0.683212, 0.635215, 0.811212, 4.839536, 0.768146, 0.642589, 0.750413, 0.818598, 0.683424, 0.681374, 0.563227, 0.642589, 0.689624, 0.600551, 0.745317, 0.616156, 0.857683, 0.655815, 1.632837, 0.681719, 0.756679, 0.628329, 0.910008, 0.644488, 0.678743, 0.767872, 0.675236, 0.61505, 0.725634, 0.577174, 0.541105, 0.646521, 0.65317, 0.596082, 0.665038, 0.590683, 0.78743, 1.035196, 0.578135, 1.217234, 0.806086, 4.247816, 0.774118, 0.642816, 0.53911, 0.69381, 0.669588, 0.601265, 0.728504, 0.568956, 0.617593, 0.565547, 0.896476, 0.65317, 0.698593, 0.838612, 0.63177, 0.859261, 0.755451, 0.627333, 0.749651, 4.010998, 0.608761, 0.931998, 0.654907, 0.605134, 0.717983, 0.656669, 0.750325, 0.679749, 0.60455, 0.722977, 4.04814, 0.685478, 0.845823, 0.552139, 0.66677, 0.501105, 0.654191, 0.873877, 0.684313, 0.566608, 0.786305, 0.624338, 0.931661, 0.62102, 0.686029, 0.788445, 0.677763, 0.806498, 0.614018, 0.742664, 0.826527, 0.59792, 0.861049, 0.573707, 0.682136, 0.703768, 0.750734, 0.550464, 0.590136, 0.677635, 0.667669, 0.533863, 0.587706, 0.633565, 0.722054, 0.5806, 0.783007, 0.70343, 0.787701, 0.639314, 0.638447, 9.53062, 0.544333, 0.805443, 0.665777, 0.625724, 0.696199, 1.064179, 0.775921, 0.808618, 0.661184, 0.77872, 0.630616, 1.131087, 0.623743, 0.669334, 0.533168, 0.789191, 0.728005, 0.747563, 0.691873, 0.820199, 0.539629, 0.619381, 0.592948, 0.67695, 0.528597, 0.656592, 0.672269, 0.601617, 0.621762, 0.715417, 3.314567, 0.643732, 0.844404, 0.699531, 0.75922, 0.623464, 0.574001, 0.925027, 0.638496, 0.693126, 5.991248, 0.720963, 0.654741, 0.47045, 0.595696, 0.65768, 0.622395, 0.62155, 0.659168, 0.781017, 0.573446, 0.763463, 0.48518, 0.539476, 0.545633, 0.694865, 0.617179, 0.57444, 0.558761, 0.701207, 0.747747, 0.593874, 0.723211, 0.632952, 0.638484, 0.707371, 0.83924, 0.88379, 0.731951, 0.70375, 0.696199, 0.589523, 0.97031, 0.542664, 0.722495, 0.616352, 0.595036, 0.508946, 0.56982, 0.925824, 0.657083, 0.732082, 0.741819, 0.785206, 0.640906, 0.678548, 0.623735, 0.681896, 0.666905, 0.592579, 0.671174, 4.164705, 0.762773, 0.693088, 0.680534, 0.793742, 0.784837, 0.717709, 0.668528, 0.632506, 0.790855, 0.776881, 0.721867, 0.551795, 0.753345, 0.67388, 0.690004, 0.544653, 0.606425, 0.660836, 0.598562, 0.695144, 0.711284, 0.631951, 0.420214, 0.623928, 0.664089, 0.741868, 0.656809, 0.652178, 0.67961, 0.65435, 0.567778, 0.645796, 0.544537, 0.598033, 0.668244, 0.846788, 0.600926, 1.393749, 0.573056, 0.618189, 0.729393, 0.792577, 0.569601, 0.785902, 0.714239, 0.619701, 0.536428, 0.712308, 0.610198, 0.829722, 0.588302, 0.653403, 0.650513, 0.584315, 0.488591, 0.7996, 0.677261, 0.607696, 0.624905, 0.733643, 0.895331, 5.845437, 0.760539, 0.672247, 0.650378, 0.757502, 0.772264, 0.613323, 0.551417, 0.616558, 0.625456, 0.655896, 0.886604, 0.960634, 0.565237, 0.665631, 0.580124, 0.596495, 0.683094, 0.610085, 0.773763, 0.656826, 0.610892, 0.60217, 0.68381, 0.764121, 0.70343, 0.618062, 0.99715, 0.571527, 0.764966, 0.612482, 0.591469, 0.60702, 0.539629, 3.989819, 0.65135, 0.593363, 0.647909, 0.59317, 0.698483, 0.762942, 4.330443, 0.669392, 0.785623, 0.656734, 0.769165, 0.61738, 0.562973, 0.612985, 0.654223, 0.648212, 3.136612, 0.6873, 11.275995, 0.669603, 0.683609, 0.735821, 11.442247, 0.673649, 0.583957, 0.681755, 0.797504, 1.886223, 0.577426, 0.715447, 0.640084, 0.699527, 0.849222, 0.565371, 0.590849, 0.587237, 0.495109, 0.592662, 0.619777, 0.689177, 0.805646, 0.528017, 0.954805, 6.018273, 0.646355, 0.586205, 0.611448, 0.711594, 0.559461, 0.700297, 0.667168, 0.763106, 0.598763, 0.671017, 0.670485, 0.628757, 0.748164, 0.564076, 0.638582, 0.577119, 0.701736, 0.558182, 0.820492, 0.604471, 0.548448, 0.522583, 4.756423, 0.637144, 0.92817, 0.513582, 0.653767, 0.564991, 0.700108, 0.888177, 0.819399, 0.561858, 0.685741, 0.610367, 0.563275, 0.596495, 0.542482, 0.829174, 0.537982, 0.743245, 0.585477, 0.666868, 0.640671, 0.686885, 0.640875, 0.647833, 0.605764, 0.847251, 0.579704, 0.664132, 0.698271, 0.605085, 9.424663, 0.604613, 0.659994, 0.748414, 0.712004, 0.859604, 0.672624, 0.674721, 0.714349, 0.964328, 0.531093, 0.520186, 0.766784, 0.780536, 0.673654, 0.668576, 0.603726, 0.604821, 5.524925, 0.633687, 0.87316, 0.610041, 0.648022, 0.628219, 0.637582, 0.514443, 0.680186, 0.625138, 4.084213, 0.584104, 0.824655, 0.747407, 0.737242, 0.672922, 0.543036, 0.825012, 0.649645, 0.661143, 0.623763, 0.559029, 0.966274, 0.814922, 0.643501, 0.599892, 0.744306, 0.809767, 0.601348, 0.772178, 0.556951, 0.845472, 0.812915, 0.569614, 0.552951, 0.797724, 0.600003, 0.676755, 0.820575, 0.732699, 0.565845, 0.698666, 0.607492, 0.65194, 0.709422, 0.52757, 0.670013, 0.608692, 0.667296, 0.717969, 6.874868, 0.887988, 0.783823, 0.823089, 0.536368, 0.592641, 0.854042, 0.617715, 0.608454, 0.664679, 0.761731, 0.532248, 0.671174, 0.750596, 0.599671, 0.703084, 3.279786, 0.704256, 0.615732, 0.596455, 0.529358, 0.698599, 0.518724, 0.744224, 6.784873, 0.713074, 0.702509, 0.736631, 0.660881, 0.552455, 0.610539, 3.055401, 0.634048, 0.823402, 0.591443, 0.667219, 0.605871, 0.679749, 0.566131, 0.620664, 0.948406, 0.640675, 0.717029, 0.853854, 0.644814, 0.477024, 0.584359, 0.772234, 0.63225, 4.203259, 1.433381, 0.647551, 0.587365, 0.586742, 0.615852, 0.621737, 0.533653, 0.502109, 0.564227, 0.808115, 0.60664, 0.622179, 0.606747, 0.89458, 0.822437, 6.896597, 0.875244, 0.69871, 0.629093, 0.590924, 0.599982, 0.618797, 0.683541, 0.604804, 0.713503, 0.585672, 0.642485, 0.624142, 0.590185, 0.605758, 4.061141, 0.759893, 0.633331, 0.675285, 0.944714, 0.659096, 1.133576, 0.745317, 0.690987, 0.574717, 0.714379, 0.56583, 0.803084, 0.576018, 0.71673, 0.636998, 0.682611, 0.582947, 0.658839, 1.21213, 0.425395, 0.562941, 0.780979, 0.63538, 0.667757, 0.653826, 0.690488, 0.605065, 0.60442, 0.591427, 0.514355, 0.699223, 0.624392, 0.586381, 0.52757, 0.601041, 0.678704, 2.276659, 0.608532, 0.679658, 0.656537, 0.656554, 0.809973, 0.728643, 0.642161, 0.621103, 0.608348, 0.736371, 0.59978, 0.802212, 0.591826, 4.46409, 0.620053, 0.625675, 0.657444, 0.691647, 0.678444, 0.721628, 0.587708, 0.641049, 0.758705, 0.668325, 0.563644, 0.607286, 0.882096, 0.629801, 0.684076, 0.696286, 0.727944, 0.684009, 0.630566, 0.706358, 0.640972, 0.547786, 0.519687, 0.665038, 0.604151, 5.35646, 0.72388, 0.566296, 0.679372, 0.533618, 0.747473, 0.610364, 0.524327, 0.817205, 0.637831, 0.638723, 0.817319, 0.58454, 0.560151, 3.076738, 0.808466, 3.734438, 0.600683, 0.602507, 1.026017, 0.645412, 0.797347, 0.648569, 0.68011, 0.673225, 5.860658, 0.632578, 0.596486, 0.667627, 0.702696, 0.701049, 0.56025, 0.640861, 0.690738, 8.491616, 0.751911, 0.53732, 0.752816, 0.752427, 0.670181, 0.714097, 0.659137, 0.61689, 0.826874, 0.522359, 0.660383, 0.78686, 0.579781, 0.713076, 0.907793, 0.733719, 3.586618, 0.819836, 0.560149, 0.546618, 0.709784, 0.636009, 0.731147, 0.662719, 0.637447, 0.548281, 0.576025, 0.672852, 0.669659, 0.697799, 0.637689, 0.593083, 0.757463, 0.544317, 0.771813, 1.114624, 0.676584, 0.565365, 0.591609, 0.56397, 0.504489, 0.578957, 0.662308, 0.823475, 0.841053, 0.75967, 1.017649, 0.755369, 0.661277, 0.629674, 0.626811, 0.722865, 0.676378, 0.532007, 3.903712, 0.693018, 0.645828, 0.544972, 0.574293, 0.826807, 0.583465, 0.70757, 0.495107, 0.60217, 0.483238, 6.522185, 0.673479, 0.618354, 0.808424, 0.80665, 0.600671, 0.596422, 0.594524, 4.250227, 0.639629, 0.502223, 0.704818, 0.72974, 0.944045, 0.689878, 0.552951, 0.815326, 0.658356, 0.63009, 0.620414, 0.599953, 0.596395, 0.573939, 0.804858, 5.664348, 0.8029, 0.675959, 0.537238, 4.79278, 0.81515, 0.691546, 1.546044, 0.451672, 0.621274, 0.697286, 0.763047, 0.658721, 0.647092, 0.69825, 0.611813, 0.626154, 0.721481, 0.800638, 0.595254, 0.646316, 0.855034, 0.482603, 0.88498, 0.613622, 0.639159, 0.627296, 0.679372, 0.648561, 0.583465, 5.456941, 0.656537, 0.587327, 0.544648, 0.624638, 0.752418, 0.508882, 0.584407, 0.636341, 0.625608, 0.57956, 0.649858, 0.710524, 0.741368, 0.493858, 0.623934, 0.66843, 0.632594, 0.795066, 0.817795, 1.105131, 0.667788, 0.633399, 0.785721, 0.578863, 0.553504, 0.754608, 0.619738, 0.889914, 0.520099, 1.008878, 0.620079, 0.908446, 0.579596, 0.691268, 0.582036, 0.568386, 0.650697, 0.71026, 0.699621, 0.588144, 5.348594, 0.650916, 0.653488, 0.675136, 0.685517, 0.928922, 0.874085, 0.641593, 0.712471, 0.862311, 0.690181, 0.695516, 0.672299};
        char ids[] = {0, 2, 1, 2, 1, 0, 0, 0, 0, 2, 2, 2, 2, 2, 0, 2, 0, 2, 0, 3, 2, 2, 2, 3, 0, 0, 0, 1, 2, 4, 0, 1, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 0, 2, 2, 4, 0, 2, 0, 0, 2, 4, 0, 2, 2, 2, 4, 2, 0, 0, 1, 0, 2, 2, 2, 0, 0, 2, 2, 2, 2, 2, 1, 2, 0, 0, 2, 2, 0, 2, 2, 1, 0, 2, 0, 0, 2, 0, 3, 2, 3, 4, 2, 0, 0, 2, 3, 2, 1, 2, 2, 2, 2, 1, 2, 0, 2, 2, 2, 0, 4, 2, 0, 2, 0, 3, 3, 2, 2, 2, 3, 2, 2, 2, 2, 0, 2, 2, 0, 2, 2, 2, 3, 2, 1, 3, 0, 0, 0, 1, 0, 0, 2, 1, 1, 1, 0, 2, 2, 2, 2, 2, 0, 2, 4, 3, 2, 2, 2, 2, 0, 2, 2, 2, 2, 2, 2, 3, 0, 0, 2, 2, 0, 0, 4, 2, 3, 2, 2, 0, 2, 3, 2, 0, 2, 1, 4, 2, 2, 4, 2, 2, 0, 0, 2, 2, 3, 4, 2, 2, 3, 1, 2, 1, 1, 0, 1, 2, 3, 2, 0, 2, 0, 2, 2, 0, 0, 2, 1, 2, 2, 3, 4, 0, 2, 2, 0, 4, 0, 2, 0, 4, 1, 1, 0, 2, 2, 2, 0, 2, 2, 0, 2, 1, 4, 2, 0, 0, 3, 0, 0, 1, 2, 1, 2, 0, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 0, 1, 2, 1, 1, 2, 0, 0, 3, 0, 4, 3, 0, 2, 1, 1, 0, 2, 1, 2, 4, 0, 1, 2, 2, 2, 2, 0, 2, 2, 2, 2, 0, 0, 0, 4, 2, 0, 1, 1, 0, 3, 2, 2, 2, 0, 2, 1, 2, 3, 2, 1, 4, 4, 2, 2, 3, 0, 0, 1, 2, 4, 2, 2, 2, 4, 2, 2, 0, 0, 1, 0, 2, 1, 2, 2, 2, 0, 0, 0, 0, 0, 0, 2, 4, 4, 4, 2, 0, 1, 1, 2, 2, 0, 2, 2, 2, 2, 3, 1, 1, 0, 0, 1, 2, 0, 1, 2, 0, 0, 0, 3, 0, 4, 2, 0, 2, 2, 2, 0, 0, 0, 2, 0, 2, 0, 4, 0, 4, 0, 2, 2, 0, 0, 0, 1, 4, 0, 3, 1, 0, 4, 1, 3, 0, 0, 0, 0, 2, 0, 1, 0, 0, 2, 1, 4, 4, 2, 4, 0, 4, 4, 0, 0, 2, 2, 0, 2, 2, 1, 0, 1, 4, 0, 0, 0, 0, 2, 2, 2, 1, 0, 1, 1, 2, 2, 4, 4, 2, 2, 2, 2, 2, 2, 3, 2, 0, 2, 2, 2, 2, 0, 2, 2, 0, 2, 1, 2, 2, 2, 2, 2, 0, 2, 2, 0, 2, 1, 2, 2, 0, 2, 0, 2, 2, 4, 0, 2, 0, 3, 0, 2, 0, 3, 2, 0, 2, 2, 2, 1, 1, 2, 2, 0, 0, 0, 4, 0, 2, 2, 2, 3, 1, 2, 2, 1, 2, 2, 0, 3, 2, 2, 0, 0, 0, 0, 2, 0, 1, 0, 1, 0, 2, 1, 0, 0, 2, 2, 2, 1, 1, 0, 2, 2, 0, 0, 0, 2, 2, 3, 0, 1, 2, 4, 2, 2, 0, 0, 0, 0, 2, 4, 2, 2, 0, 0, 1, 0, 2, 0, 0, 2, 4, 2, 0, 2, 2, 2, 2, 1, 4, 2, 0, 0, 0, 0, 2, 3, 3, 1, 2, 0, 2, 2, 0, 0, 3, 2, 2, 2, 4, 2, 1, 2, 2, 0, 1, 0, 0, 4, 2, 0, 0, 2, 4, 2, 2, 2, 1, 0, 4, 0, 0, 2, 0, 2, 0, 2, 2, 2, 3, 2, 2, 0, 2, 1, 2, 2, 2, 1, 1, 3, 2, 2, 2, 0, 1, 0, 0, 2, 1, 2, 2, 2, 4, 2, 0, 2, 2, 2, 3, 2, 2, 0, 0, 2, 2, 2, 1, 3, 0, 2, 0, 0, 2, 3, 2, 2, 2, 0, 2, 2, 0, 1, 2, 0, 2, 0, 1, 2, 2, 2, 0, 0, 2, 2, 0, 3, 0, 2, 2, 2, 2, 1, 0, 2, 1, 2, 2, 0, 0, 2, 3, 0, 2, 2, 0, 1, 2, 1, 0, 0, 2, 1, 2, 2, 2, 3, 1, 2, 0, 1, 2, 0, 0, 2, 0, 1, 2, 2, 2, 1, 2, 3, 2, 2, 1, 0, 1, 0, 4, 3, 2, 2, 1, 2, 0, 2, 1, 2, 2, 2, 2, 0, 2, 4, 0, 2, 2, 3, 0, 0, 2, 0, 2, 0, 2, 0, 2, 2, 1, 1, 2, 2, 0, 2, 3, 1, 1, 1, 1, 2, 2, 2, 4, 2, 2, 1, 0, 2, 0, 4, 2, 2, 0, 1, 2, 2, 2, 0, 3, 2, 0, 2, 3, 2, 2, 0, 2, 2, 2, 2, 0, 1, 3, 2, 1, 2, 0, 1, 1, 2, 1, 0, 1, 2, 1, 0, 0, 0, 0, 2, 0, 1, 2, 0, 0, 2, 2, 2, 2, 0, 2, 3, 4, 2, 4, 1, 3, 0, 0, 0, 2, 3, 0, 0, 4, 0, 0, 3, 3, 2, 2, 2, 2, 0, 2, 0, 2, 2, 1, 2, 0, 2, 2, 2, 0, 2, 0, 4, 0, 0, 1, 1, 1, 4, 0, 0, 0, 2, 0, 3, 0, 0, 0, 1, 3, 2, 2, 2, 2, 2, 0, 2, 2, 4, 0, 2, 2, 0, 3, 2, 3, 2, 2, 2, 1, 0, 2, 3, 0, 2, 2, 0, 2, 2, 1, 0, 1, 2, 1, 2, 0, 4, 1, 2, 0, 0, 0, 0, 0, 4, 3, 1, 1, 2, 0, 0, 2, 2, 3, 1, 1, 0, 2, 0, 2, 2, 2, 2, 2, 0, 0, 2, 1, 1, 2, 2, 0, 2, 1, 2, 2, 0, 2, 0, 0, 0, 2, 2, 2, 2, 0};
        // Add test here

        float t_presence = -1;
        float t_small_movement = -1;
        float t_large_movement = -1;
        float f_presence = -1;
        float f_small_movement = -1;
        float f_large_movement = -1;
        get_best_thresholds(&stis, &ids, 1000, &t_presence, &t_small_movement, &t_large_movement, &f_presence, &f_small_movement, &f_large_movement);
        ESP_LOGI("t", "got thresholds %f, %f, %f", t_presence, t_small_movement, t_large_movement);

        TEST_ASSERT_EQUAL_FLOAT(0.4, t_presence);
        TEST_ASSERT_EQUAL_FLOAT(0.7, t_small_movement);
        TEST_ASSERT_EQUAL_FLOAT(0.7, t_large_movement);        
}

TEST_CASE("ts 1", "[ts]")
{
        float stis[] = {0.738833, 0.632646, 0.639466, 0.805031, 0.57575, 0.660125, 0.605764, 0.655062, 0.571586, 1.352579, 0.942714, 0.628264, 3.238059, 0.626869, 0.600926, 0.746361, 0.603475, 0.509654, 0.680963, 0.574974, 0.618189, 0.800236, 0.701207, 0.592873, 0.560102, 0.621765, 0.713076, 0.576426, 0.624392, 0.598033, 0.679627, 0.676611, 0.715317, 0.667941, 0.607963, 0.644805, 0.62912, 0.568386, 0.607226, 0.915845, 0.57672, 0.612468, 0.685883, 0.784837, 0.623172, 0.672929, 0.686566, 0.569232, 0.63225, 0.533653, 0.714379, 0.716114, 0.578112, 0.70757, 0.65272, 0.648809, 0.565237, 0.616668, 0.797194, 0.553504, 0.589523, 0.66105, 0.468473, 0.697247, 0.650381, 0.663637, 0.653484, 0.637883, 0.574315, 0.511254, 0.636587, 0.656734, 0.757463, 0.640867, 0.717639, 0.650969, 1.668896, 0.719804, 0.596954, 0.649858, 0.650535, 0.757502, 0.643033, 0.743868, 0.508843, 0.570702, 5.400203, 0.498931, 0.657031, 0.572841, 0.607492, 0.681251, 0.564227, 0.547681, 0.635949, 0.623424, 0.556831, 0.640675, 0.721224, 1.008878, 0.551822, 0.678444, 0.684929, 0.614279, 0.623213, 0.582802, 0.647213, 0.544211, 0.592404, 1.001575, 0.565098, 0.549171, 0.647126, 0.845823, 0.820492, 3.222029, 0.60082, 0.797248, 0.574554, 0.712359, 0.656621, 0.741265, 0.647781, 0.802404, 7.013315, 0.554019, 0.714085, 0.644705, 0.740448, 0.776618, 0.681143, 0.590436, 5.421758, 0.720859, 0.750114, 0.589033, 0.642598, 0.60656, 0.622269, 0.574797, 0.593, 0.675049, 6.645358, 0.657154, 0.668747, 3.076738, 0.573707, 0.698639, 0.568112, 0.721173, 0.591469, 0.584451, 0.650242, 0.604151, 4.839536, 0.610634, 0.661048, 0.744306, 0.739359, 0.485744, 0.659103, 0.605984, 0.785821, 0.51381, 0.732584, 3.734438, 0.817319, 0.643251, 0.583592, 0.571985, 0.60217, 0.670498, 0.702438, 0.622098, 0.511596, 6.501296, 0.703361, 0.656351, 0.644705, 0.617593, 0.63581, 0.514009, 0.83013, 0.782371, 0.820079, 0.81655, 0.68011, 0.699608, 0.593363, 0.610508, 0.74563, 0.638576, 5.524925, 0.856963, 0.686754, 0.554535, 6.612237, 0.627281, 0.544653, 0.765648, 0.665378, 0.564447, 0.55847, 0.800499, 0.605509, 0.834201, 0.591046, 0.657001, 0.666453, 0.832699, 0.667669, 0.693018, 0.83182, 0.4439, 0.622617, 0.633825, 0.533168, 0.648416, 0.878018, 6.874868, 0.745358, 0.663431, 0.623817, 0.595696, 0.765528, 0.615852, 0.658917, 0.649341, 0.667598, 0.761909, 0.607573, 0.52757, 0.61537, 0.615457, 0.561189, 0.53738, 0.822437, 0.493858, 3.790174, 0.693874, 0.632889, 0.612244, 0.670013, 0.64094, 0.693361, 0.560056, 0.518338, 0.609725, 0.564241, 0.69459, 0.74078, 0.765609, 0.753531, 0.654612, 0.617741, 0.468535, 0.616376, 0.640511, 0.705516, 0.54845, 0.641049, 0.584407, 0.634423, 0.673672, 0.568907, 0.743549, 5.348594, 0.545774, 0.586615, 0.61689, 0.719037, 1.243043, 0.660913, 0.675236, 0.766493, 0.748414, 0.679372, 0.514494, 0.725115, 0.603605, 0.546911, 0.89286, 0.581166, 0.620686, 0.769477, 0.73009, 0.489015, 0.571984, 0.488981, 0.699251, 0.64408, 0.640084, 0.533574, 0.596841, 0.626939, 0.62888, 0.928922, 0.570002, 0.570805, 11.209962, 4.704051, 0.648524, 0.789158, 0.609405, 0.750628, 0.767872, 0.669444, 0.685079, 0.716086, 0.68814, 0.557011, 0.654635, 0.795066, 0.642801, 0.648599, 0.520099, 0.475305, 0.583534, 0.489948, 0.709641, 0.808424, 0.666886, 0.666676, 0.731899, 0.637783, 0.615974, 0.518724, 0.683025, 0.677994, 0.685478, 0.601269, 0.73326, 0.672795, 0.574965, 0.706337, 0.852283, 0.707722, 0.634833, 0.578863, 0.555772, 0.702631, 0.623763, 0.736336, 0.762269, 0.796727, 1.017649, 0.669786, 0.728504, 0.562973, 0.943793, 0.683526, 0.679471, 0.722543, 0.483238, 0.630687, 4.347304, 0.635214, 0.693965, 0.62914, 0.614943, 0.659608, 0.90013, 1.433381, 0.606144, 0.570532, 0.622179, 0.668957, 2.206558, 0.659718, 0.692085, 0.742079, 0.583445, 0.595836, 1.035196, 0.678681, 0.592768, 0.70026, 0.805575, 0.632506, 0.842842, 0.638447, 0.660264, 0.547493, 0.615408, 0.633567, 0.604821, 0.801882, 0.568254, 0.6823, 0.641897, 0.676584, 0.681755, 0.669025, 0.6308, 0.645763, 0.846327, 0.734609, 0.69459, 0.548958, 0.687837, 0.630461, 0.739448, 0.582252, 0.573056, 0.80803, 0.643016, 4.79278, 0.548297, 0.684009, 0.66168, 0.616622, 0.67796, 0.733577, 5.487283, 0.667165, 0.639159, 0.847675, 0.826527, 0.697513, 0.676766, 0.681176, 0.667834, 0.737981, 0.699223, 0.625136, 0.567188, 0.559241, 0.63824, 0.675815, 0.683903, 0.562919, 0.679269, 0.574712, 0.565288, 0.599671, 0.580239, 0.679709, 0.70343, 0.889853, 0.676378, 0.662781, 0.777342, 0.666868, 0.577254, 0.673359, 0.66077, 0.789521, 11.442247, 0.610719, 0.659136, 0.916929, 0.583465, 3.09403, 0.665226, 0.600776, 0.596841, 0.581692, 0.618048, 0.645156, 0.574219, 0.686971, 0.825711, 4.145148, 0.713723, 0.635174, 0.634211, 0.814239, 0.677763, 0.586275, 0.602742, 1.955083, 0.674164, 0.619422, 0.696314, 0.688777, 0.703723, 0.754112, 0.726267, 0.724414, 0.620206, 0.590226, 0.564707, 9.144422, 8.491616, 0.700303, 0.609228, 0.633331, 0.735202, 0.723158, 0.598562, 0.72077, 0.64571, 0.621274, 0.64915, 0.617745, 0.654182, 0.673222, 0.785902, 0.533863, 0.603421, 0.618797, 0.626767, 0.64116, 0.585079, 0.81155, 0.505566, 0.731867, 0.588636, 0.977785, 0.624428, 0.808618, 0.672624, 0.755451, 0.624244, 0.665719, 1.133576, 0.615996, 0.626153, 0.776524, 0.704472, 5.160976, 0.640877, 0.618192, 0.632745, 0.667382, 0.612328, 0.712165, 0.725634, 0.718735, 0.617715, 0.711788, 1.217234, 0.62102, 0.643154, 0.633704, 0.659109, 0.663979, 1.027607, 0.641897, 0.929751, 0.699885, 0.704024, 3.115952, 0.544969, 1.929864, 0.686989, 0.785699, 6.759668, 5.165224, 0.735693, 0.532007, 0.583405, 0.589669, 0.603304, 6.784873, 0.541105, 0.872791, 0.543071, 0.648164, 0.7496, 0.54734, 0.833521, 0.836518, 2.496253, 6.061306, 0.59562, 0.709292, 0.923298, 0.587184, 0.579508, 0.795179, 0.600926, 0.717134, 0.805958, 0.56815, 0.623735, 0.55047, 0.607686, 0.728721, 0.789855, 0.618036, 0.584428, 0.782844, 0.57687, 0.711161, 0.587969, 0.943446, 0.622315, 0.480345, 4.157569, 3.960548, 0.768146, 0.717969, 3.188094, 0.550464, 0.94083, 0.726641, 0.908624, 0.76168, 0.703849, 0.801652, 0.792488, 0.763106, 0.732699, 0.725401, 4.247816, 0.688125, 3.761051, 0.552187, 0.605134, 0.854329, 0.514465, 0.677263, 0.614886, 0.537351, 0.703386, 0.502479, 0.79124, 0.614567, 0.604278, 0.717416, 0.630664, 0.728359, 0.774349, 0.625675, 0.667627, 0.985314, 0.729734, 0.853196, 0.704808, 0.578529, 0.732082, 0.642725, 0.60047, 0.641141, 0.611714, 0.770338, 0.526266, 0.67388, 0.700413, 0.600551, 0.717901, 0.727498, 0.7256, 0.724863, 0.669678, 0.677686, 0.595707, 0.680515, 0.608486, 0.575581, 0.791353, 0.929582, 0.58887, 0.71905, 0.590185, 0.811734, 0.71006, 0.530616, 0.59593, 3.078768, 0.660999, 0.719715, 0.592888, 0.695499, 0.722001, 0.655137, 0.634243, 0.56832, 0.894404, 0.747407, 1.551735, 0.72114, 0.895331, 0.657714, 0.593525, 0.696187, 0.648943, 0.621434, 0.684572, 0.607495, 0.626154, 0.63962, 0.616883, 0.61162, 0.678809, 0.637831, 4.330443, 0.645526, 0.705802, 0.882617, 0.532128, 0.721224, 0.933184, 0.588346, 0.710457, 0.631086, 0.695144, 0.762773, 0.798449, 0.653124, 0.806236, 0.613622, 0.68735, 0.683474, 0.742525, 0.585031, 0.625724, 0.710294, 0.660932, 0.783007, 0.581731, 0.781762, 0.555838, 0.59921, 0.632807, 0.738735, 0.584615, 0.647487, 0.683138, 0.594821, 0.500142, 0.60441, 0.535495, 0.698162, 0.615574, 0.854834, 0.772178, 0.625502, 0.690419, 0.669603, 0.769112, 0.613401, 0.787075, 0.699131, 0.581603, 1.144554, 0.803084, 0.556093, 0.557887, 0.987749, 0.681418, 0.69664, 0.592641, 0.722054, 0.472996, 0.642413, 0.618947, 0.651075, 0.564742, 0.791157, 0.535049, 0.665516, 0.593576, 0.734812, 0.905373, 0.665719, 0.656255, 0.701115, 0.462516, 0.592662, 0.785065, 0.819643, 0.926307, 0.698118, 0.651202, 0.760485, 0.694254, 0.60587, 2.364908, 0.574826, 0.67293, 0.60466, 0.879278, 0.75599, 0.830683, 0.508882, 1.131013, 0.620116, 0.529373, 0.770787, 0.561498, 0.656919, 0.553504, 0.757107, 1.123002, 0.658333, 0.711594, 0.686185, 0.718828, 0.642101, 0.846788, 0.725661, 0.697759, 0.649667, 0.950189, 0.752949, 0.67255, 0.630076, 3.883291, 0.630098, 0.58676, 0.795659, 0.667644, 0.644012, 0.824655, 0.788306, 0.697855, 0.622175, 0.784729, 0.874324, 0.558711, 0.611358, 0.613353, 0.712004, 0.639727, 0.652494, 0.6294, 1.393749, 0.775704, 0.584245, 0.540633, 0.606541, 0.779198, 0.592, 0.720709, 0.649607, 0.662719, 0.732485, 0.607418, 0.645546, 0.654378, 11.472062, 4.624808, 0.650378, 0.827564, 0.725752, 0.684852, 0.605582, 0.812915, 0.527082, 0.582251, 0.516961, 0.692136, 1.21213, 0.623464, 0.747959, 0.606422, 1.046951, 0.795853, 0.493858, 0.663525, 1.027106, 0.758014, 0.664993, 0.904422, 0.710519, 0.604978, 0.614567, 0.819108, 0.551038, 0.672916, 0.638369, 0.91217, 0.760563, 0.552257, 0.605821, 4.617345, 0.648561, 0.679749, 0.777798, 0.647453, 0.581751, 0.58788, 0.642936, 1.720113, 0.511254, 0.599982, 0.607696, 0.643732, 0.941667, 0.827615, 0.576025, 0.601202, 0.578863, 0.814622, 0.687228, 0.740193, 0.745917, 0.564782, 0.525355, 0.731052, 0.860959, 0.81398, 0.779403, 0.665188, 0.844083, 0.592199, 0.65435, 0.639667, 6.257453, 0.742189, 0.671003, 0.838727, 0.75438, 0.69473, 0.686848, 0.75599, 4.087637, 0.594498, 0.637447, 0.633161, 0.63656, 0.752418, 0.586325, 0.661774, 0.778769, 0.616743, 0.700297, 0.690867, 3.674069, 0.724448, 0.485744, 0.761432, 0.823247, 0.673654, 0.63807, 0.720483, 0.697786, 0.823461, 0.68011, 0.569569, 0.722278, 1.931462, 0.596422, 1.080805, 0.582036, 0.655623, 0.564304, 0.518102, 0.638325, 0.604471, 0.995276, 0.643829, 0.608492, 0.744335, 0.604654, 0.710835, 0.709633, 0.528597, 0.676782, 0.652178, 0.639284, 0.613187, 0.788296, 0.578886, 0.743174, 0.741605, 0.737151, 0.709947, 0.569612, 0.689675, 0.913059, 0.50062, 0.61229, 0.645526, 0.569496, 0.774294, 0.664756, 0.667427, 0.621101, 0.529865, 0.53833, 0.892329, 0.714425, 0.63755, 0.559342, 0.672572, 11.120714, 0.631689, 0.846788, 0.704096, 0.692988, 0.586381, 0.625608, 0.771192, 0.603794, 1.551446, 0.639275, 0.540168, 0.651164, 0.60664, 0.595748, 0.609663, 0.686189, 0.621293, 0.817205, 0.754547, 0.700218, 0.594255};
        char ids[] = {2, 2, 3, 4, 2, 0, 2, 0, 1, 0, 2, 0, 4, 2, 2, 2, 1, 2, 0, 0, 0, 2, 2, 0, 2, 4, 0, 0, 2, 4, 3, 2, 2, 2, 2, 2, 2, 1, 2, 2, 0, 0, 1, 0, 0, 2, 1, 2, 2, 2, 2, 2, 0, 2, 2, 0, 1, 2, 2, 2, 2, 2, 0, 0, 2, 1, 1, 0, 0, 2, 2, 2, 2, 2, 2, 0, 2, 0, 2, 0, 3, 0, 4, 2, 2, 1, 2, 2, 1, 0, 4, 2, 2, 3, 0, 0, 0, 3, 2, 2, 2, 2, 2, 0, 3, 1, 1, 0, 2, 4, 2, 2, 2, 2, 2, 1, 0, 2, 2, 2, 1, 2, 1, 4, 1, 2, 0, 3, 4, 0, 0, 1, 3, 0, 0, 2, 2, 3, 2, 0, 0, 1, 0, 0, 1, 2, 2, 1, 1, 0, 2, 0, 1, 0, 2, 2, 4, 2, 4, 1, 2, 2, 3, 0, 1, 4, 2, 2, 2, 0, 2, 1, 2, 2, 1, 2, 0, 4, 2, 1, 2, 2, 0, 4, 2, 2, 2, 4, 2, 4, 0, 2, 2, 0, 0, 2, 2, 2, 0, 2, 3, 2, 0, 3, 0, 0, 0, 2, 2, 2, 1, 0, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 0, 2, 2, 1, 2, 3, 2, 2, 0, 2, 2, 0, 1, 3, 2, 0, 0, 0, 2, 0, 1, 0, 2, 2, 3, 2, 0, 2, 1, 0, 4, 1, 2, 4, 0, 2, 0, 1, 1, 0, 4, 1, 2, 1, 3, 0, 4, 2, 2, 1, 2, 0, 0, 1, 1, 1, 2, 0, 1, 2, 1, 2, 0, 2, 2, 0, 2, 0, 0, 3, 2, 0, 2, 0, 2, 0, 2, 1, 3, 1, 0, 2, 0, 0, 1, 1, 2, 0, 4, 3, 2, 2, 1, 2, 2, 0, 4, 2, 2, 0, 0, 1, 1, 2, 0, 1, 2, 0, 2, 2, 2, 3, 1, 0, 2, 2, 2, 2, 4, 2, 4, 0, 0, 2, 0, 0, 1, 2, 2, 2, 4, 1, 2, 2, 3, 2, 2, 2, 1, 2, 2, 2, 3, 0, 0, 3, 2, 1, 0, 2, 2, 0, 2, 0, 0, 1, 2, 0, 2, 1, 2, 2, 2, 0, 0, 4, 3, 2, 2, 1, 0, 0, 3, 0, 4, 2, 2, 0, 0, 0, 1, 2, 4, 0, 2, 2, 1, 3, 2, 2, 2, 3, 2, 2, 1, 1, 2, 0, 2, 0, 2, 0, 4, 2, 2, 2, 4, 1, 0, 2, 1, 0, 3, 0, 2, 1, 0, 3, 1, 2, 0, 2, 0, 2, 2, 2, 1, 2, 0, 1, 0, 4, 4, 2, 1, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 0, 1, 3, 0, 1, 2, 2, 1, 0, 2, 1, 2, 1, 2, 2, 1, 0, 2, 0, 4, 2, 2, 2, 4, 3, 2, 1, 0, 2, 0, 2, 0, 2, 2, 0, 2, 1, 0, 2, 2, 2, 1, 0, 1, 2, 2, 2, 2, 0, 0, 0, 3, 2, 0, 1, 1, 2, 3, 0, 1, 4, 2, 1, 0, 0, 2, 2, 2, 0, 0, 0, 2, 1, 2, 2, 1, 0, 2, 0, 2, 2, 1, 0, 2, 2, 2, 2, 2, 2, 1, 2, 0, 0, 1, 3, 2, 0, 4, 2, 1, 2, 0, 2, 1, 2, 0, 1, 1, 1, 3, 2, 2, 2, 0, 2, 2, 2, 2, 0, 0, 3, 0, 2, 2, 1, 2, 2, 3, 2, 2, 1, 2, 1, 2, 3, 2, 0, 2, 0, 1, 0, 2, 2, 1, 3, 2, 2, 3, 2, 2, 1, 2, 2, 0, 2, 2, 2, 0, 2, 2, 1, 2, 0, 2, 4, 4, 0, 1, 0, 3, 3, 2, 0, 3, 0, 0, 0, 2, 3, 2, 2, 1, 0, 2, 4, 1, 2, 2, 2, 1, 2, 1, 2, 1, 3, 2, 0, 2, 2, 0, 0, 4, 1, 2, 2, 2, 2, 2, 0, 2, 2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 0, 2, 2, 3, 2, 2, 2, 0, 0, 2, 0, 2, 0, 1, 0, 0, 2, 2, 0, 2, 4, 0, 2, 2, 2, 2, 2, 1, 0, 1, 4, 1, 3, 1, 2, 1, 2, 0, 0, 0, 2, 2, 1, 2, 1, 0, 2, 0, 1, 0, 4, 2, 2, 0, 3, 0, 0, 2, 0, 1, 2, 0, 1, 2, 2, 0, 2, 2, 0, 1, 2, 2, 0, 0, 2, 0, 3, 0, 0, 0, 3, 1, 0, 2, 0, 0, 2, 3, 0, 1, 2, 2, 1, 2, 0, 0, 2, 3, 0, 2, 2, 3, 0, 3, 0, 2, 2, 2, 3, 2, 2, 0, 1, 2, 1, 2, 0, 2, 2, 0, 2, 1, 3, 2, 0, 0, 4, 1, 0, 1, 4, 0, 2, 2, 0, 1, 3, 1, 2, 0, 0, 0, 2, 3, 0, 2, 0, 2, 2, 2, 1, 1, 2, 2, 3, 1, 2, 0, 4, 2, 1, 0, 0, 2, 1, 4, 3, 2, 2, 2, 0, 2, 2, 2, 0, 1, 1, 2, 0, 2, 0, 0, 0, 1, 2, 2, 3, 0, 2, 1, 2, 2, 0, 2, 0, 2, 2, 0, 0, 0, 0, 2, 2, 0, 2, 4, 0, 2, 0, 0, 3, 2, 3, 0, 2, 0, 2, 0, 2, 0, 2, 2, 2, 1, 0, 3, 2, 0, 2, 0, 1, 4, 1, 2, 2, 0, 0, 2, 2, 2, 2, 1, 2, 4, 0, 4, 1, 1, 0, 0, 1, 0, 2, 2, 2, 2, 0, 2, 1, 0, 2, 1, 1, 2, 4, 2, 3, 2, 0, 2, 1, 0, 2, 4, 0, 2, 0, 4, 0, 2, 2, 0, 2, 2, 3, 0, 0, 0, 2, 2, 3, 2, 2, 1, 0, 3, 2, 3, 1, 1, 0, 1, 1, 2, 0, 2, 2, 2, 1, 2};
        // Add test here

        float t_presence = -1;
        float t_small_movement = -1;
        float t_large_movement = -1;
        float f_presence = -1;
        float f_small_movement = -1;
        float f_large_movement = -1;
        get_best_thresholds(&stis, &ids, 1000, &t_presence, &t_small_movement, &t_large_movement, &f_presence, &f_small_movement, &f_large_movement);
        ESP_LOGI("t", "got thresholds %f, %f, %f", t_presence, t_small_movement, t_large_movement);

        TEST_ASSERT_EQUAL_FLOAT(0.4, t_presence);
        TEST_ASSERT_EQUAL_FLOAT(0.6, t_small_movement);
        TEST_ASSERT_EQUAL_FLOAT(0.9, t_large_movement);   
}

TEST_CASE("ts no activity 4", "[ts]")
{
        float stis[] = {0.736631, 0.63465, 0.847251, 0.607242, 0.540168, 0.657083, 0.648721, 0.629095, 0.638936, 0.622584, 0.924458, 0.653124, 0.618947, 0.720709, 0.624428, 0.652178, 0.501105, 0.685013, 0.722722, 0.671332, 0.695666, 0.603475, 5.228582, 0.692108, 0.606144, 0.568254, 0.735386, 0.551981, 0.661267, 0.603794, 0.57197, 0.531586, 0.529865, 0.869384, 0.652593, 0.729141, 0.772559, 4.081049, 0.681577, 0.54638, 0.508816, 0.628014, 0.94934, 0.605134, 0.533366, 0.5834, 0.724752, 0.596006, 0.606484, 0.659168, 0.543036, 0.571508, 0.624142, 0.591427, 0.741368, 0.5542, 0.741398, 0.65604, 0.698599, 0.68896, 0.645546, 1.454016, 0.566383, 0.618296, 0.734355, 0.634093, 0.58147, 0.722903, 0.625808, 0.626889, 0.670522, 0.535495, 0.832744, 0.592404, 0.718735, 0.784729, 0.818598, 0.610158, 0.683903, 0.683609, 3.14255, 0.494153, 0.659751, 0.71026, 0.693039, 0.618188, 0.69357, 0.712006, 0.662322, 0.640775, 0.574797, 0.640688, 0.696481, 1.226922, 0.615994, 0.642755, 0.618659, 0.76093, 0.567958, 0.594174, 0.722865, 0.653535, 0.712426, 0.549279, 0.705384, 0.793124, 0.551038, 0.828811, 0.745317, 0.577054, 0.636226, 0.72547, 0.689675, 0.703652, 0.586964, 0.7256, 0.700521, 0.764458, 0.585998, 5.764874, 0.689878, 0.731981, 0.776794, 0.819783, 0.613401, 0.607418, 0.593351, 0.873193, 0.66105, 0.596909, 0.560608, 0.702045, 0.654635, 0.639288, 0.785721, 0.771165, 0.584407, 4.754633, 0.734138, 0.734908, 0.560495, 0.558708, 0.514355, 0.667296, 0.690179, 4.161851, 0.680085, 0.670485, 0.643957, 0.83013, 0.576426, 0.605582, 0.76967, 0.772264, 0.660881, 0.650969, 0.81515, 0.712771, 0.640675, 0.776433, 0.693039, 0.603669, 0.554535, 0.737996, 0.62412, 0.681733, 0.544899, 11.120714, 0.493858, 0.735054, 0.760485, 0.741256, 0.686283, 0.64468, 0.568306, 0.671659, 0.520894, 0.948522, 0.61505, 0.679532, 0.640223, 0.72114, 0.561203, 8.864655, 0.632013, 0.590652, 0.822423, 0.783305, 0.783549, 0.697247, 0.672774, 0.680307, 0.56832, 0.565371, 0.622395, 0.948406, 0.574717, 0.672916, 0.616863, 0.564577, 0.689746, 0.478664, 0.601617, 0.655137, 0.748243, 0.582947, 0.597796, 0.599495, 0.677434, 0.57126, 0.526266, 0.665038, 0.570508, 0.604978, 0.695064, 0.549405, 0.793849, 0.800638, 0.628757, 0.525355, 0.70307, 0.855973, 0.665473, 0.577901, 0.672815, 0.733493, 0.632745, 0.665936, 0.452987, 0.748554, 0.588591, 0.597074, 0.805453, 0.639334, 9.5498, 0.705178, 0.679709, 0.839797, 0.673179, 0.600745, 0.703682, 0.855459, 0.488981, 0.636975, 0.715274, 0.636587, 0.680239, 0.617776, 0.582872, 0.642485, 0.553504, 0.632594, 0.513582, 0.639605, 4.10497, 0.670991, 0.585142, 0.631086, 0.689929, 0.976094, 4.330443, 0.638795, 0.678444, 0.726738, 0.648827, 1.323597, 0.665939, 0.581081, 0.700297, 0.581513, 0.882235, 3.609496, 0.743203, 0.692782, 0.903409, 0.608576, 0.590849, 0.683131, 0.623735, 0.60031, 0.644705, 0.806404, 0.658917, 0.797194, 0.775921, 0.641864, 0.733577, 0.855034, 0.553682, 0.596598, 0.62102, 0.626869, 3.09403, 0.587452, 0.558949, 0.632745, 0.844083, 0.602857, 0.694865, 0.679372, 0.634833, 0.655735, 0.89286, 0.677636, 0.544175, 0.759712, 0.672624, 0.731378, 0.826807, 0.720963, 0.953842, 0.693401, 0.650302, 0.643829, 0.653525, 0.658373, 0.592948, 0.600683, 0.46689, 0.640511, 0.580748, 0.628081, 0.608532, 0.564733, 0.57444, 0.546086, 0.748122, 0.65788, 0.559202, 0.662692, 0.62084, 0.588714, 0.554162, 0.643251, 0.959213, 0.924586, 1.080048, 0.625068, 0.824957, 0.692344, 0.682972, 0.668244, 1.551446, 0.587708, 0.752816, 0.601467, 0.502109, 0.847675, 0.952818, 0.604543, 0.570002, 0.686565, 6.501296, 0.616373, 0.599557, 0.717313, 0.581603, 0.736638, 0.698264, 0.595639, 0.587812, 0.754827, 9.424663, 0.819399, 0.614039, 0.622098, 0.707262, 0.626097, 0.725115, 0.674471, 0.677994, 0.78361, 0.714379, 9.65649, 0.664287, 0.681049, 0.669678, 0.754827, 0.690721, 0.630319, 0.703768, 0.722001, 1.021757, 0.683541, 0.632962, 0.67293, 0.776524, 0.578529, 0.612482, 0.621241, 0.658097, 6.201849, 0.599892, 0.674598, 0.620306, 0.63058, 0.788263, 0.732485, 0.52592, 0.725708, 0.723489, 0.665866, 0.97031, 0.677047, 0.9815, 0.550464, 0.716829, 3.279786, 0.679855, 0.582856, 0.63905, 0.581603, 0.750572, 0.743245, 0.790007, 0.699055, 0.722278, 0.646699, 0.630076, 0.562436, 0.709116, 0.68633, 0.936586, 0.545633, 0.658358, 0.654949, 0.519726, 0.579001, 0.68011, 0.772017, 0.626362, 0.721173, 0.561189, 0.480345, 0.576943, 0.849476, 0.703781, 0.61302, 0.660132, 0.634529, 0.877966, 0.685593, 0.60466, 0.497475, 0.667834, 0.94083, 0.701736, 0.79846, 0.668916, 0.644814, 1.781355, 0.613615, 0.668657, 0.6294, 0.616743, 0.564076, 3.29234, 0.631966, 0.567848, 0.721506, 0.80184, 1.912041, 0.647487, 0.643579, 0.715002, 4.145148, 0.665599, 0.738876, 0.619382, 0.602697, 0.685741, 3.086602, 0.887203, 0.838727, 0.759512, 0.801652, 1.027607, 0.777844, 0.604278, 0.636465, 0.810991, 0.667427, 0.709009, 0.584428, 0.752949, 0.686848, 0.532248, 0.657722, 0.536428, 0.675104, 0.495278, 0.666868, 0.599793, 2.106297, 0.704256, 0.655896, 0.845984, 0.702077, 0.632889, 0.658897, 0.603212, 0.639314, 0.820492, 0.764174, 0.651529, 0.650048, 0.578112, 0.589155, 0.597337, 0.822795, 0.672922, 0.627848, 0.710457, 0.639117, 0.701503, 0.59687, 0.654051, 0.665911, 0.585672, 0.63859, 0.609664, 8.637671, 0.626662, 0.703723, 0.798449, 0.527146, 0.64094, 0.697643, 0.942285, 0.592475, 0.629514, 0.609808, 0.681073, 0.766903, 4.498301, 0.749046, 0.695144, 0.812939, 3.314567, 0.703221, 0.871879, 0.798576, 3.586618, 1.03455, 0.61616, 0.52706, 0.831177, 0.633825, 0.667788, 0.60217, 0.744177, 0.796144, 0.59956, 0.389082, 0.599932, 0.669603, 0.557011, 0.559048, 0.640875, 0.908624, 0.750596, 0.593034, 0.856327, 0.589256, 0.640906, 0.593111, 0.529679, 0.655192, 0.637785, 0.71964, 0.613533, 11.447795, 0.639727, 0.556945, 0.478447, 0.73009, 0.688125, 0.502154, 0.960634, 0.68102, 0.656255, 3.761051, 0.674151, 0.715639, 0.69977, 0.804796, 0.649938, 0.677544, 0.713076, 0.802212, 0.620414, 0.710559, 0.741726, 0.860959, 0.648721, 0.508946, 0.562941, 0.722795, 0.60587, 0.654191, 0.780305, 0.666577, 0.574965, 0.679658, 0.656226, 0.668619, 0.855229, 0.529468, 0.603819, 0.668838, 7.633355, 0.570441, 4.498301, 0.714085, 0.645526, 0.622175, 0.595707, 0.691268, 0.615852, 0.607696, 4.365738, 0.564441, 0.714744, 0.665357, 0.971659, 0.681176, 0.66504, 0.699995, 3.698958, 0.760796, 0.77035, 0.602507, 0.585753, 0.586381, 1.114624, 0.608908, 0.513698, 0.805958, 0.69459, 0.620053, 0.620116, 0.799437, 3.109569, 0.633116, 0.641593, 0.67695, 0.721405, 0.481411, 0.868921, 0.672516, 0.997461, 0.522454, 0.556831, 0.690313, 0.657444, 0.703595, 0.503572, 0.678597, 0.588984, 0.838512, 0.797248, 0.626879, 0.672247, 0.685593, 0.67255, 0.80803, 0.669568, 0.686754, 0.634207, 0.633567, 0.576018, 0.633068, 0.47045, 0.559998, 0.726826, 0.488591, 0.497325, 0.620923, 0.708375, 0.742664, 0.665226, 0.583465, 0.722933, 1.00043, 0.858757, 1.544209, 0.697513, 0.686644, 0.606163, 0.804137, 0.574315, 0.679947, 4.30611, 0.56025, 0.598953, 0.926307, 0.69825, 0.642136, 0.582548, 0.59176, 0.661774, 0.659011, 0.570175, 0.782789, 0.546911, 0.556951, 0.644124, 0.747352, 9.677143, 0.716175, 0.726852, 0.680179, 0.873442, 1.095157, 0.648943, 0.559618, 0.656919, 0.571878, 0.572426, 0.773371, 0.60217, 0.880325, 0.642161, 6.415303, 0.632391, 0.687228, 1.668896, 0.479157, 0.541898, 0.712281, 0.569282, 0.780979, 0.661277, 0.709784, 0.620546, 0.719306, 0.508843, 0.610539, 0.682121, 0.647551, 0.537238, 0.595136, 1.022112, 0.620049, 0.622306, 0.723954, 0.873877, 0.890634, 0.63505, 0.531586, 0.665038, 0.664756, 0.609764, 0.70757, 0.724863, 0.638723, 0.612985, 1.131013, 0.718673, 0.695499, 0.854042, 0.69473, 0.778213, 0.60622, 0.71594, 0.723479, 0.543579, 0.647213, 1.095314, 0.723272, 0.592498, 0.76168, 0.55825, 0.583683, 0.552962, 0.588921, 0.587365, 1.929864, 0.593912, 0.63661, 0.62753, 0.646931, 0.749782, 0.531355, 0.808194, 0.781762, 0.801882, 0.592467, 0.712004, 0.636106, 0.634498, 0.528521, 0.740937, 0.704808, 0.623943, 0.562955, 0.689343, 0.645581, 0.639412, 0.66168, 0.668911, 0.57925, 4.46409, 0.662144, 0.673396, 2.364908, 4.084213, 0.688688, 0.66893, 0.649667, 0.583488, 0.637322, 0.845562, 0.638963, 0.683474, 0.701895, 0.629159, 0.566131, 0.633842, 0.692482, 0.764452, 0.536143, 0.620133, 0.633704, 0.713328, 0.544032, 0.859261, 0.613006, 0.697288, 0.571985, 0.598758, 0.678548, 0.584615, 1.067028, 0.5806, 0.573446, 1.21213, 0.728237, 0.682136, 0.785721, 0.722977, 0.679172, 0.685534, 0.737419, 0.664172, 0.642598, 0.661151, 0.612666, 0.787263, 0.626086, 0.633075, 0.627624, 0.634423, 0.712359, 0.644488, 0.719449, 0.68011, 0.615574, 0.722494, 0.838388, 0.874324, 0.57672, 0.896476, 0.819643, 0.581357, 0.548425, 0.673225, 0.741162, 0.562817, 0.780377, 0.739984, 0.649341, 0.572801, 0.638002, 0.533863, 0.633161, 0.70049, 0.743431, 0.54094, 0.514494, 1.064179, 0.580166, 1.131656, 0.635215, 3.997373, 0.625739, 0.916929, 0.615457, 0.765845, 0.737263, 0.722507, 0.836014, 0.743054, 0.752452, 0.685478, 0.593942, 0.635441, 0.7496, 0.670181, 0.667627, 0.787473, 0.520099, 0.712498, 0.59593, 0.717805, 0.733588, 0.699854, 0.775921, 0.940395, 0.630664, 0.567173, 0.664679, 0.638049, 0.57705, 0.638352, 0.607226, 0.684076, 0.575312, 0.585477, 0.58687, 0.708487, 0.719715, 0.594524, 0.684628, 0.593186, 0.789191, 0.640861, 0.628264, 0.720859, 0.618219, 0.625502, 0.880917, 0.67315, 0.830756, 0.623172, 0.609854, 0.783006, 0.610892, 5.705317, 0.690488, 0.656226};
        char ids[] = {2, 2, 2, 2, 1, 2, 3, 2, 0, 3, 2, 0, 2, 1, 2, 1, 2, 2, 0, 2, 2, 1, 1, 0, 1, 2, 2, 1, 2, 2, 2, 1, 0, 2, 1, 2, 0, 2, 0, 0, 1, 2, 2, 1, 2, 2, 3, 1, 2, 2, 0, 0, 2, 1, 3, 2, 2, 2, 2, 0, 0, 2, 0, 1, 3, 2, 2, 3, 1, 2, 2, 1, 2, 2, 0, 0, 2, 1, 2, 2, 1, 2, 2, 2, 2, 2, 0, 2, 2, 0, 0, 2, 2, 1, 2, 2, 0, 2, 1, 3, 2, 2, 2, 1, 2, 2, 0, 2, 2, 2, 0, 0, 0, 2, 2, 3, 2, 2, 0, 2, 2, 0, 2, 2, 1, 0, 2, 0, 2, 0, 2, 1, 2, 0, 1, 1, 0, 2, 2, 2, 1, 2, 2, 0, 0, 2, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 2, 2, 3, 0, 2, 1, 2, 2, 2, 0, 1, 2, 1, 2, 3, 2, 0, 0, 1, 0, 1, 2, 2, 2, 0, 1, 2, 3, 0, 0, 0, 2, 2, 0, 2, 0, 2, 2, 2, 1, 1, 2, 2, 0, 2, 2, 2, 0, 1, 2, 0, 1, 0, 2, 1, 2, 2, 2, 2, 1, 0, 0, 3, 0, 2, 3, 2, 0, 1, 2, 0, 2, 1, 0, 2, 1, 2, 2, 3, 2, 0, 0, 0, 2, 0, 2, 2, 2, 0, 2, 0, 2, 0, 0, 2, 0, 1, 2, 2, 2, 0, 2, 0, 0, 2, 1, 2, 1, 2, 0, 2, 0, 2, 2, 2, 0, 3, 2, 2, 1, 2, 2, 0, 2, 3, 2, 2, 2, 2, 0, 3, 0, 0, 1, 1, 2, 2, 0, 2, 0, 0, 3, 0, 1, 1, 2, 2, 0, 2, 2, 1, 2, 2, 0, 0, 2, 0, 2, 3, 1, 1, 2, 1, 2, 1, 0, 1, 1, 1, 2, 1, 0, 2, 0, 1, 1, 0, 2, 0, 2, 2, 0, 2, 0, 2, 1, 3, 0, 0, 2, 0, 3, 0, 2, 2, 3, 2, 2, 0, 2, 0, 0, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1, 1, 2, 2, 2, 0, 0, 2, 0, 1, 2, 2, 2, 2, 0, 2, 0, 1, 0, 1, 2, 2, 2, 1, 2, 2, 1, 2, 0, 2, 1, 0, 2, 2, 0, 0, 2, 1, 2, 2, 0, 1, 2, 0, 3, 2, 2, 0, 2, 1, 3, 0, 2, 0, 2, 1, 1, 0, 0, 1, 2, 3, 2, 0, 2, 0, 2, 2, 0, 2, 1, 2, 0, 0, 0, 1, 2, 2, 0, 2, 2, 0, 0, 0, 1, 1, 0, 2, 0, 0, 3, 0, 3, 0, 0, 2, 1, 3, 2, 1, 2, 2, 1, 2, 3, 0, 2, 1, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1, 2, 1, 1, 2, 3, 2, 3, 0, 2, 2, 0, 0, 1, 0, 2, 2, 2, 2, 2, 0, 1, 2, 0, 2, 2, 2, 2, 2, 1, 3, 2, 1, 2, 2, 2, 1, 2, 2, 1, 2, 0, 0, 2, 2, 0, 2, 0, 3, 2, 0, 2, 2, 0, 2, 0, 1, 1, 1, 2, 2, 2, 3, 2, 0, 2, 2, 2, 2, 2, 1, 2, 2, 3, 3, 1, 2, 2, 1, 1, 0, 2, 1, 0, 2, 2, 0, 1, 1, 1, 0, 1, 0, 0, 2, 2, 2, 2, 2, 2, 0, 2, 0, 2, 0, 0, 3, 3, 3, 0, 2, 1, 0, 0, 1, 2, 2, 0, 1, 1, 2, 0, 1, 1, 3, 0, 3, 0, 2, 2, 2, 2, 0, 2, 2, 2, 0, 1, 1, 1, 2, 2, 2, 2, 2, 2, 1, 1, 2, 2, 0, 1, 3, 2, 0, 0, 0, 0, 0, 1, 2, 1, 2, 2, 2, 0, 0, 3, 2, 1, 1, 0, 2, 0, 2, 0, 1, 0, 2, 0, 1, 0, 2, 1, 0, 0, 1, 3, 1, 0, 1, 0, 0, 2, 0, 1, 2, 0, 0, 1, 2, 2, 0, 1, 0, 3, 2, 2, 2, 0, 2, 2, 2, 2, 3, 2, 1, 2, 1, 0, 2, 0, 1, 0, 2, 0, 2, 2, 2, 1, 1, 0, 0, 0, 2, 2, 2, 1, 2, 2, 2, 0, 1, 2, 2, 0, 0, 0, 1, 2, 2, 2, 1, 0, 0, 2, 0, 2, 0, 0, 0, 0, 0, 1, 2, 0, 0, 2, 0, 2, 1, 3, 2, 3, 2, 0, 0, 1, 2, 2, 2, 1, 0, 0, 1, 2, 2, 2, 0, 2, 2, 2, 1, 3, 0, 1, 0, 1, 2, 2, 2, 1, 0, 3, 2, 1, 3, 2, 2, 1, 3, 2, 1, 2, 1, 2, 2, 0, 2, 0, 3, 0, 2, 0, 3, 2, 0, 2, 0, 2, 3, 2, 2, 0, 2, 0, 2, 0, 2, 1, 2, 2, 2, 0, 0, 0, 1, 2, 0, 2, 3, 2, 2, 1, 2, 2, 2, 1, 2, 2, 0, 1, 2, 2, 2, 2, 1, 2, 2, 2, 2, 1, 2, 2, 2, 0, 2, 0, 2, 2, 0, 2, 2, 0, 2, 2, 0, 0, 2, 2, 0, 0, 1, 0, 0, 1, 3, 0, 2, 2, 2, 0, 0, 3, 2, 0, 0, 3, 0, 2, 2, 1, 3, 2, 2, 2, 0, 1, 2, 2, 0, 1, 0, 2, 2, 2, 0, 2, 3, 2, 2, 0, 2, 0, 2, 2, 1, 2, 2, 2, 0, 0, 0, 0, 0, 2, 2, 2, 0, 2, 2, 2, 2, 0, 1};
        // Add test here

        float t_presence = -1;
        float t_small_movement = -1;
        float t_large_movement = -1;
        float f_presence = -1;
        float f_small_movement = -1;
        float f_large_movement = -1;
        get_best_thresholds(&stis, &ids, 941, &t_presence, &t_small_movement, &t_large_movement, &f_presence, &f_small_movement, &f_large_movement);
        ESP_LOGI("t", "got thresholds %f, %f, %f", t_presence, t_small_movement, t_large_movement);

        TEST_ASSERT_EQUAL_FLOAT(0.55, t_presence);
        TEST_ASSERT_EQUAL_FLOAT(1.1, t_small_movement);
        TEST_ASSERT_EQUAL_FLOAT(-1.0, t_large_movement);   
}

TEST_CASE("ts no activity 2", "[ts]")
{
        float stis[] = {0.78203, 0.478552, 0.890634, 0.752816, 0.60943, 0.736772, 0.567064, 0.971226, 0.601269, 0.735821, 0.726692, 0.663525, 0.766903, 0.579219, 0.654378, 0.780377, 0.621951, 0.690488, 0.6192, 0.566595, 0.599647, 0.616156, 0.67388, 0.567778, 0.679259, 0.595133, 0.699273, 0.650205, 0.81398, 0.661695, 0.809973, 0.667033, 0.779133, 0.737331, 0.803084, 0.590683, 0.637533, 0.638367, 3.080096, 0.70545, 0.580526, 0.600003, 0.600928, 10.409593, 0.772234, 0.893149, 0.425395, 0.728504, 0.564519, 0.754827, 0.612666, 0.618296, 0.785721, 0.549644, 0.660132, 0.591019, 0.680261, 0.77163, 0.612367, 0.562818, 0.574717, 0.685883, 0.557205, 1.021341, 0.748243, 4.250227, 0.540277, 0.568523, 0.615697, 0.663693, 0.637883, 2.106297, 0.903467, 0.74817, 0.710519, 0.602875, 0.588386, 0.727504, 0.665934, 0.692684, 0.671264, 0.648447, 9.310403, 0.647866, 0.63184, 0.590924, 0.643832, 0.581081, 1.177641, 0.531442, 0.625138, 0.632594, 0.577254, 0.599557, 0.72974, 0.596841, 0.51237, 0.60702, 0.57314, 0.76097, 0.677891, 0.699527, 0.730105, 0.588513, 1.781355, 0.502109, 0.574315, 0.522454, 0.598175, 0.726322, 0.671174, 0.648927, 0.725634, 0.689177, 0.697066, 0.679471, 0.604471, 0.482196, 0.645602, 0.716462, 0.708273, 0.785721, 0.642216, 0.679113, 0.503572, 0.777798, 0.596516, 2.271157, 0.703731, 0.692818, 0.758406, 0.798576, 0.568623, 0.569527, 0.908624, 0.711788, 0.677686, 0.713533, 0.595836, 0.586238, 0.563078, 0.866928, 0.572841, 1.02364, 0.628173, 0.738696, 0.624812, 0.61616, 0.683346, 0.677799, 0.657299, 0.671837, 0.686283, 0.726826, 4.146165, 0.662538, 0.723272, 0.629674, 0.611358, 0.632731, 0.587359, 0.632889, 0.653403, 0.592589, 0.670683, 0.864696, 3.822128, 6.038329, 0.666868, 0.584451, 0.710239, 0.763422, 0.710648, 0.659137, 0.851664, 0.570805, 0.593854, 0.701736, 0.643732, 0.652931, 0.646699, 0.632745, 0.585142, 0.625363, 0.781017, 0.57627, 1.035196, 0.692778, 0.679269, 0.784837, 0.635802, 0.841053, 0.737336, 0.527082, 0.544537, 0.676589, 0.664429, 0.468473, 0.673654, 0.656537, 0.785437, 0.661853, 0.801963, 0.640978, 0.660009, 0.723954, 0.905373, 0.760716, 0.50062, 0.749309, 0.533574, 0.587237, 0.692981, 0.783823, 0.69524, 0.666492, 0.755451, 0.740937, 0.754662, 0.769725, 0.506823, 0.629801, 0.468535, 0.656809, 0.832152, 0.566043, 0.758763, 0.655815, 0.733358, 0.874085, 0.574797, 0.782371, 0.846327, 0.702392, 0.666886, 0.628329, 0.548297, 0.678878, 0.943793, 0.628757, 0.632906, 0.504008, 0.814948, 0.57295, 0.915629, 0.776332, 0.647781, 0.72444, 0.558182, 0.716012, 0.581751, 0.714744, 0.982771, 3.943959, 0.90293, 0.875057, 0.724863, 0.642214, 0.663677, 0.592856, 0.871102, 0.679372, 0.529358, 0.582252, 0.608266, 0.838612, 0.568386, 0.633687, 0.518882, 0.86245, 0.607286, 0.617119, 0.560149, 0.668619, 0.940395, 0.555381, 0.526181, 0.659033, 0.756864, 0.539335, 0.757502, 0.752418, 0.668838, 0.511254, 0.733207, 0.535766, 0.703682, 0.68735, 0.684114, 0.709422, 0.630944, 0.638936, 0.70375, 0.620468, 0.564782, 0.587415, 0.69357, 0.593874, 0.631412, 0.820199, 0.656849, 0.596082, 0.669678, 0.514494, 0.703207, 0.579508, 0.929751, 0.683138, 0.61738, 0.765181, 0.687837, 0.653826, 0.531586, 0.723798, 0.551711, 0.832168, 0.931998, 0.574717, 0.594821, 0.573657, 0.68896, 0.754158, 0.599982, 6.415303, 0.652494, 0.576018, 0.819643, 0.68633, 0.68671, 0.71173, 0.632952, 0.51381, 0.522454, 0.766493, 0.749782, 0.685593, 0.60656, 0.557599, 0.853914, 0.548822, 0.648599, 1.882545, 0.727717, 1.131013, 0.640744, 0.725752, 0.7996, 0.761592, 0.562973, 0.588963, 0.669834, 0.416246, 0.629093, 0.528521, 0.819823, 0.740117, 0.658407, 0.620923, 0.636663, 0.558611, 0.704825, 0.568112, 0.691647, 0.632506, 0.653389, 0.560151, 4.585455, 0.659601, 0.590226, 0.679372, 0.714394, 0.562743, 0.572317, 0.722903, 0.547413, 0.71283, 0.487665, 0.672624, 0.574219, 0.661374, 0.578135, 0.772017, 0.70047, 0.620546, 0.696272, 0.623735, 0.624638, 0.586381, 0.649645, 0.653603, 0.671593, 0.669678, 0.567254, 0.551795, 0.495109, 0.608756, 0.586267, 4.520786, 0.795822, 0.696199, 0.649275, 0.618219, 1.912041, 0.777342, 0.584407, 0.743749, 0.630752, 0.710524, 0.667378, 0.70026, 0.626879, 0.616537, 1.261004, 0.656919, 0.568489, 0.666239, 0.743431, 0.649858, 0.551417, 0.788525, 0.571214, 0.665357, 0.628731, 0.621223, 0.667778, 0.886604, 0.692344, 0.803097, 0.673672, 0.714473, 0.552962, 0.636465, 0.658373, 0.547681, 0.4907, 0.60664, 0.694939, 0.681755, 7.633355, 0.663637, 9.677143, 0.502223, 0.559007, 0.596395, 0.832759, 0.66077, 0.600776, 0.619381, 0.671073, 0.523179, 0.756094, 0.536204, 0.704465, 0.571984, 0.659962, 0.737419, 0.522983, 0.589669, 0.651039, 4.641998, 0.753345, 0.531148, 0.693041, 0.69393, 1.027106, 0.612985, 0.557133, 0.616869, 0.677997, 0.640671, 0.746623, 0.611714, 0.60587, 0.58788, 0.741868, 0.556951, 0.610632, 0.585151, 0.636226, 0.568907, 3.29234, 0.595957, 0.646931, 0.637337, 1.213224, 0.636736, 0.658295, 0.661277, 0.678988, 0.623482, 3.14255, 0.605871, 0.714089, 0.548057, 0.757107, 0.685478, 1.551735, 0.715417, 0.588683, 1.095314, 4.065212, 0.603165, 0.61674, 0.583242, 0.608576, 0.653027, 0.660968, 0.64408, 0.774294, 0.748052, 0.656351, 0.525355, 0.574797, 1.025631, 0.674655, 0.789158, 0.606098, 0.672145, 0.763174, 0.679372, 0.658793, 0.566805, 0.728005, 0.59593, 0.68896, 1.131087, 0.566383, 0.718056, 0.528626, 0.720828, 0.511596, 0.663926, 0.812867};
        char ids[] = {4, 1, 0, 0, 1, 0, 0, 0, 1, 4, 1, 0, 0, 0, 0, 0, 1, 0, 0, 4, 0, 0, 0, 3, 0, 1, 0, 4, 0, 3, 4, 0, 0, 4, 0, 1, 4, 0, 1, 0, 1, 4, 0, 0, 3, 4, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 4, 1, 1, 1, 0, 4, 1, 1, 1, 0, 1, 1, 0, 3, 1, 0, 3, 3, 0, 4, 3, 4, 0, 0, 1, 0, 0, 3, 1, 0, 0, 1, 3, 0, 1, 0, 0, 0, 1, 0, 0, 3, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 3, 0, 4, 1, 0, 1, 0, 0, 0, 1, 0, 4, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 3, 4, 0, 4, 1, 0, 4, 0, 0, 4, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 3, 0, 0, 3, 3, 1, 0, 0, 0, 1, 0, 4, 0, 0, 3, 1, 0, 3, 1, 1, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 4, 0, 0, 0, 4, 0, 1, 0, 0, 1, 0, 4, 0, 0, 0, 1, 4, 1, 0, 1, 1, 0, 0, 0, 4, 1, 0, 1, 0, 3, 1, 0, 0, 3, 0, 0, 0, 0, 4, 0, 0, 1, 0, 1, 4, 0, 0, 4, 0, 0, 4, 4, 4, 3, 0, 3, 0, 1, 4, 1, 3, 3, 1, 4, 0, 3, 0, 0, 0, 0, 0, 4, 0, 3, 1, 0, 4, 1, 0, 0, 4, 0, 1, 1, 0, 4, 0, 0, 0, 1, 0, 0, 0, 4, 3, 3, 0, 1, 1, 3, 1, 0, 0, 1, 1, 0, 0, 0, 4, 1, 0, 0, 0, 0, 4, 4, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 4, 0, 1, 0, 1, 0, 4, 1, 4, 0, 0, 0, 0, 1, 4, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 3, 3, 0, 1, 3, 4, 3, 3, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 3, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 3, 1, 0, 3, 4, 1, 3, 3, 1, 4, 1, 1, 1, 1, 3, 3, 1, 3, 0, 4, 1, 3, 4, 0, 0, 4, 0, 1, 1, 1, 0, 4, 4, 3, 0, 0, 0, 0, 1, 0, 4, 4, 0, 0, 0, 1, 1, 3, 0, 0, 0, 0, 0, 0, 1, 1, 1, 3, 1, 0, 0, 1, 3, 0, 1, 1, 3, 0, 1, 0, 3, 3, 1, 1, 4, 0, 3, 0, 0, 1, 0, 1, 0, 0, 1, 3, 0, 1, 1, 0, 0, 1, 1, 0, 4, 3, 1, 0, 1, 4, 0, 3, 0, 0, 4, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 3, 0, 0, 4, 0, 0, 0, 1, 0, 0, 0, 0, 0, 4, 0, 1, 0, 3, 1, 0, 4, 1, 3, 0, 0, 4, 0, 4, 0, 0, 4, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 3, 0, 0, 1, 0, 4};
        // Add test here

        float t_presence = -1;
        float t_small_movement = -1;
        float t_large_movement = -1;
        float f_presence = -1;
        float f_small_movement = -1;
        float f_large_movement = -1;
        get_best_thresholds(&stis, &ids, 533, &t_presence, &t_small_movement, &t_large_movement, &f_presence, &f_small_movement, &f_large_movement);
        ESP_LOGI("t", "got thresholds %f, %f, %f", t_presence, t_small_movement, t_large_movement);

        TEST_ASSERT_EQUAL_FLOAT(0.45, t_presence);
        TEST_ASSERT_EQUAL_FLOAT(0.45, t_small_movement);
        TEST_ASSERT_EQUAL_FLOAT(0.65, t_large_movement);   
}

TEST_CASE("delayed hampel filter", "[hampel]")
{
        float values[] = {7.0, 6.0, 3.5, 8.5, 4.0, 6.5, 8.0, 5.0, 4.7, 3.8, 5.5, 35.4, 8.0, 4.9, 6.3, 7.2};
        float result[] = {7.0, 6.0, 3.5, 8.5, 4.0, 6.5, 8.5, 4.0, 6.5, 8.0, 5.0, 4.7, 3.8, 5.5, 5.5, 8.0};

        HampelFilter h;
        initialize_hampel_filter(&h, 3);
        for(int i=0; i<16; i++){
                TEST_ASSERT_EQUAL_FLOAT(result[i], delayed_hampel_filter(&h, values[i]));
        } 
}