#!/usr/bin/env bash
# Kadir Tastepe 05.2024

# Get the path to this script
SOURCE=${BASH_SOURCE[0]}
while [ -L "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
  DIR=$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )
  SOURCE=$(readlink "$SOURCE")
  [[ $SOURCE != /* ]] && SOURCE=$DIR/$SOURCE 
done
SCRIPTPATH=$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )

#==============================
#
# Some helper
#
#==============================

export BOLDTXT=$(tput bold)
export NORMTXT=$(tput sgr0)

# Just a lookup table for some handy abbreviations
declare -A lookup=(
  ["int"]="i"
  ["float"]="f"
  ["double"]="d"
)

# List of directories
DIRS=(src inc etc logs bin lib dat obj fig tmp out)
DIRSOUT=(bin obj logs tmp etc dat fig)

# Some more variables
__BATCHSIZE__=1
__NHITS__=5
__NTRIPLETS__="$((${__NHITS__}-2))"
export FPGA_NTRIPLETS="$((${__NHITS__}*${__BATCHSIZE__}))"

#==============================
#
# Global configuration
#
#==============================

if [ -z ${XCL_EMULATION_MODE} ]; then
  export XCL_EMULATION_MODE=hw_emu
fi
if [ -z ${FPGA_RETURNTYPE} ]; then
  export FPGA_RETURNTYPE=float
fi

# Change name
IFS='_' read -ra __XCL_EMULATION_MODE__ <<< "${XCL_EMULATION_MODE}"
export FPGA_EMULATION_MODE="${__XCL_EMULATION_MODE__[0]}"
if [ "${#__XCL_EMULATION_MODE__[@]}" -eq 2 ]; then
  export FPGA_EMULATION_MODE="${__XCL_EMULATION_MODE__[1]}_${__XCL_EMULATION_MODE__[0]}"
fi

# Set the platform
export FPGA_PLATFORM=xilinx_u280_gen3x16_xdma_1_202211_1
# Split the plattform name
IFS='_' read -ra __FPGA_PLATFORM__ <<< "${FPGA_PLATFORM}"
export FPGA_PLATFORM_SHORT="${__FPGA_PLATFORM__[0]}_${__FPGA_PLATFORM__[1]}_${__FPGA_PLATFORM__[2]}"

# Some more infos
export FPGA_RUN_MODE=debug

# Define some variable
export FPGA_PREFIX="${FPGA_EMULATION_MODE}.p_${FPGA_PLATFORM_SHORT}.rtype_${lookup[${FPGA_RETURNTYPE}]}.mode_${FPGA_RUN_MODE}"

# Define some path variables
export FPGA_PATH2HOME=${SCRIPTPATH}

# Define some path variables
for DIR in "${DIRS[@]}"; do
  export FPGA_PATH2${DIR^^}=${FPGA_PATH2HOME}/${DIR}
done

# Path to profiling configuration file
export XRT_INI_PATH=${FPGA_PATH2ETC}/xrt.ini

# Define some more path variables
export FPGA_PATH2PYTHON=${FPGA_PATH2HOME}/python
export FPGA_PATH2SCRIPTS=${FPGA_PATH2HOME}/scripts
export FPGA_PATH2EXT=${FPGA_PATH2HOME}/external
export FPGA_PATH2EMU=${FPGA_PATH2OUT}/${FPGA_PREFIX}

# Substructure of output directorie
for DIR in "${DIRSOUT[@]}"; do
  export FPGA_PATH2EMU_${DIR^^}=${FPGA_PATH2EMU}/${DIR}
done

# Create directories if they do not exist (only if they have the correct prefix)
for VAR in "${!FPGA_PATH@}"; do mkdir -p ${!VAR}; done

# Create some links
ln -snf ${FPGA_PATH2EMU} ${FPGA_PATH2OUT}/current_setup

#==============================
#
# Some convenient functions
#
#==============================

function set_mode {
  # Check arguments
  if [ $# -eq 0 ]; then
    echo "[ERROR] No identifier has been provided. Please name one [sw_emu/hw_emu/hw]."
  elif [[ "$1" == "sw_emu" || "$1" == "hw_emu" || "$1" == "hw" ]]; then
    export XCL_EMULATION_MODE=${1}
    source ${SCRIPTPATH}/setup.sh
  else
    echo "[ERROR] Argument '${1}' not valid [sw_mode/hw_mode/hw]."
  fi
}
export -f set_mode

function set_type {
  # Check arguments
  if [ $# -eq 0 ]; then
    echo "[ERROR] No type has been provided. Please name one [double/float/int]."
  elif [[ "$1" == "double" || "$1" == "float" || "$1" == "int" ]]; then
    export FPGA_RETURNTYPE=${1}
    source ${SCRIPTPATH}/setup.sh
  else
    echo "[ERROR] Type '${1}' not valid [double/float/int]."
  fi
}
export -f set_type


#==============================
#
# Write config file
#
#==============================

cat << EOF > ${FPGA_PATH2INC}/config.hpp
#ifndef CONFIG_H
#define CONFIG_H
#define __DEBUG__
#define INPUT_VALUE ${INPUT_VALUE}
#define __BATCHSIZE__ ${__BATCHSIZE__}
#define __NHITS__ ${__NHITS__}
#define __NPARARETURN__ 10
#define __NBATCHES_MAX__ 1
typedef ${FPGA_RETURNTYPE} __DATATYPE__;
#endif // CONFIG_H
EOF


#==============================
#
# Finalize
#
#==============================

# Show some config details to the user
echo "[${BOLDTXT}INFO${NORMTXT}] Emulation mode: ${BOLDTXT}${XCL_EMULATION_MODE}${NORMTXT}; return type ${BOLDTXT}${FPGA_RETURNTYPE}${NORMTXT}"

# Tell other scripts that setup.sh was called 
export FPGA_ENV_INIT=true
