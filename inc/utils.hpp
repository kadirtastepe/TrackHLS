#ifndef UTILS_H
#define UTILS_H

// HLS-related libraries
#include "hls_vector.h"  

namespace PN {
  const long unsigned int par1=0, par2=1, par3=2, par4=3, _N_=4;
};

struct Hit {
  __DATATYPE__ x , y , z;
};

struct Parameters {
  hls::vector<__DATATYPE__, PN::_N_> local;
}; 

struct Triplet {
  Hit hit1;
  Hit hit2;
  Hit hit3;
};

#endif // UTILS_H
