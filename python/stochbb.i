%module stochbb

%{
#define SWIG_FILE_WITH_INIT
//#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <stochbb/stochbb.hh>
%}

%include "numpy.i"
%include "std_vector.i"
%include "std_string.i"
%include "typemaps.i"


%init %{
import_array();
%}


%apply (double* INPLACE_ARRAY1, int DIM1) {(double* out, int N)}
%apply (double* INPLACE_FARRAY2, int DIM1, int DIM2) {(double* out, int Nrow, int Ncol)}
%apply double *OUTPUT { double *aout, double *bout }


namespace stochbb {

/*
 * Exceptions.
 */
class Error { };
%extend Error {
  std::string __str__() {
    return self->str();
  }
}

class TypeError: public Error { };
class AssumptionError: public Error { };


class Container
{
protected:
  %feature("autodoc", "1");
  Container();

  %feature("autodoc", "1");
  bool isNull() const;
};

%extend Container {
  std::string __str__() {
    std::stringstream buffer;
    buffer << *self;
    return buffer.str();
  }
}


class Density: public Container
{
protected:
  %feature("autodoc", "1");
  Density();
};


%extend Density {
  %feature("autodoc", "1");
  void eval(double Tmin, double Tmax, double* out, int N) const {
    Eigen::Map<Eigen::VectorXd> outMap(out, N);
    $self->eval(Tmin, Tmax, outMap);
  }

  %feature("autodoc", "1");
  void evalCDF(double Tmin, double Tmax, double* out, int N) const {
    Eigen::Map<Eigen::VectorXd> outMap(out, N);
    $self->evalCDF(Tmin, Tmax, outMap);
  }

  %feature("autodoc", "1");
  void rangeEst(double alpha, double *aout, double *bout) const {
    $self->rangeEst(alpha, *aout, *bout);
  }
}


class Var: public Container
{
protected:
  Var();

public:
  %feature("autodoc", "1");
  Density density() const;
  %feature("autodoc", "1");
  bool dependsOn(const Var &var);
  %feature("autodoc", "1");
  bool mutuallyIndep(const Var &var);
};

%extend Var {
  %feature("autodoc", "1");
  Var __add__(Var *other) {
    return *self + *other;
  }
  %feature("autodoc", "1");
  Var __add__(double b) {
    return *self + b;
  }
  %feature("autodoc", "1");
  Var __radd__(double b) {
    return *self + b;
  }
  %feature("autodoc", "1");
  Var __mul__(double a) {
    return (*self)*a;
  }
  %feature("autodoc", "1");
  Var __rmul__(double a) {
    return (*self)*a;
  }
}

}

// Define vector-of-variables type:
//  This allows to pass list of variables to C++ functions taking
//  std::vector<Var> arguments.
namespace std {
  %template(varvector) vector<stochbb::Var>;
};

// Define vector-of-densities type:
//  This allows to pass list of densities to C++ functions taking
//  std::vector<Density> arguments.
namespace std {
  %template(densityvector) vector<stochbb::Density>;
};

namespace stochbb {

class DerivedVar: public Var
{
protected:
  DerivedVar();

public:
  %feature("autodoc", "1");
  size_t numVariables() const;
  %feature("autodoc", "1");
  Var variable(size_t i) const;
};


class AffineTrafo: public DerivedVar
{
protected:
  AffineTrafo();

public:
  %feature("autodoc", "1");
  double scale() const;
  %feature("autodoc", "1");
  double shift() const;
};


class Chain: public DerivedVar
{
protected:
  Chain();

public:
  %feature("autodoc", "1");
  Chain(const std::vector<Var> &vars);
};


class Maximum: public DerivedVar
{
protected:
  Maximum();
};


class Minimum: public DerivedVar
{
protected:
  Minimum();
};


class Mixture: public DerivedVar
{
protected:
  Mixture();

public:
  %feature("autodoc", "1");
  double weight(size_t i) const;
};


class Conditional: public DerivedVar
{
protected:
  Conditional();
};


class CondSum: public DerivedVar
{
protected:
  CondSum();
};


class Compound: public DerivedVar
{
protected:
  Compound();
};


class ExactSampler: public Container
{
public:
  %feature("autodoc", "1");
  ExactSampler(const Var &X) throw (stochbb::TypeError);
  %feature("autodoc", "1");
  ExactSampler(const Var &X1, const Var &X2) throw (stochbb::TypeError);
  %feature("autodoc", "1");
  ExactSampler(const Var &X1, const Var &X2, const Var &X3) throw (stochbb::TypeError);
  %feature("autodoc", "1");
  ExactSampler(const std::vector<Var> &variables) throw (stochbb::TypeError);

