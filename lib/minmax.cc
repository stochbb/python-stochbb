#include "minmax.hh"
#include "exception.hh"
#include "chain.hh"

using namespace sbb;


/* ********************************************************************************************* *
 * Implementation of MaximumDensityObj
 * ********************************************************************************************* */
MaximumDensityObj::MaximumDensityObj(const std::vector<VarObj *> &variables)
  : DensityObj(), _densities()
{
  _densities.reserve(variables.size());
  for (size_t i=0; i<variables.size(); i++) {
    _densities.push_back(*variables[i]->density());
  }
}

MaximumDensityObj::MaximumDensityObj(const std::vector<Var> &variables)
  : DensityObj(), _densities()
{
  _densities.reserve(variables.size());
  for (size_t i=0; i<variables.size(); i++) {
    _densities.push_back(*variables[i].density());
  }
}

MaximumDensityObj::~MaximumDensityObj() {
  // pass...
}

void
MaximumDensityObj::mark() {
  if (isMarked()) { return; }
  DensityObj::mark();
  for (size_t i=0; i<_densities.size(); i++) {
    _densities[i]->mark();
  }
}

void
MaximumDensityObj::eval(double Tmin, double Tmax, Eigen::VectorXd &out) const {
  Eigen::VectorXd tmp(out.size());
  Eigen::MatrixXd pdfs(out.size(), _densities.size());
  Eigen::MatrixXd cdfs(out.size(), _densities.size());
  for (size_t i=0; i<_densities.size(); i++) {
    _densities[i]->eval(Tmin, Tmax, tmp);    pdfs.col(i) = tmp;
    _densities[i]->evalCDF(Tmin, Tmax, tmp); cdfs.col(i) = tmp;
  }

  out.setZero();
  for (size_t i=0; i<_densities.size(); i++) {
    tmp.setOnes();
    for (size_t j=0; j<_densities.size(); j++) {
      if (i == j) {
        tmp.array() *= pdfs.col(j).array();
      } else {
        tmp.array() *= cdfs.col(j).array();
      }
    }
    out += tmp;
  }
}

void
MaximumDensityObj::evalCDF(double Tmin, double Tmax, Eigen::VectorXd &out) const {
  Eigen::VectorXd tmp(out.size());

  out.setOnes();
  for (size_t i=0; i<_densities.size(); i++) {
    _densities[i]->evalCDF(Tmin, Tmax, tmp);
    out.array() *= tmp.array();
  }
}


/* ********************************************************************************************* *
 * Implementation of MinimumDensityObj
 * ********************************************************************************************* */
MinimumDensityObj::MinimumDensityObj(const std::vector<VarObj *> &variables)
  : DensityObj(), _densities()
{
  _densities.reserve(variables.size());
  for (size_t i=0; i<variables.size(); i++) {
    _densities.push_back(*variables[i]->density());
  }
}

MinimumDensityObj::MinimumDensityObj(const std::vector<Var> &variables)
  : DensityObj(), _densities()
{
  _densities.reserve(variables.size());
  for (size_t i=0; i<variables.size(); i++) {
    _densities.push_back(*variables[i].density());
  }
}

MinimumDensityObj::~MinimumDensityObj() {
  // pass...
}

void
MinimumDensityObj::mark() {
  if (isMarked()) { return; }
  DensityObj::mark();
  for (size_t i=0; i<_densities.size(); i++) {
    _densities[i]->mark();
  }
}

void
MinimumDensityObj::eval(double Tmin, double Tmax, Eigen::VectorXd &out) const {
  Eigen::VectorXd tmp(out.size());
  Eigen::MatrixXd pdfs(out.size(), _densities.size());
  Eigen::MatrixXd cdfs(out.size(), _densities.size());
  for (size_t i=0; i<_densities.size(); i++) {
    _densities[i]->eval(Tmin, Tmax, tmp);    pdfs.col(i) = tmp;
    _densities[i]->evalCDF(Tmin, Tmax, tmp); cdfs.col(i) = tmp;
  }

  out.setZero();
  for (size_t i=0; i<_densities.size(); i++) {
    tmp.setOnes();
    for (size_t j=0; j<_densities.size(); j++) {
      if (i == j) {
        tmp.array() *= (-pdfs.col(j).array());
      } else {
        tmp.array() *= (1-cdfs.col(j).array());
      }
    }
    out += tmp;
  }
  out = -out;
}

void
MinimumDensityObj::evalCDF(double Tmin, double Tmax, Eigen::VectorXd &out) const {
  Eigen::VectorXd tmp(out.size());

  out.setOnes();
  for (size_t i=0; i<_densities.size(); i++) {
    _densities[i]->evalCDF(Tmin, Tmax, tmp);
    out.array() *= (1-tmp.array());
  }
  out.array() = (1-out.array());
}


/* ********************************************************************************************* *
 * Implementation of MaximumObj
 * ********************************************************************************************* */
