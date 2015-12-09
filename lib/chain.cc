#include "chain.hh"
#include "exception.hh"
#include <unsupported/Eigen/FFT>
#include <list>
#include <algorithm>

using namespace sbb;

// tiny helper function to sort vectors of densities
inline int density_pointer_compare(const DensityObj *a, const DensityObj *b) {
  return a->compare(*b);
}

/* ********************************************************************************************* *
 * combine densities
 * ********************************************************************************************* */
// Check if two densities can be combined
bool convolution_can_combine(DensityObj *a, DensityObj *b) {
  if (DeltaDensityObj *delta_a = dynamic_cast<DeltaDensityObj *>(a)) {
    // If LHS is a delta density ...
    // ... and its delay is 0 -> identity operation
    if (0 == delta_a->delay()) { return true; }
    // .. other wise check by RHS type
    if (dynamic_cast<DeltaDensityObj *>(b)) {
      // If RHS is a delta density too -> yes
      return true;
    } else if (dynamic_cast<UniformDensityObj *>(b)) {
      // If RHS is a uniform density -> yes
      return true;
    } else if (dynamic_cast<NormalDensityObj *>(b)) {
      // If RHS is a normal density -> yes
      return true;
    }
  } else if (dynamic_cast<UniformDensityObj *>(a)) {
    // If LHS is a uniform density ...
    if (dynamic_cast<DeltaDensityObj *>(b)) {
      // If RHS is a delta density -> yes
      return true;
    }
  } else if (dynamic_cast<NormalDensityObj *>(a)) {
    // If RHS is a normal distribution
    if (dynamic_cast<DeltaDensityObj *>(b)) {
      // If RHS is a delta density -> yes
      return true;
    } else if (dynamic_cast<NormalDensityObj *>(b)) {
      // If RHS is a normal density -> yes
      return true;
    }
  } else if (GammaDensityObj *gamma_a = dynamic_cast<GammaDensityObj *>(a)) {
    if (DeltaDensityObj *delta_b = dynamic_cast<DeltaDensityObj *>(b)) {
      // If RHS is a gamma density with 0-delay -> identity
      return 0 == delta_b->delay();
    } else if (GammaDensityObj *gamma_b = dynamic_cast<GammaDensityObj *>(b)) {
      // If LHS is a gamma too and has the same scale
      return gamma_a->theta() == gamma_b->theta();
    }
  }
  // Otherwise -> no
  return false;
}