  %feature("autodoc", "1");
  void sample(Eigen::MatrixXd &out) const;
};

%extend ExactSampler {
  %feature("autodoc", "1");
  void sample(double* out, int Nrow, int Ncol) {
    Eigen::Map<Eigen::MatrixXd> outMap(out, Nrow, Ncol);
    self->sample(outMap);
  }
}

class MarginalSampler: public Container
{
public:
  %feature("autodoc", "1");
  MarginalSampler(const Var &var, double Tmin, double Tmax, size_t steps);
};

%extend MarginalSampler {
  %feature("autodoc", "1");
  void sample(double* out, int N) {
    Eigen::Map<Eigen::VectorXd> outMap(out, N);
    self->sample(outMap);
  }
}


%feature("autodoc", "1");
Var delta(double value);

%feature("autodoc", "1");
Var uniform(double a, double b) throw (Error);

%feature("autodoc", "1");
Var normal(double mu, double sigma) throw (Error);
%feature("autodoc", "1");
Var normal(double mu, const Var &sigma) throw (Error);
%feature("autodoc", "1");
Var normal(const Var &mu, double sigma) throw (Error);
%feature("autodoc", "1");
Var normal(const Var &mu, const Var &sigma) throw (Error);

%feature("autodoc", "1");
Var gamma(double k, double theta) throw (Error);
%feature("autodoc", "1");
Var gamma(double k, const Var &theta) throw (Error);
%feature("autodoc", "1");
Var gamma(const Var &k, double theta) throw (Error);
%feature("autodoc", "1");
Var gamma(const Var &k, const Var &theta) throw (Error);

%feature("autodoc", "1");
Var invgamma(double alpha, double beta) throw (Error);
%feature("autodoc", "1");
Var invgamma(double alpha, const Var &beta) throw (Error);
%feature("autodoc", "1");
Var invgamma(const Var &alpha, double beta) throw (Error);
%feature("autodoc", "1");
Var invgamma(const Var &alpha, const Var &beta) throw (Error);

%feature("autodoc", "1");
Var weibull(double k, double lam) throw (Error);
%feature("autodoc", "1");
Var weibull(const Var& k, double lam) throw (Error);
%feature("autodoc", "1");
Var weibull(double k, const Var& lam) throw (Error);
%feature("autodoc", "1");
Var weibull(const Var& k, const Var& lam) throw (Error);

%feature("autodoc", "1");
Var studt(double nu) throw (Error);
%feature("autodoc", "1");
Var studt(const Var& nu) throw (Error);

%feature("autodoc", "1");
Var affine(const Var &var, double scale, double shift);

%feature("autodoc", "1");
Var chain(const std::vector<Var> &vars) throw (Error);

%feature("autodoc", "1");
Var minimum(const Var &X1, const Var &X2) throw (Error);
%feature("autodoc", "1");
Var minimum(const Var &X1, const Var &X2, const Var &X3) throw( Error );
%feature("autodoc", "1");
Var minimum(const std::vector<Var> &variables) throw( Error );

%feature("autodoc", "1");
Var maximum(const Var &X1, const Var &X2) throw( Error );
%feature("autodoc", "1");
Var maximum(const Var &X1, const Var &X2, const Var &X3) throw( Error );
%feature("autodoc", "1");
Var maximum(const std::vector<Var> &variables) throw( Error );

%feature("autodoc", "1");
Var mixture(double w1, const Var &X1, double w2, const Var &X2) throw( Error );
%feature("autodoc", "1");
Var mixture(double w1, const Var &X1, double w2, const Var &X2, double w3, const Var &X3) throw( Error );
%feature("autodoc", "1");
Var mixture(const std::vector<double> weights, const std::vector<Var> &variables) throw( Error );

%feature("autodoc", "1");
Var conditional(const Var &X1, const Var &X2, const Var &Y1, const Var &Y2) throw( Error );
%feature("autodoc", "1");
Var condsum(const Var &X1, const Var &X2, const Var &Y1, const Var &Y2) throw( Error );

%feature("autodoc", "1");
Density directConvolve(const std::vector<Density> &densities);
%feature("autodoc", "1");
Density directConvolve(const Density &a, const Density &b);
%feature("autodoc", "1");
Density directConvolve(const Density &a, const Density &b, const Density &c);

%feature("autodoc", "1");
double moment(const Var &X, size_t n, double xMin, double xMax, size_t N);

%feature("autodoc", "1");
double dnorm(double x);
%feature("autodoc", "1");
double pnorm(double x);
%feature("autodoc", "1");
double qnorm(double p);

%feature("autodoc", "1");
double dgamma(double x, double k, double theta);
%feature("autodoc", "1");
double pgamma(double x, double k, double theta);
%feature("autodoc", "1");
double qgamma(double p, double k, double theta);

%feature("autodoc", "1");
double dinvgamma(double x, double k, double theta);
%feature("autodoc", "1");
double pinvgamma(double x, double k, double theta);
%feature("autodoc", "1");
double qinvgamma(double p, double k, double theta);

%feature("autodoc", "1");
double dweibull(double x, double k, double theta);
%feature("autodoc", "1");
double pweibull(double x, double k, double theta);
%feature("autodoc", "1");
double qweibull(double p, double k, double theta);

%extend Container {
  %feature("autodoc", "1");
  bool isDensity() const { return self->is<stochbb::Density>(); }
  %feature("autodoc", "1");
  stochbb::Density asDensity() { return self->as<stochbb::Density>(); }

  %feature("autodoc", "1");
  bool isVar() const { return self->is<stochbb::Var>(); }
  %feature("autodoc", "1");
  stochbb::Var asVar() { return self->as<stochbb::Var>(); }

  %feature("autodoc", "1");
  bool isDerivedVar() const { return self->is<stochbb::DerivedVar>(); }
  %feature("autodoc", "1");
  stochbb::DerivedVar asDerivedVar() { return self->as<stochbb::DerivedVar>(); }

  %feature("autodoc", "1");
  bool isAffineTrafo() const { return self->is<stochbb::AffineTrafo>(); }
  %feature("autodoc", "1");
  stochbb::AffineTrafo asAffineTrafo() { return self->as<stochbb::AffineTrafo>(); }

  %feature("autodoc", "1");
  bool isChain() const { return self->is<stochbb::Chain>(); }
  %feature("autodoc", "1");
  stochbb::Chain asChain() { return self->as<stochbb::Chain>(); }

  %feature("autodoc", "1");
  bool isMaximum() const { return self->is<stochbb::Maximum>(); }
  %feature("autodoc", "1");
  stochbb::Maximum asMaximum() { return self->as<stochbb::Maximum>(); }

  %feature("autodoc", "1");
  bool isMinimum() const { return self->is<stochbb::Minimum>(); }
  %feature("autodoc", "1");
  stochbb::Minimum asMinimum() { return self->as<stochbb::Minimum>(); }

  %feature("autodoc", "1");
  bool isMixture() const { return self->is<stochbb::Mixture>(); }
  %feature("autodoc", "1");
  stochbb::Mixture asMixture() { return self->as<stochbb::Mixture>(); }

  %feature("autodoc", "1");
  bool isConditional() const { return self->is<stochbb::Conditional>(); }
  %feature("autodoc", "1");
  stochbb::Conditional asConditional() { return self->as<stochbb::Conditional>(); }

  %feature("autodoc", "1");
  bool isCompound() const { return self->is<stochbb::Compound>(); }
  %feature("autodoc", "1");
  stochbb::Compound asCompound() { return self->as<stochbb::Compound>(); }
}

class LogMessage
{
public:
  typedef enum {
    LDEBUG = 0,
    LINFO,
    LWARNING,
    LERROR
  } Level;

public:
  %feature("autodoc", "1");
  LogMessage();
  %feature("autodoc", "1");
  LogMessage(const std::string &filename, int line, Level level, const std::string &message);
  %feature("autodoc", "1");
  LogMessage(const LogMessage &other);

