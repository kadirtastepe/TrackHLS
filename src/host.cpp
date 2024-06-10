// Standard system includes
#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <map>

// XRT includes
#include "xrt/xrt_bo.h"
#include <experimental/xrt_xclbin.h>
#include "xrt/xrt_device.h"
#include "xrt/xrt_kernel.h"

// External includes
#include "args.hpp"

// Custom includes
#include "kernels.hpp"
#include "config.hpp"
#include "utils.hpp"


int main (int argc, char ** argv){
  //============================================
  //
  // Command Line Arguments
  //
  //============================================

  // Pass commandline arguments
  Args args(argc, argv, "FPGA studies");

  // Column map
  std::vector<std::pair<std::string, unsigned int>> COLMAP {
    std::make_pair("par1", PN::par1),
    std::make_pair("par2", PN::par2),
    std::make_pair("par3", PN::par3),
    std::make_pair("par4", PN::par4),
  };

  //============================================
  //
  // Device Configuration
  //
  //============================================

  // Ensure that an XCL binary file is provided
  if (args.fXCLBinFile.empty()) {
    std::cout << "[WARNING] No XCL binary provided! Software and hardware emulation will not be performed." << std::endl;
    return 1;
  }

  // Read settings
  std::string binaryFile = args.fXCLBinFile; // XCL binary file path
  int deviceIndex = 0;

  std::cout << "[INFO] Opening the target device: " << deviceIndex << std::endl;
  auto targetDevice = xrt::device(deviceIndex); // Open the Target Device

  // Load the XCL binary
  std::cout << "[INFO] Loading XCLbin: " << binaryFile << std::endl;
  auto ConfigID = targetDevice.load_xclbin(binaryFile);

  // Calculate size of vector in bytes
  size_t vector_size_bytes_in = sizeof(Triplet)*(__NHITS__-2)*__BATCHSIZE__;
  size_t vector_size_bytes_out = sizeof(Parameters)*(__NHITS__-2)*__BATCHSIZE__;
  std::cout << "[INFO] Size of Triplet: " << vector_size_bytes_in << " Bytes" << std::endl;
  std::cout << "[INFO] Size of Parameters: " << vector_size_bytes_out << " Bytes" <<std::endl;

  std::cout << "[INFO] Allocate Buffer in Global Memory" << std::endl;
  // Number of buffer objects to create
  const int numBuffersIn = 1, numBuffersOut = 1; 
			    
  // Define Kernel Function to be executed on Device
  auto krnl = xrt::kernel(targetDevice, ConfigID, "fit_local", xrt::kernel::cu_access_mode::exclusive);

  // Array to hold buffer objects
  // - Input buffers
  std::vector<xrt::bo> buffObjIn(numBuffersIn);
  std::vector<Triplet*> buffMapIn(numBuffersIn);
  // - Output buffers
  std::vector<xrt::bo> buffObjOut(numBuffersOut);
  std::vector<Parameters*> buffMapOut(numBuffersOut);

  // TODO
  unsigned int pos(0);

  // Allocating Memory for INPUT
  for (unsigned int iBuf=0; iBuf<numBuffersIn; ++iBuf) {
    // Creating buffer objects
    buffObjIn[iBuf] = xrt::bo(targetDevice, vector_size_bytes_in, krnl.group_id(pos));    
    // Mapping the buffer objects to host memory
    buffMapIn[iBuf] = buffObjIn[iBuf].map<Triplet*>();
    // Filling the mapped buffer with zeros
//    std::fill(buffMap[iBuf], buffMap[iBuf]+sizeInput*__BATCHSIZE__, 0);
    pos++;
  }

  // Allocating Memory for OUTPUT
  for (unsigned int iBuf=0; iBuf<numBuffersOut; ++iBuf) {
    buffObjOut[iBuf] = xrt::bo(targetDevice, vector_size_bytes_out, krnl.group_id(pos));
    buffMapOut[iBuf] = buffObjOut[iBuf].map<Parameters*>();
//  std::fill(buffMap[buffObj.size()-1], buffMap[buffObj.size()-1]+(nHits-2)*sizeOutput*__BATCHSIZE__, 0);
    pos++;
  }

  //============================================
  //
  // DATA PREPARE
  //
  //============================================

  //size = (__NHITS__-2) * __BATCHSIZE__


  // Define variables
  std::vector<std::vector<__DATATYPE__>> output(static_cast<int>(sizeof(Parameters)/sizeof(__DATATYPE__)));


      Triplet trpl1;
    // Hit 1
    trpl1.hit1.x = 20.0387;
    trpl1.hit1.y = 34.6186;
    trpl1.hit1.z = 5.32276;

    // Hit 2
    trpl1.hit2.x = 62.0629;
    trpl1.hit2.y = 96.8153;
    trpl1.hit2.z = 5.66036;

    // Hit 3
    trpl1.hit3.x = 109.783;
    trpl1.hit3.y = 155.073;
    trpl1.hit3.z = 6.03175;

  unsigned int nEvents = sizeof(trpl1);
  std::cout << "[INFO] Number of Entries -> " << nEvents <<std::endl;

  //============================================
  //
  // Start main event loop
  //
  //============================================

  // Define Arrays
  // - Input
  Triplet arrayIn[(__NHITS__-2)*__BATCHSIZE__];
  // - Output
  Parameters arrayOut[(__NHITS__-2)*__BATCHSIZE__];

  // Running over batches of events
  std::cout << "[INFO] Start main event loop ..." << std::endl;
  for (unsigned int iBatch=0; iBatch< nEvents/__BATCHSIZE__; ++iBatch){

    // End main event loop earlier
    if (__NBATCHES_MAX__ <= iBatch) break;

    // Running over events in Batch
    for (unsigned int jEvent=iBatch*__BATCHSIZE__, kEntry=0; jEvent< (iBatch+1)*__BATCHSIZE__; ++jEvent, ++kEntry){
      // Get the entries associated with this event
      // Run over the coordinates/hits
      for (unsigned int lTrip=0; lTrip< __NHITS__-2; ++lTrip){

        // Store hit coordinates for each event in the Batch into an array
        arrayIn[(__NHITS__-2)*kEntry+lTrip] = Triplet
        {
	  { trpl1.hit1.x, trpl1.hit1.y, trpl1.hit1.z }, // hit1
          { trpl1.hit2.x, trpl1.hit2.y, trpl1.hit2.z }, // hit2
          { trpl1.hit3.x, trpl1.hit3.y, trpl1.hit3.z }  // hit3
        };
      }
    }

    #ifdef __DEBUG__
      std::cout << "[    ] Batch number " << iBatch << std::endl;
      for (unsigned int iEvent=0, kEntry=0; iEvent<__BATCHSIZE__; ++iEvent, ++kEntry){
        std::cout << "       (" << iEvent << ") coord. (x) ";
        for (unsigned int jTrip=0; jTrip<__NHITS__-2; ++jTrip){
	  std::cout <<" hit1-> " << arrayIn[(__NHITS__-2)*kEntry+jTrip].hit1.x << " ";
	  std::cout <<" hit2-> " << arrayIn[(__NHITS__-2)*kEntry+jTrip].hit2.x << " ";
	  std::cout <<" hit3-> " << arrayIn[(__NHITS__-2)*kEntry+jTrip].hit3.x << " ";
	}
	std::cout << std::endl;
      }
    #endif

    // Prepare data to be send to FPGA
    unsigned int pos(0);
    for (auto & array : {arrayIn}){
      // Fill buffers
      for (unsigned int jItem=0; jItem<(__NHITS__-2)*__BATCHSIZE__; ++jItem){
        buffMapIn[pos][jItem] = array[jItem]; 
      }
      // We now have a batch of events; synchronize with memory of FPGA
      buffObjIn[pos].sync(XCL_BO_SYNC_BO_TO_DEVICE);
      pos++;
    }

    // Run kernel on FPGA
    //======================
    std::cout << "[INFO] Size of Buffer Sending -> " << buffObjIn[0].size() << " Bytes" << std::endl;
    // Run the kernel
    std::cout << "[    ] Executing the kernel" << std::endl;
    auto run = krnl(buffObjIn[0], buffObjOut[0], 27 /* (__NHITS__-2)*__BATCHSIZE__ */) ;
    run.wait(); 

    // Copy data from the FPGA back to the HOST
    std::cout << "[    ] Copy data from the FPGA back to the HOST" << std::endl;
    buffObjOut[buffObjOut.size()-1].sync(XCL_BO_SYNC_BO_FROM_DEVICE);
    std::memcpy(arrayOut, buffMapOut[buffMapOut.size()-1], vector_size_bytes_out);
    std::cout << "[INFO] Size of Buffer Received -> " << buffObjOut[0].size() << " Bytes" << std::endl;

    // Save data to file
    //======================
 
    // Print some DEBUG information
    #ifdef __DEBUG__
      std::cout << "[    ] Output array " << iBatch << std::endl;
    #endif

    for (unsigned int iEvent=0; iEvent<__BATCHSIZE__; ++iEvent){
      #ifdef __DEBUG__
        std::cout << "       (" << iEvent << ") - ";
      #endif
      // Loop over the parameters 
      for (unsigned int jParam=0; jParam<output.size(); ++jParam){
        #ifdef __DEBUG__
          std::cout << COLMAP[jParam].first << "[0] ";
        #endif
        std::vector<__DATATYPE__> vtmp;
        for (unsigned int kTrip=0; kTrip<__NHITS__-2; ++kTrip){
    	  unsigned int pos = iEvent*(__NHITS__-2) + kTrip;
          vtmp.push_back(arrayOut[pos].local[jParam]);
	}
        #ifdef __DEBUG__
	  if (jParam != output.size()-1){
            std::cout << vtmp[0] << ", ";
	  } else {
            std::cout << vtmp[0] << std::endl;;
	  }
        #endif
        output[jParam] = vtmp;
      }
    }
  }

  //============================================
  //
  // Close files; clean up allocated memory etc.
  //
  //============================================

  return 0;
}