// Derives new the analytic convolution of densities (if possible, check with
// convolution_can_combine). Returns a new reference to a convolution object
DensityObj *convolution_combine(DensityObj *a, DensityObj *b) {
  logDebug() << "Convolve densities " << *a << " and " << *b;
  if (DeltaDensityObj *delta_a = dynamic_cast<DeltaDensityObj *>(a)) {
    // If delay is 0 -> identity
    if (0 == delta_a->delay()) { b->ref(); return b; }
    // If LHS is a delta density ...
    if (DeltaDensityObj *delta_b = dynamic_cast<DeltaDensityObj *>(b)) {
      // If RHS is a delta density too
      return new DeltaDensityObj(delta_a->delay() + delta_b->delay());
    } else if (UniformDensityObj *unif_b = dynamic_cast<UniformDensityObj *>(b)) {
      // If RHS is a uniform density
      return new UniformDensityObj(unif_b->a()+delta_a->delay(),
                                   unif_b->b()+delta_a->delay());
    } else if (NormalDensityObj *norm_b = dynamic_cast<NormalDensityObj *>(b)) {
      // If RHS is a normal density
      return new NormalDensityObj(norm_b->mu()+delta_a->delay(), norm_b->sigma());
    }
  } else if (UniformDensityObj *unif_a = dynamic_cast<UniformDensityObj *>(a)) {
    // If LHS is a uniform density ...
    if (DeltaDensityObj *delta_b = dynamic_cast<DeltaDensityObj *>(b)) {
      // If RHS is a delta density -> yes
      return new UniformDensityObj(unif_a->a()+delta_b->delay(),
                                   unif_a->b()+delta_b->delay());
    }
  } else if (NormalDensityObj *norm_a = dynamic_cast<NormalDensityObj *>(a)) {
    // If RHS is a normal distribution
    // If LHS is a delta density ...
    if (DeltaDensityObj *delta_b = dynamic_cast<DeltaDensityObj *>(b)) {
      // If RHS is a delta density
      return new NormalDensityObj(norm_a->mu()+delta_b->delay(), norm_a->sigma());
    } else if (NormalDensityObj *norm_b = dynamic_cast<NormalDensityObj *>(b)) {
      // If RHS is a normal density too
      return new NormalDensityObj(norm_a->mu()+norm_b->mu(),
                                  std::sqrt(norm_a->sigma()*norm_a->sigma()
                                            + norm_b->sigma()*norm_b->sigma()));
    }
  } else if (GammaDensityObj *gamma_a = dynamic_cast<GammaDensityObj *>(a)) {
    // If LHS is a gamma density
    if (DeltaDensityObj *delta_b = dynamic_cast<DeltaDensityObj *>(b)) {
      // if RHS is a delta with 0-delay -> identity
      if (0 == delta_b->delay()) { a->ref(); return a; }
    } else if (GammaDensityObj *gamma_b = dynamic_cast<GammaDensityObj *>(b)) {
      // If LHS is a gamma too and has the same scale
      if (gamma_a->theta() == gamma_b->theta()) {
        return new GammaDensityObj(gamma_a->k()+gamma_b->k(), gamma_a->theta());
      }
    }
  }
  // Otherwise --> oops
  AssumptionError err;
  err << "Cannot combine densities.";
  throw err;
}


/* ********************************************************************************************* *
 * Implementation of ConvolutionDensityObj
 * ********************************************************************************************* */
ConvolutionDensityObj::ConvolutionDensityObj(const std::vector<DensityObj *> &densities, double scale, double shift)
  : DensityObj(), _densities(densities), _scale(scale), _shift(shift)
{
  // pass...
}

ConvolutionDensityObj::ConvolutionDensityObj(const std::vector<Var> &variables, double scale, double shift)
  : DensityObj(), _densities(), _scale(scale), _shift(shift)
{
  // Get & store the densities of all variables, assuming they are mutually independent.
  _densities.reserve(variables.size());
  for (size_t i=0; i<variables.size(); i++) {
    _densities.push_back(*variables[i]->density());
  }

  // Try to reduce the number of densities
  _combine_densities();
}

ConvolutionDensityObj::ConvolutionDensityObj(const std::vector<VarObj *> &variables, double scale, double shift)
  : DensityObj(), _densities(), _scale(scale), _shift(shift)
{
  // Get & store the densities of all variables, assuming they are mutually independent.
  _densities.reserve(variables.size());
  for (size_t i=0; i<variables.size(); i++) {
    _densities.push_back(*variables[i]->density());
  }

  // Try to reduce the number of densities
  _combine_densities();
}


void
ConvolutionDensityObj::_combine_densities() {
  // Sort densities w.r.t type and parameters
  std::sort(_densities.begin(), _densities.end(), density_pointer_compare);

  // Try to combine some of the densities
  std::vector<DensityObj *>::iterator last = _densities.begin();
  std::vector<DensityObj *>::iterator current = _densities.begin(); current++;
  while (current!=_densities.end()) {
    if (convolution_can_combine(*last, *current)) {
      // If densities can be combined -> combine & replace last density
      *last = convolution_combine(*last, *current);
      // release reference to new (combined) densitiy
      (*last)->unref();
      // erase combined densities
      current = _densities.erase(current);
    } else {
      // If densities cannot be combined -> advance iterators
      last++; current++;
    }
  }
}

