#/usr/bin/env bash

# Make sure all directories needed for this run exist
for VAR in "${!FPGA_PATH2EMU_@}"; do mkdir -p ${!VAR}; done

# Path to directory that holds generated anafiles
PATH2DATA=${FPGA_PATH2EMU_DAT}/ana
mkdir -p ${PATH2DATA}



  echo "[INFO] Running test for mode: ${XCL_EMULATION_MODE}"
  ${FPGA_PATH2EMU_BIN}/app_${XCL_EMULATION_MODE} \
    --xclbin ${FPGA_PATH2EMU_BIN}/kernels.xclbin \
    --treename ${XCL_EMULATION_MODE} \
  #break

