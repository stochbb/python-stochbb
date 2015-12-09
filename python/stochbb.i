%module stochbb

%{
//#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include "lib/api.hh"
#include "lib/randomvariable.hh"
#include "lib/density.hh"
#define SWIG_FILE_WITH_INIT
%}

%include "numpy.i"

%init %{
import_array();
%}

namespace sbb {

class Container
{
protected:
  Container();

  bool isNull() const;
};


class Density: public Container
{
protected:
  Density();
};

%numpy_typemaps(double, NPY_DOUBLE, int)
%apply (double* INPLACE_ARRAY1, int DIM1) {(double* out, int len)};
%extend Density {
  void eval(double Tmin, double Tmax, double* out, int len) {
    Eigen::Map<Eigen::VectorXd> outMap(out, len);
    self->eval(Tmin, Tmax, outMap);
  }

  void evalCDF(double Tmin, double Tmax, double* out, int len) {
    Eigen::Map<Eigen::VectorXd> outMap(out, len);
    self->evalCDF(Tmin, Tmax, outMap);
  }
}


class Var: public Container
{
protected:
  Var();

public:
  Density density() const;
  bool dependsOn(const Var &var);
  bool mutuallyIndep(const Var &var);
};


class DerivedVar: public Var
{
protected:
  DerivedVar();

public:
  size_t numVariables() const;
  Var variable(size_t i) const;
};


class ExactSampler: public Container
{
public:
  ExactSampler(const Var &X);
  ExactSampler(const Var &X1, const Var &X2);
  ExactSampler(const Var &X1, const Var &X2, const Var &X3);
  ExactSampler(const std::vector<Var> &variables);

  void sample(Eigen::MatrixXd &out) const;
};

%numpy_typemaps(double , NPY_DOUBLE, int)
%apply (double* INPLACE_FARRAY2, int DIM1, int DIM2) {(double* out, int rows, int cols)};
%extend ExactSampler {
  void sample(double *out, int rows, int cols) {
    Eigen::Map<Eigen::MatrixXd> outMap(out, rows, cols);
    self->sample(outMap);
  }
}


class MarginalSampler: public Container
{
public:
  MarginalSampler(const Var &var, double Tmin, double Tmax, size_t steps);
};

%numpy_typemaps(double , NPY_DOUBLE, int)
%apply (double* INPLACE_ARRAY1, int DIM1) {(double* out, int len)};
%extend MarginalSampler {
  void sample(double *out, int len) {
    Eigen::Map<Eigen::VectorXd> outMap(out, len);
    self->sample(outMap);
  }
}

Var delta(double value);

Var uniform(double a, double b);

Var normal(double mu, double sigma);
Var normal(double mu, const Var &sigma);
Var normal(const Var &mu, double sigma);
Var normal(const Var &mu, const Var &sigma);

Var gamma(double k, double theta);
Var gamma(double k, const Var &theta);
Var gamma(const Var &k, double theta);
Var gamma(const Var &k, const Var &theta);

Var affine(const Var &var, double scale, double shift);

Var chain(const std::vector<Var> &vars);

Var minimum(const std::vector<Var> &variables);

Var maximum(const std::vector<Var> &variables);

%extend Container {
  bool isVar() const { return self->is<sbb::Var>(); }
  sbb::Var asVar() { return self->as<sbb::Var>(); }
  bool isDerivedVar() const { return self->is<sbb::DerivedVar>(); }
  sbb::DerivedVar asDerivedVar() { return self->as<sbb::DerivedVar>(); }
  bool isDensity() const { return self->is<sbb::Density>(); }
  sbb::Density asDensity() { return self->as<sbb::Density>(); }
}
}