ConvolutionDensityObj::~ConvolutionDensityObj() {
  // pass...
}

void
ConvolutionDensityObj::mark() {
  if (isMarked()) { return; }
  DensityObj::mark();
  for (size_t i=0; i<_densities.size(); i++) {
    _densities[i]->mark();
  }
}

void
ConvolutionDensityObj::eval(double Tmin, double Tmax, Eigen::Ref<Eigen::VectorXd> out) const {
  // Apply affine transform
  Tmin = (Tmin-_shift)/_scale;
  Tmax = (Tmax-_shift)/_scale;

  Eigen::FFT<double> fft;
  Eigen::VectorXd tmp1(2*out.size());  tmp1.setZero();
  Eigen::VectorXcd tmp2(2*out.size());
  Eigen::VectorXcd prod(2*out.size()); prod.setOnes();
  // Perform FFT convolution
  double dt = (Tmax-Tmin)/out.size();
  for (size_t i=0; i<_densities.size(); i++) {
    _densities[i]->eval(Tmin, Tmax, out);
    tmp1.head(out.size()) = out*dt/_scale;
    fft.fwd(tmp2, tmp1); prod.array() *= tmp2.array();
  }
  fft.inv(tmp1, prod);
  out = tmp1.head(out.size())/dt;
}

void
ConvolutionDensityObj::evalCDF(double Tmin, double Tmax, Eigen::Ref<Eigen::VectorXd> out) const {
  // Apply affine transform
  Tmin = (Tmin-_shift)/_scale;
  Tmax = (Tmax-_shift)/_scale;

  Eigen::FFT<double> fft;
  Eigen::VectorXd tmp1(2*out.size());  tmp1.setZero();
  Eigen::VectorXcd tmp2(2*out.size());
  Eigen::VectorXcd prod(2*out.size()); prod.setOnes();
  // Perform FFT convolution
  for (size_t i=0; i<_densities.size(); i++) {
    _densities[i]->evalCDF(Tmin, Tmax, out);
    tmp1.head(out.size()) = out;
    fft.fwd(tmp2, tmp1); prod.array() *= tmp2.array();
  }
  fft.inv(tmp1, prod);
  out = tmp1.head(out.size());
}

Density
ConvolutionDensityObj::affine(double scale, double shift) const {
  return new ConvolutionDensityObj(_densities, _scale*scale, scale*_shift+shift);
}

int
ConvolutionDensityObj::compare(const DensityObj &other) const {
  // Compare by type
  if (int res = DensityObj::compare(other)) { return res; }
  // If types match
  const ConvolutionDensityObj *o_conv = dynamic_cast<const ConvolutionDensityObj *>(&other);
  // Compare by number of densities
  if (_densities.size() < o_conv->_densities.size()) { return -1; }
  else if (_densities.size() > o_conv->_densities.size()) { return 1; }
  // Compare densities
  for (size_t i=0; i<_densities.size(); i++) {
    if (int res = _densities[i]->compare(*o_conv->_densities[i])) { return res; }
  }
  // equal
  return 0;
}

void
ConvolutionDensityObj::print(std::ostream &stream) const {
  stream << "<ConvolutionDensityObj of";
  for (size_t i=0; i<_densities.size(); i++) {
    stream << " "; _densities[i]->print(stream);
  }
  stream << " #" << this << ">";
}


/* ********************************************************************************************* *
 * Implementation of ChainObj
 * ********************************************************************************************* */
ChainObj::ChainObj(const std::vector<Var> &variables, const std::string &name)
  : DerivedVarObj(variables, name), _density(0)
{
  _density = new ConvolutionDensityObj(_variables);
}

ChainObj::~ChainObj() {
  // pass...
}

void
ChainObj::mark() {
  if (isMarked()) { return; }
  DerivedVarObj::mark();
  // mark density
  if (_density) {
    _density->mark();
  }
}

Density ChainObj::density() {
  _density->ref();
  return _density;
}