  %feature("autodoc", "1");
  const std::string &filename() const;
  %feature("autodoc", "1");
  int linenumber() const;
  %feature("autodoc", "1");
  Level level() const;
  %feature("autodoc", "1");
  const std::string &message() const;
};


class LogHandler: public Container
{
protected:
  LogHandler(LogHandlerObj *obj);

public:
  %feature("autodoc", "1");
  void handleMessage(const LogMessage &msg);
};


class IOLogHandler: public LogHandler
{
public:
  IOLogHandler(std::ostream &stream=std::cerr, LogMessage::Level level=LogMessage::DEBUG);
};

class Logger
{
protected:
  Logger();

public:
  %feature("autodoc", "1");
  %feature("docstring", "Logs the given message.");
  static void log(const LogMessage &msg);

  %feature("autodoc", "1");
  %feature("docstring", "Adds a log handler to the logger.");
  static void addHandler(const LogHandler &handler);
};

}

%inline {
  double logLikelihood(const stochbb::Var &X, double tmin, double tmax, size_t N, const double *data, int M) throw (Error) {
    Eigen::Map<const Eigen::VectorXd> dataMap(data, M);
    return stochbb::logLikelihood(X, tmin, tmax, N, dataMap);
  }

  double kolmogorov(const stochbb::Var &X, double tmin, double tmax, size_t N, const double *data, int M) throw (Error) {
    Eigen::Map<const Eigen::VectorXd> dataMap(data, M);
    return stochbb::kolmogorov(X, tmin, tmax, N, dataMap);
  }
}
