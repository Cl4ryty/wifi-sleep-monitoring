#include "using_eigen.h"
#include <eigen3/Eigen/Eigen>


// definition in a .cpp file:
extern "C" float is_first_pc_positive(float *data, int data_length) {
    Eigen::Map<Eigen::MatrixXf> mf(data,data_length,1);
    Eigen::MatrixXf centered = mf.rowwise() - mf.colwise().mean();
    Eigen::MatrixXf covariance = Eigen::MatrixXf::Zero(centered.cols(), centered.cols());
    covariance.selfadjointView<Eigen::Lower>().rankUpdate(covariance.adjoint());
    Eigen::SelfAdjointEigenSolver<Eigen::MatrixXf> eigensolver(covariance);
    double value = eigensolver.eigenvectors().coeff(0,0);
    if (value >= 0){
        return 1.0;
    }
    else{
        return -1.0;
    }
}