#pragma once

#include <vector>
#include "constant_jerk_trajectory1d.hpp"
#include "curve1d.hpp"
#include <algorithm>

class PiecewiseJerkTrajectory1d : public Curve1d {
private:
  std::vector<ConstantJerkTrajectory1d> segments_;
  double last_p_;
  double last_v_;
  double last_a_;
  std::vector<double> param_;

 public:
  PiecewiseJerkTrajectory1d(const double p, const double v, const double a){
    last_p_ = p;
    last_v_ = v;
    last_a_ = a;
    param_.push_back(0.0);
  }

  virtual ~PiecewiseJerkTrajectory1d() = default;

  double Evaluate(const std::uint32_t order, const double param) const{
    auto it_lower = std::lower_bound(param_.begin(), param_.end(), param);
    if (it_lower == param_.begin()) {
        return segments_[0].Evaluate(order, param);
    }
    if (it_lower == param_.end()) {
        auto index = std::max(0, static_cast<int>(param_.size() - 2));
        return segments_.back().Evaluate(order, param - param_[index]);
    }
    auto index = std::distance(param_.begin(), it_lower);
    return segments_[index - 1].Evaluate(order, param - param_[index - 1]);
  }

  double ParamLength() const{
    return param_.back();
  }

  void AppendSegment(const double jerk, const double param_s){
    param_.push_back(param_.back() + param_s);
    segments_.emplace_back(last_p_, last_v_, last_a_, jerk, param_s);
    last_p_ = segments_.back().end_position();
    last_v_ = segments_.back().end_velocity();
    last_a_ = segments_.back().end_acceleration();
  }
};