#ifndef KERNELS_H
#define KERNELS_H

// Some headers
#include "config.hpp"
#include "utils.hpp"

extern "C" {
  void fit_local (const Triplet *, Parameters *, const unsigned int);
}
#endif // KERNELS_H