MaximumObj::MaximumObj(const Var &a, const Var &b, const std::string &name)
  : VarObj(name), _variables(), _density(0)
{
  if (! a.mutuallyIndep(b)) {
    AssumptionError err;
    err << "Cannot assemble maximum variable, arguments are not mutually independent.";
    throw err;
  }

  if (MaximumObj *max_a = dynamic_cast<MaximumObj *>(*a)) {
    _variables.reserve(max_a->numVariables());
    for (size_t i=0; i<max_a->numVariables(); i++) {
      _variables.push_back(*max_a->variable(i));
    }
  } else {
    _variables.push_back(*a);
  }

  if (MaximumObj *max_b = dynamic_cast<MaximumObj *>(*b)) {
    for (size_t i=0; i<max_b->numVariables(); i++) {
      _variables.push_back(*max_b->variable(i));
    }
  } else {
    _variables.push_back(*b);
  }

  // Collect dependencies
  for (size_t i=0; i<_variables.size(); i++) {
    // Add implicit dependencies
    _dependencies.insert(_variables[i]->dependencies().begin(),
                         _variables[i]->dependencies().end());
    // add variable itself
    _dependencies.insert(_variables[i]);
  }

  _density = new MaximumDensityObj(_variables);
}

MaximumObj::MaximumObj(const std::vector<Var> &variables, const std::string &name)
  : VarObj(name), _variables(), _density(0)
{
  // Reserve some space
  _variables.reserve(variables.size());

  // Expand maximum object
  for (size_t i=0; i<variables.size(); i++) {
    if (MaximumObj *max = dynamic_cast<MaximumObj *>(*variables[i])) {
      for (size_t j=0; j<max->numVariables(); j++) {
        _variables.push_back(*max->variable(j));
      }
    }
  }

  // Collect dependencies and check for independence
  for (size_t i=0; i<_variables.size(); i++) {
    if (! this->mutuallyIndep(_variables[i])) {
      AssumptionError err;
      err << "Cannot assemble maximum variable, arguments are not mutually independent.";
      throw err;
    }
    // Add implicit dependencies
    _dependencies.insert(_variables[i]->dependencies().begin(),
                         _variables[i]->dependencies().end());
    // add variable itself
    _dependencies.insert(_variables[i]);
  }

  _density = new MaximumDensityObj(_variables);
}

MaximumObj::~MaximumObj() {
  // pass...
}

void
MaximumObj::mark() {
  if (isMarked()) { return; }
  VarObj::mark();
  for (size_t i=0; i<_variables.size(); i++) {
    _variables[i]->mark();
  }
  _density->mark();
}

Density
MaximumObj::density() {
  _density->ref();
  return _density;
}


/* ********************************************************************************************* *
 * Implementation of MinimumObj
 * ********************************************************************************************* */
MinimumObj::MinimumObj(const Var &a, const Var &b, const std::string &name)
  : VarObj(name), _variables(), _density(0)
{
  if (! a.mutuallyIndep(b)) {
    AssumptionError err;
    err << "Cannot assemble minimum variable, arguments are not mutually independent.";
    throw err;
  }

  if (MinimumObj *min_a = dynamic_cast<MinimumObj *>(*a)) {
    _variables.reserve(min_a->numVariables());
    for (size_t i=0; i<min_a->numVariables(); i++) {
      _variables.push_back(*min_a->variable(i));
    }
  } else {
    _variables.push_back(*a);
  }
  if (MinimumObj *min_b = dynamic_cast<MinimumObj *>(*b)) {
    for (size_t i=0; i<min_b->numVariables(); i++) {
      _variables.push_back(*min_b->variable(i));
    }
  } else {
    _variables.push_back(*b);
  }

  // Collect dependencies
  for (size_t i=0; i<_variables.size(); i++) {
    // Add implicit dependencies
    _dependencies.insert(_variables[i]->dependencies().begin(),
                         _variables[i]->dependencies().end());
    // add variable itself
    _dependencies.insert(_variables[i]);
  }

  _density = new MinimumDensityObj(_variables);
}

MinimumObj::MinimumObj(const std::vector<Var> &variables, const std::string &name)
  : VarObj(name), _variables(), _density(0)
{
  // Get & store variable objects
  _variables.reserve(variables.size());
  for (size_t i=0; i<variables.size(); i++) { _variables.push_back(*variables[i]); }

  // Collect dependencies
  for (size_t i=0; i<_variables.size(); i++) {
    if (! this->mutuallyIndep(_variables[i])) {
      AssumptionError err;
      err << "Cannot assemble minimum variable, arguments are not mutually independent.";
      throw err;
    }
    // Add implicit dependencies
    _dependencies.insert(_variables[i]->dependencies().begin(),
                         _variables[i]->dependencies().end());
    // add variable itself
    _dependencies.insert(_variables[i]);
  }

  _density = new MinimumDensityObj(_variables);
}

MinimumObj::~MinimumObj() {
  // pass...
}

void
MinimumObj::mark() {
  if (isMarked()) { return; }
  VarObj::mark();
  for (size_t i=0; i<_variables.size(); i++) {
    _variables[i]->mark();
  }
  _density->mark();
}

Density MinimumObj::density() {
  _density->ref();
  return _density;
}
