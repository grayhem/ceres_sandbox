#pragma once

#include <ceres/ceres.h>
#include "lie/quat.h"

using namespace Eigen;

class QuatParameterization : public ceres::LocalParameterization
{
public:
    ~QuatParameterization() {}
    bool Plus(const double* x,
                      const double* delta,
                      double* x_plus_delta) const
    {
        double d_exp[4];
        exp(delta, d_exp);
        otimes(x, d_exp, x_plus_delta);
        return true;
    }

    void exp(const double* v, double* out) const
    {
        double norm_v = sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
        if (norm_v > 1e-4)
        {
            double v_scale = sin(norm_v/2.0)/norm_v;
            out[0] = cos(norm_v/2.0);
            out[1] = v_scale*v[0];
            out[2] = v_scale*v[1];
            out[3] = v_scale*v[2];
        }
        else
        {
            out[0] = 1.0;
            out[1] = v[0]/2.0;
            out[2] = v[1]/2.0;
            out[3] = v[2]/2.0;

            double out_norm = sqrt(out[0]*out[0] + out[1]*out[1] + out[2]*out[2] + out[3]*out[3]);
            out[0] /= out_norm;
            out[1] /= out_norm;
            out[2] /= out_norm;
            out[3] /= out_norm;
        }
    }

    void otimes(const double* q1, const double*q2, double * out) const
    {
        double q1w = q1[0];
        double q1x = q1[1];
        double q1y = q1[2];
        double q1z = q1[3];

        double q2w = q2[0];
        double q2x = q2[1];
        double q2y = q2[2];
        double q2z = q2[3];

        out[0] = q1w*q2w - q1x*q2x - q1y*q2y - q1z*q2z;
        out[1] = q1w*q2x + q1x*q2w + q1y*q2z - q1z*q2y;
        out[2] = q1w*q2y - q1x*q2z + q1y*q2w + q1z*q2x;
        out[3] = q1w*q2z + q1x*q2y - q1y*q2x + q1z*q2w;
    }

    bool ComputeJacobian(const double* x, double* jacobian) const
    {
        jacobian[0] = -x[1]/2.0; jacobian[1]  = -x[2]/2.0; jacobian[2]  = -x[3]/2.0;
        jacobian[3] =  x[0]/2.0; jacobian[4]  = -x[3]/2.0; jacobian[5]  =  x[2]/2.0;
        jacobian[6] =  x[3]/2.0; jacobian[7]  =  x[0]/2.0; jacobian[8]  = -x[1]/2.0;
        jacobian[9] = -x[2]/2.0; jacobian[10] =  x[1]/2.0; jacobian[11] =  x[0]/2.0;
    }
    int GlobalSize() const {return 4;}
    int LocalSize() const {return 3;}
};

class QuatFactor : public ceres::SizedCostFunction<3,4>
{
public:
    QuatFactor(double *x)
    {
        quat_.arr_ = Map<Vector4d>(x);
    }
    virtual bool Evaluate(double const* const* parameters, double* residuals, double** jacobians) const
    {
        Eigen::Map<Eigen::Vector3d> res(residuals);
        quat::Quat z(parameters[0]);
        res = quat_ - z;

        if (jacobians)
        {
            if (jacobians[0])
            {
                double qw = quat_.w();
                double qx = quat_.x();
                double qy = quat_.y();
                double qz = quat_.z();
                double sqw = sign(qw);
                double sqx = sign(qx);
                double sqy = sign(qy);
                double sqz = sign(qz);
                jacobians[0][0] =  0.0; //2.0 * sqw*qx;
                jacobians[0][1] = -2.0 * sqx*qw;
                jacobians[0][2] = -2.0 * sqy*qz;
                jacobians[0][3] =  2.0 * sqz*qy;

                jacobians[0][4] =  0.0;//2.0 * sqw*qy;
                jacobians[0][5] =  2.0 * sqx*qz;
                jacobians[0][6] = -2.0 * sqy*qw;
                jacobians[0][7] = -2.0 * sqz*qx;

                jacobians[0][8]  =  0.0;//2.0 * sqw*qz;
                jacobians[0][9]  = -2.0 * sqx*qy;
                jacobians[0][10] =  2.0 * sqy*qx;
                jacobians[0][11] = -2.0 * sqz*qw;
            }
        }
        return true;
    }

private:
    quat::Quat quat_;
    double sign(const double x) const
    {
        return (x >= 0) - (x < 0);
    }
};