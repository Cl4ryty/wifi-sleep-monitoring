#include "unity.h"
#include "utilities.h"


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


TEST_CASE("test calculate_variance_features", "[variance]")
{
        Features features;
        features_initialize(&features);

        POI_List pois;
        poi_list_initialize(&pois, 20);

        POI poi;
        poi_initialize(&poi, true, 1000000, 1, 0);
        poi_list_append(&pois, &poi);
        for(int i=1; i<9; i++){
                POI poi;
                if(i%2==0){
                        poi_initialize(&poi, true, 1000000, 1, i);
                }else{
                        poi_initialize(&poi, false, 1000000, -1, i);
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

        poi_initialize(&poi, false, 1000000, -1, 0);
        poi_list_append(&pois, &poi);
        for(int i=1; i<9; i++){
                POI poi;
                if(i%2!=0){
                        poi_initialize(&poi, false, 1000000, 1, i);
                }else{
                        poi_initialize(&poi, true, 1000000, -1, i);
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

TEST_CASE("test calculate_variance_features 2", "[variance]")
{
        Features features;
        features_initialize(&features);

        POI_List pois;
        poi_list_initialize(&pois, 20);

        POI poi;
        poi_initialize(&poi, true, 1000000, 1, 0);
        poi_list_append(&pois, &poi);
        for(int i=1; i<7; i++){
                POI poi;
                if(i%2==0){
                        poi_initialize(&poi, true, 1000000, 1, i);
                }else{
                        poi_initialize(&poi, false, 1000000, -1, i);
                }
                poi_list_append(&pois, &poi);
        }
        poi_initialize(&poi, false, 1000000, -11, 7);
        poi_list_append(&pois, &poi);
        poi_initialize(&poi, true, 1000000, 11, 8);
        poi_list_append(&pois, &poi);
        poi_initialize(&poi, false, 1000000, -1, 9);
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
TEST_CASE("test calculate_variance_features 3", "[variance]")
{
        Features features;
        features_initialize(&features);

        POI_List pois;
        poi_list_initialize(&pois, 20);

        POI poi;
        poi_initialize(&poi, false, 1000000, 1, 0);
        poi_list_append(&pois, &poi);
        for(int i=1; i<7; i++){
                POI poi;
                if(i%2==0){
                        poi_initialize(&poi, false, 1000000, 1, i);
                }else{
                        poi_initialize(&poi, true, 1000000, -1, i);
                }
                poi_list_append(&pois, &poi);
        }
        poi_initialize(&poi, true, 1000000, -11, 7);
        poi_list_append(&pois, &poi);
        poi_initialize(&poi, false, 1000000, 11, 8);
        poi_list_append(&pois, &poi);
        poi_initialize(&poi, true, 1000000, -1, 9);
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


#define SECONDS_TO_MICROSECONDS 1000000


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
        poi_initialize(&poi, 1, 31080133, 9.693646, 1125);
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
        poi_initialize(&poi1, 0, 10744651, 10.558222, 36);
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
        poi_initialize(&poi2, 1, 5228664, 1.002261, 236);
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
        poi_initialize(&poi3, 0, 1766116, 0.261949, 304);
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
        poi_initialize(&poi4, 1, 120532267, 2.935997, 368);
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


        // features.mean_peak_rate_over_window.current_mean = 1.0;
        // features.mean_valley_rate_over_window.current_mean = 1.0;
        // features.mean_up_stroke_length.current_mean = 1.0;
        // features.mean_down_stroke_length.current_mean = 1.0;
        // features.mean_up_stroke_amplitude.current_mean = -1.0;
        // features.mean_down_stroke_amplitude.current_mean = 1.0;

        // calculate_variance_features(&features, &pois);
        // TEST_ASSERT_EQUAL_FLOAT(0.0, features.variance_of_peak_rate_in_window);
        // TEST_ASSERT_EQUAL_FLOAT(0.0, features.variance_of_valley_rate_in_window);
        // TEST_ASSERT_EQUAL_FLOAT(0.0, features.up_stroke_length_variance);
        // TEST_ASSERT_EQUAL_FLOAT(0.0, features.down_stroke_length_variance);
        // TEST_ASSERT_EQUAL_FLOAT(20.0, features.up_stroke_amplitude_variance);
        // TEST_ASSERT_EQUAL_FLOAT(20.0, features.down_stroke_amplitude_variance);
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

// TEST_CASE("test dumb running mean", "[mean]")
// {
//         // Add test here
// }