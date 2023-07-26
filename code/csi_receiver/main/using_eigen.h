#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

float is_first_pc_positive(float *data, int data_length);
void doppler_music_speed_variance(int8_t *csi_samples, int first_sample_index, int last_sample_index, int M, int N, int L, float start_speed, float end_speed, float speed_step_size, float f, float delta_time, float *speed_spectrum_out);

#ifdef __cplusplus
}
#endif

