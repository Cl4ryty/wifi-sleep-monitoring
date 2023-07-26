#define EIGEN_STACK_ALLOCATION_LIMIT 20000
#define NDEBUG 1

#include "using_eigen.h"
#include <complex> 
#include <iostream>
#include <stdint.h>
#include <eigen3/Eigen/Eigen>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


#define C_SPEED 299792458

int get_next_index(int current_index, int array_length){
    int next_index = current_index + 1;
    if(next_index >= array_length){
        next_index = 0;
    }
    return next_index;
}

// definition in a .cpp file:
extern "C" float is_first_pc_positive(float *data, int data_length) {
    Eigen::Map<Eigen::MatrixXf> mf(data,data_length,1);
    Eigen::MatrixXf centered = mf.rowwise() - mf.colwise().mean();
    Eigen::MatrixXf covariance = Eigen::MatrixXf::Zero(centered.cols(), centered.cols());
    covariance.selfadjointView<Eigen::Lower>().rankUpdate(covariance.adjoint());
    Eigen::SelfAdjointEigenSolver<Eigen::MatrixXf> eigensolver(covariance);
    double value = eigensolver.eigenvectors().coeff(0,0);
    // printf("value: %f", value);
    if (value >= 0){
        return 1.0;
    }
    else{
        return -1.0;
    }

    
}

extern "C" void doppler_music_speed_variance(int8_t *csi_samples, int first_sample_index, int last_sample_index, int M, int N, int L, float start_speed, float end_speed, float speed_step_size, float f, float delta_time, float *speed_spectrum_out) {
    // put the samples in a matrix for further processing
    // Eigen::MatrixXcf mf(M,N);
    ESP_LOGW("doppler_music", "stack watermark 1 %d", uxTaskGetStackHighWaterMark(NULL));
    Eigen::MatrixXcf mf = Eigen::MatrixXcf::Zero(M, N);
    

    ESP_LOGI("doppler_music", "startn M %d, N %d, L %d, first_sample_index %d, last_sample_index %d", M, N, L, first_sample_index, last_sample_index);

    for(int i=first_sample_index; i!=last_sample_index; i=get_next_index(i, M)){
        for(int j=0; j<N/2; j++){
            std::complex<float> c((float)csi_samples[M*i+2*j], (float)csi_samples[M*i+2*j+1]);
            mf(i, j) = c;
        }
    }
    ESP_LOGI("doppler_music", "created matrix");
    
    // calculate the MxM correlation matrix for the data, which are M consecutive CSI samples

    // first calculate the covariance matrix, from which the correlation matrix can then be computed
    ESP_LOGW("doppler_music", "stack watermark 2 %d", uxTaskGetStackHighWaterMark(NULL));
    Eigen::MatrixXcf centered = mf.rowwise() - mf.colwise().mean();
    ESP_LOGW("doppler_music", "stack watermark 3 %d", uxTaskGetStackHighWaterMark(NULL));
    Eigen::MatrixXcf covariance = (centered.adjoint() * centered) / double(mf.rows() - 1);
    ESP_LOGW("doppler_music", "stack watermark 4 %d", uxTaskGetStackHighWaterMark(NULL));
    // Eigen::SelfAdjointEigenSolver<Eigen::MatrixXf> eigensolver(covariance);
    // double value = eigensolver.eigenvectors().coeff(0,0);

    // Eigen::MatrixXcf centered = mf.rowwise() - mf.colwise().mean();
    // ESP_LOGI("doppler_music", "0");
    // Eigen::MatrixXcf covariance = (centered.adjoint() * centered) / double(mf.rows() - 1);
    // ESP_LOGI("doppler_music", "1");
    // //covariance = covariance * centered;
    // covariance = Eigen::MatrixXcf::Zero(centered.cols(), centered.cols());
    //covariance.selfadjointView<Eigen::Lower>().rankUpdate(covariance.adjoint());

    ESP_LOGI("doppler_music", "2");

    // TODO
    Eigen::MatrixXcf correlation = Eigen::MatrixXcf::Zero(M, M);
    for(int i = 0; i<M; i++){
        for(int j=i; j<M; j++){
            correlation(i,j) = covariance(i,j) / (std::sqrt(covariance(i,i)) * std::sqrt(covariance(j,j)));
        }
    }
    ESP_LOGI("doppler_music", "3");

    ESP_LOGW("doppler_music", "stack watermark 5 %d", uxTaskGetStackHighWaterMark(NULL));

    // perform eigenvalue decomposition of the correlation matrix -> only uses the lower triangle, so it suffices to fill only that part
    Eigen::SelfAdjointEigenSolver<Eigen::MatrixXcf> eigensolver(correlation);
    ESP_LOGI("doppler_music", "4");
    // get the noise subspace, which is spanned by the M-L eigenvectors corresponding to the smallest eigenvalues
    // eigenvalues and corresponding -vectors are returned in increasing order, so we only want the M-L first ones for the noise subspace
    Eigen::MatrixXcf En(M-L, M);
    En = eigensolver.eigenvectors().leftCols(M-L);

    

    ESP_LOGW("doppler_music", "stack watermark 6 %d", uxTaskGetStackHighWaterMark(NULL));

    // int i = 0;
    // // calculate P(v) for all vs to get the speed spectrum for the current window
    // for(float v=start_speed; v<=end_speed; v+=speed_step_size){
    //     std::complex<float> e(cos(2*M_PI*f*(v*delta_time/C_SPEED)), sin(2*M_PI*f*(v*delta_time/C_SPEED))); // conjugate (transpose) so no minus in front of the imaginary part
    //     speed_spectrum_out[i] = ( (En*e).colwise().norm() * (En*e).colwise().norm() ).sum();
    //     i++;
    // }
    ESP_LOGI("doppler_music", "5");

    
}