#pragma once
#include "curve1d.hpp"
class ConstantJerkTrajectory1d : public Curve1d {
 private:
  double p0_;
  double v0_;
  double a0_;

  double p1_;
  double v1_;
  double a1_;

  double param_;
  double jerk_;
 public:
  ConstantJerkTrajectory1d(const double p0, const double v0, const double a0,
                           const double j, const double param): p0_(p0), v0_(v0), a0_(a0), param_(param), jerk_(j) {
  p1_ = Evaluate(0, param_);
  v1_ = Evaluate(1, param_);
  a1_ = Evaluate(2, param_);
}

  virtual ~ConstantJerkTrajectory1d() = default;

  double Evaluate(const std::uint32_t order, const double param) const{
    switch (order) {
        case 0: {
        return p0_ + v0_ * param + 0.5 * a0_ * param * param +
                jerk_ * param * param * param / 6.0;
        }
        case 1: {
        return v0_ + a0_ * param + 0.5 * jerk_ * param * param;
        }
        case 2: {
        return a0_ + jerk_ * param;
        }
        case 3: {
        return jerk_;
        }
        default:
        return 0.0;
    }
  }

  double ParamLength() const{ return param_; }

  double start_position() const { return p0_; }

  double start_velocity() const { return v0_; }

  double start_acceleration() const { return a0_; }

  double end_position() const { return p1_; }

  double end_velocity() const { return v1_; }

  double end_acceleration() const { return a1_; }

  double jerk() const  { return jerk_; }
};