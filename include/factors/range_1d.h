#include <ceres/ceres.h>

class Range1dFactor : public ceres::SizedCostFunction<1,1,1>
{
public:
    Range1dFactor(double z, double var) :
        range_(z),
        var_(var)
    {}

    virtual bool Evaluate(const double * const *parameters, double *residuals, double **jacobians) const
    {
        double l = parameters[0][0];
        double x = parameters[1][0];
        residuals[0] = (range_ - (l - x)) / var_;

        if (jacobians)
        {
            if (jacobians[0])
            {
                jacobians[0][0] = -1/var_;
            }
            if (jacobians[1])
            {
                jacobians[1][0] = 1/var_;
            }
        }
        return true;
    }

private:
    double range_;
    double var_;

};