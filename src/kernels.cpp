// HLS-related includes
#include "hls_math.h"
#include "hls_vector.h"
#include <hls_stream.h>

// Custom includes
#include "kernels.hpp"
#include "config.hpp"
#include "utils.hpp"

#define DATA_SIZE 1

__DATATYPE__ sin_App(__DATATYPE__ x) {
    #pragma HLS inline off
    #pragma HLS PIPELINE
    __DATATYPE__ aa = x*x;
    __DATATYPE__ bb = x/6.0;
    __DATATYPE__ cc = aa*bb;
    __DATATYPE__ dd = x-cc;

//    __DATATYPE__ dd = sin(x);
    return dd;
}

__DATATYPE__ asin_App(__DATATYPE__ x) {
    #pragma HLS inline off
    #pragma HLS PIPELINE
    __DATATYPE__ aaa = x*x;
    __DATATYPE__ bbb = x/6.0;
    __DATATYPE__ ccc = aaa*bbb;
    __DATATYPE__ ddd = x + ccc;
//    __DATATYPE__ ddd = sin(x);
    return ddd;
}

// TRIPCOUNT identifier
const int c_size = DATA_SIZE;

static void read_input(const Triplet* in, hls::stream<Triplet>& inStream, int size) {
 // Auto-pipeline is going to apply pipeline to this loop
mem_rd:
  for (int i = 0; i < size; i++) {
  #pragma HLS LOOP_TRIPCOUNT min = c_size max = c_size
  #pragma HLS PIPELINE II=1
  #pragma HLS ARRAY_PARTITION variable=in complete dim=1
  inStream << in[i];
  }
}

static void compute_operation(hls::stream<Triplet>& inStream, hls::stream<Parameters>& outStream, int size) {
  // Auto-pipeline is going to apply pipeline to this loop
      execute:
      for (int i = 0; i < size; i++) {
      #pragma HLS LOOP_TRIPCOUNT min = c_size max = c_size
      #pragma HLS inline off
      #pragma HLS pipeline II=1

      Triplet trpl = inStream.read();
      // Some Calculations
      __DATATYPE__ dx = trpl.hit3.x-trpl.hit1.x;
      __DATATYPE__ dy = trpl.hit3.y-trpl.hit2.y;
      __DATATYPE__ dz = trpl.hit2.z-trpl.hit1.z; 


      __DATATYPE__ part1 = sqrt(dx);
      __DATATYPE__ part2 = cos(dy);
      __DATATYPE__ part3 = dx*sin_App(dz);
      __DATATYPE__ part4 = part2 + part3;

      Parameters params;
      params.local[PN::par1]    =   part1;
      params.local[PN::par2]    =   part2;
      params.local[PN::par3]    =   part3;
      params.local[PN::par4]    =   part4;

      outStream << params; 
    }
}

void write_result(Parameters * out, hls::stream<Parameters>& outStream, int size) {
  mem_wr:
    for (int i = 0; i < size; i++) {
      #pragma HLS LOOP_TRIPCOUNT min = c_size max = c_size
      out[i] = outStream.read();
    }
}

  void fit_local(const Triplet * in, Parameters * out, const unsigned int size) {
    #pragma HLS INTERFACE m_axi port = in offset=slave
    #pragma HLS INTERFACE m_axi port = out offset=slave
    #pragma HLS dataflow

    static hls::stream<Triplet> inStream("input_stream");
    static hls::stream<Parameters> outStream("output_stream");

    // dataflow pragma instruct compiler to run following three APIs in parallel
    read_input(in, inStream, size);
    compute_operation(inStream, outStream, size);
    write_result(out, outStream, size);
  }




